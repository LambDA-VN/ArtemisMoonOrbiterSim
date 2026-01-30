#pragma once

namespace Constants {
    // Moon parameters
    constexpr double MOON_MU = 4902.800066e9;           // m^3/s^2 gravitational parameter
    constexpr double MOON_RADIUS = 1737400.0;           // meters
    
    // Standard gravity (for Isp calculations)
    constexpr double G0 = 9.80665;                      // m/s^2
    
    // Spacecraft defaults
    constexpr double DEFAULT_MASS = 26000.0;            // kg
    constexpr double DEFAULT_DRY_MASS = 18000.0;        // kg
    constexpr double DEFAULT_MAX_THRUST = 25000.0;      // N
    constexpr double DEFAULT_ISP = 320.0;               // s
    
    // Simulation
    constexpr double FIXED_TIMESTEP = 0.02;             // seconds (50 Hz physics)
    constexpr int MAX_TIME_WARP = 100;
    
    // Rendering
    constexpr double RENDER_SCALE = 1000.0;             // 1 render unit = 1 km
    constexpr int ORBIT_PREDICTION_STEPS = 2000;
    constexpr double ORBIT_PREDICTION_HORIZON = 7200.0; // seconds
    
    // Math
    constexpr double PI = 3.14159265358979323846;
    constexpr double TWO_PI = 2.0 * PI;
    constexpr double DEG_TO_RAD = PI / 180.0;
    constexpr double RAD_TO_DEG = 180.0 / PI;
}
