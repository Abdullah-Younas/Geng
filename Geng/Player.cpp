#include "Player.h"
//#include "Transformations.h"

//Transformations transformer;

Player::Player(const glm::vec3& startPosition)
    : camera(startPosition),
    spherePosition(startPosition),
    sphereCenter(startPosition),
    sphereScale(0.15f),
    verticalVelocity(0.0f),
    jumpCount(0),
    isJumping(false),
    collisionResponse(0.15, 0.26, 0.21),
    maxBounces(5),
    skinWidth(0.015f),
    velocity(0.0f, 0.0f, 0.0f)  // Initialize it
{
    // Camera follows sphere from the start
    UpdateCameraFromSphere();
}

void Player::ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime) {
    glm::vec3 front = camera.Front;
    glm::vec3 rightVec = camera.Right;

    front.y = 0.0f;
    rightVec.y = 0.0f;
    front = glm::normalize(front);
    rightVec = glm::normalize(rightVec);

    float speed = camera.MovementSpeed * deltaTime;

    velocity = glm::vec3(0.0f);  // Reset horizontal velocity

    if (forward)
        velocity += front * speed;
    if (backward)
        velocity -= front * speed;
    if (left)
        velocity -= rightVec * speed;
    if (right)
        velocity += rightVec * speed;

    velocity.y = verticalVelocity;  // Keep vertical velocity

    spherePosition += velocity;
    sphereCenter = spherePosition;
    camera.ProcessKeyboard(forward, backward, left, right, deltaTime);
}

void Player::Jump() {
    if (jumpCount < MAX_JUMPS) {
        verticalVelocity = JUMP_FORCE;
        jumpCount++;
        isJumping = true;
    }
}

void Player::Update(float deltaTime) {
    ApplyGravity(deltaTime);
    CheckGroundCollision();
    UpdateCameraFromSphere();
}

void Player::UpdateCameraFromSphere() {
    camera.Position = spherePosition + glm::vec3(0.0f, 0.5f, 0.0f);
}

void Player::ApplyGravity(float deltaTime) {
    verticalVelocity += GRAVITY * deltaTime;
    spherePosition.y += verticalVelocity * deltaTime;
    sphereCenter.y = spherePosition.y;

    // Update speed based on air/ground state
    if (spherePosition.y > GROUND_LEVEL) {
        camera.UpdateSpeed(AIR_SPEED);
    }
}

void Player::CheckGroundCollision() {
    if (spherePosition.y <= GROUND_LEVEL) {
        spherePosition.y = GROUND_LEVEL;
        sphereCenter.y = GROUND_LEVEL;
        verticalVelocity = 0.0f;
        jumpCount = 0;
        isJumping = false;
        camera.UpdateSpeed(GROUND_SPEED);
    }
}

// Add this method for external collision response
void Player::SetSpherePosition(const glm::vec3& newPosition) {
    spherePosition = newPosition;
    sphereCenter = newPosition;
    UpdateCameraFromSphere();
}

// Add this method if you need to move the sphere by a delta
void Player::MoveSphere(const glm::vec3& delta) {
    spherePosition += delta;
    sphereCenter += delta;
    UpdateCameraFromSphere();
}

glm::vec3 Player::CollideAndSlide(glm::vec3 vel, glm::vec3 pos, int depth) {
    if (depth >= maxBounces) {
        return glm::vec3(0.0f);
    }

    float dist = glm::length(vel) + skinWidth;
    glm::vec3  snapToSurface = glm::normalize(vel) * (dist - skinWidth);
    glm::vec3 leftover = vel - snapToSurface;

    if (glm::length(snapToSurface) <= skinWidth) {
        snapToSurface = glm::vec3(0.0f);
    }

    float mag = glm::length(leftover);
    // leftover = transformer.ProjectOnPlane(leftover, Normal of hit); then normalize this whole projectonplane
    //furtheron
}

glm::vec3 Player::GetVelocity() const {
    return velocity;
}