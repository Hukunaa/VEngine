#include "Game.h"

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
    VObject monkey("sphere");
    monkey.m_mesh.LoadMesh("shaders/models/sphere.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);

    monkey.m_material.colorAndRoughness = {0.1, 0.2, 0.5, 0};
    monkey.m_material.ior = {1, 0, 0, 0};

    monkey.SetPosition({0, -4, -6});
    monkey.SetRotation({180, 0, 0});
    monkey.SetScale(0.08);
    m_objects.push_back(monkey);

     VObject sphere2("sphere2");
    sphere2.m_mesh.LoadMesh("shaders/models/monkey.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);

    sphere2.m_material.colorAndRoughness = {0.8, 0.6, 0.2, 0};
    sphere2.m_material.ior = {2, 0.3, 0, 0};

    sphere2.SetPosition({0, -5, -2});
    sphere2.SetRotation({180, 200, 0});
    sphere2.SetScale(1.4);
    m_objects.push_back(sphere2);

    VObject plane("floor");
    plane.m_mesh.LoadMesh("shaders/models/plane.obj", true);

    plane.m_material.colorAndRoughness = {0.4,0.8,0.0, 0};
    plane.m_material.ior = {1, 0, 0, 0};

    plane.SetPosition({0, -2.8, 0});
    plane.SetRotation({0, 0, 0});
    plane.SetScale(1);
    m_objects.push_back(plane);

    /*VObject plane2("floor2");
    plane2.m_mesh.LoadMesh("shaders/models/plane.obj", true);
    plane2.m_material.colorAndRoughness = {1,1,1.0, 0};
    plane2.m_material.ior = {-25, 0, 0, 0};
    plane2.SetPosition({0, 10, -8});
    plane2.SetRotation({90, 0, 0});
    plane2.SetScale(1);
    m_objects.push_back(plane2);

    VObject plane3("floor3");
    plane3.m_mesh.LoadMesh("shaders/models/plane.obj", true);
    plane3.m_material.colorAndRoughness = {0,0,1.0, 0};
    plane3.m_material.ior = {-25, 0, 0, 0};
    plane3.SetPosition({-8, 10, -8});
    plane3.SetRotation({90, 0, 90});
    plane3.SetScale(1);
    m_objects.push_back(plane3);

    VObject plane4("floor4");
    plane4.m_mesh.LoadMesh("shaders/models/plane.obj", true);
    plane4.m_material.colorAndRoughness = {0,1.0,0.0, 0};
    plane4.m_material.ior = {-25, 0, 0, 0};
    plane4.SetPosition({8, 10, -8});
    plane4.SetRotation({90, 0, -90});
    plane4.SetScale(1);
    m_objects.push_back(plane4);*/

    GameInstance->setupRayTracingSupport(m_objects, trianglesNumber);
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        time[0] += 0.1f;
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
