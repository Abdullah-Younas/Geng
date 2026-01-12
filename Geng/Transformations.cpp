#include "Transformations.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transformations::RotMeshX(glm::mat4 trans, float RotValue) {
    return glm::rotate(trans, glm::radians(-RotValue), glm::vec3(1.0, 0.0, 0.0));
}   
glm::mat4 Transformations::RotMeshY(glm::mat4 trans, float RotValue) {
    return glm::rotate(trans, glm::radians(-RotValue), glm::vec3(0.0, 1.0, 0.0));
}
glm::mat4 Transformations::RotMeshZ(glm::mat4 trans, float RotValue) {
    return glm::rotate(trans, glm::radians(-RotValue), glm::vec3(0.0, 0.0, 1.0));
}
glm::mat4 Transformations::ScaleMeshComb(glm::mat4 trans, float scale) {
    return glm::scale(trans, glm::vec3(scale, scale, scale));
}
glm::mat4 Transformations::ScaleMeshXYZ(glm::mat4 trans, float X, float Y, float Z) {
    return glm::scale(trans, glm::vec3(X, Y, Z));
}

glm::vec3 ProjectOnPlane(const glm::vec3& vector, const glm::vec3& planeNormal) {
	float distance = glm::dot(vector, planeNormal);
	glm::vec3 projectionOnToNormal = planeNormal * distance;

	return vector - projectionOnToNormal;
}