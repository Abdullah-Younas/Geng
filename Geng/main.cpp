// ================== Includes ==================
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <Windows.h>
using namespace std;

bool FileExists(const std::string& path) {
    DWORD attribs = GetFileAttributesA(path.c_str());
    return (attribs != INVALID_FILE_ATTRIBUTES &&
        !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}

#include "shader_utils.h"
#include "shaders.h"
#include "Camera.h"
#include "Transformations.h"
#include "CallBacks.h"

// ================== Globals ==================
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool InteractCursor = false;

float ScreenColor[4] = { 0.75f, 0.80f, 0.85f, 1.0f };

float DirLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float DirLightDiff[3] = { 0.4f, 0.4f, 0.4f };

float PointLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float PointLightDiff[3] = { 0.4f, 0.4f, 0.4f };

float SpotLightDiff[3] = { 0.4f, 0.4f, 0.4f };
float SpotLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float SpotlightInnerCutoff = 8.0f;
float SpotlightOuterCutoff = 12.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
Transformations transformer;

// ================= MODEL LOADING ==================
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};
struct Material
{
    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    std::string diffuseTexture;  // Path to texture
};
struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material material;
};

class OBJLoader {
public:
    static bool Load(const std::string& objPath, std::vector<Mesh>& meshes) {
        // Open OBJ file
        std::ifstream file(objPath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Failed to open OBJ file: " << objPath << std::endl;
            return false;
        }

        size_t vertexCount = 0;
        size_t faceCount = 0;


        // Extract directory for MTL/texture paths
        std::string baseDir = objPath.substr(0, objPath.find_last_of("/\\") + 1);

        // Data containers
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<Material> materials;
        std::string currentMtl;

        // Parse file line by line
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {  // Vertex position
                glm::vec3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            }
            else if (prefix == "vn") {  // Vertex normal
                glm::vec3 norm;
                iss >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
            }
            else if (prefix == "vt") {  // Texture coordinate
                glm::vec2 uv;
                iss >> uv.x >> uv.y;
                texCoords.push_back(uv);
            }
            else if (prefix == "usemtl") {  // Material assignment
                iss >> currentMtl;
            }
            else if (prefix == "f") {  // Face definition
                if (currentMtl.empty()) {
                    std::cerr << "WARNING: Face before material assignment" << std::endl;
                    currentMtl = "default_material";
                }
                Mesh& mesh = GetOrCreateMesh(meshes, currentMtl, materials);
                if (!ProcessFace(iss, positions, normals, texCoords, mesh)) {
                    std::cerr << "ERROR: Failed to process face: " << line << std::endl;
                }
            }
            else if (prefix == "mtllib") {  // Material library
                std::string mtlFile;
                iss >> mtlFile;
                if (!LoadMTL(baseDir + mtlFile, materials)) {
                    std::cerr << "WARNING: Failed to load MTL: " << mtlFile << std::endl;
                }
            }
            else if (prefix == "vn") {
                glm::vec3 norm;
                iss >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
                std::cout << "Loaded normal " << normals.size() << ": "
                    << norm.x << ", " << norm.y << ", " << norm.z << std::endl;
            }
            else if (prefix == "vt") {
                glm::vec2 uv;
                iss >> uv.x >> uv.y;
                texCoords.push_back(uv);
                std::cout << "Loaded UV " << texCoords.size() << ": "
                    << uv.x << ", " << uv.y << std::endl;
            }
        }

        std::cout << "\nOBJ File Statistics:" << std::endl;
        std::cout << "- Positions: " << positions.size() << std::endl;
        std::cout << "- Normals: " << normals.size() << std::endl;
        std::cout << "- TexCoords: " << texCoords.size() << std::endl;
        std::cout << "- Faces processed: " << faceCount << std::endl;

        // Validation
        if (meshes.empty()) {
            std::cerr << "ERROR: No meshes created from OBJ file" << std::endl;
            return false;
        }

        return true;
    }

