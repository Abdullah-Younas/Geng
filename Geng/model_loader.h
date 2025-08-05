// model_loader.h
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <unordered_map>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Material {
    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    std::string diffuseTexture;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
    GLuint VAO;
};

class Model {
public:
    std::vector<Mesh> meshes;
    std::unordered_map<std::string, GLuint> loadedTextures;
    std::vector<glm::vec3> GetVertexPositions() const {
        std::vector<glm::vec3> positions;
        for (const auto& mesh : meshes) {
            for (const auto& vertex : mesh.vertices) {
                positions.push_back(vertex.position);
            }
        }
        return positions;
    }

    bool Load(const std::string& path);
    void Render(GLuint shaderProgram);
    void Cleanup();

private:
    bool LoadOBJ(const std::string& path);
    bool LoadMTL(const std::string& path, std::vector<Material>& materials);
    GLuint SetupMeshVAO(const Mesh& mesh);
    GLuint LoadTexture(const std::string& path);
    bool ProcessFace(std::istringstream& iss,
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& texCoords,
        Mesh& mesh);
};