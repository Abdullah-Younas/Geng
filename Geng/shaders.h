#ifndef SHADERS_H
#define SHADERS_H

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource1 = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
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
