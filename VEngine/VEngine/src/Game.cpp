#include <VGame.h>
//#include "basics.h"

void Game::InitAPI()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GameInstance = new VContext;
    GameInstance->CREATETHEFUCKINGWINDOW(WIDTH, HEIGHT, "VEngine");
    GameInstance->SetupInstance();
    GameInstance->SetupDebugMessenger();
    GameInstance->SelectGPU();
    GameInstance->createLogicalDevice();

    glfwSetInputMode(GameInstance->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback( GameInstance->window, CursorCallBack);
    GameInstance->initSwapChain();
    GameInstance->CreateCommandPool();
    GameInstance->setupSwapChain(WIDTH, HEIGHT, false);
    GameInstance->CreateCommandBuffers();
    GameInstance->createSynchronizationPrimitives();
    GameInstance->setupDepthstencil();
    GameInstance->setupRenderPass();
    GameInstance->createPipelineCache();
    GameInstance->setupFrameBuffer();
    //GameInstance->InitOptix();

    time.push_back(0.5f);
}
void Game::SetupGame()
{
    /*Material type:
    Diffuse = 1;
    Metal (Dieletric) = 2;
    (not working yet) Emissive = 3;*/
    
    VObject sphere2("sphere");
    sphere2.m_mesh.LoadMesh("shaders/models/sphere.obj", true);
    sphere2.SetColor(0.9, 0.9, 0.9);
    sphere2.SetMaterialType(2);
    sphere2.SetReflectivity(0.5);

    sphere2.SetPosition({0, -5, -12});
    sphere2.Rotate({180, 0, 0});
    sphere2.SetScale(3);
    m_objects.push_back(sphere2);

    VObject sphere3("sphere2");
    sphere3.m_mesh.LoadMesh("shaders/models/monkey.obj", true);
    sphere3.SetColor(0.9, 0.1, 0.9);
    sphere3.SetMaterialType(1);

    sphere3.SetPosition({ -3, -4, 4 });
    sphere3.Rotate({ 180, 180, 0 });
    sphere3.SetScale(1.5);
    m_objects.push_back(sphere3);

    VObject monkey("house");
    monkey.m_mesh.LoadMesh("shaders/models/Pantheon.obj", true);
    monkey.SetColor(0.9, 0.9, 0.9);
    monkey.SetMaterialType(1);

    monkey.SetPosition({0, -1, 0});
    monkey.Rotate({180, 90, 0});
    monkey.SetScale(0.5);
    m_objects.push_back(monkey);

    VObject plane("floor");
    plane.m_mesh.LoadMesh("shaders/models/plane.obj", true);
    plane.SetColor(0.3,0.95,0.2);
    plane.SetMaterialType(1);

    plane.SetPosition({0, -1, 0});
    plane.Rotate({0, 0, 0});
    plane.SetScale(1);
    m_objects.push_back(plane);

    GameInstance->setupRayTracingSupport(m_objects, trianglesNumber);
    //SetupIMGUI();
    GameInstance->buildCommandbuffers();
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    double xpos = 0;
    double ypos = 0;
    double sensivity = 0.2;

    float lastX = 0;
    float lastY = 0;
    glm::vec3 lastPos;
    int frameCount = 0;
    float lastFPS = glfwGetTime();
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        float currentTime = glfwGetTime();
        frameCount++;
        time[0] += 0.001f;
        glfwPollEvents();


        InputManager();


        double x, y;
        x = y = 0;
        glfwGetCursorPos( GameInstance->window, &x, &y );

        if (mouseControl)
        {
            GameInstance->camera.Pitch -= (ypos - y) * sensivity;
            GameInstance->camera.Yaw -= (xpos - x) * sensivity;
        }
        //FindObject("sphere2")->SetPosition(glm::vec3( -3, -4, 1 ) + glm::vec3(cos(sinus * 0.5) * 2, 0, sin(sinus * 0.5) * 2));
        GameInstance->UpdateObjects(m_objects);
        GameInstance->draw();

        if (GameInstance->camera.updated)
        {
            
            GameInstance->camera.updateViewMatrix();
            bool updateAccumulation = false;
            if(GameInstance->camera.Front.x == lastX && GameInstance->camera.Front.y == lastY && GameInstance->camera.position == lastPos)
                updateAccumulation = true;

            GameInstance->updateUniformBuffers(updateAccumulation);
        }
        if ( currentTime - lastFPS >= 1.0 )
        {
            // Display the frame count here any way you want.
            glfwSetWindowTitle(GameInstance->window,std::to_string(frameCount).c_str());

            frameCount = 0;
            lastFPS = currentTime;
        }
        //glfwSetWindowTitle(GameInstance->window, std::to_string(GameInstance->uniformData.data.y).c_str());
        sinus += 0.25f;
        GameInstance->UpdateTime(time);
        xpos = x;
        ypos = y;

        lastX = GameInstance->camera.Front.x;
        lastY = GameInstance->camera.Front.y;
        lastPos = GameInstance->camera.position;
    }
    GameInstance->CleanUp();
}
void Game::InputManager()
{
    float moveSpeed = 0.1;
    if(glfwGetKey(GameInstance->window, GLFW_KEY_W) == GLFW_PRESS)
    {
        GameInstance->camera.setPosition(GameInstance->camera.position + GameInstance->camera.Front * moveSpeed);
    }
    if(glfwGetKey(GameInstance->window, GLFW_KEY_S) == GLFW_PRESS)
    {
        GameInstance->camera.setPosition(GameInstance->camera.position - GameInstance->camera.Front * moveSpeed);
    }
    if(glfwGetKey(GameInstance->window, GLFW_KEY_A) == GLFW_PRESS)
    {
        GameInstance->camera.setPosition(GameInstance->camera.position - GameInstance->camera.Right * moveSpeed);
    }
    if(glfwGetKey(GameInstance->window, GLFW_KEY_D) == GLFW_PRESS)
    {
        GameInstance->camera.setPosition(GameInstance->camera.position + GameInstance->camera.Right * moveSpeed);
    }
    if (glfwGetKey(GameInstance->window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        mouseControl = !mouseControl;
    }
}
void Game::CursorCallBack(GLFWwindow* window, double xpos, double ypos)
{
}

void Game::SetupIMGUI()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
        //Setup IMGUI renderpass
    ImGui_ImplGlfw_InitForVulkan(GameInstance->window, true);
    VkAttachmentDescription attachment = {};
    attachment.format = GameInstance->swapChain.colorFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    if (vkCreateRenderPass(GameInstance->device.logicalDevice, &info, nullptr, &GameInstance->imGuiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = GameInstance->instance;
    init_info.PhysicalDevice = GameInstance->device.physicalDevice;
    init_info.Device = GameInstance->device.logicalDevice;
    init_info.QueueFamily = GameInstance->queueFamily.graphicsFamily.value();
    init_info.Queue = GameInstance->graphicsQueue;
    init_info.PipelineCache = GameInstance->pipelineCache;
    init_info.DescriptorPool = GameInstance->descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = GameInstance->minImageCount + 1;
    init_info.ImageCount = GameInstance->swapChain.imageCount;
    init_info.CheckVkResultFn = GameInstance->CHECK_ERROR;
    ImGui_ImplVulkan_Init(&init_info, GameInstance->imGuiRenderPass);

    nvvkpp::SingleCommandBuffer sc(GameInstance->dev, 0);
    vk::CommandBuffer cmdBuffer = sc.createCommandBuffer();

    ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
    sc.flushCommandBuffer(cmdBuffer);
}

