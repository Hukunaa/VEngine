#include <Context.h>

#include <algorithm>
#include <array>
#include <set>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


#define WIDTH 1280
#define HEIGHT 720

#define INDEX_RAYGEN 0
#define INDEX_MISS 1
#define INDEX_SHADOWMISS 2
#define INDEX_SHADOWHIT 4
#define INDEX_CLOSEST_HIT 3

#define NUM_SHADER_GROUPS 5

#pragma region Queues
QueueFamilyIndices VContext::FindQueueFamilies(VkPhysicalDevice p_device)
{
    /**
    A queue family just describes a set of queues with identical properties. 
    The device supports three kinds of queues:

    One kind can do graphics, compute, transfer, and sparse binding operations
    */

    QueueFamilyIndices indices;

    //Get QueueFamily size from the GPU
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queueFamilyCount, nullptr);

    //Get actual info of the GPU's QueueFamily
    device.queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queueFamilyCount, device.queueFamilyProperties.data());

    int i = 0;
    for (const auto& queueFamily : device.queueFamilyProperties)
    {
        //Finding support for image rendering (presentation)
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(p_device, i, device.surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        //GraphicFamily and Present Family might very likely be the same id, but for support compatibility purpose,
        //we separate them into 2 queue Famlilies

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }
    return indices;
}
#pragma endregion
#pragma region GPU Selection
bool VContext::IsDeviceSuitable(VkPhysicalDevice p_device)
{
    vkGetPhysicalDeviceMemoryProperties(p_device, &device.memoryProperties);

    const QueueFamilyIndices indices = FindQueueFamilies(p_device);

    const bool extensionsSupported = checkDeviceExtensionSupport(p_device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(p_device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    else
    {
        std::cout << "Extension isn't supported by the GPU!\n";
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VContext::checkDeviceExtensionSupport(VkPhysicalDevice device) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
/*void VContext::UpdateMesh(VMesh& p_mesh)
{
    //Set new build info for Acceleration Structure
    AccelerationStructure newToplevelAcc;
    CreateTopLevelAccelerationStructure(newToplevelAcc);

    VkAccelerationStructureInfoNV buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
    buildInfo.pGeometries = nullptr;
    buildInfo.geometryCount = 0;
    buildInfo.instanceCount = 1;

    //p_mesh.UpdateTransform();

    createBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &p_mesh.meshBuffer,
        sizeof(GeometryInstance),
        &p_mesh.meshGeometry);

    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
    memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;

    VkMemoryRequirements2 memReqTopLevelAS;
    memoryRequirementsInfo.accelerationStructure = newToplevelAcc.accelerationStructure;
    vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo, &memReqTopLevelAS);

    const VkDeviceSize scratchBufferSize = std::max( memReqBottomLevelAS.memoryRequirements.size, memReqTopLevelAS.memoryRequirements.size);

    VBuffer::Buffer scratchBuffer;
    createBuffer(
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &scratchBuffer,
        scratchBufferSize);

    VkCommandBuffer cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    vkCmdBuildAccelerationStructureNV(
    cmdBuffer,
    &buildInfo,
    p_mesh.meshBuffer.buffer,
    0,
    VK_FALSE,
    newToplevelAcc.accelerationStructure,
    nullptr,
    scratchBuffer.buffer,
    0);

    VkMemoryBarrier memoryBarrier = Initializers::memoryBarrier();
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    vkCmdPipelineBarrier(cmdBuffer, 
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 
        0, 
        1, 
        &memoryBarrier, 
        0, 
        nullptr, 
        0, 
        nullptr);

    vkCmdCopyAccelerationStructureNV(cmdBuffer, topLevelAS.accelerationStructure, 
                                            newToplevelAcc.accelerationStructure, 
                                            VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    flushCommandBuffer(cmdBuffer, graphicsQueue, true);
    vkFreeMemory(device.logicalDevice, newToplevelAcc.memory, nullptr);
    vkDestroyAccelerationStructureNV(device.logicalDevice, newToplevelAcc.accelerationStructure, nullptr);
    scratchBuffer.destroy();
    p_mesh.meshBuffer.destroy();
    

}*/
void VContext::SelectGPU()
{
    device.physicalDevice = nullptr;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("No GPU support found For the Renderer");

    std::vector<VkPhysicalDevice> GPUs(deviceCount);

    vkEnumeratePhysicalDevices(instance, &deviceCount, GPUs.data());

    std::cout << "GPUs available: \n";
    for (size_t i = 0; i < GPUs.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(GPUs[i], &device.properties);

        std::cout << i << ": " << device.properties.deviceName << '\n';
    }
    int id;
    while (true)
    {
        std::cin >> id;
        if (IsDeviceSuitable(GPUs[id]))
        {
            device.physicalDevice = GPUs[id];
            vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &device.memoryProperties);
            break;
        }
        std::cout << "Device is not suitable for the Renderer! \n";
    }
    vkGetPhysicalDeviceProperties(device.physicalDevice, &device.properties);

    std::cout << "Selected GPU: " << device.properties.deviceName << '\n';

    if (device.physicalDevice == nullptr)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}
#pragma endregion
#pragma region Instance
void VContext::CreateWindow(int width, int height, const char* name)
{
     window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}
void VContext::SetupInstance()
{
    if (!CheckValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "VRenderer";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequieredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
    const VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &device.surface);

    if (err)
        throw std::runtime_error("failed to create surface!");
}
void VContext::createLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(device.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &device.enabledFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(device.physicalDevice, &createInfo, nullptr, &device.logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device.logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device.logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

SwapChainSupportDetails VContext::querySwapChainSupport(VkPhysicalDevice p_device) const
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device, device.surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, device.surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, device.surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, device.surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, device.surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void VContext::CHECK_ERROR(VkResult result)
{
    if (result != VK_SUCCESS)
        throw std::runtime_error(("Error with Vulkan function"));
}

void VContext::setupSwapChain(uint32_t width, uint32_t height, bool vsync)
{
    if (!getSupportedDepthFormat(device.physicalDevice, &depthFormat))
        throw std::runtime_error("can't find suitable format");

    VkSwapchainKHR oldSwapchain = swapChain.swapChain;

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surfCaps;
    CHECK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, device.surface, &surfCaps));

    // Get available present modes
    uint32_t presentModeCount;
    CHECK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, device.surface, &presentModeCount, nullptr));
    assert(presentModeCount > 0);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    CHECK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, device.surface, &presentModeCount, presentModes.data()));

    VkExtent2D swapchain_extent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (surfCaps.currentExtent.width == static_cast<uint32_t>(-1))
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchain_extent.width = width;
        swapchain_extent.height = height;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchain_extent = surfCaps.currentExtent;
        width = surfCaps.currentExtent.width;
        height = surfCaps.currentExtent.height;
    }


    // Select a present mode for the swapchain

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!vsync)
    {
        for (size_t i = 0; i < presentModeCount; i++)
        {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if ((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    // Determine the number of images
    uint32_t desired_swapchain_images = surfCaps.minImageCount + 1;
    if ((surfCaps.maxImageCount > 0) && (desired_swapchain_images > surfCaps.maxImageCount))
    {
        desired_swapchain_images = surfCaps.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = surfCaps.currentTransform;
    }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags)
    {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
        {
            compositeAlpha = compositeAlphaFlag;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.pNext = nullptr;
    swapchain_ci.surface = device.surface;
    swapchain_ci.minImageCount = desired_swapchain_images;
    swapchain_ci.imageFormat = swapChain.colorFormat;
    swapchain_ci.imageColorSpace = swapChain.colorSpace;
    swapchain_ci.imageExtent = { swapchain_extent.width, swapchain_extent.height };
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;
    swapchain_ci.pQueueFamilyIndices = nullptr;
    swapchain_ci.presentMode = swapchain_present_mode;
    swapchain_ci.oldSwapchain = oldSwapchain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.compositeAlpha = compositeAlpha;

    // Enable transfer source on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    CHECK_ERROR(vkCreateSwapchainKHR(device.logicalDevice, &swapchain_ci, nullptr, &swapChain.swapChain));

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (oldSwapchain != nullptr)
    {
        for (uint32_t i = 0; i < swapChain.imageCount; i++)
        {
            vkDestroyImageView(device.logicalDevice, swapChain.buffers[i].view, nullptr);
        }
        vkDestroySwapchainKHR(device.logicalDevice, oldSwapchain, nullptr);
    }
    CHECK_ERROR(vkGetSwapchainImagesKHR(device.logicalDevice, swapChain.swapChain, &swapChain.imageCount, nullptr));

    // Get the swap chain images
    swapChain.images.resize(swapChain.imageCount);
    CHECK_ERROR(vkGetSwapchainImagesKHR(device.logicalDevice, swapChain.swapChain, &swapChain.imageCount, swapChain.images.data()));

    // Get the swap chain buffers containing the image and imageview
    swapChain.buffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < swapChain.imageCount; i++)
    {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = nullptr;
        colorAttachmentView.format = swapChain.colorFormat;
        colorAttachmentView.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.baseMipLevel = 0;
        colorAttachmentView.subresourceRange.levelCount = 1;
        colorAttachmentView.subresourceRange.baseArrayLayer = 0;
        colorAttachmentView.subresourceRange.layerCount = 1;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0;

        swapChain.buffers[i].image = swapChain.images[i];

        colorAttachmentView.image = swapChain.buffers[i].image;

        vkCreateImageView(device.logicalDevice, &colorAttachmentView, nullptr, &swapChain.buffers[i].view);
    }
}

uint32_t VContext::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
    for (uint32_t i = 0; i < device.memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((device.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    }
    throw std::runtime_error("Could not find a matching memory type");
}

VkCommandBuffer VContext::createCommandBuffer(VkCommandBufferLevel level, bool begin) const
{
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = Initializers::commandBufferAllocateInfo(commandPool, level, 1);

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device.logicalDevice, &cmdBufAllocateInfo, &cmdBuffer);

    // If requested, also start recording for the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = Initializers::commandBufferBeginInfo();
        vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo);
    }

    return cmdBuffer;
}

bool VContext::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}
void VContext::CleanUp()
{
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroyCommandPool(device.logicalDevice, commandPool, nullptr);

    if (swapChain.swapChain != nullptr)
    {
        for (uint32_t i = 0; i < swapChain.imageCount; i++)
        {
            vkDestroyImageView(device.logicalDevice, swapChain.buffers[i].view, nullptr);
        }
    }
    if (device.surface != nullptr)
    {
        vkDestroySwapchainKHR(device.logicalDevice, swapChain.swapChain, nullptr);
        vkDestroySurfaceKHR(instance, device.surface, nullptr);
    }
    device.surface = nullptr;
    swapChain.swapChain = nullptr;

    for (auto frame_buffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device.logicalDevice, frame_buffer, nullptr);
    }

    vkDestroyPipeline(device.logicalDevice, Rpipeline, nullptr);
    vkDestroyPipelineLayout(device.logicalDevice, RpipelineLayout, nullptr);
    vkDestroyRenderPass(device.logicalDevice, renderPass, nullptr);

    vkDestroySwapchainKHR(device.logicalDevice, swapChain.swapChain, nullptr);
    vkDestroyDevice(device.logicalDevice, nullptr);

    vkDestroySurfaceKHR(GetInstance(), device.surface, nullptr);
    vkDestroyInstance(GetInstance(), nullptr);

    glfwDestroyWindow(GetWindow());
    glfwTerminate();
}
void VContext::UpdateObjects(std::vector<VObject>& objects)
{
    VkCommandBuffer cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    //Generate TLAS
    std::vector<GeometryInstance> instances;
    for(auto& obj: objects)
        instances.push_back(obj.m_mesh.meshGeometry);

    //Get all instances and create a buffer with all of them
    VBuffer::Buffer instanceBuffer;
    createBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &instanceBuffer,
        sizeof(GeometryInstance) * instances.size(),
        instances.data());

    //Generate TLAS
    AccelerationStructure newDataAS;
    CreateTopLevelAccelerationStructure(newDataAS, instances.size());

    //Get memory requirements
    VkMemoryRequirements2 memReqTopLevelAS;
    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo2{};
    memoryRequirementsInfo2.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    memoryRequirementsInfo2.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    memoryRequirementsInfo2.accelerationStructure = newDataAS.accelerationStructure;
    vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo2, &memReqTopLevelAS);

    const VkDeviceSize scratchBufferSize = memReqTopLevelAS.memoryRequirements.size;

    //Generate Scratch buffer
    VBuffer::Buffer scratchBuffer;
    createBuffer(
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &scratchBuffer,
        scratchBufferSize);

    //Generate build info for TLAS
    VkAccelerationStructureInfoNV buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
    buildInfo.pGeometries = nullptr;
    buildInfo.geometryCount = 0;
    buildInfo.instanceCount = instances.size();

    //Build Actual TLAS
    vkCmdBuildAccelerationStructureNV(
        cmdBuffer,
        &buildInfo,
        instanceBuffer.buffer,
        0,
        VK_FALSE,
        newDataAS.accelerationStructure,
        nullptr,
        scratchBuffer.buffer,
        0);

    VkMemoryBarrier memoryBarrier = Initializers::memoryBarrier();
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    vkCmdCopyAccelerationStructureNV(cmdBuffer, topLevelAS.accelerationStructure, newDataAS.accelerationStructure, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    flushCommandBuffer(cmdBuffer, graphicsQueue, true);
    vkDestroyAccelerationStructureNV(device.logicalDevice, newDataAS.accelerationStructure, nullptr);
    vkFreeMemory(device.logicalDevice, newDataAS.memory, nullptr);

    instances.clear();
    scratchBuffer.destroy();
    instanceBuffer.destroy();
}
#pragma endregion
#pragma region Extensions
std::vector<const char*> VContext::GetRequieredExtensions()
{
    uint32_t extension_count = 0;
    const char** extensions_name = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char*> extensions(extensions_name, extensions_name + extension_count);
    std::cout << "Extensions Loaded: \n";

    for (const char* typo : extensions)
        std::cout << typo << "\n";

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);


    return extensions;
}
#pragma endregion
#pragma region Debug
void VContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VContext::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

