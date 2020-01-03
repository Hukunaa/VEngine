#pragma once
#include <VMesh.h>
#include <glm/glm.hpp>
#include <vector>

struct VMaterial
{
    glm::vec4 colorAndRoughness;
    glm::vec4 ior;
};

struct Material
{
    std::vector<VMaterial> m_materials;
};

class VObject
{
public:
    VObject(const char* name): m_name(name),
    translationMat(glm::mat4(1.0f)),
    rotationMat(glm::mat4(1.0f)),
    scaleMat(glm::mat4(1.0f))
    {
        m_material.colorAndRoughness = glm::vec4(1);
        m_material.ior = glm::vec4(0);
    }

    ~VObject() = default;

    void SetPosition(glm::vec3 pos)
    {
        const glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        translationMat = translate;
        m_transform = glm::transpose(translationMat * rotationMat * scaleMat);
        m_mesh.meshGeometry.transform = m_transform;
    }
    void Translate(glm::vec3 pos)
    {
        const glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        translationMat *= translate;
        m_transform = glm::transpose(translationMat * rotationMat * scaleMat);
        m_mesh.meshGeometry.transform = m_transform;
    }
    void Rotate(glm::vec3 angle)
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
    void SetColor(float r, float g, float b)
    {
        m_material.colorAndRoughness = {r, g, b, m_material.colorAndRoughness.w};
    }
    void SetMaterialType(int type)
    {
        m_material.colorAndRoughness.w = type;
    }

    //Only if material type is "2"
    void SetReflectivity(float factor)
    {
        m_material.ior.x = factor;
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