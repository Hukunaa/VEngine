#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Context.h>
#include <Mesh.h>
#include <Object.h>

#include <vector>

class Game
{
public:
    Game(const int width, const int height) : GameInstance(nullptr), WIDTH(width), HEIGHT(height) {}
    Game() : GameInstance(nullptr) {}
    ~Game() {delete GameInstance;}

    VContext* GameInstance;
    std::vector<VObject> m_objects;

    void InitAPI();
    void SetupGame();
    void GameLoop();

    const int WIDTH{0};
    const int HEIGHT{0};
};

