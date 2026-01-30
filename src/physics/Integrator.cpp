#include "Integrator.h"
#include "core/Constants.h"
#include <cmath>

void Integrator::step(SpacecraftState& state, double dt, Type type,
                     const DerivativeFunc& computeDerivatives) {
    switch (type) {
        case Type::Euler:
            stepEuler(state, dt, computeDerivatives);
            break;
        case Type::SemiImplicitEuler:
            stepSemiImplicitEuler(state, dt, computeDerivatives);
            break;
        case Type::RK4:
        default:
            stepRK4(state, dt, computeDerivatives);
            break;
    }
}

void Integrator::stepEuler(SpacecraftState& state, double dt,
                          const DerivativeFunc& computeDerivatives) {
    glm::dvec3 accel(0.0);
    glm::dvec3 velocityDeriv(0.0);
    computeDerivatives(state, accel, velocityDeriv);
    
    state.position += state.velocity * dt;
    state.velocity += accel * dt;
}

void Integrator::stepSemiImplicitEuler(SpacecraftState& state, double dt,
                                      const DerivativeFunc& computeDerivatives) {
    glm::dvec3 accel(0.0);
    glm::dvec3 velocityDeriv(0.0);
    computeDerivatives(state, accel, velocityDeriv);
    
    // Update velocity first, then position
    state.velocity += accel * dt;
    state.position += state.velocity * dt;
}

void Integrator::stepRK4(SpacecraftState& state, double dt,
                        const DerivativeFunc& computeDerivatives) {
    // k1
    glm::dvec3 k1v(0.0), k1a(0.0);
    computeDerivatives(state, k1a, k1v);
    k1v = state.velocity;
    
    // k2
    SpacecraftState state2 = state;
    state2.position = state.position + k1v * (dt * 0.5);
    state2.velocity = state.velocity + k1a * (dt * 0.5);
    glm::dvec3 k2v(0.0), k2a(0.0);
    computeDerivatives(state2, k2a, k2v);
    k2v = state2.velocity;
    
    // k3
    SpacecraftState state3 = state;
    state3.position = state.position + k2v * (dt * 0.5);
    state3.velocity = state.velocity + k2a * (dt * 0.5);
    glm::dvec3 k3v(0.0), k3a(0.0);
    computeDerivatives(state3, k3a, k3v);
    k3v = state3.velocity;
    
    // k4
    SpacecraftState state4 = state;
    state4.position = state.position + k3v * dt;
    state4.velocity = state.velocity + k3a * dt;
    glm::dvec3 k4v(0.0), k4a(0.0);
    computeDerivatives(state4, k4a, k4v);
    k4v = state4.velocity;
    
    // Combine
    state.position += (k1v + 2.0 * k2v + 2.0 * k3v + k4v) * (dt / 6.0);
    state.velocity += (k1a + 2.0 * k2a + 2.0 * k3a + k4a) * (dt / 6.0);
}

std::vector<glm::dvec3> Integrator::predictTrajectory(
    const SpacecraftState& initialState,
    double duration,
    double dt,
    int maxSteps,
    const DerivativeFunc& computeDerivatives,
    double bodyRadius) {
    
    std::vector<glm::dvec3> trajectory;
    trajectory.reserve(maxSteps);
    
    SpacecraftState state = initialState;
    double t = 0.0;
    int steps = 0;
    
    trajectory.push_back(state.position);
    
    while (t < duration && steps < maxSteps) {
        stepRK4(state, dt, computeDerivatives);
        t += dt;
        steps++;
        
        // Check for collision with body
        if (bodyRadius > 0.0 && glm::length(state.position) <= bodyRadius) {
            trajectory.push_back(state.position);
            break;
        }
        
        trajectory.push_back(state.position);
    }
    
    return trajectory;
}
