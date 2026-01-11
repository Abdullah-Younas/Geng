#include "LevelCollision.h"
#include <iostream>
#include <algorithm>

// ================== BoundingBox Methods ==================

bool BoundingBox::Contains(const glm::vec3& point) const {
    return point.x >= min.x && point.x <= max.x &&
        point.y >= min.y && point.y <= max.y &&
        point.z >= min.z && point.z <= max.z;
}

bool BoundingBox::IntersectsSphere(const glm::vec3& sphereCenter, float sphereRadius) const {
    // Find the closest point on the AABB to the sphere center
    glm::vec3 closestPoint;
    closestPoint.x = glm::clamp(sphereCenter.x, min.x, max.x);
    closestPoint.y = glm::clamp(sphereCenter.y, min.y, max.y);
    closestPoint.z = glm::clamp(sphereCenter.z, min.z, max.z);

    // Calculate distance from sphere center to closest point
    float distanceSquared = glm::dot(sphereCenter - closestPoint, sphereCenter - closestPoint);

    return distanceSquared < (sphereRadius * sphereRadius);
}

glm::vec3 BoundingBox::GetCenter() const {
    return (min + max) * 0.5f;
}

glm::vec3 BoundingBox::GetSize() const {
    return max - min;
}

// ================== LevelCollision Methods ==================

LevelCollision::LevelCollision() {
    modelMatrix = glm::mat4(1.0f);
}

LevelCollision::~LevelCollision() {
    Clear();
}

void LevelCollision::BuildFromModel(const Model& model, const glm::mat4& modelMat) {
    Clear();
    modelMatrix = modelMat;

    // Process each mesh in the model
    for (size_t i = 0; i < model.meshes.size(); i++) {
        std::string meshName = "Mesh_" + std::to_string(i);
        CollisionMesh colMesh = TransformMesh(model.meshes[i], modelMatrix, meshName);
        collisionMeshes.push_back(colMesh);
    }

    // Calculate world AABB that encompasses all meshes
    if (!collisionMeshes.empty()) {
        worldAABB = collisionMeshes[0].aabb;
        for (size_t i = 1; i < collisionMeshes.size(); i++) {
            worldAABB.min = glm::min(worldAABB.min, collisionMeshes[i].aabb.min);
            worldAABB.max = glm::max(worldAABB.max, collisionMeshes[i].aabb.max);
        }
    }

    /*
    std::cout << "Collision system built: " << collisionMeshes.size() << " meshes" << std::endl;
    std::cout << "World AABB: Min(" << worldAABB.min.x << ", " << worldAABB.min.y << ", " << worldAABB.min.z << ")" << std::endl;
    std::cout << "            Max(" << worldAABB.max.x << ", " << worldAABB.max.y << ", " << worldAABB.max.z << ")" << std::endl;
    */
}

// ========== NEW: Detailed Collision Check with Normal Data ==========
CollisionResult LevelCollision::CheckSphereCollisionDetailed(const glm::vec3& sphereCenter, float sphereRadius) {
    CollisionResult result;
    result.collided = false;

    // Quick rejection test against world AABB
    if (!worldAABB.IntersectsSphere(sphereCenter, sphereRadius)) {
        return result;
    }

    float closestDistance = FLT_MAX;

    // Check against each collision mesh
    for (const auto& mesh : collisionMeshes) {
        // First check against mesh AABB (broad phase)
        if (!mesh.aabb.IntersectsSphere(sphereCenter, sphereRadius)) {
            continue;
        }

        // Precise triangle collision (narrow phase)
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            glm::vec3 v0 = mesh.vertices[mesh.indices[i]];
            glm::vec3 v1 = mesh.vertices[mesh.indices[i + 1]];
            glm::vec3 v2 = mesh.vertices[mesh.indices[i + 2]];

            CollisionResult triangleResult;
            if (SphereTriangleIntersectionDetailed(sphereCenter, sphereRadius, v0, v1, v2, triangleResult)) {
                // Keep the closest collision (most important one)
                float distance = glm::distance(sphereCenter, triangleResult.collisionPoint);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    result = triangleResult;
                    result.collided = true;
                    result.meshName = mesh.name;
                }
            }
        }
    }

    return result;
}

// ========== Simple Boolean Collision Check ==========
bool LevelCollision::CheckSphereCollision(const glm::vec3& sphereCenter, float sphereRadius) {
    CollisionResult result = CheckSphereCollisionDetailed(sphereCenter, sphereRadius);
    return result.collided;
}

bool LevelCollision::CheckSphereCollisionWithMesh(const glm::vec3& sphereCenter, float sphereRadius, const std::string& meshName) {
    for (const auto& mesh : collisionMeshes) {
        if (mesh.name != meshName) {
            continue;
        }

        if (!mesh.aabb.IntersectsSphere(sphereCenter, sphereRadius)) {
            return false;
        }

        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            glm::vec3 v0 = mesh.vertices[mesh.indices[i]];
            glm::vec3 v1 = mesh.vertices[mesh.indices[i + 1]];
            glm::vec3 v2 = mesh.vertices[mesh.indices[i + 2]];

            if (SphereTriangleIntersection(sphereCenter, sphereRadius, v0, v1, v2)) {
                return true;
            }
        }
    }

    return false;
}

void LevelCollision::Clear() {
    collisionMeshes.clear();
    worldAABB = BoundingBox();
}

// ================== Private Helper Methods ==================

