#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

unsigned int compileShader(unsigned int type, const char* source);
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);

#endif
