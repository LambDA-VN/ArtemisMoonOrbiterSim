#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct SpacecraftState {
    glm::dvec3 position{0.0};        // meters, Moon-centered inertial frame
    glm::dvec3 velocity{0.0};        // m/s
    glm::dquat attitude{1.0, 0.0, 0.0, 0.0};  // quaternion (w, x, y, z)
    glm::dvec3 angularVelocity{0.0}; // rad/s
    double mass = 26000.0;           // kg
};

class Spacecraft {
public:
    void init();
    void setMass(double mass, double dryMass);
    void setThrust(double maxThrust, double isp);
    
    // State accessors
    const SpacecraftState& getState() const { return m_state; }
    SpacecraftState& getState() { return m_state; }
    void setState(const SpacecraftState& state) { m_state = state; }
    
    // Properties
    double getMass() const { return m_state.mass; }
    double getDryMass() const { return m_dryMass; }
    double getMaxThrust() const { return m_maxThrust; }
    double getIsp() const { return m_isp; }
    double getFuelMass() const { return m_state.mass - m_dryMass; }
    bool hasFuel() const { return m_state.mass > m_dryMass; }
    
    // Thrust
    double getThrottle() const { return m_throttle; }
    void setThrottle(double throttle);
    glm::dvec3 getThrustDirection() const { return m_thrustDirection; }
    void setThrustDirection(const glm::dvec3& direction);
    
    // Thrust direction modes
    enum class ThrustMode {
        Prograde,
        Retrograde,
        RadialIn,
        RadialOut,
        Normal,
        AntiNormal,
        Custom
    };
    ThrustMode getThrustMode() const { return m_thrustMode; }
    void setThrustMode(ThrustMode mode) { m_thrustMode = mode; }
    
    // Compute actual thrust vector based on mode and current state
    glm::dvec3 computeThrustVector() const;
    
    // Apply thrust for a duration (returns mass consumed)
    double applyThrust(double dt);
    
    // Reset to initial state
    void reset();

private:
    SpacecraftState m_state;
    SpacecraftState m_initialState;
    
    double m_dryMass = 18000.0;
    double m_maxThrust = 25000.0;  // N
    double m_isp = 320.0;          // s
    double m_throttle = 0.0;       // 0..1
    glm::dvec3 m_thrustDirection{1.0, 0.0, 0.0};
    ThrustMode m_thrustMode = ThrustMode::Prograde;
};
