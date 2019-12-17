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

    static enum MESH_PRIMITIVE
    {
        CUBE,
        TRIANGLE
    };

    VMesh()
    {
        meshGeometry.instanceId = 0;
        meshGeometry.mask = 0xff;
        meshGeometry.instanceOffset = 0;
        meshGeometry.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
    };
    ~VMesh() = default;

    void SetMeshType(MESH_PRIMITIVE primitive)
    {
        if(primitive == MESH_PRIMITIVE::CUBE)
        {
            PushVertex({{1.000000, -1.000000, -1.000000}});
            PushVertex({{1.000000, -1.000000, 1.000000}});
            PushVertex({{-1.000000, -1.000000, 1.000000}});
            PushVertex({{-1.000000, -1.000000, -1.000000}});
            PushVertex({{1.000000, 1.000000, -0.999999}});
            PushVertex({{0.999999, 1.000000, 1.000001}});
            PushVertex({{-1.000000, 1.000000, 1.000000}});
            PushVertex({{-1.000000, 1.000000, -1.000000}});
            SetIndices({ 1, 2, 3, 7, 6, 5, 4, 5, 1, 5, 6, 2, 2, 6, 7, 0, 3, 7, 1, 1, 3, 4, 7, 5, 0, 4, 1, 1, 5, 2, 3, 2, 7, 4, 0, 7});
        }
    }

    void PushVertex(const Vertex p_vert){ vertices.push_back(p_vert); }
    void PushIndex(const uint32_t p_index){ indices.push_back(p_index); }
    void SetIndices(std::vector<uint32_t> p_indices) {indices = p_indices;}

    void UpdateMesh();

    const std::vector<Vertex>& GetVertices() const {return vertices;}
    const std::vector<uint32_t>& GetIndices() const {return indices;}

    VBuffer::Buffer meshBuffer;
    GeometryInstance meshGeometry;
private:
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
};