private:
    static Mesh& GetOrCreateMesh(std::vector<Mesh>& meshes,
        const std::string& mtlName,
        const std::vector<Material>& materials) {
        // Find existing mesh
        for (Mesh& mesh : meshes) {
            if (mesh.material.name == mtlName) return mesh;
        }

        // Create new mesh with material
        meshes.emplace_back();
        Mesh& newMesh = meshes.back();
        newMesh.material = FindMaterial(materials, mtlName);
        return newMesh;
    }

    static Material FindMaterial(const std::vector<Material>& materials,
        const std::string& name) {
        // Default material
        Material defaultMat;
        defaultMat.name = name;
        defaultMat.diffuse = glm::vec3(1.0f, 0.0f, 1.0f);  // Magenta = missing material

        // Search for matching material
        for (const Material& mat : materials) {
            if (mat.name == name) return mat;
        }

        std::cerr << "WARNING: Using default material for " << name << std::endl;
        return defaultMat;
    }
    
    static bool ProcessFace(std::istringstream& iss,
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& texCoords,
        Mesh& mesh) {
        std::vector<Vertex> faceVertices;
        std::string vertexData;
        size_t vertexCount = 0;

        std::cout << "Processing face: ";

        while (iss >> vertexData) {
            vertexCount++;
            std::cout << vertexData << " ";

            // Replace all '/' with spaces for easier parsing
            std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
            std::istringstream viss(vertexData);

            Vertex vertex;
            int posIdx = 0, texIdx = 0, normIdx = 0;

            // Parse indices (OBJ uses 1-based indexing)
            if (!(viss >> posIdx)) {
                std::cerr << "\nERROR: Failed to parse position index in '" << vertexData << "'";
                return false;
            }

            // Check if texture coordinate index exists
            if (viss.peek() != EOF) viss >> texIdx;

            // Check if normal index exists
            if (viss.peek() != EOF) viss >> normIdx;

            // Validate position index
            if (posIdx < 1 || posIdx > positions.size()) {
                std::cerr << "\nERROR: Invalid position index " << posIdx
                    << " (valid range: 1-" << positions.size() << ")";
                return false;
            }
            vertex.position = positions[posIdx - 1];

            // Validate and assign texture coordinate (if specified)
            if (texIdx > 0) {
                if (texIdx > texCoords.size()) {
                    std::cerr << "\nWARNING: Texture coordinate index " << texIdx
                        << " out of range (1-" << texCoords.size() << ")";
                    vertex.texCoord = glm::vec2(0.0f); // Default UV
                }
                else {
                    vertex.texCoord = texCoords[texIdx - 1];
                }
            }
            else {
                vertex.texCoord = glm::vec2(0.0f); // Default UV
            }

            // Validate and assign normal (if specified)
            if (normIdx > 0) {
                if (normIdx > normals.size()) {
                    std::cerr << "\nWARNING: Normal index " << normIdx
                        << " out of range (1-" << normals.size() << ")";
                    vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default normal
                }
                else {
                    vertex.normal = normals[normIdx - 1];
                }
            }
            else {
                vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default normal
            }

            faceVertices.push_back(vertex);
        }

        // Validate we got at least 3 vertices for a triangle
        if (faceVertices.size() < 3) {
            std::cerr << "\nERROR: Face needs at least 3 vertices, got "
                << faceVertices.size();
            return false;
        }

        // Triangulate polygon (works for triangles, quads, etc.)
        for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
            mesh.vertices.push_back(faceVertices[0]);
            mesh.vertices.push_back(faceVertices[i]);
            mesh.vertices.push_back(faceVertices[i + 1]);

            // Add indices
            for (int j = 0; j < 3; ++j) {
                mesh.indices.push_back(mesh.indices.size());
            }
        }

        std::cout << "\nSuccessfully processed face with "
            << faceVertices.size() << " vertices (created "
            << (faceVertices.size() - 2) << " triangles)\n";

        return true;
    }

    static bool LoadMTL(const std::string& mtlPath, std::vector<Material>& materials) {
        std::ifstream file(mtlPath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Failed to open MTL file: " << mtlPath << std::endl;
            return false;
        }

        Material* currentMtl = nullptr;
        std::string line;
        std::string baseDir = mtlPath.substr(0, mtlPath.find_last_of("/\\") + 1);

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "newmtl") {
                materials.emplace_back();
                currentMtl = &materials.back();
                iss >> currentMtl->name;
                // Initialize defaults
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
                }
            }
        }

        return !materials.empty();
    }
};

GLuint SetupMeshVAO(const Mesh& mesh) {
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO (Vertex data)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

    // EBO (Indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0);  // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);  // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);  // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
    return VAO;
}

// ================== Input Handling ==================
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

// ================== Texture Loader ==================
unsigned int loadTexture(const char* path);

