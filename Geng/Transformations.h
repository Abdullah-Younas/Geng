#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transformations
{
public:
    glm::mat4 RotMeshX(glm::mat4 trans, float RotValue);
    glm::mat4 RotMeshY(glm::mat4 trans, float RotValue);
    glm::mat4 RotMeshZ(glm::mat4 trans, float RotValue);
    glm::mat4 ScaleMeshComb(glm::mat4 trans, float scale);
    glm::mat4 ScaleMeshXYZ(glm::mat4 trans, float X, float Y, float Z);
};

