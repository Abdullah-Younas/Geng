#ifndef SHADERS_H
#define SHADERS_H

// Shared vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // Important
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = aTexCoords;
}
)";

// Lighting fragment shader (multiplies light and object color)
    //vec3 position for diff lights
const char* fragmentShaderSource1 = R"(
#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

in vec2 TexCoords;

struct Light {
    vec3 position;  // Position of the spotlight
    vec3 direction; // Direction the spotlight is pointing
    float cutOff;   // Inner cutoff angle (cosine)
    float outerCutOff; // Outer cutoff angle (cosine)
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;  
uniform vec3 viewPos;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos; 

void main()
{
    // Calculate vector from fragment to light
    vec3 lightDir = normalize(light.position - FragPos);
    
    // Calculate the angle between light direction and fragment-to-light vector
    float theta = dot(lightDir, normalize(-light.direction));
    
    // Attenuation (distance-based dimming)
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                             light.quadratic * (distance * distance));
    
    // Ambient lighting (always present)
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    
    // Initialize diffuse and specular
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    // Only calculate diffuse and specular if within spotlight cone
    if(theta > light.outerCutOff) 
    {
        // Calculate spotlight intensity (soft edges)
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        
        // Diffuse lighting
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        diffuse *= intensity;
        
        // Specular lighting
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        specular *= intensity;
    }
    
    // Combine all lighting components with attenuation
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

// Lamp fragment shader (just white)
//uniform vec3 lightColor;
//FragColor = vec4(lightColor, 1.0); // white
const char* lampFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;


void main()
{
    FragColor = vec4(1.0); // white
}
)";

#endif