std::vector<char> VContext::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void VContext::SetupDebugMessenger()
{
    if constexpr (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    const auto func = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}



VkResult VContext::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    const auto func = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}
#pragma endregion
#pragma region RayTracing

/**
    * Acquires the next image in the swap chain
    *
    * @param presentCompleteSemaphore (Optional) Semaphore that is signaled when the image is ready for use
    * @param imageIndex Pointer to the image index that will be increased if the next image could be acquired
    *
    * @note The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX
    *
    * @return VkResult of the image acquisition
    */
VkResult VContext::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const
{
    // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
    // With that we don't have to handle VK_NOT_READY
    return vkAcquireNextImageKHR(device.logicalDevice, swapChain.swapChain, UINT64_MAX, presentCompleteSemaphore, static_cast<VkFence>(nullptr), imageIndex);
}

/**
* Queue an image for presentation
*
* @param queue Presentation queue for presenting the image
* @param imageIndex Index of the swapchain image to queue for presentation
* @param waitSemaphore (Optional) Semaphore that is waited on before the image is presented (only used if != VK_NULL_HANDLE)
*
* @return VkResult of the queue presentation
*/
VkResult VContext::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = nullptr) const
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain;
    presentInfo.pImageIndices = &imageIndex;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (waitSemaphore != nullptr)
    {
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &presentInfo);
}

