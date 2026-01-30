#pragma once

#include <glm/glm.hpp>

struct OrbitalElements {
    double semiMajorAxis = 0.0;      // meters
    double eccentricity = 0.0;
    double inclination = 0.0;        // radians
    double raan = 0.0;               // Right Ascension of Ascending Node (radians)
    double argOfPeriapsis = 0.0;     // radians
    double trueAnomaly = 0.0;        // radians
    double periapsisAltitude = 0.0;  // meters
    double apoapsisAltitude = 0.0;   // meters
    double orbitalPeriod = 0.0;      // seconds
    double specificEnergy = 0.0;     // J/kg
    double angularMomentum = 0.0;    // m²/s
};

class Orbit {
public:
    // Compute orbital elements from state vector
    static OrbitalElements computeElements(const glm::dvec3& position, 
                                           const glm::dvec3& velocity, 
                                           double mu);
    
    // Compute state vector from orbital elements
    static void computeStateFromElements(const OrbitalElements& elements,
                                        double mu,
                                        glm::dvec3& outPosition,
                                        glm::dvec3& outVelocity);
    
    // Create initial state from orbital parameters
    static void createCircularOrbit(double altitude, double inclination, 
                                   double raan, double trueAnomaly,
                                   double mu, double bodyRadius,
                                   glm::dvec3& outPosition,
                                   glm::dvec3& outVelocity);
    
    static void createEllipticalOrbit(double periapsisAltitude, 
                                     double apoapsisAltitude,
                                     double inclination, double raan,
                                     double argOfPeriapsis, double trueAnomaly,
                                     double mu, double bodyRadius,
                                     glm::dvec3& outPosition,
                                     glm::dvec3& outVelocity);
    
    // Utility functions
    static double computeOrbitalVelocity(double radius, double mu, double semiMajorAxis);
    static double computeCircularVelocity(double radius, double mu);
    static double computeEscapeVelocity(double radius, double mu);
    static double computePeriod(double semiMajorAxis, double mu);
    
    // Helper to compute altitude from position
    static double computeAltitude(const glm::dvec3& position, double bodyRadius);
};
