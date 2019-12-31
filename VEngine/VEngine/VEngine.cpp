#include <iostream>
#include <functional>
#include <cstdlib>
#include <VGame.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define WIDTH 1280
#define HEIGHT 720


int main()
{
    Game game(WIDTH, HEIGHT);

    try
    {
        game.InitAPI();
        game.SetupGame();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
