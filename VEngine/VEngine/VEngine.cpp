#include <iostream>
#include <functional>
#include <cstdlib>
#include <Context.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define WIDTH 1280
#define HEIGHT 720

class HelloTriangleApplication {
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
        context.CreateWindow(WIDTH, HEIGHT, "VEngine");

        context.SetupInstance();
        context.SetupDebugMessenger();
        context.SelectGPU();
        context.createLogicalDevice();

        context.initSwapChain();
        context.CreateCommandPool();
        context.setupSwapChain(WIDTH, HEIGHT, false);
        context.CreateCommandBuffers();
        context.createSynchronizationPrimitives();
        context.setupDepthstencil();
        context.setupRenderPass();
        context.createPipelineCache();
        context.setupFrameBuffer();

        context.setupRayTracingSupport();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(context.GetWindow()))
        {
            glfwPollEvents();
            context.draw();
            if (context.camera.updated)
            {
                context.camera.translate(glm::vec3(0.001, 0, 0));
                context.camera.rotate({0, -0.01, 0});
                context.camera.updateViewMatrix();
                context.updateUniformBuffers();
            }
        }
    }
};

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
