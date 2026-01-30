#include "Application.h"
#include "Constants.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <chrono>

// Static instance for callbacks
static Application* s_instance = nullptr;

bool Application::init(int width, int height, const char* title) {
    s_instance = this;
    m_width = width;
    m_height = height;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);  // Enable vsync
    
    // Set callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    
    // Initialize glad
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize glad" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Initialize subsystems
    m_time.init();
    m_spacecraft.init();
    
    if (!m_renderer.init(width, height)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    if (!m_ui.init(m_window)) {
        std::cerr << "Failed to initialize UI" << std::endl;
        return false;
    }
    
    // Set UI callbacks
    m_ui.setResetCallback([this](int scenarioIndex) {
        initScenario(scenarioIndex);
    });
    
    // Initialize default scenario
    initScenario(0);
    
    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        m_time.update();
        
        processInput();
        update();
        render();
        
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::shutdown() {
    m_ui.shutdown();
    m_renderer.shutdown();
    
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Application::processInput() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void Application::update() {
    if (m_ui.isImpactOccurred() || m_time.isPaused()) {
        return;
    }
    
    auto physicsStart = std::chrono::high_resolution_clock::now();
    
    // Fixed timestep physics
    double dt = Constants::FIXED_TIMESTEP;
    double frameTime = m_time.getDeltaTime() * m_time.getTimeWarp();
    m_physicsAccumulator += frameTime;
    
    // Limit accumulator to prevent spiral of death
    const double maxAccumulator = 0.5;
    if (m_physicsAccumulator > maxAccumulator) {
        m_physicsAccumulator = maxAccumulator;
    }
    
    // Handle burn timing from UI
    if (m_ui.isBurnActive()) {
        // The burn is active, throttle is set from UI
    } else {
        // Reset throttle when burn is not active
        m_spacecraft.setThrottle(0.0f);
    }
    
    // Get integrator type from UI
    Integrator::Type integratorType = static_cast<Integrator::Type>(m_ui.getSelectedIntegrator());
    
    // Set thrust parameters from UI
    m_spacecraft.setThrustMode(m_ui.getThrustMode());
    if (m_ui.isBurnActive()) {
        m_spacecraft.setThrottle(m_ui.getThrottle());
    }
    
    while (m_physicsAccumulator >= dt) {
        // Apply thrust acceleration if burning
        glm::dvec3 thrustAccel(0.0);
        if (m_spacecraft.getThrottle() > 0.0 && m_spacecraft.hasFuel()) {
            glm::dvec3 thrustForce = m_spacecraft.computeThrustVector();
            thrustAccel = thrustForce / m_spacecraft.getMass();
            m_spacecraft.applyThrust(dt);
        }
        
        // Integration step with combined forces
        SpacecraftState& state = m_spacecraft.getState();
        
        auto derivatives = [this, thrustAccel](const SpacecraftState& s, 
                                               glm::dvec3& accel, glm::dvec3& velDeriv) {
            computeDerivatives(s, accel, velDeriv);
            accel += thrustAccel;
        };
        
        Integrator::step(state, dt, integratorType, derivatives);
        
        // Check for collision with Moon surface
        double altitude = Orbit::computeAltitude(state.position, Constants::MOON_RADIUS);
        if (altitude <= 0) {
            m_ui.setImpactOccurred(true);
            m_spacecraft.setThrottle(0.0f);
            break;
        }
        
        m_physicsAccumulator -= dt;
    }
    
    // Update orbital elements
    m_currentElements = Orbit::computeElements(
        m_spacecraft.getState().position,
        m_spacecraft.getState().velocity,
        Constants::MOON_MU
    );
    
    // Record telemetry
    double altitude = Orbit::computeAltitude(m_spacecraft.getState().position, Constants::MOON_RADIUS);
    double speed = glm::length(m_spacecraft.getState().velocity);
    m_ui.recordTelemetry(m_time.getSimulationTime(), altitude, speed, m_currentElements.eccentricity);
    
    // Update trajectory prediction periodically
    m_trajectoryUpdateTimer += m_time.getDeltaTime();
    if (m_trajectoryUpdateTimer >= 0.5) {  // Update every 0.5 real seconds
        updateTrajectoryPrediction();
        m_trajectoryUpdateTimer = 0.0;
    }
    
    // Update camera target for chase mode
    m_renderer.getCamera().setTarget(glm::vec3(m_spacecraft.getState().position / Constants::RENDER_SCALE));
    m_renderer.getCamera().setTargetVelocity(glm::vec3(m_spacecraft.getState().velocity));
    m_renderer.getCamera().update(static_cast<float>(m_time.getDeltaTime()));
    
    auto physicsEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> physicsTime = physicsEnd - physicsStart;
    m_time.setPhysicsTime(physicsTime.count());
}

void Application::render() {
    auto renderStart = std::chrono::high_resolution_clock::now();
    
    m_renderer.beginFrame();
    
    // Render 3D scene
    m_renderer.renderMoon();
    m_renderer.renderSpacecraft(m_spacecraft.getState(), 
                                static_cast<float>(m_spacecraft.getThrottle()));
    
    // Render orbit path
    if (m_renderer.getShowOrbitPath()) {
        m_renderer.renderOrbitPath(m_predictedTrajectory, glm::vec3(0.0f, 1.0f, 0.5f));
    }
    
    // Render velocity vector
    if (m_renderer.getShowVelocityVector()) {
        m_renderer.renderVector(m_spacecraft.getState().position,
                               m_spacecraft.getState().velocity,
                               50.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
    
    m_renderer.endFrame();
    
    // Render UI
    bool showOrbit = m_renderer.getShowOrbitPath();
    bool showVel = m_renderer.getShowVelocityVector();
    bool showThrust = m_renderer.getShowThrustVector();
    
    m_ui.beginFrame();
    m_ui.render(m_spacecraft.getState(), m_currentElements, m_time, 
               m_renderer.getCamera(), showOrbit, showVel, showThrust);
    m_ui.endFrame();
    
    m_renderer.setShowOrbitPath(showOrbit);
    m_renderer.setShowVelocityVector(showVel);
    m_renderer.setShowThrustVector(showThrust);
    
    auto renderEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> renderTime = renderEnd - renderStart;
    m_time.setRenderTime(renderTime.count());
}

void Application::initScenario(int index) {
    m_spacecraft.init();
    m_time.reset();
    m_physicsAccumulator = 0.0;
    
    SpacecraftState& state = m_spacecraft.getState();
    
    switch (index) {
        case 0:  // Circular Low Lunar Orbit (100km)
            Orbit::createCircularOrbit(
                100000.0,  // 100 km altitude
                28.0 * Constants::DEG_TO_RAD,  // inclination
                0.0,  // RAAN
                0.0,  // true anomaly
                Constants::MOON_MU,
                Constants::MOON_RADIUS,
                state.position,
                state.velocity
            );
            break;
            
        case 1:  // Elliptical Capture Orbit
            Orbit::createEllipticalOrbit(
                100000.0,   // 100 km periapsis
                5000000.0,  // 5000 km apoapsis
                90.0 * Constants::DEG_TO_RAD,  // polar orbit
                0.0,  // RAAN
                0.0,  // arg of periapsis
                180.0 * Constants::DEG_TO_RAD,  // start at apoapsis
                Constants::MOON_MU,
                Constants::MOON_RADIUS,
                state.position,
                state.velocity
            );
            break;
            
        case 2:  // Near Surface Skimming
            Orbit::createEllipticalOrbit(
                15000.0,   // 15 km periapsis
                120000.0,  // 120 km apoapsis
                45.0 * Constants::DEG_TO_RAD,
                0.0,
                0.0,
                0.0,  // start at periapsis
                Constants::MOON_MU,
                Constants::MOON_RADIUS,
                state.position,
                state.velocity
            );
            break;
    }
    
    // Update elements and trajectory
    m_currentElements = Orbit::computeElements(state.position, state.velocity, Constants::MOON_MU);
    updateTrajectoryPrediction();
    
    std::cout << "Scenario " << index << " initialized" << std::endl;
    std::cout << "  Position: " << state.position.x << ", " << state.position.y << ", " << state.position.z << std::endl;
    std::cout << "  Velocity: " << state.velocity.x << ", " << state.velocity.y << ", " << state.velocity.z << std::endl;
    std::cout << "  Altitude: " << (glm::length(state.position) - Constants::MOON_RADIUS) / 1000.0 << " km" << std::endl;
}

void Application::computeDerivatives(const SpacecraftState& state,
                                    glm::dvec3& outAccel, glm::dvec3& outVelDeriv) {
    // Two-body gravity: a = -mu * r / |r|^3
    double r = glm::length(state.position);
    if (r > 1.0) {  // Avoid division by zero
        outAccel = -Constants::MOON_MU * state.position / (r * r * r);
    } else {
        outAccel = glm::dvec3(0.0);
    }
    outVelDeriv = state.velocity;
}

void Application::updateTrajectoryPrediction() {
    auto derivatives = [this](const SpacecraftState& s, 
                             glm::dvec3& accel, glm::dvec3& velDeriv) {
        computeDerivatives(s, accel, velDeriv);
    };
    
    double predictionDt = Constants::ORBIT_PREDICTION_HORIZON / Constants::ORBIT_PREDICTION_STEPS;
    
    m_predictedTrajectory = Integrator::predictTrajectory(
        m_spacecraft.getState(),
        Constants::ORBIT_PREDICTION_HORIZON,
        predictionDt,
        Constants::ORBIT_PREDICTION_STEPS,
        derivatives,
        Constants::MOON_RADIUS
    );
}

// GLFW Callbacks
void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    if (s_instance) {
        s_instance->m_width = width;
        s_instance->m_height = height;
        s_instance->m_renderer.resize(width, height);
    }
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;
    
    if (!s_instance) return;
    
    // Let ImGui handle key input first
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;
    
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    
    // Camera controls
    s_instance->m_renderer.getCamera().processKeyboard(key, pressed);
    
    // Simulation controls (only on key press, not repeat)
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_SPACE:
                s_instance->m_time.togglePause();
                break;
            case GLFW_KEY_LEFT_BRACKET:
                s_instance->m_time.decreaseTimeWarp();
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                s_instance->m_time.increaseTimeWarp();
                break;
            case GLFW_KEY_O:
                s_instance->m_renderer.setShowOrbitPath(!s_instance->m_renderer.getShowOrbitPath());
                break;
            case GLFW_KEY_R:
                s_instance->initScenario(0);
                s_instance->m_ui.setImpactOccurred(false);
                break;
        }
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    
    if (!s_instance) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        s_instance->m_rightMousePressed = (action == GLFW_PRESS);
        if (action == GLFW_PRESS) {
            s_instance->m_firstMouse = true;
        }
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    
    if (!s_instance) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (s_instance->m_firstMouse) {
        s_instance->m_lastMouseX = xpos;
        s_instance->m_lastMouseY = ypos;
        s_instance->m_firstMouse = false;
    }
    
    float xOffset = static_cast<float>(xpos - s_instance->m_lastMouseX);
    float yOffset = static_cast<float>(s_instance->m_lastMouseY - ypos);  // Reversed
    
    s_instance->m_lastMouseX = xpos;
    s_instance->m_lastMouseY = ypos;
    
    s_instance->m_renderer.getCamera().processMouse(xOffset, yOffset, s_instance->m_rightMousePressed);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    
    if (!s_instance) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    s_instance->m_renderer.getCamera().processScroll(static_cast<float>(yoffset));
}