VkShaderModule VContext::createShaderModule(const std::vector<char>& code) const
{
    //Set the shader
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    //Create the shader
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VContext::initSwapChain()
{
    // Get available queue family properties
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device.physicalDevice, &queueCount, nullptr);
    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device.physicalDevice, &queueCount, queueProps.data());

    // Iterate over each queue to learn whether it supports presenting:
    // Find a queue with present support
    // Will be used to present the swap chain images to the windowing system
    std::vector<VkBool32> supportsPresent(queueCount);
    for (uint32_t i = 0; i < queueCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(device.physicalDevice, i, device.surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueCount; i++)
    {
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (graphicsQueueNodeIndex == UINT32_MAX)
            {
                graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE)
            {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX)
    {
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < queueCount; ++i)
        {
            if (supportsPresent[i] == VK_TRUE)
            {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
    {
        throw std::runtime_error("Could not find a graphics and/or presenting queue!");
    }

    // todo : Add support for separate graphics and presenting queue
    if (graphicsQueueNodeIndex != presentQueueNodeIndex)
    {
        throw std::runtime_error("Separate graphics and presenting queues are not supported yet!");
    }

    swapChain.queueNodeIndex = graphicsQueueNodeIndex;

    // Get list of supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, device.surface, &formatCount, nullptr);
    assert(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, device.surface, &formatCount, surfaceFormats.data());

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        swapChain.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swapChain.colorSpace = surfaceFormats[0].colorSpace;
    }
    else
    {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool found_B8G8R8A8_UNORM = false;
        for (auto&& surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                swapChain.colorFormat = surfaceFormat.format;
                swapChain.colorSpace = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!found_B8G8R8A8_UNORM)
        {
            swapChain.colorFormat = surfaceFormats[0].format;
            swapChain.colorSpace = surfaceFormats[0].colorSpace;
        }
    }
}

void VContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CHECK_ERROR(vkCreateCommandPool(device.logicalDevice, &cmdPoolInfo, nullptr, &commandPool));
}

void VContext::CreateCommandBuffers()
{
    // Create one command buffer for each swap chain image and reuse for rendering
    commandBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        Initializers::commandBufferAllocateInfo(
            commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(commandBuffers.size()));

    CHECK_ERROR(vkAllocateCommandBuffers(device.logicalDevice, &cmdBufAllocateInfo, commandBuffers.data()));
}

void VContext::CreateBottomLevelAccelerationStructure(const VkGeometryNV* geometries)
{
    AccelerationStructure newBottomAS;
    VkAccelerationStructureInfoNV accelerationStructureInfo{};
    accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    accelerationStructureInfo.instanceCount = 0;
    accelerationStructureInfo.geometryCount = 1;
    accelerationStructureInfo.pGeometries = geometries;
    
    VkAccelerationStructureCreateInfoNV accelerationStructureCI{};
    accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    accelerationStructureCI.info = accelerationStructureInfo;

    vkCreateAccelerationStructureNV(device.logicalDevice, &accelerationStructureCI, nullptr, &newBottomAS.accelerationStructure);

    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
    memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    memoryRequirementsInfo.accelerationStructure = newBottomAS.accelerationStructure;

    VkMemoryRequirements2 memoryRequirements2{};
    vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo, &memoryRequirements2);

    VkMemoryAllocateInfo memoryAllocateInfo = Initializers::memoryAllocateInfo();
    memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &newBottomAS.memory);

    VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo{};
    accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
    accelerationStructureMemoryInfo.accelerationStructure = newBottomAS.accelerationStructure;
    accelerationStructureMemoryInfo.memory = newBottomAS.memory;
    vkBindAccelerationStructureMemoryNV(device.logicalDevice, 1, &accelerationStructureMemoryInfo);

    vkGetAccelerationStructureHandleNV(device.logicalDevice, newBottomAS.accelerationStructure, sizeof(uint64_t), &newBottomAS.handle);
    bottomLevelAS.push_back(newBottomAS);
}
//VALID
void VContext::CreateTopLevelAccelerationStructure(AccelerationStructure& accelerationStruct, int instanceCount)
{
    VkAccelerationStructureInfoNV accelerationStructureInfo{};
    accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
    accelerationStructureInfo.instanceCount = instanceCount;
    accelerationStructureInfo.geometryCount = 0;

    VkAccelerationStructureCreateInfoNV accelerationStructureCI{};
    accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    accelerationStructureCI.info = accelerationStructureInfo;
    vkCreateAccelerationStructureNV(device.logicalDevice, &accelerationStructureCI, nullptr, &accelerationStruct.accelerationStructure);

    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
    memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    memoryRequirementsInfo.accelerationStructure = accelerationStruct.accelerationStructure;

    VkMemoryRequirements2 memoryRequirements2{};
    vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo, &memoryRequirements2);

    VkMemoryAllocateInfo memoryAllocateInfo = Initializers::memoryAllocateInfo();
    memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &accelerationStruct.memory);
    VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo{};
    accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
    accelerationStructureMemoryInfo.accelerationStructure = accelerationStruct.accelerationStructure;
    accelerationStructureMemoryInfo.memory = accelerationStruct.memory;
    vkBindAccelerationStructureMemoryNV(device.logicalDevice, 1, &accelerationStructureMemoryInfo);

    vkGetAccelerationStructureHandleNV(device.logicalDevice, accelerationStruct.accelerationStructure, sizeof(uint64_t), &accelerationStruct.handle);
}
//VALID
void VContext::CreateStorageImage()
{
    VkImageCreateInfo image = Initializers::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = swapChain.colorFormat;
    image.extent.width = WIDTH;
    image.extent.height = HEIGHT;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkCreateImage(device.logicalDevice, &image, nullptr, &storageImage.image);

    VkMemoryRequirements memory_requierements;
    vkGetImageMemoryRequirements(device.logicalDevice, storageImage.image, &memory_requierements);
    VkMemoryAllocateInfo memoryAllocateInfo = Initializers::memoryAllocateInfo();
    memoryAllocateInfo.allocationSize = memory_requierements.size;
    memoryAllocateInfo.memoryTypeIndex = getMemoryType(memory_requierements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &storageImage.memory);
    vkBindImageMemory(device.logicalDevice, storageImage.image, storageImage.memory, 0);

    VkImageViewCreateInfo colorImageView = Initializers::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = swapChain.colorFormat;
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = storageImage.image;
    vkCreateImageView(device.logicalDevice, &colorImageView, nullptr, &storageImage.view);

    const VkCommandBuffer cmd_buffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    setImageLayout(cmd_buffer, storageImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    flushCommandBuffer(cmd_buffer, graphicsQueue);
}
//VALID
VkResult VContext::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data) const
{
    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = Initializers::bufferCreateInfo(usageFlags, size);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, nullptr, buffer);

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memory_requirements;
    VkMemoryAllocateInfo memAlloc = Initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(device.logicalDevice, *buffer, &memory_requirements);
    memAlloc.allocationSize = memory_requirements.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = getMemoryType(memory_requirements.memoryTypeBits, memoryPropertyFlags);
    vkAllocateMemory(device.logicalDevice, &memAlloc, nullptr, memory);

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        void* mapped;
        vkMapMemory(device.logicalDevice, *memory, 0, size, 0, &mapped);
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            VkMappedMemoryRange mappedRange = Initializers::mappedMemoryRange();
            mappedRange.memory = *memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(device.logicalDevice, 1, &mappedRange);
        }
        vkUnmapMemory(device.logicalDevice, *memory);
    }

    // Attach the memory to the buffer object
    vkBindBufferMemory(device.logicalDevice, *buffer, *memory, 0);

    return VK_SUCCESS;
}
VkBool32 VContext::getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthFormat = format;
            return true;
        }
    }

    return false;
}
void VContext::createScene(std::vector<VObject>& objects)
{
    int j = 0;
    VkCommandBuffer cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    for(auto obj : objects)
    {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for(auto vertex : obj.m_mesh.GetVertices())
        {
            vertices.push_back(vertex);
        }

        for(auto index : obj.m_mesh.GetIndices())
        {
            indices.push_back(index);
            sceneIndices.push_back(index);
        }

        indexCount = static_cast<uint32_t>(indices.size());

        //Vertex buffer
        createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &vertexBuffer,
            vertices.size() * sizeof(Vertex),
            vertices.data());

        // Index buffer
        createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &indexBuffer,
            indices.size() * sizeof(uint32_t),
            indices.data());

        //Generate Geometry data
        VkGeometryNV geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        geometry.geometry.triangles.vertexData = vertexBuffer.buffer;
        geometry.geometry.triangles.vertexOffset = 0;
        geometry.geometry.triangles.vertexCount = static_cast<uint32_t>(vertices.size());
        geometry.geometry.triangles.vertexStride = sizeof(Vertex);
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        geometry.geometry.triangles.indexData = indexBuffer.buffer;
        geometry.geometry.triangles.indexOffset = 0;
        geometry.geometry.triangles.indexCount = indexCount;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        geometry.geometry.triangles.transformData = nullptr;
        geometry.geometry.triangles.transformOffset = 0;
        geometry.geometry.aabbs = {};
        geometry.geometry.aabbs.sType = { VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV };
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

        //Create Bottom Level AS for specific geometry
        CreateBottomLevelAccelerationStructure(&geometry);

        //Get memory requirements for BLAS
         VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;

        memoryRequirementsInfo.accelerationStructure = bottomLevelAS[j].accelerationStructure;
        vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo, &memReqBottomLevelAS);
        const VkDeviceSize scratchBufferSize = memReqBottomLevelAS.memoryRequirements.size;

        //Create scratch buffer, because BLAS needs temp memory to be built
        VBuffer::Buffer scratchBuffer;
        createBuffer(
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &scratchBuffer,
        scratchBufferSize);

        //Set build info
        VkAccelerationStructureInfoNV buildInfo{};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        buildInfo.geometryCount = 1;
        buildInfo.pGeometries = &geometry;

        //Build BLAS for specific object
        vkCmdBuildAccelerationStructureNV(
            cmdBuffer,
            &buildInfo,
            nullptr,
            0,
            VK_FALSE,
            bottomLevelAS[j].accelerationStructure,
            nullptr,
            scratchBuffer.buffer,
            0);

        //Create memory barrier to prevent issues (it creates a command dependency)
        VkMemoryBarrier memoryBarrier = Initializers::memoryBarrier();
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

        //set correct object acceleration structure to the one we just built
        objects[j].m_mesh.meshGeometry.accelerationStructureHandle = bottomLevelAS[j].handle;
        objects[j].m_mesh.meshGeometry.instanceId = j;

        j++;
    }
    j = 0;

    //Generate TLAS
    std::vector<GeometryInstance> instances;
    for(auto& obj: objects)
        instances.push_back(obj.m_mesh.meshGeometry);

    //Get all instances and create a buffer with all of them
    VBuffer::Buffer instanceBuffer;
    createBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &instanceBuffer,
        sizeof(GeometryInstance) * instances.size(),
        instances.data());

    //Generate TLAS
    CreateTopLevelAccelerationStructure(topLevelAS, instances.size());

    //Get memory requirements
    VkMemoryRequirements2 memReqTopLevelAS;
    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo2{};
    memoryRequirementsInfo2.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    memoryRequirementsInfo2.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    memoryRequirementsInfo2.accelerationStructure = topLevelAS.accelerationStructure;
    vkGetAccelerationStructureMemoryRequirementsNV(device.logicalDevice, &memoryRequirementsInfo2, &memReqTopLevelAS);

    const VkDeviceSize scratchBufferSize = memReqTopLevelAS.memoryRequirements.size;

    //Generate Scratch buffer
    VBuffer::Buffer scratchBuffer;
    createBuffer(
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &scratchBuffer,
        scratchBufferSize);

    //Generate build info for TLAS
    VkAccelerationStructureInfoNV buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
    buildInfo.pGeometries = nullptr;
    buildInfo.geometryCount = 0;
    buildInfo.instanceCount = instances.size();

    //Build Actual TLAS
    vkCmdBuildAccelerationStructureNV(
        cmdBuffer,
        &buildInfo,
        instanceBuffer.buffer,
        0,
        VK_FALSE,
        topLevelAS.accelerationStructure,
        nullptr,
        scratchBuffer.buffer,
        0);

    VkMemoryBarrier memoryBarrier = Initializers::memoryBarrier();
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    flushCommandBuffer(cmdBuffer, graphicsQueue);

    scratchBuffer.destroy();
    instanceBuffer.destroy();
}



