#include "Spacecraft.h"
#include "core/Constants.h"
#include <algorithm>
#include <cmath>

void Spacecraft::init() {
    m_state = SpacecraftState{};
    m_state.mass = Constants::DEFAULT_MASS;
    m_dryMass = Constants::DEFAULT_DRY_MASS;
    m_maxThrust = Constants::DEFAULT_MAX_THRUST;
    m_isp = Constants::DEFAULT_ISP;
    m_throttle = 0.0;
    m_thrustMode = ThrustMode::Prograde;
    m_initialState = m_state;
}

void Spacecraft::setMass(double mass, double dryMass) {
    m_state.mass = std::max(mass, dryMass);
    m_dryMass = dryMass;
}

void Spacecraft::setThrust(double maxThrust, double isp) {
    m_maxThrust = maxThrust;
    m_isp = isp;
}

void Spacecraft::setThrottle(double throttle) {
    m_throttle = std::clamp(throttle, 0.0, 1.0);
}

void Spacecraft::setThrustDirection(const glm::dvec3& direction) {
    double len = glm::length(direction);
    if (len > 1e-10) {
        m_thrustDirection = direction / len;
    }
}

glm::dvec3 Spacecraft::computeThrustVector() const {
    if (m_throttle < 1e-10 || !hasFuel()) {
        return glm::dvec3(0.0);
    }
    
    glm::dvec3 direction;
    
    // Compute direction vectors based on orbital mechanics
    glm::dvec3 radial = glm::normalize(m_state.position);
    glm::dvec3 prograde = glm::normalize(m_state.velocity);
    glm::dvec3 normal = glm::cross(radial, prograde);
    if (glm::length(normal) > 1e-10) {
        normal = glm::normalize(normal);
    } else {
        normal = glm::dvec3(0.0, 0.0, 1.0);
    }
    
    switch (m_thrustMode) {
        case ThrustMode::Prograde:
            direction = prograde;
            break;
        case ThrustMode::Retrograde:
            direction = -prograde;
            break;
        case ThrustMode::RadialIn:
            direction = -radial;
            break;
        case ThrustMode::RadialOut:
            direction = radial;
            break;
        case ThrustMode::Normal:
            direction = normal;
            break;
        case ThrustMode::AntiNormal:
            direction = -normal;
            break;
        case ThrustMode::Custom:
        default:
            direction = m_thrustDirection;
            break;
    }
    
    return direction * m_throttle * m_maxThrust;
}

double Spacecraft::applyThrust(double dt) {
    if (m_throttle < 1e-10 || !hasFuel()) {
        return 0.0;
    }
    
    double thrust = m_throttle * m_maxThrust;
    double massFlowRate = thrust / (m_isp * Constants::G0);
    double massConsumed = massFlowRate * dt;
    
    // Don't consume more fuel than we have
    double fuelAvailable = m_state.mass - m_dryMass;
    massConsumed = std::min(massConsumed, fuelAvailable);
    
    m_state.mass -= massConsumed;
    m_state.mass = std::max(m_state.mass, m_dryMass);
    
    return massConsumed;
}

void Spacecraft::reset() {
    m_state = m_initialState;
    m_throttle = 0.0;
}
