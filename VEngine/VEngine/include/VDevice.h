#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

namespace VDevice
{
    struct Device
    {
        VkPhysicalDevice physicalDevice;
        /** @brief Logical device representation (application's view of the device) */
        VkDevice logicalDevice;
        /** @brief Properties of the physical device including limits that the application can check against */
        VkPhysicalDeviceProperties properties;
        /** @brief Features of the physical device that an application can use to check if a feature is supported */
        VkPhysicalDeviceFeatures features;
        /** @brief Features that have been enabled for use on the physical device */
        VkPhysicalDeviceFeatures enabledFeatures;
        /** @brief Memory types and heaps of the physical device */
        VkPhysicalDeviceMemoryProperties memoryProperties;
        /** @brief Queue family properties of the physical device */
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        /** @brief KHR Surface of te device */
        VkSurfaceKHR surface;
        /** @brief List of extensions supported by the device */
        std::vector<std::string> supportedExtensions;

        uint32_t swapChainImageCount;
    };
}