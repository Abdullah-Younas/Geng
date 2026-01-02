#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float rollAng = 15.0f; // max roll
const float rollSpeed = 100.0f; // degrees per second
const float SPEED = 6.5f;
const float SENSITIVITY = 0.15f;
const float ZOOM = 75.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    float Roll;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    void UpdateSpeed(float speed) {
        MovementSpeed = speed;
	}

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = rollAng) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        Roll = roll;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float roll) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        Roll = roll;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        glm::vec3 front = glm::normalize(Front);
        glm::vec3 right = glm::normalize(glm::cross(front, WorldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        // Apply roll rotation around the forward vector
        glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), glm::radians(Roll), front);
        up = glm::vec3(rollMat * glm::vec4(up, 0.0f));
        right = glm::normalize(glm::cross(front, up));

        return glm::lookAt(Position, Position + front, up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        float velocity = MovementSpeed * deltaTime;

        float targetRoll = 0.0f; // default: no roll

        if (direction == FORWARD)
            Position += flatFront * velocity;
        if (direction == BACKWARD)
            Position -= flatFront * velocity;
        if (direction == LEFT)
        {
            Position -= Right * velocity; // move left
            targetRoll = -rollAng;        // roll left
        }
        if (direction == RIGHT)
        {
            Position += Right * velocity; // move right
            targetRoll = rollAng;         // roll right
        }

        // Smoothly interpolate roll toward targetRoll
        if (Roll < targetRoll)
        {
            Roll += rollSpeed * deltaTime;
            if (Roll > targetRoll) Roll = targetRoll;
        }
        else if (Roll > targetRoll)
        {
            Roll -= rollSpeed * deltaTime;
            if (Roll < targetRoll) Roll = targetRoll;
        }

        // clamp roll
        if (Roll > rollAng) Roll = rollAng;
        if (Roll < -rollAng) Roll = -rollAng;
    }


    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 30.0f)
            Zoom = 30.0f;
        if (Zoom > 75.0f)
            Zoom = 75.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif