#include "Log.h"
#include "SolarSystem.hpp"

#include "GLFW/glfw3.h"

int main() {
    Log::debug("Starting main");

    // WINDOW
    glfwInit();
    {
        SolarSystem solarSystem{};
        solarSystem.Run();
    }
    glfwTerminate();
    return 0;
}
