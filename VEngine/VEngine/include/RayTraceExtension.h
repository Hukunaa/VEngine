#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <Context.h>
#include <utility>
#include <nv_helpers_vk/BottomLevelASGenerator.h>
#include <nv_helpers_vk/TopLevelASGenerator.h>
#include <nv_helpers_vk/VKHelpers.h>

struct GeometryInstance
{
    VkBuffer vertexBuffer;
    uint32_t vertexCount;
    VkDeviceSize vertexOffset;

    VkBuffer indexBuffer;
    uint32_t indexCount;
    VkDeviceSize indexOffset;

    glm::mat4x4 transform;
};

struct AccelerationStructure
{
    VkBuffer scratchBuffer = VK_NULL_HANDLE;
    VkDeviceMemory scratchMem = VK_NULL_HANDLE;
    VkBuffer resultBuffer = VK_NULL_HANDLE;
    VkDeviceMemory resultMem = VK_NULL_HANDLE;
    VkBuffer instancesBuffer = VK_NULL_HANDLE;
    VkDeviceMemory instancesMem = VK_NULL_HANDLE;
    VkAccelerationStructureNV structure = VK_NULL_HANDLE;
};

class RayTraceExtension
{
public:
    RayTraceExtension(VContext* p_context) : context{ p_context }{}

    ~RayTraceExtension() = default;

    void initRayTracing(int maxRecursionDepth);
    void createGeometryInstances(VkBuffer m_vertexBuffer, uint32_t m_nbVertices, VkBuffer m_indexBuffer, uint32_t m_nbIndices);

    AccelerationStructure createBottomLevelAS(VkCommandBuffer commandBuffer, std::vector<GeometryInstance> vVertexBuffers);
    void createTopLevelAS(VkCommandBuffer commandBuffer, const std::vector<std::pair<VkAccelerationStructureNV, glm::mat4x4>>& instances, VkBool32 updateOnly);
    void createAccelerationStructures();
    void destroyAccelerationStructure(const AccelerationStructure& as);

    VkPhysicalDeviceRayTracingPropertiesNV m_raytracingProperties = {};
    nv_helpers_vk::TopLevelASGenerator m_topLevelASGenerator;
    AccelerationStructure              m_topLevelAS;
    std::vector<AccelerationStructure> m_bottomLevelAS;

    std::vector<GeometryInstance> m_geometryInstances;
    VContext* context;
};

