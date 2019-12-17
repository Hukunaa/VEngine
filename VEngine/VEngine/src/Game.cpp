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
    VObject cube1;
    cube1.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    cube1.SetPosition({-5, 0, 0});
    cube1.SetRotation({20, 20, 0});
    m_objects.push_back(cube1);

    VObject cube2;
    cube2.m_mesh.SetMeshType(VMesh::MESH_PRIMITIVE::CUBE);
    cube2.SetPosition({0, 0, 5});
    cube2.SetRotation({45, 45, 0});
    m_objects.push_back(cube2);
    GameInstance->setupRayTracingSupport(m_objects);
    GameLoop();
}
void Game::GameLoop()
{
     float sinus = 0;
    while (!glfwWindowShouldClose(GameInstance->GetWindow()))
    {
        glfwPollEvents();
        //context.m_mesh.pos = {sinus, 0, 5};
        //GameInstance->m_mesh.Rotate({0.1f, 0.1f, 0.1f});

        //KNOWN MEMORY LEAK HERE
        //context.UpdateMesh(context.m_mesh);
        GameInstance->draw();
        if (GameInstance->camera.updated)
        {
            GameInstance->camera.updateViewMatrix();
            GameInstance->updateUniformBuffers();
        }
        sinus += 0.01f;
    }
    GameInstance->CleanUp();
}
