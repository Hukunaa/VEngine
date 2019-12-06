#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <Context.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
        context.CreateWinow(1280, 720, "VEngine");

        context.SetupInstance();
        context.SetupDebugMessenger();
        context.SelectGPU();
        context.CreateLogicalDevice();
        context.createSwapChain();
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(context.GetWindow())) 
        {
            glfwPollEvents();
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