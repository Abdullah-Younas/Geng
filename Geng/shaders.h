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
const char* fragmentShaderSource1 = R"(
#version 330 core
struct Material {
    sampler2D diffuse;
    
    vec3 specular;
    float shininess;
}; 

in vec2 TexCoords;

uniform Material material;

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos; 
uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos; 

void main()
{
    vec3 ambient  = light.ambient * vec3(texture(material.diffuse, TexCoords));
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular); 

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
