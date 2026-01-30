#pragma once

#include "Time.h"
#include "physics/Spacecraft.h"
#include "physics/Integrator.h"
#include "physics/Orbit.h"
#include "render/Renderer.h"
#include "ui/Ui.h"
#include <vector>

struct GLFWwindow;

class Application {
public:
    bool init(int width, int height, const char* title);
    void run();
    void shutdown();
    
private:
    void processInput();
    void update();
    void render();
    
    void initScenario(int index);
    void computeDerivatives(const SpacecraftState& state, 
                           glm::dvec3& outAccel, glm::dvec3& outVelDeriv);
    void updateTrajectoryPrediction();
    
    GLFWwindow* m_window = nullptr;
    int m_width = 1280;
    int m_height = 720;
    
    Time m_time;
    Spacecraft m_spacecraft;
    Renderer m_renderer;
    Ui m_ui;
    
    OrbitalElements m_currentElements;
    std::vector<glm::dvec3> m_predictedTrajectory;
    
    double m_physicsAccumulator = 0.0;
    double m_trajectoryUpdateTimer = 0.0;
    
    // Mouse state
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouse = true;
    bool m_rightMousePressed = false;
    
    // Scenario definitions
    static constexpr int NUM_SCENARIOS = 3;
    
    // Callbacks
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};
