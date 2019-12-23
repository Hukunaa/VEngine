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
    std::vector<int> trianglesNumber;

    void InitAPI();
    void SetupGame();
    void GameLoop();

    VObject* FindObject(const char* name)
    {
        for(auto& obj : m_objects)
            if(obj.GetName() == name)
                return &obj;

        return nullptr;
    }
    const int WIDTH{0};
    const int HEIGHT{0};
};

