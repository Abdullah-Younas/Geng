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

    out vec4 FragColor;

    in vec3 Normal;
    in vec3 FragPos;

    // Function prototypes
    vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
    vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
    vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

    float rgbToGray(vec3 color) {
        return dot(color, vec3(0.299, 0.587, 0.114));
    }

    float near = 0.1; 
    float far  = 100.0; 
    uniform float FogIntensity;
    uniform vec3 fogColor;

    float LinearizeDepth(float depth) 
    {
        float z = depth * 2.0 - 1.0; // back to NDC 
        return (2.0 * near*far) / (z * (far - near) - (far + near));	
    }

    void main()
    {
        // Properties
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
    
        // Directional lighting
        vec3 result = CalcDirLight(dirLight, norm, viewDir);
    
        // Point lights
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    
        // Spotlight
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
        
        float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
        vec4 depthVec4 = vec4(fogColor * pow(depth, FogIntensity), 1.0);
        FragColor = vec4(result, 1.0) * (1 - depthVec4) + depthVec4;
    }

    // Calculates directional light
    vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
    {
        vec3 lightDir = normalize(-light.direction);
        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // Combine results
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
        return (ambient + diffuse + specular);
    }

    // Calculates point light
    vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
    {
        vec3 lightDir = normalize(light.position - fragPos);
        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // Attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                        light.quadratic * (distance * distance));
        // Combine results
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
        float specIntensity = rgbToGray(texture(material.specular, TexCoords).rgb);
        vec3 specular = light.specular * spec * vec3(specIntensity);
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        return (ambient + diffuse + specular);
    }

    // Calculates spotlight
    vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
    {
        vec3 lightDir = normalize(light.position - fragPos);
        // Check if inside spotlight cone
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // Attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                        light.quadratic * (distance * distance));
        // Combine results
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
        vec3 specular = light.specular * spec * vec3(1.0);
        ambient *= attenuation * intensity;
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;
        return (ambient + diffuse + specular);
    }
    )";

    // Lamp fragment shader (just white)
    //uniform vec3 lightColor;
    //FragColor = vec4(lightColor, 1.0); // white
    const char* lampFragmentShaderSource = R"(
    #version 330 core

    uniform vec3 Color;

    out vec4 FragColor;


    void main()
    {
        FragColor = vec4(Color, 1.0); // white
    }
    )";

    #endif