// ================== Main ==================
int main() {
    // GLFW Init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Lighting", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, CallBacks::mouse_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(window, CallBacks::scroll_callback);

    // ================== Vertex Data ==================
    float vertices[] = {
        // positions          // normals           // tex coords
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };

    std::string objPath = "nissan_silvia_s13_low-poly.obj";
    std::ifstream file(objPath, std::ios::binary); // Use binary mode to prevent CRLF issues
    if (!file) {
        std::cerr << "ERROR: Failed to open " << objPath << "\n";
        return false;
    }

    // Verify file content
    std::string line;
    while (std::getline(file, line)) {
        std::cout << "Raw line: " << line << "\n";
        // Check for non-ASCII characters
        for (char c : line) {
            if (static_cast<unsigned char>(c) > 127) {
                std::cerr << "WARNING: Non-ASCII character in file\n";
            }
        }
    }
    file.clear();
    file.seekg(0); // Rewind for actual parsing

    /*std::string objPath = "model.obj";
    if (!FileExists(objPath)) {
        std::cerr << "Error: File not found at:\n" << objPath << "\n";
        return -1;
    }
    else {
        cout << "File Found and Loaded!" << endl;
    }*/


    std::vector<GLuint> meshVAOs;
    std::vector<Mesh> meshes;

    // In your loading code after OBJLoader::Load()
    if (OBJLoader::Load(objPath, meshes)) {
        // Clear previous VAOs if any
        for (auto vao : meshVAOs) {
            glDeleteVertexArrays(1, &vao);
        }
        meshVAOs.clear();

        // Create VAO for each mesh
        for (const auto& mesh : meshes) {
            GLuint vao = SetupMeshVAO(mesh);
            meshVAOs.push_back(vao);
            std::cout << "Created VAO " << vao << " for mesh with "
                << mesh.indices.size() << " indices\n";
        }
    }
    /*std::vector<Mesh> meshes;
    if (OBJLoader::Load("model.obj", meshes)) {
        std::cout << "OBJ file loaded successfully!" << std::endl;
        std::cout << "Number of meshes: " << meshes.size() << std::endl;

        for (size_t i = 0; i < meshes.size(); ++i) {
            const Mesh& mesh = meshes[i];
            std::cout << "\nMesh " << i << ":" << std::endl;
            std::cout << "  - Material: " << mesh.material.name << std::endl;
            std::cout << "  - Diffuse color: ("
                << mesh.material.diffuse.r << ", "
                << mesh.material.diffuse.g << ", "
                << mesh.material.diffuse.b << ")" << std::endl;
            std::cout << "  - Texture: " << mesh.material.diffuseTexture << std::endl;
            std::cout << "  - Vertex count: " << mesh.vertices.size() << std::endl;
            std::cout << "  - Triangle count: " << mesh.indices.size() / 3 << std::endl;

            // Print first vertex data as a sample
            if (!mesh.vertices.empty()) {
                const Vertex& v = mesh.vertices[0];
                std::cout << "  - First vertex: " << std::endl;
                std::cout << "    Position: (" << v.position.x << ", " << v.position.y << ", " << v.position.z << ")" << std::endl;
                std::cout << "    Normal: (" << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << ")" << std::endl;
                std::cout << "    UV: (" << v.texCoord.x << ", " << v.texCoord.y << ")" << std::endl;
            }

            GLuint VAO = SetupMeshVAO(mesh);
            std::cout << "  - VAO created: " << VAO << std::endl;
        }
    }
    else {
        std::cerr << "Failed to load OBJ file!" << std::endl;
    }*/

    // ================== VAO/VBO Setup ==================
    unsigned int VBO, cubeVAO, lightVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    // Cube VAO
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light VAO
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ================== Textures ==================
    unsigned int specularMap = loadTexture("container2.jpg");
    unsigned int diffuseMap = loadTexture("material_baseColor.jpg");

    // ================== Shaders ==================
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int lampShader = createShaderProgram(vertexShaderSource, lampFragmentShaderSource);

    glUseProgram(lightingShader);
    glUniform1i(glGetUniformLocation(lightingShader, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader, "material.specular"), 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ================== Main Render Loop ==================
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear Buffers
        glClearColor(ScreenColor[0], ScreenColor[1], ScreenColor[2], ScreenColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ========== Lighting Pass ==========
        glUseProgram(lightingShader);
        glm::mat4 model = glm::mat4(1.0f);
        model = transformer.RotMeshY(model, glfwGetTime() * 30.0f);
        model = transformer.RotMeshX(model, glfwGetTime() * 20.0f);
        model = transformer.ScaleMeshComb(model, 0.75f);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lighting + Material Setup
        /*/for (int i = 0; i < 1; i++) {
            glm::vec3 lightPos = cubePositions[i] * 2.0f; // or your light positions
            string uniformName = "lightPos" + to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(lightingShader, uniformName.c_str()), 1, glm::value_ptr(lightPos));
        }*/
        //glUniform3f(glGetUniformLocation(lightingShader, "light.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(camera.Position));

        // In your render loop, replace the lighting setup with:

        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.diffuse"), DirLightDiff[0], DirLightDiff[1], DirLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.specular"), DirLightSpec[0], DirLightSpec[1], DirLightSpec[2]);

        // Point lights
        for (int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(lightingShader, (base + ".position").c_str()), 1, &pointLightPositions[i][0]);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".ambient").c_str()), 0.05f, 0.05f, 0.05f);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".diffuse").c_str()), PointLightDiff[0], PointLightDiff[1], PointLightDiff[2]);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".specular").c_str()), PointLightSpec[0], PointLightSpec[1], PointLightSpec[2]);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".linear").c_str()), 0.09f);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".quadratic").c_str()), 0.032f);
        }

        // Spotlight (flashlight)
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.position"), 1, &camera.Position[0]);
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.direction"), 1, &camera.Front[0]);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.diffuse"), SpotLightDiff[0], SpotLightDiff[1], SpotLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.specular"), SpotLightSpec[0], SpotLightSpec[1], SpotLightSpec[2]);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.cutOff"), glm::cos(glm::radians(8.5f)));
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.outerCutOff"), glm::cos(glm::radians(10.0f)));

        // Material properties
        glUniform1f(glGetUniformLocation(lightingShader, "material.shininess"), 32.0f);

        glUniform3f(glGetUniformLocation(lightingShader, "material.specular"), 1.0f, 1.0f, 1.0f);




        // Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // Render Cube
        glBindVertexArray(cubeVAO);

        /*for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f); // Start with identity
            model = glm::translate(model, cubePositions[i]);

            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            // === Send model matrix to shader manually ===
            int modelLoc = glGetUniformLocation(lightingShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // === Now draw the object ===
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/
        for (size_t i = 0; i < meshes.size(); ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Position at origin
            model = glm::scale(model, glm::vec3(1.0f)); // Scale if needed

            glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

            // Bind the mesh's VAO
            glBindVertexArray(meshVAOs[i]);

            // Draw the mesh
            glDrawElements(GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
        }
        // ========== Lamp Pass ==========
        
        glUseProgram(lampShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.25f));

        glUniformMatrix4fv(glGetUniformLocation(lampShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(lampShader, "Color"), PointLightDiff[0], PointLightDiff[1], PointLightDiff[2]);

        for (unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // Multiply original position by a factor to spread them out more
            glm::vec3 adjustedPos = pointLightPositions[i]; // e.g., double spacing

            // Optional: add some offset to Y or Z to shift them in interesting ways
            adjustedPos.y += sin(i); // for wave effect

            model = glm::translate(model, adjustedPos);

            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            model = transformer.ScaleMeshComb(model, 0.3f);

            int modelLoc = glGetUniformLocation(lampShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        ImGui::Begin("Hehe, me is window");
        ImGui::ColorEdit4("Sky Color", ScreenColor);
        ImGui::Text("Directional Light");
        ImGui::ColorEdit3("Directional Light Specular", DirLightSpec);
        ImGui::ColorEdit3("Directional Light Diffuse", DirLightDiff);
        ImGui::Text("Point Light");
        ImGui::ColorEdit3("Point Light Specular", PointLightSpec);
        ImGui::ColorEdit3("Point Light Diffuse", PointLightDiff);
        ImGui::Text("Spot Light");
        ImGui::ColorEdit3("Spot Light Specular", SpotLightSpec);
        ImGui::ColorEdit3("Spot Light Diffuse", SpotLightDiff);
        ImGui::SliderFloat("Inner Cut Off", &SpotlightInnerCutoff, 3.0f, 20.0f);
        ImGui::SliderFloat("Inner Outer Off", &SpotlightOuterCutoff, 5.0f, 25.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // ========== End Frame ==========
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // ================== Cleanup ==================
    for (auto vao : meshVAOs) {
        glDeleteVertexArrays(1, &vao);
    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader);
    glDeleteProgram(lampShader);

    glfwTerminate();
    return 0;
}
