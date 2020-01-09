#include <VGame.h>
//#include "basics.h"

#include <optix_function_table_definition.h>

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

    //glfwSetKeyCallback(GameInstance->window, InputManager);
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
    InitOptix();
    time.push_back(0.5f);
}
void Game::SetupGame()
{
    /*Material type:
    Diffuse = 1;
    Metal (Dieletric) = 2;
    (not working yet) Emissive = 3;*/
    
    VObject sphere2("sphere");
    sphere2.m_mesh.LoadMesh("shaders/models/monkey.obj", true);
    sphere2.SetColor(0.9, 0.1, 0.1);
    sphere2.SetMaterialType(2);
    sphere2.SetReflectivity(0.0);

    sphere2.SetPosition({0, -4, 0});
    sphere2.Rotate({180, 0, 0});
    sphere2.SetScale(2);
    m_objects.push_back(sphere2);

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
    plane.SetColor(123.0/255.0,192.0/255.0,67.0/255.0);
    plane.SetMaterialType(1);

    plane.SetPosition({0, -1, 0});
    plane.Rotate({0, 0, 0});
    plane.SetScale(1);
    m_objects.push_back(plane);

    GameInstance->setupRayTracingSupport(m_objects, trianglesNumber);
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
        glfwGetCursorPos( GameInstance->window, &x, &y );
        GameInstance->camera.Pitch -= (ypos - y) * sensivity;
        GameInstance->camera.Yaw -= (xpos - x) * sensivity;

        GameInstance->UpdateObjects(m_objects);
        GameInstance->buildCommandbuffers();
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
}
void Game::CursorCallBack(GLFWwindow* window, double xpos, double ypos)
{
}
void Game::DenoiseImage(const VkImage& imgIn, VkImage& imgOut)
{
    VkDeviceSize bufferSize = 1280 * 720 * 4 * sizeof(float);
}
void Game::InitOptix()
{
   /* cudaFree(nullptr);
    CUcontext cuCtx;
    CUresult  cuRes = cuCtxGetCurrent(&cuCtx);
    if(cuRes != CUDA_SUCCESS)
    {
    std::cerr << "Error querying current context: error code " << cuRes << "\n";
    }
    OPTIX_CHECK(optixInit());
    OPTIX_CHECK(optixDeviceContextCreate(cuCtx, nullptr, &m_optixDevice));
    OPTIX_CHECK(optixDeviceContextSetLogCallback(m_optixDevice, context_log_cb, nullptr, 4));

    OptixPixelFormat pixelFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
    size_t           sizeofPixel = sizeof(float4);


    m_dOptions.inputKind   = OPTIX_DENOISER_INPUT_RGB;
    m_dOptions.pixelFormat = pixelFormat;
    OPTIX_CHECK(optixDenoiserCreate(m_optixDevice, &m_dOptions, &m_denoiser));
    OPTIX_CHECK(optixDenoiserSetModel(m_denoiser, OPTIX_DENOISER_MODEL_KIND_HDR, nullptr, 0));*/
}

