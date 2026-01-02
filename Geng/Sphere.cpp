// Sphere.cpp implementation
#include "Sphere.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Sphere::Sphere(float radius, unsigned int rings, unsigned int sectors)
    : position(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    scale(1.0f, 1.0f, 1.0f),
    velocity(0.0f, 0.0f, 0.0f),
    acceleration(0.0f, 0.0f, 0.0f),
    mass(1.0f)
{
    GenerateSphere(radius, rings, sectors);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Sphere::~Sphere() {
    Cleanup();
}

void Sphere::GenerateSphere(float radius, unsigned int rings, unsigned int sectors) {
    const float PI = 3.14159265359f;

    for (unsigned int i = 0; i <= rings; ++i) {
        float phi = PI * float(i) / float(rings);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * PI * float(j) / float(sectors);

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (normalized position for sphere)
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);

            // TexCoords
            vertices.push_back(float(j) / float(sectors));
            vertices.push_back(float(i) / float(rings));
        }
    }

    // Generate indices
    for (unsigned int i = 0; i < rings; ++i) {
        for (unsigned int j = 0; j < sectors; ++j) {
            unsigned int first = i * (sectors + 1) + j;
            unsigned int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    indexCount = indices.size();
}

glm::mat4 Sphere::GetModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
}

float floorHeightAtX(float x)
{
    return 0.1f * x; // slope
}

void Sphere::UpdatePhysics(float deltaTime, float radius)
{
    acceleration.y = -9.8f;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    float floorY = floorHeightAtX(position.x) + radius;

    if (position.y <= floorY)
    {
        position.y = floorY;

        if (velocity.y < 0.0f)
        {
            velocity.y = -velocity.y;   // vertical bounce
            velocity.x *= -1.0f;        // reverse direction on slope
        }
    }
}

void Sphere::Render(unsigned int shaderProgram) {
    glm::mat4 model = GetModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::Cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}