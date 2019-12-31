#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VContext.h>
#include <VMesh.h>
#include <VObject.h>
#include <VLight.h>

#include <vector>

class Game
{
public:
    Game(const int width, const int height) : GameInstance(nullptr), WIDTH(width), HEIGHT(height) {}
    Game() : GameInstance(nullptr) {}
    ~Game() {delete GameInstance;}


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

    VContext* GameInstance;
    std::vector<VObject> m_objects;
    std::vector<VLight> m_lights;
    std::vector<int> trianglesNumber;
    std::vector<float> time{};
};

