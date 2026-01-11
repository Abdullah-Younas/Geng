#include "Player.h"

Player::Player(const glm::vec3& startPosition)
    : camera(startPosition),
    spherePosition(0.0f),
    sphereCenter(0.0f),
    sphereScale(0.2f),
    verticalVelocity(0.0f),
    jumpCount(0),
    isJumping(false)
{
    UpdateSpherePosition();
}

void Player::ProcessKeyboard(bool forward, bool backward, bool left, bool right, float deltaTime) {
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
    UpdateSpherePosition();
}

void Player::UpdateSpherePosition() {
    // Maintain offset for collision sphere (below and in front of camera)
    spherePosition = camera.Position + glm::vec3(0.0f, -1.0f, -4.0f);
    sphereCenter = spherePosition;
}

void Player::ApplyGravity(float deltaTime) {
    verticalVelocity += GRAVITY * deltaTime;
    camera.Position.y += verticalVelocity * deltaTime;

    // Update speed based on air/ground state
    if (camera.Position.y > GROUND_LEVEL) {
        camera.UpdateSpeed(AIR_SPEED);
    }
}

void Player::CheckGroundCollision() {
    if (camera.Position.y <= GROUND_LEVEL) {
        camera.Position.y = GROUND_LEVEL;
        verticalVelocity = 0.0f;
        jumpCount = 0;
        isJumping = false;
        camera.UpdateSpeed(GROUND_SPEED);
    }
}