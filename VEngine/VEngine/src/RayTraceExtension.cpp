#include "RayTraceExtension.h"

void RayTraceExtension::initRayTracing(int maxRecursionDepth)
{
    m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    m_raytracingProperties.pNext = nullptr;
    m_raytracingProperties.maxRecursionDepth = maxRecursionDepth;
    m_raytracingProperties.shaderGroupHandleSize = 0;
    VkPhysicalDeviceProperties2 props;
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props.pNext = &m_raytracingProperties;
    props.properties = {};
    vkGetPhysicalDeviceProperties2(context->SelectedGPU, &props);
}

void RayTraceExtension::createGeometryInstances(VkBuffer m_vertexBuffer, uint32_t m_nbVertices, VkBuffer m_indexBuffer, uint32_t m_nbIndices)
{
    glm::mat4x4 mat = glm::mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    m_geometryInstances.push_back({ m_vertexBuffer, m_nbVertices, 0, m_indexBuffer, m_nbIndices, 0, mat });
}

AccelerationStructure RayTraceExtension::createBottomLevelAS(VkCommandBuffer commandBuffer, std::vector<GeometryInstance> vVertexBuffers)
{
    nv_helpers_vk::BottomLevelASGenerator bottomLevelAS;

    // Adding all vertex buffers and not transforming their position.
    for (const auto& buffer : vVertexBuffers)
    {
        if (buffer.indexBuffer == VK_NULL_HANDLE)
        {
            // No indices
            bottomLevelAS.AddVertexBuffer(buffer.vertexBuffer, buffer.vertexOffset, buffer.vertexCount,
                3 * sizeof(float), VK_NULL_HANDLE, 0);
        }
        else
        {
            // Indexed geometry
            bottomLevelAS.AddVertexBuffer(buffer.vertexBuffer, buffer.vertexOffset, buffer.vertexCount,
                3 * sizeof(float), buffer.indexBuffer, buffer.indexOffset,
                buffer.indexCount, VK_NULL_HANDLE, 0);
        }
    }
    AccelerationStructure buffers;

    // Once the overall size of the geometry is known, we can create the handle
    // for the acceleration structure
    buffers.structure = bottomLevelAS.CreateAccelerationStructure(context->logicalDevice, VK_FALSE);

    // The AS build requires some scratch space to store temporary information.
 // The amount of scratch memory is dependent on the scene complexity.
    VkDeviceSize scratchSizeInBytes = 0;
    // The final AS also needs to be stored in addition to the existing vertex
    // buffers. It size is also dependent on the scene complexity.
    VkDeviceSize resultSizeInBytes = 0;
    bottomLevelAS.ComputeASBufferSizes(context->logicalDevice, buffers.structure, &scratchSizeInBytes,
        &resultSizeInBytes);

    // Once the sizes are obtained, the application is responsible for allocating
    // the necessary buffers. Since the entire generation will be done on the GPU,
    // we can directly allocate those in device local mem
    nv_helpers_vk::createBuffer(context->SelectedGPU, context->logicalDevice, scratchSizeInBytes,
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &buffers.scratchBuffer,
        &buffers.scratchMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    nv_helpers_vk::createBuffer(context->SelectedGPU, context->logicalDevice, resultSizeInBytes,
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &buffers.resultBuffer,
        &buffers.resultMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    bottomLevelAS.Generate(context->logicalDevice, commandBuffer, buffers.structure, buffers.scratchBuffer,
        0, buffers.resultBuffer, buffers.resultMem, false, VK_NULL_HANDLE);

    return buffers;
}

//--------------------------------------------------------------------------------------------------
// Create the main acceleration structure that holds all instances of the scene.
// Similarly to the bottom-level AS generation, it is done in 3 steps: gathering
// the instances, computing the memory requirements for the AS, and building the
// AS itself #VKRay
void RayTraceExtension::createTopLevelAS(VkCommandBuffer commandBuffer, const std::vector<std::pair<VkAccelerationStructureNV, glm::mat4x4>>& instances, VkBool32 updateOnly)
{
    if (!updateOnly)
    {
        // Gather all the instances into the builder helper
        for (size_t i = 0; i < instances.size(); i++)
        {
            // For each instance we set its instance index to its index i in the
            // instance vector, and set its hit group index to 2*i. The hit group
            // index defines which entry of the shader binding table will contain the
            // hit group to be executed when hitting this instance. We set this index
            // to i due to the use of 1 type of rays in the scene: the camera rays
            m_topLevelASGenerator.AddInstance(instances[i].first, instances[i].second,
                static_cast<uint32_t>(i), static_cast<uint32_t>(i));
        }

        // Once all instances have been added, we can create the handle for the TLAS
        m_topLevelAS.structure =
            m_topLevelASGenerator.CreateAccelerationStructure(context->logicalDevice, VK_TRUE);

        VkDeviceSize scratchSizeInBytes, resultSizeInBytes, instanceDescsSizeInBytes;
        m_topLevelASGenerator.ComputeASBufferSizes(context->logicalDevice, m_topLevelAS.structure,
            &scratchSizeInBytes, &resultSizeInBytes,
            &instanceDescsSizeInBytes);

        // Create the scratch and result buffers. Since the build is all done on
        // GPU, those can be allocated in device local memory
        nv_helpers_vk::createBuffer(context->SelectedGPU, context->logicalDevice, scratchSizeInBytes,
            VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &m_topLevelAS.scratchBuffer,
            &m_topLevelAS.scratchMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        nv_helpers_vk::createBuffer(context->SelectedGPU, context->logicalDevice, resultSizeInBytes,
            VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &m_topLevelAS.resultBuffer,
            &m_topLevelAS.resultMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // The buffer describing the instances: ID, shader binding information,
        // matrices ... Those will be copied into the buffer by the helper through
        // mapping, so the buffer has to be allocated in host visible memory.

        nv_helpers_vk::createBuffer(context->SelectedGPU, context->logicalDevice,
            instanceDescsSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
            &m_topLevelAS.instancesBuffer, &m_topLevelAS.instancesMem,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    m_topLevelASGenerator.Generate(context->logicalDevice, commandBuffer, m_topLevelAS.structure,
        m_topLevelAS.scratchBuffer, 0, m_topLevelAS.resultBuffer,
        m_topLevelAS.resultMem, m_topLevelAS.instancesBuffer,
        m_topLevelAS.instancesMem, updateOnly,
        updateOnly ? m_topLevelAS.structure : VK_NULL_HANDLE);
}

// Create the bottom-level and top-level acceleration structures
// #VKRay
void RayTraceExtension::createAccelerationStructures()
{
    // Create a one-time command buffer in which the AS build commands will be
    // issued
    /*VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = context->getCommandPool()[VkCtx.getFrameIndex()];
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;*/
}