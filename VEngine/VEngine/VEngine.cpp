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
        context.createLogicalDevice();
        context.createSwapChain();
        context.createImageViews();
        context.createRenderpass();

        //----------
        //INSERT BACK HERE
        //----------
        context.createGraphicPipeline();
        context.createFramebuffers();
        context.createCommandpool();
        context.createCommandbuffers();
        context.createSemaphores();

        //RASTERIZER PART
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(context.GetWindow())) 
        {
            glfwPollEvents();
            context.drawFrame();
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