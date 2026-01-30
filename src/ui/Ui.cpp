#include "Ui.h"
#include "core/Constants.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cmath>

bool Ui::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    // Customize style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.Alpha = 0.95f;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    return true;
}

void Ui::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Ui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Ui::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Ui::render(const SpacecraftState& state, const OrbitalElements& elements,
               Time& time, Camera& camera, bool& showOrbitPath,
               bool& showVelocityVector, bool& showThrustVector) {
    
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Simulation Controls", nullptr, &m_showSimControls);
            ImGui::MenuItem("Telemetry", nullptr, &m_showTelemetry);
            ImGui::MenuItem("Maneuver Planner", nullptr, &m_showManeuverPlanner);
            ImGui::MenuItem("Camera", nullptr, &m_showCameraControls);
            ImGui::MenuItem("Graphs", nullptr, &m_showGraphs);
            ImGui::MenuItem("Performance", nullptr, &m_showPerformance);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    if (m_showSimControls) renderSimulationControls(time);
    if (m_showTelemetry) renderTelemetry(state, elements);
    if (m_showManeuverPlanner) renderManeuverPlanner(state);
    if (m_showCameraControls) renderCameraControls(camera, showOrbitPath, showVelocityVector, showThrustVector);
    if (m_showGraphs) renderGraphs();
    if (m_showPerformance) renderPerformanceOverlay(time);
    
    if (m_impactOccurred) {
        renderImpactScreen();
    }
}

void Ui::renderSimulationControls(Time& time) {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 200), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Simulation Controls", &m_showSimControls)) {
        // Play/Pause
        bool paused = time.isPaused();
        if (ImGui::Button(paused ? "▶ Play" : "⏸ Pause", ImVec2(100, 0))) {
            time.togglePause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset", ImVec2(80, 0))) {
            if (m_resetCallback) {
                m_resetCallback(m_selectedScenario);
            }
            m_impactOccurred = false;
        }
        
        // Time warp
        ImGui::Separator();
        ImGui::Text("Time Warp: %dx", time.getTimeWarp());
        if (ImGui::Button("[-]")) {
            time.decreaseTimeWarp();
        }
        ImGui::SameLine();
        if (ImGui::Button("[+]")) {
            time.increaseTimeWarp();
        }
        ImGui::SameLine();
        if (ImGui::Button("1x")) {
            time.setTimeWarp(1);
        }
        
        // Scenario selector
        ImGui::Separator();
        const char* scenarios[] = { 
            "Circular Low Lunar Orbit (100km)",
            "Elliptical Capture Orbit",
            "Near Surface Skimming"
        };
        if (ImGui::Combo("Scenario", &m_selectedScenario, scenarios, 3)) {
            if (m_resetCallback) {
                m_resetCallback(m_selectedScenario);
            }
            m_impactOccurred = false;
        }
        
        // Integrator selector
        const char* integrators[] = { "Euler", "Semi-Implicit Euler", "RK4" };
        ImGui::Combo("Integrator", &m_selectedIntegrator, integrators, 3);
        
        // Fixed timestep display
        ImGui::Text("Fixed dt: %.3f s", Constants::FIXED_TIMESTEP);
        
        // Simulation time
        double simTime = time.getSimulationTime();
        int hours = static_cast<int>(simTime / 3600.0);
        int minutes = static_cast<int>(fmod(simTime, 3600.0) / 60.0);
        double seconds = fmod(simTime, 60.0);
        ImGui::Text("Sim Time: %02d:%02d:%05.2f", hours, minutes, seconds);
    }
    ImGui::End();
}

