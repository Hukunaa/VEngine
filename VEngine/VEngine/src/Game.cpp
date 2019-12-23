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

}
void Game::SetupGame()
{
    VObject wuson("wuson");
    wuson.m_mesh.LoadMesh("shaders/models/monkey.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    wuson.m_material.colorAndRoughness = {1, 0.2, 0.2, 0.4};
    wuson.m_material.ior = {1.5, 0, 0, 0};
    wuson.SetPosition({0, -4, -5});
    wuson.SetRotation({180, 0, 0});
    wuson.SetScale(1);
    m_objects.push_back(wuson);

     VObject wuson1("wuson1");
    wuson1.m_mesh.LoadMesh("shaders/models/sphere.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    wuson1.m_material.colorAndRoughness = {0.2, 1, 0.2, 0.8};
    wuson.m_material.ior = {5, 0, 0, 0};
    wuson1.SetPosition({3, -4, -5});
    wuson1.SetRotation({0, 0, 0});
    wuson1.SetScale(1.5);
    m_objects.push_back(wuson1);

    VObject wuson2("wuson2");
    wuson2.m_mesh.LoadMesh("shaders/models/bunny.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    wuson2.m_material.colorAndRoughness = {0.2, 0.5, 1, 0.0};
    wuson.m_material.ior = {0.470, 0, 0, 0};
    wuson2.SetPosition({-3, -3, -5});
    wuson2.SetRotation({180, 0, 0});
    wuson2.SetScale(0.1);
    m_objects.push_back(wuson2);

    VObject plane("floor");
    plane.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::PLANE);
    plane.m_material.colorAndRoughness = {1,1,1, 0.0};
    plane.m_material.ior = {-25, 0, 0, 0};
    plane.SetPosition({-4, -3, -5});
    plane.SetRotation({0, 0, 0});
    plane.SetScale(4);
    m_objects.push_back(plane);

    GameInstance->setupRayTracingSupport(m_objects, trianglesNumber);
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        glfwPollEvents();

        FindObject("wuson")->SetRotation({0, sinus, 0});
        FindObject("wuson1")->SetRotation({0, 0, -sinus});
        FindObject("wuson2")->SetRotation({0, sinus,0});
        GameInstance->UpdateObjects(m_objects);
        GameInstance->draw();
        if (GameInstance->camera.updated)
        {
            GameInstance->camera.updateViewMatrix();
            GameInstance->updateUniformBuffers();
        }
        sinus = 0.025f;
    }
    GameInstance->CleanUp();
}
