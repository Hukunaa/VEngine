#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Initializers.h>
#include <Tools.h>

#include <cstdint>
#include <optional>
#include <glm/glm.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <map>

struct VkGeometryInstance 
{
    float transform[12];
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

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

struct RTAccelerationStructure 
{
    VkDeviceMemory              memory;
    VkAccelerationStructureNV  accelerationStructure;
    uint64_t                    handle;
    VkAccelerationStructureInfoNV accInfo;
};

struct UniformData
{
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 nrm;
    glm::vec3 color;
    glm::vec2 texCoord;
};

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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
    void SelectGPU();
    void createLogicalDevice();
    void createQueues();
    void createSwapChain();
    void createImageViews();
    void createGraphicPipeline();
    void createRenderpass();
    void createFramebuffers();
    void createCommandpool();
    void createCommandbuffers();
    void createSemaphores();
    void drawFrame();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void CleanUp();

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    VkInstance& GetInstance() { return instance; }
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkCommandBuffer beginSingleTimeCommands();
    static uint32_t GetMemoryType(VkMemoryRequirements& memoryRequiriments, VkMemoryPropertyFlags memoryProperties, Device* device);
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);


    bool CheckValidationLayerSupport();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    std::vector<const char*> GetRequieredExtensions();
    std::vector<const char*>& GetValidationLayers() { return validationLayers; }


    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static std::vector<char> readFile(const std::string& filename);


    GLFWwindow* GetWindow() { return window; }

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        VK_NV_RAY_TRACING_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME};


    bool CreateAS(const VkAccelerationStructureTypeNV type,
        const uint32_t 
        
        
        Count,
        const VkGeometryNV* geometries,
        const uint32_t instanceCount,
        RTAccelerationStructure& _as);
    void FillCommandBuffer(VkCommandBuffer commandBuffer, const size_t imageIndex);

    void CreateScene();

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    Device device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkViewport viewport = {};
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    VkCommandPool commandPool;
    
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    const float vertices[9] =
    {
        0.25f, 0.25f, 0.0f,
        0.75f, 0.25f, 0.0f,
        0.50f, 0.75f, 0.0f
    };

    const uint32_t indices[3] = { 0, 1, 2 };

    std::vector<RTAccelerationStructure>  bottomLevelAS;
    RTAccelerationStructure topLevelAS;

    VkDescriptorSetLayout   mRTDescriptorSetLayout;
    VkPipelineLayout        mRTPipelineLayout;
    VkPipeline              mRTPipeline;
    VkDescriptorPool        mRTDescriptorPool;
    VkDescriptorSet         mRTDescriptorSet;

    VBuffer::Buffer mShaderBindingTable;
};
