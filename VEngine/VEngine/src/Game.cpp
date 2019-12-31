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
    VObject sphere("sphere");
    sphere.m_mesh.LoadMesh("shaders/models/sphere.obj", true);

    sphere.m_material.colorAndRoughness = {238.0/255.0, 64.0/255.0, 53.0/255.0, 0};
    sphere.m_material.ior = {1, 0.5, 0, 0};

    sphere.SetPosition({0, -2, -6});
    sphere.SetRotation({180, 0, 0});
    sphere.SetScale(2);
    m_objects.push_back(sphere);

    VObject sphere2("light");
    sphere2.m_mesh.LoadMesh("shaders/models/sphere.obj", true);
    sphere2.m_material.colorAndRoughness = {243.0/255.0, 119.0/255.0, 54.0/255.0, 0};
    sphere2.m_material.ior = {1, 0.4, 0, 0};

    sphere2.SetPosition({-6, -1, 0});
    sphere2.SetRotation({180, 0, 0});
    sphere2.SetScale(1);
    m_objects.push_back(sphere2);

    VObject monkey("monkey");
    monkey.m_mesh.LoadMesh("shaders/models/monkey.obj", true);

    monkey.m_material.colorAndRoughness = {253.0/255.0, 244.0/255.0, 152.0/255.0, 0};
    monkey.m_material.ior = {1, 0, 0, 0};

    monkey.SetPosition({0, -3, 0});
    monkey.SetRotation({180, 200, 0});
    monkey.SetScale(2);
    m_objects.push_back(monkey);

    VObject plane("floor");
    plane.m_mesh.LoadMesh("shaders/models/plane.obj", true);

    plane.m_material.colorAndRoughness = {123.0/255.0,192.0/255.0,67.0/255.0, 0};
    plane.m_material.ior = {1, 0, 0, 0};

    plane.SetPosition({0, 0.3, 0});
    plane.SetRotation({0, 0, 0});
    plane.SetScale(1);
    m_objects.push_back(plane);

    VObject sphereOmbra("sphereOmbra");
    sphereOmbra.m_mesh.LoadMesh("shaders/models/sphere.obj", true);

    sphereOmbra.m_material.colorAndRoughness = {0.9, 0.9, 0.9, 0};//{3.0/255.0,146.0/255.0,207.0/255.0, 0};
    sphereOmbra.m_material.ior = {2, 0.05, 0, 0};

    sphereOmbra.SetPosition({-2, -1.5, 2});
    sphereOmbra.SetRotation({0, 0, 0});
    sphereOmbra.SetScale(1.5);
    m_objects.push_back(sphereOmbra);

    VObject sphereOmbra2("sphereOmbra");
    sphereOmbra2.m_mesh.LoadMesh("shaders/models/sphere.obj", true);

    sphereOmbra2.m_material.colorAndRoughness = {1,1,1, 0};
    sphereOmbra2.m_material.ior = {2, 0.2, 0, 0};

    sphereOmbra2.SetPosition({2, -1.6, 4});
    sphereOmbra2.SetRotation({0, 0, 0});
    sphereOmbra2.SetScale(1.6);
    m_objects.push_back(sphereOmbra2);

    GameInstance->setupRayTracingSupport(m_objects, trianglesNumber);
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        time[0] += 0.001f;
        glfwPollEvents();

        FindObject("sphere")->Translate({cos(sinus * 0.02) * 0.015, 0, 0});
        FindObject("sphere")->Translate({0, 0, sin(sinus * 0.02) * 0.015});
        //FindObject("wuson1")->SetRotation({0, 0, -sinus});
        //FindObject("wuson2")->SetRotation({0, sinus,0});
        //GameInstance->camera.setTranslation({sinus * 0.01, 0, 0});
        GameInstance->UpdateObjects(m_objects);
        GameInstance->draw();
        if (GameInstance->camera.updated)
        {
            
            GameInstance->camera.updateViewMatrix();
            GameInstance->updateUniformBuffers();
        }
        sinus += 0.25f;
        GameInstance->UpdateTime(time);
    }
    GameInstance->CleanUp();
}
