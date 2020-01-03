#pragma once
#include <cmath>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Camera
{
private:
    float fov;
    float znear, zfar;

public:

    Camera() : fov(60.0f), znear(0.1f), zfar(512.0f){}

    void updateViewMatrix()
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));

        updated = true;
        matrices.view = glm::lookAt(position, position + Front, Up);
    }

    float Pitch;
    float Yaw;

    glm::vec3 position = glm::vec3();
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up;

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;
    int sample = 2;

    bool updated = false;

    struct
    {
        glm::mat4 perspective{};
        glm::mat4 view{};
    } matrices;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys;

    void setPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    };

    void setPosition(glm::vec3 position)
    {
        this->position = position;
        updateViewMatrix();
    }

    void setTranslation(glm::vec3 translation)
    {
        this->position += translation;
        updateViewMatrix();
    };

    void translate(glm::vec3 delta)
    {
        this->position += delta;
        updateViewMatrix();
    }

};