#ifndef SHADERS_H
#define SHADERS_H

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";

const char* fragmentShaderSource1 = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    FragColor = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.5);
}
)";

/*const char* fragmentShaderSource2 = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
}
)";*/

#endif
