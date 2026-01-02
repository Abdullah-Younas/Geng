// Sphere.h
#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Sphere {
public:
    // Transform properties
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // Physics properties (you'll populate these when adding physics)
    glm::vec3 speed;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;

    Sphere(float radius = 1.0f, unsigned int rings = 30, unsigned int sectors = 30);
    ~Sphere();

    void Render(unsigned int shaderProgram);
    void UpdatePhysics(float deltaTime, float radius); // Placeholder for your physics
    glm::mat4 GetModelMatrix();

    void Cleanup();

private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indexCount;

    void GenerateSphere(float radius, unsigned int rings, unsigned int sectors);
};

#endif