BoundingBox LevelCollision::CalculateAABB(const std::vector<glm::vec3>& vertices) {
    BoundingBox aabb;

    if (vertices.empty()) {
        return aabb;
    }

    aabb.min = vertices[0];
    aabb.max = vertices[0];

    for (const auto& vertex : vertices) {
        aabb.min = glm::min(aabb.min, vertex);
        aabb.max = glm::max(aabb.max, vertex);
    }

    return aabb;
}

CollisionMesh LevelCollision::TransformMesh(const Mesh& mesh, const glm::mat4& transform, const std::string& name) {
    CollisionMesh colMesh(name);

    // Transform all vertices to world space
    for (const auto& vertex : mesh.vertices) {
        glm::vec4 worldPos = transform * glm::vec4(vertex.position, 1.0f);
        colMesh.vertices.push_back(glm::vec3(worldPos));
    }

    // Copy indices
    colMesh.indices = mesh.indices;

    // Calculate AABB for this mesh
    colMesh.aabb = CalculateAABB(colMesh.vertices);

    return colMesh;
}

// ========== Simple Sphere-Triangle Intersection (No Normal Data) ==========
bool LevelCollision::SphereTriangleIntersection(const glm::vec3& sphereCenter, float sphereRadius,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {

    // Find closest point on triangle to sphere center
    glm::vec3 closestPoint = ClosestPointOnTriangle(sphereCenter, v0, v1, v2);

    // Check if distance is less than radius
    glm::vec3 diff = sphereCenter - closestPoint;
    float distanceSquared = glm::dot(diff, diff);

    return distanceSquared <= (sphereRadius * sphereRadius);
}

// ========== NEW: Detailed Sphere-Triangle Intersection with Normal ==========
bool LevelCollision::SphereTriangleIntersectionDetailed(const glm::vec3& sphereCenter, float sphereRadius,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    CollisionResult& result) {

    // Calculate triangle normal (perpendicular to the surface)
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 triangleNormal = glm::normalize(glm::cross(edge1, edge2));

    // Find closest point on triangle to sphere center
    glm::vec3 closestPoint = ClosestPointOnTriangle(sphereCenter, v0, v1, v2);

    // Calculate distance and direction from closest point to sphere center
    glm::vec3 diff = sphereCenter - closestPoint;
    float distance = glm::length(diff);

    // Check if collision occurred
    if (distance <= sphereRadius) {
        result.collisionPoint = closestPoint;
        result.penetrationDepth = sphereRadius - distance;

        // Determine collision normal
        if (distance > 0.0001f) {
            // Normal points from collision point toward sphere center
            result.collisionNormal = glm::normalize(diff);
        }
        else {
            // Sphere center is exactly on triangle surface, use triangle normal
            result.collisionNormal = triangleNormal;
        }

        // Ensure normal points away from triangle (toward the sphere)
        // If the normal is pointing into the triangle, flip it
        if (glm::dot(result.collisionNormal, triangleNormal) < 0.0f) {
            result.collisionNormal = triangleNormal;
        }

        return true;
    }

    return false;
}

glm::vec3 LevelCollision::ClosestPointOnTriangle(const glm::vec3& point,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {

    // Check if point projects inside triangle
    glm::vec3 edge0 = v1 - v0;
    glm::vec3 edge1 = v2 - v0;
    glm::vec3 v0ToPoint = point - v0;

    float d00 = glm::dot(edge0, edge0);
    float d01 = glm::dot(edge0, edge1);
    float d11 = glm::dot(edge1, edge1);
    float d20 = glm::dot(v0ToPoint, edge0);
    float d21 = glm::dot(v0ToPoint, edge1);

    float denom = d00 * d11 - d01 * d01;

    if (std::abs(denom) < 1e-6f) {
        // Degenerate triangle, return closest vertex
        float dist0 = glm::distance(point, v0);
        float dist1 = glm::distance(point, v1);
        float dist2 = glm::distance(point, v2);

        if (dist0 <= dist1 && dist0 <= dist2) return v0;
        if (dist1 <= dist2) return v1;
        return v2;
    }

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    // Check if point is inside triangle
    if (u >= 0.0f && v >= 0.0f && w >= 0.0f) {
        return v0 * u + v1 * v + v2 * w;
    }

    // Point is outside triangle, find closest point on edges
    glm::vec3 closestPoint = v0;
    float minDist = glm::distance(point, v0);

    // Check edge v0-v1
    float t = glm::clamp(glm::dot(point - v0, edge0) / d00, 0.0f, 1.0f);
    glm::vec3 edgePoint = v0 + edge0 * t;
    float dist = glm::distance(point, edgePoint);
    if (dist < minDist) {
        minDist = dist;
        closestPoint = edgePoint;
    }

    // Check edge v0-v2
    t = glm::clamp(glm::dot(point - v0, edge1) / d11, 0.0f, 1.0f);
    edgePoint = v0 + edge1 * t;
    dist = glm::distance(point, edgePoint);
    if (dist < minDist) {
        minDist = dist;
        closestPoint = edgePoint;
    }

    // Check edge v1-v2
    glm::vec3 edge2 = v2 - v1;
    float d22 = glm::dot(edge2, edge2);
    t = glm::clamp(glm::dot(point - v1, edge2) / d22, 0.0f, 1.0f);
    edgePoint = v1 + edge2 * t;
    dist = glm::distance(point, edgePoint);
    if (dist < minDist) {
        closestPoint = edgePoint;
    }

    return closestPoint;
}