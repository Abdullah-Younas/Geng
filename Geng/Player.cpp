#include "Player.h"
#include "Formulas.h"
#include "LevelCollision.h"

Formulas formulas;

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
    skinWidth(0.0005f),
    velocity(0.0f, 0.0f, 0.0f),
    Colliding(false),
    collisionDepth(0),
    collisionNormal(0.0f, 0.0f, 0.0f),
    intendedMovement(0.0f, 0.0f, 0.0f),
    isRayHitting(false),
    rayHitPoint(0.0f, 0.0f, 0.0f),
    rayHitNormal(0.0f, 0.0f, 0.0f),
    rayDistance(0.0f)
{
    UpdateCameraFromSphere();
}

void Player::CalculateMovement(float deltaTime) {
    // Calculate gravity effect
    verticalVelocity += GRAVITY * deltaTime;

    // Build the intended movement vector (DON'T apply it yet!)
    intendedMovement = glm::vec3(0.0f);
    intendedMovement.y = verticalVelocity * deltaTime;

    // Horizontal movement is already in velocity from ProcessKeyboard
    intendedMovement.x = velocity.x;
    intendedMovement.z = velocity.z;
}

void Player::ApplyMovement(const glm::vec3& movement) {
    // Actually move the player
    spherePosition += movement;
    sphereCenter = spherePosition;
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
    glm::vec3 horizontalVelocity = glm::vec3(0.0f);

    if (forward)
        horizontalVelocity += front * speed;
    if (backward)
        horizontalVelocity -= front * speed;
    if (left)
        horizontalVelocity -= rightVec * speed;
    if (right)
        horizontalVelocity += rightVec * speed;

    // ONLY store velocity, DON'T move yet
    velocity = horizontalVelocity;
    // Don't set velocity.y here - it's managed by gravity

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
    // Just update camera speed based on position
    if (spherePosition.y > GROUND_LEVEL) {
        camera.UpdateSpeed(AIR_SPEED);
    }
    else {
        camera.UpdateSpeed(GROUND_SPEED);
    }
}

void Player::PerformRaycast(LevelCollision& collision, float maxDistance) {
    isRayHitting = false;
    rayDistance = maxDistance;

    glm::vec3 origin = camera.Position;

    glm::vec3 rayDir = glm::normalize(camera.Front);

    const int steps = 100;
    float stepSize = maxDistance / steps;

    for (int i = 0; i <= steps; i++) {
        float currentDistance = i * stepSize;
        glm::vec3 TestPoint = origin + (rayDir * currentDistance);

        CollisionResult result = collision.CheckSphereCollisionDetailed(TestPoint, 0.01f);

        if (result.collided) {
            isRayHitting = true;
            rayHitPoint = TestPoint;
            rayHitNormal = result.collisionNormal;
            rayDistance = currentDistance;
            break;
        }
    }
}


void Player::UpdateCameraFromSphere() {
    camera.Position = spherePosition + glm::vec3(0.0f, 1.5f, 0.0f);
}

void Player::passCollisionData(int depth, glm::vec3& CollisionNormal) {
    collisionDepth = depth;
    collisionNormal = CollisionNormal;
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

glm::vec3 Player::CollideAndSlide(const glm::vec3& vel, int depth) {
    if (depth >= maxBounces) {
        return glm::vec3(0.0f);
    }

    float dist = glm::length(vel);

    // If velocity is too small, stop
    if (dist < skinWidth * 0.01f) {
        return glm::vec3(0.0f);
    }

    if (Colliding) {
        // Calculate how far we can move before hitting
        glm::vec3 moveDir = glm::normalize(vel);
        float safeDistance = dist - skinWidth;

        if (safeDistance < 0.0f) {
            safeDistance = 0.0f;
        }

        glm::vec3 snapToSurface = moveDir * safeDistance;
        glm::vec3 leftover = vel - snapToSurface;

        // Project leftover onto the collision plane
        glm::vec3 slidingVel = formulas.ProjectOnPlane(leftover, collisionNormal);

        // Reduce magnitude slightly to prevent sliding into surfaces
        slidingVel *= 0.95f;

        // Return the safe movement plus any sliding movement
        return snapToSurface + slidingVel;
    }

    return vel;
}

glm::vec3 Player::GetVelocity() const {
    return velocity;
}