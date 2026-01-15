#include "Formulas.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Notice: Formulas:: prefix to make it a member function
glm::vec3 Formulas::ProjectOnPlane(const glm::vec3& v, const glm::vec3& normal) {
    glm::vec3 n = glm::normalize(normal);
    return v - glm::dot(v, n) * n;
}