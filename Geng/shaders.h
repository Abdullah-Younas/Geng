#ifndef SHADERS_H
#define SHADERS_H

// Shadow mapping depth shader - Vertex
const char* shadowDepthVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    gl_Position.z += 0.001; // Small bias, adjust as needed
}
)";

// Shadow mapping depth shader - Fragment
const char* shadowDepthFragmentShader = R"(
#version 330 core

void main()
{
    // gl_FragDepth is written automatically
}
)";

// Vertex shader with shadow support
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// Fragment shader with shadow support
const char* fragmentShaderSource1 = R"(
#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

in vec2 TexCoords;
in vec4 FragPosLightSpace;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform vec3 viewPos;
uniform float dirIntensity;
uniform sampler2D shadowMap;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

float rgbToGray(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 linearToSRGB(vec3 linear) {
    return pow(linear, vec3(1.0/2.2));
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLight.direction);

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, shadow);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(linearToSRGB(result), 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 texColor = texture(material.diffuse, TexCoords).rgb;
    float specIntensity = rgbToGray(texture(material.specular, TexCoords).rgb);
    
    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * vec3(specIntensity);

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    
    return lighting * dirIntensity;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                    light.quadratic * (distance * distance));
    
    vec3 texColor = texture(material.diffuse, TexCoords).rgb;
    float specIntensity = rgbToGray(texture(material.specular, TexCoords).rgb);
    
    vec3 ambient = light.ambient * texColor * 0.05;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * vec3(specIntensity);
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                    light.quadratic * (distance * distance));
    
    vec3 texColor = texture(material.diffuse, TexCoords).rgb;
    
    vec3 ambient = light.ambient * texColor * 0.02;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = light.specular * spec * vec3(1.0);
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}
)";

const char* lampFragmentShaderSource = R"(
#version 330 core

uniform vec3 Color;
out vec4 FragColor;

vec3 linearToSRGB(vec3 linear) {
    return pow(linear, vec3(1.0/2.2));
}

void main()
{
    FragColor = vec4(linearToSRGB(Color), 1.0);
}
)";

const char* CubeMapVShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  
)";

const char* CubeMapFShader = R"(
#version 330 core

out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube skybox;

vec3 linearToSRGB(vec3 linear) {
    return pow(linear, vec3(1.0/2.2));
}

void main()
{    
    vec3 color = texture(skybox, TexCoords).rgb;
    FragColor = vec4(linearToSRGB(color), 1.0);
} 
)";

const char* debugVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* debugFragmentShader = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 debugColor;

void main()
{
    FragColor = vec4(debugColor, 0.3);
}
)";

#endif