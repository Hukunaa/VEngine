#pragma once
#include <Mesh.h>
#include <glm/glm.hpp>
#include <vector>

struct VMaterial
{
    glm::vec4 colorAndRoughness;
};

struct Material
{
    std::vector<VMaterial> m_materials;
};

class VObject
{
public:
    VObject(const char* name): translationMat(glm::mat4(1.0f)),
    rotationMat(glm::mat4(1.0f)),
    scaleMat(glm::mat4(1.0f)),
    m_name(name) {}

    ~VObject() = default;

    void SetPosition(glm::vec3 pos)
    {
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        translationMat = translate;
        m_transform = glm::transpose(translationMat * rotationMat * scaleMat);
        m_mesh.meshGeometry.transform = m_transform;
    }
    void SetRotation(glm::vec3 angle)
    {
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.x), glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.y), glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.z), glm::vec3(0, 0, 1));
        m_transform = glm::transpose(translationMat * rotationMat * scaleMat);
        m_mesh.meshGeometry.transform = m_transform;
    }
    void SetScale(float factor)
    {
        scaleMat = glm::scale(scaleMat, glm::vec3(factor));
        m_transform = glm::transpose(translationMat * rotationMat * scaleMat);
        m_mesh.meshGeometry.transform = m_transform;
    }

    const char* GetName() const
    { return m_name; }

    VMesh m_mesh;
    VMaterial m_material;

    glm::mat3x4 m_transform;
    glm::vec3 position;
    glm::vec3 rotation;
private:
    const char* m_name;

    glm::mat4 translationMat;
    glm::mat4 rotationMat;
    glm::mat4 scaleMat;
};