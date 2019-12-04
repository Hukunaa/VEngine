#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>

class Context
{
public:

    Context() = default;
    ~Context() = default;

    void CreateWinow(int width, int height, const char* name)
    {
        window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }

    void SetupInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequieredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    void SetupDebugMessenger();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    GLFWwindow* GetWindow() { return window; }
    VkInstance& GetInstance() { return instance; }
    VkResult& GetResult() { return result; }
    std::vector<const char*>& GetValidationLayers() { return validationLayers; }

private:

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GLFWwindow* window;

    VkInstance instance;
    VkResult result;
    VkDebugUtilsMessengerEXT debugMessenger;
};