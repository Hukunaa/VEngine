#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Camera.h>
#include <Device.h>
#include <Initializers.h>
#include <Tools.h>

#pragma region Structures
struct Semaphore {
    // Swap chain image presentation
    VkSemaphore presentComplete;
    // Command buffer submission and execution
    VkSemaphore renderComplete;
};

struct UniformData {
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
};

struct SwapChainBuffer {
    VkImage image;
    VkImageView view;
};

struct SwapChain {
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    /** @brief Handle to the current swap chain, required for recreation */
    VkSwapchainKHR swapChain = nullptr;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<SwapChainBuffer> buffers;
    /** @brief Queue family index of the detected graphics and presenting device queue */
    uint32_t queueNodeIndex = 5000000;
};

struct GeometryInstance {
    glm::mat3x4 transform;
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

struct AccelerationStructure {
    VkDeviceMemory memory;
    VkAccelerationStructureNV accelerationStructure;
    uint64_t handle;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct StorageImage {
    VkDeviceMemory memory;
    VkImage image;
    VkImageView view;
    VkFormat format;
};
#pragma endregion

class VContext
{
public:

    VContext() = default;
    ~VContext() = default;

#pragma region Void Methods
    void CreateWindow(int width, int height, const char* name);
    void SetupInstance();
    void SelectGPU();
    void createLogicalDevice();
    void initSwapChain();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void setupSwapChain(uint32_t width, uint32_t height, bool vsync = false);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true) const;
    void CreateBottomLevelAccelerationStructure(const VkGeometryNV* geometries);
    void CreateTopLevelAccelerationStructure();
    void CreateStorageImage();
    void createScene();
    void createRayTracingPipeline();
    void createSynchronizationPrimitives();
    void createPipelineCache();
    void setupFrameBuffer();
    void setupDepthstencil();
    void setupRenderPass();
    void createShaderBindingTable();
    void createDescriptorSets();
    void createUniformBuffer();
    void updateUniformBuffers();
    void buildCommandbuffers();
    void setupRayTracingSupport();
    void prepareFrame();
    void submitFrame() const;
    void draw();
    void SetupDebugMessenger();
    void CleanUp();

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    static void CHECK_ERROR(VkResult result);

    static void setImageLayout(
        VkCommandBuffer cmd_buffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask);

    static void setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask);
#pragma endregion
#pragma region Methods
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static std::vector<char> readFile(const std::string& filename);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    VkShaderModule createShaderModule(const std::vector<char>& code) const;
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin) const;

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
    VkPipelineShaderStageCreateInfo loadShader(std::string file_name, VkShaderStageFlagBits stage);
    static VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);
    VkDeviceSize copyShaderIdentifier(uint8_t* data, const uint8_t* shaderHandleStorage, uint32_t groupIndex) const;
    bool CheckValidationLayerSupport();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

#pragma endregion
#pragma region VkResult Methods
    VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
    VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) const;
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr) const;
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VBuffer::Buffer* buffer, VkDeviceSize size, void* data = nullptr) const;
#pragma endregion 
#pragma region Getter Setters
    static std::vector<const char*> GetRequieredExtensions();
    const std::vector<const char*>& GetValidationLayers()
    {
        return validationLayers;
    }
    VkInstance& GetInstance()
    {
        return instance;
    }


    [[nodiscard]] GLFWwindow* GetWindow() const
    {
        return window;
    }

#pragma endregion
#pragma region Variables
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_NV_RAY_TRACING_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
    };

    //GLFW
    GLFWwindow* window{};
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};

    VDevice::Device device;

    VkQueue graphicsQueue{};
    VkQueue presentQueue{};



    //Commands
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;

    //Semaphores
    VkSemaphore imageAvailableSemaphore{};
    VkSemaphore renderFinishedSemaphore{};

    //AccelerationStructure
    AccelerationStructure bottomLevelAS{};
    AccelerationStructure topLevelAS{};

    //SwapChain
    SwapChain swapChain;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //RenderPass
    VkRenderPass renderPass{};

    //Rendering Pipeline
    VkPipeline Rpipeline{};
    VkPipelineLayout RpipelineLayout{};

    //Descriptor Sets
    VkDescriptorSet RdescriptorSet{};
    VkDescriptorSetLayout RdescriptorSetLayout{};

    VkDescriptorPool descriptorPool{};
    std::vector<VkShaderModule> shaderModules;

    uint32_t indexCount{};

    VBuffer::Buffer mShaderBindingTable;
    VBuffer::Buffer vertexBuffer;
    VBuffer::Buffer indexBuffer;
    VBuffer::Buffer ubo;

    StorageImage storageImage{};
    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};
    std::vector<VkFence> waitFences;
    VkFormat depthFormat;
    VkPipelineCache pipelineCache{};
    UniformData uniformData{};

    uint32_t currentBuffer = 0;
    Semaphore semaphores{};
    VkSubmitInfo submitInfo{};
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Camera camera;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depthStencil{};
#pragma endregion
};
