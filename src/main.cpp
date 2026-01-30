#include "core/Application.h"
#include <iostream>

int main() {
    Application app;
    
    if (!app.init(1280, 720, "Artemis Moon Orbiter Simulation")) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    std::cout << "Application initialized successfully" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Space: Toggle pause" << std::endl;
    std::cout << "  [ / ]: Decrease/Increase time warp" << std::endl;
    std::cout << "  O: Toggle orbit path" << std::endl;
    std::cout << "  R: Reset simulation" << std::endl;
    std::cout << "  Right Mouse + Drag: Rotate camera" << std::endl;
    std::cout << "  Scroll: Zoom in/out" << std::endl;
    std::cout << "  WASD/QE: Move camera (in free fly mode)" << std::endl;
    std::cout << std::endl;
    
    app.run();
    app.shutdown();
    
    return 0;
}
