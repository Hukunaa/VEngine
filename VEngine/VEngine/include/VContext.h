#pragma once

#include <accctrl.h>
#include <cstdint>
#include <optional>
#include <vector>

#include <windows.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <VCamera.h>
#include <VDevice.h>
#include <VInitializers.h>
#include <VTools.h>
#include <VObject.h>

//#include <vulkan/vulkan.h>
//#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include <cuda_runtime.h>
#include <cuda.h>
#include <optix.h>
#include <optix_stubs.h>
#include <optix_types.h>
#include <basics.h>

#include <nvvkpp/allocator_dedicated_vkpp.hpp>
#include <nvvkpp/allocator_dma_vkpp.hpp>
#include <nvvkpp/images_vkpp.hpp>
#include <nvvkpp/commands_vkpp.hpp>
#include <nvvkpp/utilities_vkpp.hpp>
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
    glm::vec4 data;
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

using nvvkTexture = nvvkpp::TextureDma;
using nvvkBuffer = nvvkpp::BufferDedicated;

// Holding the Buffer for Cuda interop
struct BufferCuda
{
    nvvkBuffer bufVk;  // The Vulkan allocated buffer

    // Extra for Cuda
    HANDLE handle = nullptr;  // The Win32 handle
    void* cudaPtr = nullptr;

    void destroy(nvvkpp::AllocatorVkExport& alloc)
    {
        alloc.destroy(bufVk);
        CloseHandle(handle);
    }
};
#pragma endregion

class VContext
{
public:

    VContext() = default;
    ~VContext() = default;

#pragma region Void Methods
    void CREATETHEFUCKINGWINDOW(int width, int height, const char* name);
    void SetupInstance();
    void SelectGPU();
    void createLogicalDevice();
    void initSwapChain();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void setupSwapChain(uint32_t width, uint32_t height, bool vsync = false);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true) const;
    void CreateBottomLevelAccelerationStructure(const VkGeometryNV* geometries);
    void CreateTopLevelAccelerationStructure(AccelerationStructure& accelerationStruct, int instanceCount) const;
    void CreateStorageImage();
    void createScene(std::vector<VObject>& objects);
    void createRayTracingPipeline();
    void createSynchronizationPrimitives();
    void createPipelineCache();
    void setupFrameBuffer();
    void setupDepthstencil();
    void setupRenderPass();
    void createShaderBindingTable();
    void createDescriptorSets();
    void createUniformBuffer();
    void updateUniformBuffers(bool updateAcc);
    void buildCommandbuffers();
    void setupRayTracingSupport(std::vector<VObject>& objects, std::vector<int>& trianglesNumber);
    void prepareFrame();
    void submitFrame() const;
    void draw();
    void SetupDebugMessenger();
    void CleanUp();
    void UpdateObjects(std::vector<VObject>& objects);
    
    void InitOptix();
    void AllocateBuffers();
    void ConvertVulkan2Optix(VkImage& vkIMG, vk::Buffer& pixelBuffer);
    void ConvertOptix2Vulkan(VkImage& vkIMG, vk::Buffer& pixelBuffer);
    void DenoiseImage();
    HANDLE getVulkanMemoryHandle(VkDevice device, VkDeviceMemory memory);
    void exportToCudaPointer(BufferCuda& buf);
    void CreateImageBuffer(vk::Buffer& buf, vk::MemoryAllocateInfo& bufAlloc, vk::MemoryRequirements& bufReqs, vk::DeviceMemory& bufMemory, vk::BufferUsageFlags usage);
    
    
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
    bool CheckValidationLayerSupport() const;
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

    void UpdateTime(std::vector<float>& time)
    {
        if(time[0] > 1)
            time[0] = 0.001f;

        TimeBuffer.map();
        memcpy(TimeBuffer.mapped, time.data(), time.size() * sizeof(float));
        TimeBuffer.unmap();
    }

#pragma endregion
#pragma region VkResult Methods
    VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
    VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) const;
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr) const;
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VBuffer::Buffer* buffer, VkDeviceSize size, void* data = nullptr) const;
#pragma endregion

#pragma region Getter Setters
    static std::vector<const char*> GetRequieredExtensions();
    const std::vector<const char*>& GetValidationLayers() const
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
    const std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_NV_RAY_TRACING_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
    };

    //GLFW
    GLFWwindow* window{};
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};

    VDevice::Device device;

    QueueFamilyIndices queueFamily;
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};



    //Commands
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;

    //Semaphores
    VkSemaphore imageAvailableSemaphore{};
    VkSemaphore renderFinishedSemaphore{};

    //AccelerationStructure
    std::vector<AccelerationStructure> bottomLevelAS{};
    AccelerationStructure topLevelAS{};

    //SwapChain
    SwapChain swapChain;
    uint32_t minImageCount;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //RenderPass
    VkRenderPass renderPass{};
    VkRenderPass imGuiRenderPass{};

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
    VBuffer::Buffer matBuffer;
    VBuffer::Buffer vertBuffer;
    VBuffer::Buffer NumberOfTriangles;
    VBuffer::Buffer TimeBuffer;

    StorageImage storageImage{};
    StorageImage accImage{};
    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};
    std::vector<VkFence> waitFences;
    VkFormat depthFormat;
    VkPipelineCache pipelineCache{};
    UniformData uniformData{};

    uint32_t currentBuffer = 0;
    Semaphore semaphores{};
    VkSubmitInfo submitInfo{};
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkMemoryRequirements2 memReqBottomLevelAS;
    Camera camera;

    std::vector<float> bufferVertices;
    std::vector<uint32_t> sceneIndices;
    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depthStencil{};
    HANDLE fd;
    std::vector<float> t{};


    OptixDeviceContext m_optixDevice;
    OptixDenoiser        m_denoiser{ nullptr };
    OptixDenoiserOptions m_dOptions{};
    OptixDenoiserSizes   m_dSizes{};
    CUdeviceptr          m_dState{ 0 };
    CUdeviceptr          m_dScratch{ 0 };
    CUdeviceptr          m_dIntensity{ 0 };
    CUdeviceptr          m_dMinRGB{ 0 };
    cudaStream_t cudaStream;
    nvvkpp::AllocatorVkExport m_alloc;

    BufferCuda   m_pixelBufferIn;
    BufferCuda   m_pixelBufferOut;

    vk::Device dev;

    vk::Buffer pixelBufferOut;
    vk::MemoryAllocateInfo memAllocPixelBufferOut;
    vk::MemoryRequirements memReqsPixelBufferOut;
    vk::DeviceMemory memoryPixelBufferOut;

    vk::Buffer pixelBufferIn;
    vk::MemoryAllocateInfo memAllocPixelBufferIn;
    vk::MemoryRequirements memReqsPixelBufferIn;
    vk::DeviceMemory memoryPixelBufferIn;

#pragma endregion
};
