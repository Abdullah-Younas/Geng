#ifndef LEVEL_COLLISION_H
#define LEVEL_COLLISION_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "model_loader.h"
#include <cfloat>

// ================== Collision Result Structure ==================
struct CollisionResult {
    bool collided;
    glm::vec3 collisionPoint;      // Where the collision occurred
    glm::vec3 collisionNormal;     // Surface normal at collision point
    float penetrationDepth;        // How far sphere penetrated
    std::string meshName;          // Which mesh was hit

    CollisionResult()
        : collided(false),
        collisionPoint(0.0f),
        collisionNormal(0.0f, 1.0f, 0.0f),
        penetrationDepth(0.0f),
        meshName("") {
    }
};

// ================== Bounding Box Structure ==================
struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    // Default constructor using initializer list
    BoundingBox() {
        min = glm::vec3(FLT_MAX);
        max = glm::vec3(-FLT_MAX);
    }

    // Constructor with min/max points
    BoundingBox(const glm::vec3& minPoint, const glm::vec3& maxPoint) {
        min = minPoint;
        max = maxPoint;
    }

    bool Contains(const glm::vec3& point) const;
    bool IntersectsSphere(const glm::vec3& sphereCenter, float sphereRadius) const;
    glm::vec3 GetCenter() const;
    glm::vec3 GetSize() const;
};

// ================== Collision Mesh Structure ==================
struct CollisionMesh {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    BoundingBox aabb;
    std::string name;

    CollisionMesh() {}
    CollisionMesh(const std::string& meshName) : name(meshName) {}
};

// ================== Level Collision Class ==================
class LevelCollision {
public:
    LevelCollision();
    ~LevelCollision();

    // Build collision data from a Model and its transformation matrix
    void BuildFromModel(const Model& model, const glm::mat4& modelMatrix);

    // Check sphere collision with detailed collision information (NEW - USE THIS!)
    CollisionResult CheckSphereCollisionDetailed(const glm::vec3& sphereCenter, float sphereRadius);

    // Check sphere collision against all meshes (simple boolean)
    bool CheckSphereCollision(const glm::vec3& sphereCenter, float sphereRadius);

    // Check sphere collision against a specific mesh component
    bool CheckSphereCollisionWithMesh(const glm::vec3& sphereCenter, float sphereRadius, const std::string& meshName);

    // Get bounding box for entire level
    BoundingBox GetWorldAABB() const { return worldAABB; }

    // Get number of collision meshes
    size_t GetMeshCount() const { return collisionMeshes.size(); }

    // Get specific mesh by index
    const CollisionMesh& GetMesh(size_t index) const { return collisionMeshes[index]; }

    // For debugging - get all bounding boxes
    std::vector<BoundingBox> GetBoundingBoxes() const {
        std::vector<BoundingBox> boxes;
        boxes.push_back(worldAABB); // Add world AABB
        for (const auto& mesh : collisionMeshes) {
            boxes.push_back(mesh.aabb); // Add each mesh AABB
        }
        return boxes;
    }

    // Clear all collision data
    void Clear();

private:
    std::vector<CollisionMesh> collisionMeshes;
    BoundingBox worldAABB;
    glm::mat4 modelMatrix;

    // Helper function to calculate AABB from vertices
    BoundingBox CalculateAABB(const std::vector<glm::vec3>& vertices);

    // Helper function to transform a mesh to world space
    CollisionMesh TransformMesh(const Mesh& mesh, const glm::mat4& transform, const std::string& name);

    // Check if sphere intersects with a triangle (simple)
    bool SphereTriangleIntersection(const glm::vec3& sphereCenter, float sphereRadius,
        const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

    // Check sphere-triangle intersection with detailed collision info (NEW!)
    bool SphereTriangleIntersectionDetailed(const glm::vec3& sphereCenter, float sphereRadius,
        const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
        CollisionResult& result);

    // Get closest point on triangle to a point
    glm::vec3 ClosestPointOnTriangle(const glm::vec3& point,
        const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
};

#endif // LEVEL_COLLISION_H