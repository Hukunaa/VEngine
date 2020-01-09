#pragma once
#include <optix_stubs.h>
#include <basics.h>
#include <VContext.h>
#include <VLight.h>

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

    OptixDeviceContext m_optixDevice;
    OptixDenoiser        m_denoiser{nullptr};
    OptixDenoiserOptions m_dOptions{};
    OptixDenoiserSizes   m_dSizes{};
    CUdeviceptr          m_dState{0};
    CUdeviceptr          m_dScratch{0};
    CUdeviceptr          m_dIntensity{0};
    CUdeviceptr          m_dMinRGB{0};

    //nvvkpp::AllocatorVkExport m_alloc;

    //BufferCuda   m_pixelBufferIn;
    //BufferCuda   m_pixelBufferOut;

    std::vector<VObject> m_objects;
    std::vector<VLight> m_lights;
    std::vector<int> trianglesNumber;
    std::vector<float> time{};
};