//VALID
void VContext::createRayTracingPipeline()
{
    //Binding Uniforms to specific shader,
    //here we set the bindings for the RAYGEN shader (VK_SHADER_STAGE_RAYGEN_BIT_NV)

    VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
    accelerationStructureLayoutBinding.binding = 0;
    accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    accelerationStructureLayoutBinding.descriptorCount = 1;
    accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

    VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
    resultImageLayoutBinding.binding = 1;
    resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    resultImageLayoutBinding.descriptorCount = 1;
    resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

    VkDescriptorSetLayoutBinding uniformBufferBinding{};
    uniformBufferBinding.binding = 2;
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV |  VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

    VkDescriptorSetLayoutBinding matBufferBinding{};
    matBufferBinding.binding = 3;
    matBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    matBufferBinding.descriptorCount = 1;
    matBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

    VkDescriptorSetLayoutBinding vertexBufferBinding{};
	vertexBufferBinding.binding = 4;
	vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferBinding.descriptorCount = 1;
	vertexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

    VkDescriptorSetLayoutBinding timeBufferBinding{};
	timeBufferBinding.binding = 5;
	timeBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	timeBufferBinding.descriptorCount = 1;
	timeBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

    VkDescriptorSetLayoutBinding TriNumberBinding{};
	TriNumberBinding.binding = 6;
	TriNumberBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	TriNumberBinding.descriptorCount = 1;
	TriNumberBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
    //create a Binding vector for Uniform bindings
    std::vector<VkDescriptorSetLayoutBinding> bindings({
        accelerationStructureLayoutBinding,
        resultImageLayoutBinding,
        uniformBufferBinding,
        matBufferBinding,
        vertexBufferBinding,
        timeBufferBinding,
        TriNumberBinding
    });

    //Create the buffer that will map the shader uniforms to the actual shader
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    vkCreateDescriptorSetLayout(device.logicalDevice, &layoutInfo, nullptr, &RdescriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &RdescriptorSetLayout;

    vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &RpipelineLayout);

    const uint32_t shader_index_ray = 0;
    const uint32_t shaderIndexMiss = 1;
    const uint32_t shaderIndexShadowMiss = 2;
    const uint32_t shaderIndexClosestHit = 3;
    std::array<VkPipelineShaderStageCreateInfo, 4> shaderStages{};
    shaderStages[shader_index_ray] = loadShader("shaders/bin/ray_gen.spv", VK_SHADER_STAGE_RAYGEN_BIT_NV);
    shaderStages[shaderIndexMiss] = loadShader("shaders/bin/ray_miss.spv", VK_SHADER_STAGE_MISS_BIT_NV);
    shaderStages[shaderIndexShadowMiss] = loadShader("shaders/bin/ray_smiss.spv", VK_SHADER_STAGE_MISS_BIT_NV);
    shaderStages[shaderIndexClosestHit] = loadShader("shaders/bin/ray_chit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    /*
        Setup ray tracing shader groups
    */

    std::array<VkRayTracingShaderGroupCreateInfoNV, NUM_SHADER_GROUPS> groups{};
    for (auto& group : groups)
    {
        // Init all groups with some default values
       		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
			group.generalShader = VK_SHADER_UNUSED_NV;
			group.closestHitShader = VK_SHADER_UNUSED_NV;
			group.anyHitShader = VK_SHADER_UNUSED_NV;
			group.intersectionShader = VK_SHADER_UNUSED_NV;
    }

    // Links shaders and types to ray tracing shader groups
    groups[INDEX_RAYGEN].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    groups[INDEX_RAYGEN].generalShader = shader_index_ray;

    groups[INDEX_MISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    groups[INDEX_MISS].generalShader = shaderIndexMiss;

    groups[INDEX_SHADOWMISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    groups[INDEX_SHADOWMISS].generalShader = shaderIndexShadowMiss;

    groups[INDEX_CLOSEST_HIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    groups[INDEX_CLOSEST_HIT].generalShader = VK_SHADER_UNUSED_NV;
    groups[INDEX_CLOSEST_HIT].closestHitShader = shaderIndexClosestHit;

    groups[INDEX_SHADOWHIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	groups[INDEX_SHADOWHIT].generalShader = VK_SHADER_UNUSED_NV;
		// Reuse shadow miss shader
	groups[INDEX_SHADOWHIT].closestHitShader = shaderIndexShadowMiss;

    VkRayTracingPipelineCreateInfoNV rayPipelineInfo{};
    rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    rayPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    rayPipelineInfo.pStages = shaderStages.data();
    rayPipelineInfo.groupCount = static_cast<uint32_t>(groups.size());
    rayPipelineInfo.pGroups = groups.data();
    rayPipelineInfo.maxRecursionDepth = 1;
    rayPipelineInfo.layout = RpipelineLayout;
    vkCreateRayTracingPipelinesNV(device.logicalDevice, nullptr, 1, &rayPipelineInfo, nullptr, &Rpipeline);
}

VkPipelineShaderStageCreateInfo VContext::loadShader(const std::string file_name, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
    shaderStage.module = Tools::loadShader(file_name.c_str(), device.logicalDevice);
#endif
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

void VContext::createSynchronizationPrimitives()
{
    // Wait fences to sync command buffer access
    VkFenceCreateInfo fenceCreateInfo = Initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    waitFences.resize(commandBuffers.size());
    for (auto& fence : waitFences)
    {
        vkCreateFence(device.logicalDevice, &fenceCreateInfo, nullptr, &fence);
    }
}

void VContext::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(device.logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
}

void VContext::setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = nullptr;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = WIDTH;
    frameBufferCreateInfo.height = HEIGHT;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    swapChainFramebuffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < swapChainFramebuffers.size(); i++)
    {
        attachments[0] = swapChain.buffers[i].view;
        CHECK_ERROR(vkCreateFramebuffer(device.logicalDevice, &frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]));
    }
}

void VContext::setupDepthstencil()
{
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthFormat;
    imageCI.extent = { WIDTH, HEIGHT, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    CHECK_ERROR(vkCreateImage(device.logicalDevice, &imageCI, nullptr, &depthStencil.image));
    VkMemoryRequirements memory_requierements{};
    vkGetImageMemoryRequirements(device.logicalDevice, depthStencil.image, &memory_requierements);

    VkMemoryAllocateInfo memory_alloc{};
    memory_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc.allocationSize = memory_requierements.size;
    memory_alloc.memoryTypeIndex = getMemoryType(memory_requierements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_ERROR(vkAllocateMemory(device.logicalDevice, &memory_alloc, nullptr, &depthStencil.mem));
    CHECK_ERROR(vkBindImageMemory(device.logicalDevice, depthStencil.image, depthStencil.mem, 0));

    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = depthStencil.image;
    imageViewCI.format = depthFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
    {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    CHECK_ERROR(vkCreateImageView(device.logicalDevice, &imageViewCI, nullptr, &depthStencil.view));
}

void VContext::setupRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = swapChain.colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &colorReference;
    subpass_description.pDepthStencilAttachment = &depthReference;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = nullptr;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = nullptr;
    subpass_description.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass_description;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    CHECK_ERROR(vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass));
}

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
VkResult VContext::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VBuffer::Buffer* buffer, VkDeviceSize size, void* data) const
{
    buffer->device = device.logicalDevice;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = Initializers::bufferCreateInfo(usageFlags, size);
    vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, nullptr, &buffer->buffer);

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memory_requierements;
    VkMemoryAllocateInfo memAlloc = Initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(device.logicalDevice, buffer->buffer, &memory_requierements);
    memAlloc.allocationSize = memory_requierements.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = getMemoryType(memory_requierements.memoryTypeBits, memoryPropertyFlags);
    vkAllocateMemory(device.logicalDevice, &memAlloc, nullptr, &buffer->memory);

    buffer->alignment = memory_requierements.alignment;
    buffer->size = memAlloc.allocationSize;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        buffer->map();
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            CHECK_ERROR(buffer->flush());

        buffer->unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->setupDescriptor();

    // Attach the memory to the buffer object
    return buffer->bind();
}

void VContext::setImageLayout(const VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask,
                              VkPipelineStageFlags dstStageMask)
{
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier = Initializers::imageMemoryBarrier();
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source 
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
        cmd_buffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}
//VALID
void VContext::setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = aspectMask;
    subresource_range.baseMipLevel = 0;
    subresource_range.levelCount = 1;
    subresource_range.layerCount = 1;
    setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresource_range, srcStageMask, dstStageMask);
}
//VALID
void VContext::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) const
{
    if (commandBuffer == nullptr)
    {
        return;
    }

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = Initializers::submitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo = Initializers::fenceCreateInfo(0);
    VkFence fence;
    vkCreateFence(device.logicalDevice, &fenceInfo, nullptr, &fence);

    // Submit to the queue
    vkQueueSubmit(queue, 1, &submitInfo, fence);
    // Wait for the fence to signal that command buffer has finished executing
    vkWaitForFences(device.logicalDevice, 1, &fence, VK_TRUE, 100000000000);

    vkDestroyFence(device.logicalDevice, fence, nullptr);

    if (free)
    {
        vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);
    }
}

