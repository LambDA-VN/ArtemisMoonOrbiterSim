#pragma once

#include "Spacecraft.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

// Derivative function type: takes state, returns derivatives
using DerivativeFunc = std::function<void(const SpacecraftState&, glm::dvec3&, glm::dvec3&)>;

class Integrator {
public:
    enum class Type {
        Euler,
        SemiImplicitEuler,
        RK4
    };
    
    static void step(SpacecraftState& state, double dt, Type type, 
                    const DerivativeFunc& computeDerivatives);
    
    // Predict future trajectory (no thrust)
    static std::vector<glm::dvec3> predictTrajectory(
        const SpacecraftState& initialState,
        double duration,
        double dt,
        int maxSteps,
        const DerivativeFunc& computeDerivatives,
        double bodyRadius = 0.0);  // Stop if hits body
    
private:
    static void stepEuler(SpacecraftState& state, double dt, 
                         const DerivativeFunc& computeDerivatives);
    static void stepSemiImplicitEuler(SpacecraftState& state, double dt,
                                     const DerivativeFunc& computeDerivatives);
    static void stepRK4(SpacecraftState& state, double dt,
                       const DerivativeFunc& computeDerivatives);
};
