#include <VGame.h>

void Game::InitAPI()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GameInstance = new VContext;
    GameInstance->CreateWindow(WIDTH, HEIGHT, "VEngine");
    GameInstance->SetupInstance();
    GameInstance->SetupDebugMessenger();
    GameInstance->SelectGPU();
    GameInstance->createLogicalDevice();

    //glfwSetKeyCallback(GameInstance->window, InputManager);
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
    time.push_back(0.5f);
}
void Game::SetupGame()
{

    VObject sphere2("light");
    sphere2.m_mesh.LoadMesh("shaders/models/sphere.obj", true);
    sphere2.m_material.colorAndRoughness = {243.0/255.0, 119.0/255.0, 54.0/255.0, 0};
    sphere2.m_material.ior = {1, 0.4, 0, 0};

    sphere2.SetPosition({-6, -1, 0});
    sphere2.SetRotation({180, 0, 0});
    sphere2.SetScale(1);
    m_objects.push_back(sphere2);

    VObject monkey("monkey");
    monkey.m_mesh.LoadMesh("shaders/models/house.obj", true);

    monkey.m_material.colorAndRoughness = {253.0/255.0, 244.0/255.0, 152.0/255.0, 0};
    monkey.m_material.ior = {1, 0, 0, 0};

    monkey.SetPosition({0, -1, 0});
    monkey.SetRotation({180, 0, 0});
    monkey.SetScale(0.8);
    m_objects.push_back(monkey);

    VObject plane("floor");
    plane.m_mesh.LoadMesh("shaders/models/plane.obj", true);

    plane.m_material.colorAndRoughness = {123.0/255.0,192.0/255.0,67.0/255.0, 0};
    plane.m_material.ior = {1, 0, 0, 0};

    plane.SetPosition({0, 0.3, 0});
    plane.SetRotation({0, 0, 0});
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
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        time[0] += 0.001f;
        glfwPollEvents();

        //FindObject("sphere")->Translate({cos(sinus * 0.02) * 0.015, 0, 0});
        //FindObject("sphere")->Translate({0, 0, sin(sinus * 0.02) * 0.015});
        //FindObject("wuson1")->SetRotation({0, 0, -sinus});
        //FindObject("wuson2")->SetRotation({0, sinus,0});
        //GameInstance->camera.setTranslation({sinus * 0.01, 0, 0});
        InputManager();

        double x, y;
        glfwGetCursorPos( GameInstance->window, &x, &y );
        GameInstance->camera.Pitch -= (ypos - y) * sensivity;
        GameInstance->camera.Yaw -= (xpos - x) * sensivity;
        GameInstance->UpdateObjects(m_objects);
        GameInstance->draw();
        if (GameInstance->camera.updated)
        {
            
            GameInstance->camera.updateViewMatrix();
            GameInstance->updateUniformBuffers();
        }
        sinus += 0.25f;
        GameInstance->UpdateTime(time);
        xpos = x;
        ypos = y;
    }
    GameInstance->CleanUp();
}
void Game::InputManager()
{
    float moveSpeed = 0.2;
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
