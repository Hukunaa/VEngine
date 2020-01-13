#pragma once
#include <optional>
#include <optix_stubs.h>
#include <basics.h>
#include <VContext.h>
#include <VLight.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

class Game
{
public:
    Game(const int width, const int height) : GameInstance(nullptr), WIDTH(width), HEIGHT(height) {}
    Game() : GameInstance(nullptr) {}
    ~Game() {delete GameInstance;}


    void InitAPI();
    void SetupGame();
    void GameLoop();
    void InputManager();
    static void CursorCallBack(GLFWwindow *window, double xpos, double ypos );
    void DenoiseImage(const VkImage& imgIn, VkImage& imgOut);
    void InitOptix();
    void SetupIMGUI();
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

    bool mouseControl = false;
    std::vector<VObject> m_objects;
    std::vector<VLight> m_lights;
    std::vector<int> trianglesNumber;
    std::vector<float> time{};
};

