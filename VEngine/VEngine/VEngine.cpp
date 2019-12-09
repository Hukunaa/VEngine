#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <Context.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define WIDTH 800
#define HEIGHT 800
class HelloTriangleApplication 
{
public:

    VContext context;

    void run() 
    {
        initVulkan();
        mainLoop();
        context.CleanUp();
    }

private:

    void initVulkan() 
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        context.CreateWinow(WIDTH, HEIGHT, "VEngine");

        context.SetupInstance();
        context.SetupDebugMessenger();
        context.SelectGPU();
        context.CreateLogicalDevice();
        context.createSwapChain();
        context.createImageViews();
        context.CreateRenderPass();

        //----------
        //INSERT BACK HERE
        //----------
        context.CreateCommandPool();
        //https://github.com/SaschaWillems/Vulkan/blob/master/examples/nv_ray_tracing_basic/nv_ray_tracing_basic.cpp#L125
        //RASTERIZER PART
        //context.CreateGraphicPipeLine();
        //context.CreateFrameBuffers();
        //context.CreateCommandBuffers();
        //context.CreateSemaphores();
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(context.GetWindow())) 
        {
            glfwPollEvents();
            context.DrawFrame();
        }
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}