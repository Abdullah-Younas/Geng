#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "Camera.h"

class LevelCollision;

class Player {
public:
    // Camera component
    Camera camera;

    // Sphere collision component
    glm::vec3 spherePosition;
    glm::vec3 sphereCenter;
    float sphereScale;
    glm::vec3 collisionResponse;
    int maxBounces;
    float skinWidth;
    bool Colliding;
    int collisionDepth;
	glm::vec3 collisionNormal;

    // Raycast properties
    bool isRayHitting;
	glm::vec3 rayHitPoint;
	glm::vec3 rayHitNormal;
    float rayDistance;

    // Movement properties
    float verticalVelocity;
    int jumpCount;
    bool isJumping;

    // Constants
    static constexpr float GRAVITY = -14.0f;
    static constexpr float JUMP_FORCE = 7.0f;
    static constexpr float GROUND_SPEED = 6.5f;
    static constexpr float AIR_SPEED = 5.5f;
    static constexpr float GROUND_LEVEL = 0.0f;
    static constexpr int MAX_JUMPS = 2;
    static constexpr float DEFAULT_RAY_DISTANCE = 15.0f;

    // Constructor
    Player(const glm::vec3& startPosition = glm::vec3(0.0f, 0.5f, 1.0f));

    // Update methods
    void ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime);
    void Jump();
    void Update(float deltaTime);

    // Raycast methods
	void PerformRaycast(LevelCollision& collision, float maxDistance = DEFAULT_RAY_DISTANCE);
    bool GetRayHit() const { return isRayHitting; }
    glm::vec3 GetRayHitPoint() const { return rayHitPoint; }
    glm::vec3 GetRayHitNormal() const { return rayHitNormal; }
    float GetRayDistance() const { return rayDistance; }


    // Collision response methods (call these from main)
    void SetSpherePosition(const glm::vec3& newPosition);
    void MoveSphere(const glm::vec3& delta);
	void passCollisionData(int depth, glm::vec3& CollisionNormal);

    // Getters
    glm::vec3 GetPosition() const { return camera.Position; }
    glm::vec3 GetSpherePosition() const { return spherePosition; }
    glm::vec3 GetFront() const { return camera.Front; }
    glm::mat4 GetViewMatrix() { return camera.GetViewMatrix(); }
    glm::vec3 GetVelocity() const;
    float GetZoom() const { return camera.Zoom; }

    glm::vec3 CollideAndSlide(const glm::vec3& vel, int depth = 0);
    void CalculateMovement(float deltaTime);  // Calculate intended movement
    void ApplyMovement(const glm::vec3& movement);  // Actually move the player
    glm::vec3 GetIntendedMovement() const { return intendedMovement; }

private:
    glm::vec3 intendedMovement;
    void UpdateCameraFromSphere();  // Changed from UpdateSpherePosition
    //void ApplyGravity(float deltaTime);
    glm::vec3 velocity;
};

#endif // PLAYER_H