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
    wuson.m_mesh.LoadMesh("shaders/bunny.obj", true);
    //cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    wuson.m_material.colorAndRoughness = {0.5, 0.5, 1, 1};
    wuson.SetPosition({0, -2, -5});
    wuson.SetRotation({90, 0, 0});
    wuson.SetScale(0.1f);
    m_objects.push_back(wuson);

    VObject plane("floor");
    plane.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::PLANE);
    plane.m_material.colorAndRoughness = {0, 1, 1, 1};
    plane.SetPosition({0, -1, -5});
    plane.SetRotation({0, 0, 0});
    plane.SetScale(1);

    m_objects.push_back(plane);
    GameInstance->setupRayTracingSupport(m_objects);
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        glfwPollEvents();

        //FindObject("wuson")->SetPosition({sin(sinus), 0, -5});
        FindObject("wuson")->SetRotation({0, 0, sinus});
        GameInstance->UpdateObjects(m_objects);
        GameInstance->draw();
        if (GameInstance->camera.updated)
        {
            GameInstance->camera.updateViewMatrix();
            GameInstance->updateUniformBuffers();
        }
        sinus += 0.001f;
    }
    GameInstance->CleanUp();
}