void VContext::createShaderBindingTable()
{
    // Create buffer for the shader binding table
    const uint32_t sbtSize = rayTracingProperties.shaderGroupHandleSize * NUM_SHADER_GROUPS;
    createBuffer(
        VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &mShaderBindingTable,
        sbtSize);
    mShaderBindingTable.map();

    const auto shaderHandleStorage = new uint8_t[sbtSize];
    // Get shader identifiers
    vkGetRayTracingShaderGroupHandlesNV(device.logicalDevice, Rpipeline, 0, NUM_SHADER_GROUPS, sbtSize, shaderHandleStorage);
    auto* data = static_cast<uint8_t*>(mShaderBindingTable.mapped);
    // Copy the shader identifiers to the shader binding table
    data += copyShaderIdentifier(data, shaderHandleStorage, INDEX_RAYGEN);
    data += copyShaderIdentifier(data, shaderHandleStorage, INDEX_MISS);
    data += copyShaderIdentifier(data, shaderHandleStorage, INDEX_SHADOWMISS);
    data += copyShaderIdentifier(data, shaderHandleStorage, INDEX_CLOSEST_HIT);
    data += copyShaderIdentifier(data, shaderHandleStorage, INDEX_SHADOWHIT);
    mShaderBindingTable.unmap();
}