void Ui::renderTelemetry(const SpacecraftState& state, const OrbitalElements& elements) {
    ImGui::SetNextWindowPos(ImVec2(10, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 350), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Orbit & Telemetry", &m_showTelemetry)) {
        // Position
        ImGui::Text("Position (km):");
        ImGui::Text("  X: %.2f", state.position.x / 1000.0);
        ImGui::Text("  Y: %.2f", state.position.y / 1000.0);
        ImGui::Text("  Z: %.2f", state.position.z / 1000.0);
        
        // Velocity
        double speed = glm::length(state.velocity);
        ImGui::Separator();
        ImGui::Text("Velocity (m/s):");
        ImGui::Text("  Vx: %.2f", state.velocity.x);
        ImGui::Text("  Vy: %.2f", state.velocity.y);
        ImGui::Text("  Vz: %.2f", state.velocity.z);
        ImGui::Text("  Speed: %.2f", speed);
        
        // Altitude
        double altitude = glm::length(state.position) - Constants::MOON_RADIUS;
        ImGui::Separator();
        ImGui::Text("Altitude: %.2f km", altitude / 1000.0);
        
        // Orbital elements
        ImGui::Separator();
        ImGui::Text("Orbital Elements:");
        ImGui::Text("  a: %.2f km", elements.semiMajorAxis / 1000.0);
        ImGui::Text("  e: %.6f", elements.eccentricity);
        ImGui::Text("  i: %.2f°", elements.inclination * Constants::RAD_TO_DEG);
        ImGui::Text("  Ω: %.2f°", elements.raan * Constants::RAD_TO_DEG);
        ImGui::Text("  ω: %.2f°", elements.argOfPeriapsis * Constants::RAD_TO_DEG);
        ImGui::Text("  ν: %.2f°", elements.trueAnomaly * Constants::RAD_TO_DEG);
        
        // Apsides
        ImGui::Separator();
        ImGui::Text("Periapsis Alt: %.2f km", elements.periapsisAltitude / 1000.0);
        ImGui::Text("Apoapsis Alt: %.2f km", elements.apoapsisAltitude / 1000.0);
        
        // Period
        if (elements.orbitalPeriod > 0 && elements.orbitalPeriod < 1e10) {
            int periodMin = static_cast<int>(elements.orbitalPeriod / 60.0);
            double periodSec = fmod(elements.orbitalPeriod, 60.0);
            ImGui::Text("Period: %d min %.1f s", periodMin, periodSec);
        } else {
            ImGui::Text("Period: N/A (escape)");
        }
        
        // Energy & Angular Momentum
        ImGui::Separator();
        ImGui::Text("Specific Energy: %.0f J/kg", elements.specificEnergy);
        ImGui::Text("Ang. Momentum: %.0f m²/s", elements.angularMomentum);
        
        // Mass
        ImGui::Separator();
        ImGui::Text("Mass: %.1f kg", state.mass);
    }
    ImGui::End();
}

void Ui::renderManeuverPlanner(const SpacecraftState& state) {
    ImGui::SetNextWindowPos(ImVec2(300, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 230), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Maneuver Planner", &m_showManeuverPlanner)) {
        // Thrust mode selector
        const char* modes[] = { 
            "Prograde", "Retrograde", "Radial In", "Radial Out", "Normal", "Anti-Normal"
        };
        int modeInt = static_cast<int>(m_thrustMode);
        if (ImGui::Combo("Burn Direction", &modeInt, modes, 6)) {
            m_thrustMode = static_cast<Spacecraft::ThrustMode>(modeInt);
        }
        
        // Throttle slider
        ImGui::SliderFloat("Throttle", &m_throttle, 0.0f, 1.0f, "%.2f");
        
        // Burn duration
        ImGui::InputFloat("Duration (s)", &m_burnDuration, 1.0f, 10.0f, "%.1f");
        m_burnDuration = std::max(0.1f, m_burnDuration);
        
        // Execute burn button
        ImGui::Separator();
        if (!m_burnActive) {
            if (ImGui::Button("Execute Burn", ImVec2(-1, 30))) {
                m_burnActive = true;
                m_burnTimeRemaining = m_burnDuration;
            }
        } else {
            ImGui::ProgressBar(1.0f - m_burnTimeRemaining / m_burnDuration, 
                              ImVec2(-1, 20), "Burning...");
            if (ImGui::Button("Cancel Burn", ImVec2(-1, 25))) {
                m_burnActive = false;
                m_throttle = 0.0f;
            }
            ImGui::Text("Time remaining: %.1f s", m_burnTimeRemaining);
        }
        
        // Fuel info
        ImGui::Separator();
        double fuelMass = state.mass - Constants::DEFAULT_DRY_MASS;
        ImGui::Text("Fuel remaining: %.1f kg", fuelMass);
        if (fuelMass <= 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "OUT OF FUEL!");
        }
    }
    ImGui::End();
}

void Ui::renderCameraControls(Camera& camera, bool& showOrbitPath,
                             bool& showVelocityVector, bool& showThrustVector) {
    ImGui::SetNextWindowPos(ImVec2(560, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 200), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Camera", &m_showCameraControls)) {
        // Camera mode
        const char* cameraModes[] = { "Free Fly", "Chase", "Orbit Moon", "Top Down" };
        if (ImGui::Combo("Mode", &m_selectedCameraMode, cameraModes, 4)) {
            camera.setMode(static_cast<Camera::Mode>(m_selectedCameraMode));
        }
        
        // FOV slider
        float fov = camera.getFov();
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 90.0f)) {
            camera.setFov(fov);
        }
        
        // Orbit distance (for orbit/topdown modes)
        if (m_selectedCameraMode == 2 || m_selectedCameraMode == 3) {
            float dist = camera.getOrbitDistance();
            if (ImGui::SliderFloat("Distance", &dist, 100.0f, 20000.0f, "%.0f km")) {
                camera.setOrbitDistance(dist);
            }
        }
        
        // Visibility toggles
        ImGui::Separator();
        ImGui::Checkbox("Show Orbit Path", &showOrbitPath);
        ImGui::Checkbox("Show Velocity Vector", &showVelocityVector);
        ImGui::Checkbox("Show Thrust Vector", &showThrustVector);
    }
    ImGui::End();
}

