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
        glm::mat4 rotM = glm::mat4(1.0f);

        rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 transM = glm::translate(glm::mat4(1.0f), position);

        matrices.view = transM * rotM;
        updated = true;
    };

    enum CameraType { lookat, firstperson };
    CameraType type = CameraType::lookat;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

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

    bool moving() const
    {
        return keys.left || keys.right || keys.up || keys.down;
    }

    float getNearClip() const
    {
        return znear;
    }

    float getFarClip() const
    {
        return zfar;
    }

    void setPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    };

    void updateAspectRatio(float aspect)
    {
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    }

    void setPosition(glm::vec3 position)
    {
        this->position = position;
        updateViewMatrix();
    }

    void setRotation(glm::vec3 rotation)
    {
        this->rotation = rotation;
        updateViewMatrix();
    };

    void rotate(glm::vec3 delta)
    {
        this->rotation += delta;
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

    void update(float deltaTime)
    {
        updated = false;
        if (type == CameraType::firstperson)
        {
            if (moving())
            {
                glm::vec3 camFront;
                camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                camFront.y = sin(glm::radians(rotation.x));
                camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                camFront = glm::normalize(camFront);

                const float moveSpeed = deltaTime * movementSpeed;

                if (keys.up)
                    position += camFront * moveSpeed;
                if (keys.down)
                    position -= camFront * moveSpeed;
                if (keys.left)
                    position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
                if (keys.right)
                    position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

                updateViewMatrix();
            }
        }
    };

};