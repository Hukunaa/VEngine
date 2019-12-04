#include <vulkan/vulkan.h>

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

    Context context;

    void run() {
        initVulkan();
        mainLoop();
        cleanup();
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
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(context.GetWindow())) 
        {
            glfwPollEvents();
        }
    }

    void cleanup() 
    {
        vkDestroyInstance(context.GetInstance(), nullptr);
        glfwDestroyWindow(context.GetWindow());
        glfwTerminate();
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