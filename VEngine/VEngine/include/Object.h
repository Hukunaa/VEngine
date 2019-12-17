#pragma once
#include <Mesh.h>
#include <glm/glm.hpp>

class VObject
{
public:
    VObject(): translationMat(glm::mat4(1.0f)), rotationMat(glm::mat4(1.0f)) {}
    ~VObject() = default;

    void SetPosition(glm::vec3 pos)
    {
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        translationMat = translate;
        m_transform = glm::transpose(translationMat * rotationMat);
        m_mesh.meshGeometry.transform = m_transform;
    }
    void SetRotation(glm::vec3 angle)
    {
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.x), glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.y), glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.z), glm::vec3(0, 0, 1));
        m_transform = glm::transpose(translationMat * rotationMat);
        m_mesh.meshGeometry.transform = m_transform;
    }

    glm::mat3x4 m_transform;
    VMesh m_mesh;

    glm::vec3 position;
    glm::vec3 rotation;

private:
    glm::mat4 translationMat;
    glm::mat4 rotationMat;
};