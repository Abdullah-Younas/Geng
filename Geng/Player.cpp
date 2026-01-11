#include "Player.h"

Player::Player(const glm::vec3& startPosition)
    : camera(startPosition),
    spherePosition(startPosition),
    sphereCenter(startPosition),
    sphereScale(0.15f),
    verticalVelocity(0.0f),
    jumpCount(0),
    isJumping(false)
{
    // Camera follows sphere from the start
    UpdateCameraFromSphere();
}

void Player::ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime) {
    // Move the sphere instead of the camera directly
    glm::vec3 front = camera.Front;
    glm::vec3 rightVec = camera.Right;

    // Project onto horizontal plane (ignore Y component for movement)
    front.y = 0.0f;
    rightVec.y = 0.0f;
    front = glm::normalize(front);
    rightVec = glm::normalize(rightVec);

    float speed = camera.MovementSpeed * deltaTime;

    if (forward)
        spherePosition += front * speed;
    if (backward)
        spherePosition -= front * speed;
    if (left)
        spherePosition -= rightVec * speed;
    if (right)
        spherePosition += rightVec * speed;

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