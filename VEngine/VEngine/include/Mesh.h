#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <Context.h>

class VContext;

struct Vertex
{
    glm::vec3 pos;
};

struct GeometryInstance
{
    glm::mat3x4 transform;
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

class VMesh
{
public:
    VMesh(): transform(glm::identity<glm::mat3x4>())
    {
        meshGeometry.instanceId = 0;
        meshGeometry.mask = 0xff;
        meshGeometry.instanceOffset = 0;
        meshGeometry.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
    };

    ~VMesh() = default;

    void PushVertex(const Vertex p_vert){ vertices.push_back(p_vert); }
    void PushIndex(const uint32_t p_index){ indices.push_back(p_index); }
    void SetIndices(std::vector<uint32_t> p_indices) {indices = p_indices;}

    void Translate(glm::vec3 tr)
    {
        pos += tr;
        UpdateTransform();
    }

    void Rotate(glm::vec3 rotation)
    {
        rot += rotation;
        UpdateTransform();
    }

    void UpdateMesh();
    void UpdateTransform()
    {
        glm::mat4 translation;
        glm::mat4 rotation;

        translation = glm::translate(glm::mat4(1.0f), pos);

        rotation = glm::rotate(rotation, glm::radians(rot.x), glm::vec3(1, 0, 0));
        rotation = glm::rotate(rotation, glm::radians(rot.y), glm::vec3(0, 1, 0));
        rotation = glm::rotate(rotation, glm::radians(rot.z), glm::vec3(0, 0, 1));

        in_transform = glm::transpose(translation * rotation);
        transform = in_transform;
        meshGeometry.transform = transform;

    }
    const std::vector<Vertex>& GetVertices() const {return vertices;}
    const std::vector<uint32_t>& GetIndices() const {return indices;}

    glm::mat3x4 transform;
    glm::mat4 in_transform;

    glm::vec3 pos;
    glm::vec3 rot;

    VBuffer::Buffer meshBuffer;
    GeometryInstance meshGeometry;
private:
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
};
