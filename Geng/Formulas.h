#ifndef FORMULAS_H
#define FORMULAS_H

#include <glm/glm.hpp>

class Formulas {
public:
    glm::vec3 ProjectOnPlane(const glm::vec3& v, const glm::vec3& normal);
};

#endif