VkDeviceSize VContext::copyShaderIdentifier(uint8_t* data, const uint8_t* shaderHandleStorage, uint32_t groupIndex) const
{
    const uint32_t shaderGroupHandleSize = rayTracingProperties.shaderGroupHandleSize;
    memcpy(data, shaderHandleStorage + groupIndex * shaderGroupHandleSize, shaderGroupHandleSize);
    data += shaderGroupHandleSize;
    return shaderGroupHandleSize;
}

void VContext::createDescriptorSets()
{
    const std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Initializers::descriptorPoolCreateInfo(poolSizes, 1);
    vkCreateDescriptorPool(device.logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Initializers::descriptorSetAllocateInfo(descriptorPool, &RdescriptorSetLayout, 1);
    vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocateInfo, &RdescriptorSet);

    //Acceleration Structure
    VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo{};
    descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.accelerationStructure;

    VkWriteDescriptorSet accelerationStructureWrite{};
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
    accelerationStructureWrite.dstSet = RdescriptorSet;
    accelerationStructureWrite.dstBinding = 0;
    accelerationStructureWrite.descriptorCount = 1;
    accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkDescriptorImageInfo storageImageDescriptor{};
    storageImageDescriptor.imageView = storageImage.view;
    storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VBuffer::Buffer indexBuff;

    VkDescriptorBufferInfo vertexBufferDescriptor{};
	vertexBufferDescriptor.buffer = vertBuffer.buffer;
	vertexBufferDescriptor.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo TriNumberDescriptor{};
	TriNumberDescriptor.buffer = NumberOfTriangles.buffer;
	TriNumberDescriptor.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo TimeBufferDescriptor{};
	TriNumberDescriptor.buffer = TimeBuffer.buffer;
	TriNumberDescriptor.range = VK_WHOLE_SIZE;
    //Storage Image
    const VkWriteDescriptorSet resultImageWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
    //Uniform Data
    const VkWriteDescriptorSet uniformBufferWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.descriptor);
    const VkWriteDescriptorSet matBufferWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &matBuffer.descriptor);
    VkWriteDescriptorSet vertexBufferWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &vertBuffer.descriptor);
	VkWriteDescriptorSet TimeBufferWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, &TimeBuffer.descriptor);
	VkWriteDescriptorSet TriNumberWrite = Initializers::writeDescriptorSet(RdescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, &NumberOfTriangles.descriptor);

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        accelerationStructureWrite,
        resultImageWrite,
        uniformBufferWrite,
        matBufferWrite,
        vertexBufferWrite,
        TimeBufferWrite,
        TriNumberWrite
    };

    vkUpdateDescriptorSets(device.logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VContext::createUniformBuffer()
{
    CHECK_ERROR(createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &ubo,
        sizeof(uniformData),
        &uniformData));
    CHECK_ERROR(ubo.map());

    updateUniformBuffers();
}

