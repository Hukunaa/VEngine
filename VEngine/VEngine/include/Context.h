#pragma once
#include <optional>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <map>

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

class VContext
{
public:

    VContext() = default;
    ~VContext() = default;

    void CreateWinow(int width, int height, const char* name)
    {
        window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }

    void SetupInstance();
    void CreateLogicalDevice();
    void SelectGPU();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void CleanUp();
    void SetupDebugMessenger();
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    VkResult& GetResult() { return result; }
    VkInstance& GetInstance() { return instance; }

    bool CheckValidationLayerSupport();
    bool IsDeviceSuitable(VkPhysicalDevice device);

    std::vector<const char*> GetRequieredExtensions();
    std::vector<const char*>& GetValidationLayers() { return validationLayers; }

    GLFWwindow* GetWindow() { return window; }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
private:

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GLFWwindow* window;

    VkInstance instance;
    VkResult result;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice logicalDevice;
    VkPhysicalDevice SelectedGPU;
    VkQueue graphicsQueue;
    std::vector<VkPhysicalDevice> GPUs;
};