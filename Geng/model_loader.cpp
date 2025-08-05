// model_loader.cpp
#include "model_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool Model::Load(const std::string& path) {
    return LoadOBJ(path);
}

void Model::Render(GLuint shaderProgram) {
    for (auto& mesh : meshes) {
        // Bind textures
        GLuint diffuseTex = 0;
        if (!mesh.material.diffuseTexture.empty()) {
            auto it = loadedTextures.find(mesh.material.diffuseTexture);
            if (it != loadedTextures.end()) {
                diffuseTex = it->second;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseTex);
                glUniform1i(glGetUniformLocation(shaderProgram, "material.diffuse"), 0);

                // Use same texture for specular if no separate specular map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, diffuseTex);
                glUniform1i(glGetUniformLocation(shaderProgram, "material.specular"), 1);
            }
        }

        // Set material properties
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1,
            glm::value_ptr(mesh.material.ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1,
            glm::value_ptr(mesh.material.diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1,
            glm::value_ptr(mesh.material.specular));
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 10);
//            mesh.material.shininess);

        // Draw the mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

void Model::Cleanup() {
    for (auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
    }
    for (auto& tex : loadedTextures) {
        glDeleteTextures(1, &tex.second);
    }
    meshes.clear();
    loadedTextures.clear();
}

bool Model::LoadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::string baseDir = path.substr(0, path.find_last_of("/\\") + 1);
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<Material> materials;
    std::string currentMtl;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        }
        else if (prefix == "usemtl") {
            iss >> currentMtl;
        }
        else if (prefix == "f") {
            if (currentMtl.empty()) {
                currentMtl = "default_material";
            }

            // Find or create mesh for this material
            Mesh* mesh = nullptr;
            for (auto& m : meshes) {
                if (m.material.name == currentMtl) {
                    mesh = &m;
                    break;
                }
            }

            if (!mesh) {
                meshes.emplace_back();
                mesh = &meshes.back();
                mesh->material = Material();
                for (const auto& mat : materials) {
                    if (mat.name == currentMtl) {
                        mesh->material = mat;
                        break;
                    }
                }
                if (mesh->material.name.empty()) {
                    mesh->material.name = currentMtl;
                    mesh->material.diffuse = glm::vec3(1.0f, 0.0f, 1.0f);
                }
            }

            if (!ProcessFace(iss, positions, normals, texCoords, *mesh)) {
                std::cerr << "ERROR: Failed to process face: " << line << std::endl;
            }
        }
        else if (prefix == "mtllib") {
            std::string mtlFile;
            iss >> mtlFile;
            LoadMTL(baseDir + mtlFile, materials);
        }
    }

    // Create VAOs for all meshes
    for (auto& mesh : meshes) {
        mesh.VAO = SetupMeshVAO(mesh);
    }

    return !meshes.empty();
}

bool Model::LoadMTL(const std::string& path, std::vector<Material>& materials) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open MTL file: " << path << std::endl;
        return false;
    }

    std::string baseDir = path.substr(0, path.find_last_of("/\\") + 1);
    Material* currentMtl = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "newmtl") {
            materials.emplace_back();
            currentMtl = &materials.back();
            iss >> currentMtl->name;
            currentMtl->ambient = glm::vec3(0.1f);
            currentMtl->diffuse = glm::vec3(0.8f);
            currentMtl->specular = glm::vec3(0.5f);
            currentMtl->shininess = 32.0f;
        }
        else if (currentMtl) {
            if (prefix == "Ka") iss >> currentMtl->ambient.r >> currentMtl->ambient.g >> currentMtl->ambient.b;
            else if (prefix == "Kd") iss >> currentMtl->diffuse.r >> currentMtl->diffuse.g >> currentMtl->diffuse.b;
            else if (prefix == "Ks") iss >> currentMtl->specular.r >> currentMtl->specular.g >> currentMtl->specular.b;
            else if (prefix == "Ns") iss >> currentMtl->shininess;
            else if (prefix == "map_Kd") {
                std::string texFile;
                iss >> texFile;
                currentMtl->diffuseTexture = baseDir + texFile;
                LoadTexture(currentMtl->diffuseTexture); // Preload texture
            }
        }
    }

    return !materials.empty();
}

GLuint Model::SetupMeshVAO(const Mesh& mesh) {
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
    return VAO;
}

GLuint Model::LoadTexture(const std::string& path) {
    if (path.empty()) return 0;
    if (loadedTextures.find(path) != loadedTextures.end()) {
        return loadedTextures[path];
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        loadedTextures[path] = textureID;
    }
    else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

bool Model::ProcessFace(std::istringstream& iss,
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& texCoords,
    Mesh& mesh) {
    std::vector<Vertex> faceVertices;
    std::string vertexData;

    while (iss >> vertexData) {
        std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
        std::istringstream viss(vertexData);

        Vertex vertex;
        int posIdx = 0, texIdx = 0, normIdx = 0;

        if (!(viss >> posIdx)) return false;
        if (viss.peek() != EOF) viss >> texIdx;
        if (viss.peek() != EOF) viss >> normIdx;

        if (posIdx < 1 || posIdx > positions.size()) return false;
        vertex.position = positions[posIdx - 1];

        if (texIdx > 0 && texIdx <= texCoords.size()) {
            vertex.texCoord = texCoords[texIdx - 1];
        }
        else {
            vertex.texCoord = glm::vec2(0.0f);
        }

        if (normIdx > 0 && normIdx <= normals.size()) {
            vertex.normal = normals[normIdx - 1];
        }
        else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        faceVertices.push_back(vertex);
    }

    if (faceVertices.size() < 3) return false;

    for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
        mesh.vertices.push_back(faceVertices[0]);
        mesh.vertices.push_back(faceVertices[i]);
        mesh.vertices.push_back(faceVertices[i + 1]);

        for (int j = 0; j < 3; ++j) {
            mesh.indices.push_back(mesh.indices.size());
        }
    }

    return true;
}