void VContext::updateUniformBuffers()
{
    uniformData.projInverse = inverse(camera.matrices.perspective);
    uniformData.viewInverse = inverse(camera.matrices.view);
    memcpy(ubo.mapped, &uniformData, sizeof(uniformData));
}

void VContext::buildCommandbuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = Initializers::commandBufferBeginInfo();

    const VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        CHECK_ERROR(vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo));

        /*
            Dispatch the ray tracing commands
        */
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, Rpipeline);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, RpipelineLayout, 0, 1, &RdescriptorSet, 0, nullptr);

        // Calculate shader binding offsets, which is pretty straight forward in our example 
        const VkDeviceSize bindingOffsetRayGenShader = rayTracingProperties.shaderGroupHandleSize * INDEX_RAYGEN;
        const VkDeviceSize bindingOffsetMissShader = rayTracingProperties.shaderGroupHandleSize * INDEX_MISS;
        const VkDeviceSize bindingOffsetHitShader = rayTracingProperties.shaderGroupHandleSize * INDEX_CLOSEST_HIT;
        const VkDeviceSize bindingStride = rayTracingProperties.shaderGroupHandleSize;

        vkCmdTraceRaysNV(commandBuffers[i],
            mShaderBindingTable.buffer, bindingOffsetRayGenShader,
            mShaderBindingTable.buffer, bindingOffsetMissShader, bindingStride,
            mShaderBindingTable.buffer, bindingOffsetHitShader, bindingStride,
            nullptr, 0, 0,
            WIDTH, HEIGHT, 1);

        /*
            Copy raytracing output to swap chain image
        */

        // Prepare current swapchain image as transfer destination
        Tools::setImageLayout(
            commandBuffers[i],
            swapChain.images[i],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        // Prepare ray tracing output image as transfer source
        Tools::setImageLayout(
            commandBuffers[i],
            storageImage.image,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.dstOffset = { 0, 0, 0 };
        copyRegion.extent = { WIDTH, HEIGHT, 1 };
        vkCmdCopyImage(commandBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        // Transition swap chain image back for presentation
        Tools::setImageLayout(
            commandBuffers[i],
            swapChain.images[i],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        // Transition ray tracing output image back to general layout
        Tools::setImageLayout(
            commandBuffers[i],
            storageImage.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        CHECK_ERROR(vkEndCommandBuffer(commandBuffers[i]));
    }
}

void VContext::setupRayTracingSupport(std::vector<VObject>& objects, std::vector<int>& trianglesNumber)
{
    // Query the ray tracing properties of the current implementation, we will need them later on
    rayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    VkPhysicalDeviceProperties2 deviceProps2{};
    deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProps2.pNext = &rayTracingProperties;
    vkGetPhysicalDeviceProperties2(device.physicalDevice, &deviceProps2);

    VkSemaphoreCreateInfo semaphoreCreateInfo = Initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    CHECK_ERROR(vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    CHECK_ERROR(vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

    camera.type = Camera::lookat;
    camera.setPosition(glm::vec3(0, 6, -6));
    camera.setPerspective(60.0f, static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1, 1024);
    camera.setRotation(glm::vec3(-10, 0, 0));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    submitInfo = Initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    createScene(objects);
    CreateStorageImage();

    std::vector<float> mat;

    for(auto obj : objects)
    {
        trianglesNumber.push_back(obj.m_mesh.GetVertices().size() / 3);
        std::cout << "NUMBER OF TRIANGLES: " << obj.m_mesh.GetVertices().size() / 3
                  << " INSTANCE ID: " << obj.m_mesh.meshGeometry.instanceId << '\n';

        for(auto vertex : obj.m_mesh.GetVertices())
        {
            bufferVertices.push_back(vertex.pos.x);
            bufferVertices.push_back(vertex.pos.y);
           // bufferVertices.push_back(vertex.pos.z);
            bufferVertices.push_back(vertex.pos.z);
            bufferVertices.push_back(obj.m_mesh.meshGeometry.instanceId);
            bufferVertices.push_back(vertex.normal.x);
            bufferVertices.push_back(vertex.normal.y);
            bufferVertices.push_back(vertex.normal.z);
            bufferVertices.push_back(0);
        }

        mat.push_back(obj.m_material.colorAndRoughness.x);
        mat.push_back(obj.m_material.colorAndRoughness.y);
        mat.push_back(obj.m_material.colorAndRoughness.z);
        mat.push_back(obj.m_material.colorAndRoughness.w);
        mat.push_back(obj.m_material.ior.x);
        mat.push_back(obj.m_material.ior.y);
        mat.push_back(obj.m_material.ior.z);
        mat.push_back(obj.m_material.ior.w);
    }
    t.push_back(1.0f);
    CHECK_ERROR(createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &TimeBuffer,
        t.size() * sizeof(float),
        t.data()));

    CHECK_ERROR(createBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &matBuffer,
        mat.size() * sizeof(float),
        mat.data()));

    CHECK_ERROR(createBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &vertBuffer,
        bufferVertices.size() * sizeof(float),
        bufferVertices.data()));
    
    CHECK_ERROR(createBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &NumberOfTriangles,
        trianglesNumber.size() * sizeof(int),
        trianglesNumber.data()));

    //CHECK_ERROR(ubo.map());

    createUniformBuffer();
    createRayTracingPipeline();
    createShaderBindingTable();


    createDescriptorSets();
    buildCommandbuffers();
}

void VContext::prepareFrame()
{
    // Acquire the next image from the swap chain
    const VkResult result = acquireNextImage(semaphores.presentComplete, &currentBuffer);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
    {
        //windowResize();
    }
    else
    {
        CHECK_ERROR(result);
    }
}
void VContext::submitFrame() const
{
    const VkResult result = queuePresent(graphicsQueue, currentBuffer, semaphores.renderComplete);
    if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Swap chain is no longer compatible with the surface and needs to be recreated
            //windowResize();
            return;
        }
        CHECK_ERROR(result);
    }
    CHECK_ERROR(vkQueueWaitIdle(graphicsQueue));
}

void VContext::draw()
{
    prepareFrame();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
    CHECK_ERROR(vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr));
    submitFrame();
}

#pragma endregion
