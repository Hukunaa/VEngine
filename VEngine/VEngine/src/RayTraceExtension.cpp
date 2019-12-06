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

void RayTraceExtension::createGeometryInstances()
{
}
