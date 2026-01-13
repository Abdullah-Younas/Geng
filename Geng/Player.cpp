#include "Player.h"

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
    velocity(0.0f, 0.0f, 0.0f),
    Colliding(false),
    collisionDepth(0)
{
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

    // Build horizontal velocity only
    glm::vec3 horizontalVelocity = glm::vec3(0.0f);

    if (forward)
        horizontalVelocity += front * speed;
    if (backward)
        horizontalVelocity -= front * speed;
    if (left)
        horizontalVelocity -= rightVec * speed;
    if (right)
        horizontalVelocity += rightVec * speed;

    // Apply only horizontal movement here
    spherePosition.x += horizontalVelocity.x;
    spherePosition.z += horizontalVelocity.z;
    sphereCenter = spherePosition;

    // Store for GetVelocity (combine horizontal + vertical for external use)
    velocity = horizontalVelocity;
    velocity.y = verticalVelocity;

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

    // Apply vertical movement ONCE here
    spherePosition.y += verticalVelocity * deltaTime;
    sphereCenter.y = spherePosition.y;

    // Update speed based on air/ground state
    if (spherePosition.y > GROUND_LEVEL) {
        camera.UpdateSpeed(AIR_SPEED);
    }
}

void Player::passCollisionData(int depth) {
    collisionDepth = depth;
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

void Player::SetSpherePosition(const glm::vec3& newPosition) {
    spherePosition = newPosition;
    sphereCenter = newPosition;
    UpdateCameraFromSphere();
}

void Player::MoveSphere(const glm::vec3& delta) {
    spherePosition += delta;
    sphereCenter += delta;
    UpdateCameraFromSphere();
}

glm::vec3 Player::CollideAndSlide() {
    glm::vec3 Pvelocity = GetVelocity();
    glm::vec3 Pposition = GetPosition();

    if (collisionDepth >= maxBounces) {
        return glm::vec3(0.0f);
    }

    float dist = glm::length(Pvelocity) + skinWidth;

    if (Colliding == true) {
        glm::vec3 snapToSurface = glm::normalize(Pvelocity) * (dist - skinWidth);
        glm::vec3 leftover = Pvelocity - snapToSurface;

        if (glm::length(snapToSurface) <= skinWidth) {
            snapToSurface = glm::vec3(0.0f);
        }

        float mag = glm::length(leftover);
        // leftover = transformer.ProjectOnPlane(leftover, Normal of hit); then normalize this whole projectonplane
        //furtheron
        //return snapToSurface = CollideAndSlide(leftover, pos + snapToSurface, depth + 1);
    }

    return Pvelocity;
}

glm::vec3 Player::GetVelocity() const {
    return velocity;
}