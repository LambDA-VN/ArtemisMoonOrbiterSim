#pragma once

#include "physics/Spacecraft.h"
#include "physics/Orbit.h"
#include "core/Time.h"
#include "render/Camera.h"
#include <deque>
#include <functional>

struct GLFWwindow;

class Ui {
public:
    bool init(GLFWwindow* window);
    void shutdown();
    
    void beginFrame();
    void endFrame();
    
    // Render all UI panels
    void render(const SpacecraftState& state, const OrbitalElements& elements,
               Time& time, Camera& camera, bool& showOrbitPath,
               bool& showVelocityVector, bool& showThrustVector);
    
    // Command callbacks
    using ResetCallback = std::function<void(int scenarioIndex)>;
    using BurnCallback = std::function<void(Spacecraft::ThrustMode mode, float throttle)>;
    
    void setResetCallback(ResetCallback callback) { m_resetCallback = callback; }
    void setBurnCallback(BurnCallback callback) { m_burnCallback = callback; }
    
    // Telemetry history for graphs
    void recordTelemetry(double simTime, double altitude, double speed, double eccentricity);
    
    // Integrator selection
    int getSelectedIntegrator() const { return m_selectedIntegrator; }
    
    // Thrust settings
    float getThrottle() const { return m_throttle; }
    Spacecraft::ThrustMode getThrustMode() const { return m_thrustMode; }
    bool isBurnActive() const { return m_burnActive; }
    
    // Impact screen
    bool isImpactOccurred() const { return m_impactOccurred; }
    void setImpactOccurred(bool impact) { m_impactOccurred = impact; }
    
private:
    void renderSimulationControls(Time& time);
    void renderTelemetry(const SpacecraftState& state, const OrbitalElements& elements);
    void renderManeuverPlanner(const SpacecraftState& state);
    void renderCameraControls(Camera& camera, bool& showOrbitPath,
                             bool& showVelocityVector, bool& showThrustVector);
    void renderGraphs();
    void renderImpactScreen();
    void renderPerformanceOverlay(const Time& time);
    
    ResetCallback m_resetCallback;
    BurnCallback m_burnCallback;
    
    // UI state
    int m_selectedScenario = 0;
    int m_selectedIntegrator = 2;  // RK4 by default
    int m_selectedCameraMode = 2;  // OrbitAroundMoon by default
    
    // Maneuver planner state
    float m_throttle = 0.0f;
    Spacecraft::ThrustMode m_thrustMode = Spacecraft::ThrustMode::Prograde;
    bool m_burnActive = false;
    float m_burnDuration = 10.0f;
    float m_burnTimeRemaining = 0.0f;
    
    // Impact state
    bool m_impactOccurred = false;
    
    // Telemetry history for graphs
    static constexpr int MAX_HISTORY_POINTS = 500;
    std::deque<double> m_timeHistory;
    std::deque<double> m_altitudeHistory;
    std::deque<double> m_speedHistory;
    std::deque<double> m_eccentricityHistory;
    double m_lastRecordTime = 0.0;
    
    // UI window states
    bool m_showSimControls = true;
    bool m_showTelemetry = true;
    bool m_showManeuverPlanner = true;
    bool m_showCameraControls = true;
    bool m_showGraphs = true;
    bool m_showPerformance = true;
};