void Ui::renderGraphs() {
    ImGui::SetNextWindowPos(ImVec2(790, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Graphs", &m_showGraphs)) {
        // Convert deques to vectors for plotting
        std::vector<float> altitudes(m_altitudeHistory.begin(), m_altitudeHistory.end());
        std::vector<float> speeds(m_speedHistory.begin(), m_speedHistory.end());
        std::vector<float> eccentricities(m_eccentricityHistory.begin(), m_eccentricityHistory.end());
        
        // Altitude plot
        if (!altitudes.empty()) {
            float minAlt = *std::min_element(altitudes.begin(), altitudes.end());
            float maxAlt = *std::max_element(altitudes.begin(), altitudes.end());
            ImGui::Text("Altitude (km)");
            ImGui::PlotLines("##altitude", altitudes.data(), 
                           static_cast<int>(altitudes.size()), 0, nullptr, 
                           minAlt - 10.0f, maxAlt + 10.0f, ImVec2(-1, 80));
        }
        
        // Speed plot
        if (!speeds.empty()) {
            float minSpd = *std::min_element(speeds.begin(), speeds.end());
            float maxSpd = *std::max_element(speeds.begin(), speeds.end());
            ImGui::Text("Speed (m/s)");
            ImGui::PlotLines("##speed", speeds.data(), 
                           static_cast<int>(speeds.size()), 0, nullptr,
                           minSpd - 10.0f, maxSpd + 10.0f, ImVec2(-1, 80));
        }
        
        // Eccentricity plot
        if (!eccentricities.empty()) {
            float minEcc = *std::min_element(eccentricities.begin(), eccentricities.end());
            float maxEcc = *std::max_element(eccentricities.begin(), eccentricities.end());
            ImGui::Text("Eccentricity");
            ImGui::PlotLines("##eccentricity", eccentricities.data(), 
                           static_cast<int>(eccentricities.size()), 0, nullptr,
                           minEcc - 0.01f, maxEcc + 0.01f, ImVec2(-1, 80));
        }
    }
    ImGui::End();
}

void Ui::renderImpactScreen() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 
                                   ImGui::GetIO().DisplaySize.y * 0.5f),
                           ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
    
    ImGui::Begin("Impact!", nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoCollapse);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("SURFACE IMPACT!");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Separator();
    ImGui::Text("The spacecraft has collided with");
    ImGui::Text("the lunar surface.");
    ImGui::Separator();
    
    if (ImGui::Button("Reset Simulation", ImVec2(-1, 30))) {
        if (m_resetCallback) {
            m_resetCallback(m_selectedScenario);
        }
        m_impactOccurred = false;
    }
    
    ImGui::End();
}

void Ui::renderPerformanceOverlay(const Time& time) {
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 80), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.5f);
    
    if (ImGui::Begin("Performance", &m_showPerformance, 
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        ImGui::Text("Frame: %.2f ms (%.1f FPS)", time.getFrameTime(), 
                   1000.0 / std::max(time.getFrameTime(), 0.001));
        ImGui::Text("Physics: %.2f ms", time.getPhysicsTime());
        ImGui::Text("Render: %.2f ms", time.getRenderTime());
    }
    ImGui::End();
}

void Ui::recordTelemetry(double simTime, double altitude, double speed, double eccentricity) {
    // Record at most once per second of simulation time
    if (simTime - m_lastRecordTime < 1.0) return;
    m_lastRecordTime = simTime;
    
    m_timeHistory.push_back(simTime);
    m_altitudeHistory.push_back(static_cast<float>(altitude / 1000.0));  // Convert to km
    m_speedHistory.push_back(static_cast<float>(speed));
    m_eccentricityHistory.push_back(static_cast<float>(eccentricity));
    
    // Trim history
    while (m_timeHistory.size() > MAX_HISTORY_POINTS) {
        m_timeHistory.pop_front();
        m_altitudeHistory.pop_front();
        m_speedHistory.pop_front();
        m_eccentricityHistory.pop_front();
    }
}
