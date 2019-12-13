#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Initializers.h>
#include <Tools.h>
#include <Device.h>
#include <Camera.h>

#include <cstdint>
#include <optional>
#include <glm/glm.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <map>

struct UniformData 
{
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
};

struct SwapChainBuffer
{
    VkImage image;
    VkImageView view;
};

struct SwapChain
{
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    /** @brief Handle to the current swap chain, required for recreation */
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<SwapChainBuffer> buffers;
    /** @brief Queue family index of the detected graphics and presenting device queue */
    uint32_t queueNodeIndex = 5000000;
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

struct AccelerationStructure 
{
    VkDeviceMemory memory;
    VkAccelerationStructureNV accelerationStructure;
    uint64_t handle;
};

struct RTAccelerationStructure 
{
    VkDeviceMemory              memory;
    VkAccelerationStructureNV  accelerationStructure;
    uint64_t                    handle;
    VkAccelerationStructureInfoNV accInfo;
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

struct StorageImage
{
    VkDeviceMemory memory;
    VkImage image;
    VkImageView view;
    VkFormat format;
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
    void initSwapChain();
    void createImageViews();
    void createRenderpass();
    void createFramebuffers();
    void createCommandpool();
    //void createGraphicPipeline();
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
    VkShaderModule createShaderModule(const std::vector<char>& code);
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


    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VDevice::Device device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkCommandPool commandPool;
    
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    AccelerationStructure bottomLevelAS;
    AccelerationStructure topLevelAS;

    SwapChain swapChain;
    VkRenderPass renderPass;
    VkPipeline Rpipeline;
    VkPipelineLayout RpipelineLayout;
    VkDescriptorSet RdescriptorSet;
    VkDescriptorSetLayout RdescriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkShaderModule> shaderModules;

    uint32_t indexCount;

    VBuffer::Buffer mShaderBindingTable;
    VBuffer::Buffer vertexBuffer;
    VBuffer::Buffer indexBuffer;
    VBuffer::Buffer ubo;

    StorageImage storageImage;
    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};
    std::vector<VkFence> waitFences;
    VkFormat depthFormat;
    VkPipelineCache pipelineCache;
    UniformData uniformData;
    Camera camera;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depthStencil;

    void CHECK_ERROR(VkResult result);
    void setupSwapChain(uint32_t width, uint32_t height, bool vsync = false);
    VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
    VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
    VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
    VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);
    void CreateBLAS(const VkGeometryNV* geometries);
    void CreateTLAS();
    void CreateStorageImage();
    void createScene();
    void createRayTracingPipeline();
    void createSynchronizationPrimitives();
    void createPipelineCache();
    void setupFrameBuffer();
    void setupDepthstencil();
    void setupRenderPass();

    /**
    * Create a buffer on the device
    *
    * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex, uniform buffer)
    * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
    * @param buffer Pointer to a vk::Vulkan buffer object
    * @param size Size of the buffer in byes
    * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
    *
    * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
    */
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VBuffer::Buffer* buffer, VkDeviceSize size, void* data = nullptr);

    void setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask);

    void setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask);

    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
    void createShaderBindingTable();
    VkDeviceSize copyShaderIdentifier(uint8_t* data, const uint8_t* shaderHandleStorage, uint32_t groupIndex);
    void createDescriptorSets();
    void createUniformBuffer();
    void updateUniformBuffers();

    void buildCommandbuffers();

    void setupRayTracingSupport();

    // Function pointers
   /* PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;*/

};
