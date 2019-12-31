#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <VInitializers.h>

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

    VMesh()
    {
        meshGeometry.instanceId = 0;
        meshGeometry.mask = 0xff;
        meshGeometry.instanceOffset = 0;
        meshGeometry.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
    };
    ~VMesh() = default;

    void LoadMesh(const std::string& path, bool flipNormals);
    void processNode(aiNode *node, const aiScene *scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);




    void SetOffset(uint32_t offset)
    {
        meshGeometry.instanceOffset = offset;
    }

    void PushVertex(const Vertex p_vert){ vertices.push_back(p_vert); }
    void PushIndex(const uint32_t p_index){ indices.push_back(p_index); }
    void SetIndices(std::vector<uint32_t> p_indices) {indices = p_indices;}

    void UpdateMesh();

    const std::vector<Vertex>& GetVertices() const {return vertices;}
    const std::vector<uint32_t>& GetIndices() const {return indices;}

    VBuffer::Buffer meshBuffer;
    GeometryInstance meshGeometry;
    std::vector<float> bufferVertices;

private:
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::string directory;
};
