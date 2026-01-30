#include "Orbit.h"
#include "core/Constants.h"
#include <cmath>
#include <algorithm>

OrbitalElements Orbit::computeElements(const glm::dvec3& position, 
                                       const glm::dvec3& velocity, 
                                       double mu) {
    OrbitalElements elements;
    
    double r = glm::length(position);
    double v = glm::length(velocity);
    
    // Specific orbital energy
    elements.specificEnergy = (v * v) / 2.0 - mu / r;
    
    // Angular momentum vector
    glm::dvec3 h = glm::cross(position, velocity);
    double hMag = glm::length(h);
    elements.angularMomentum = hMag;
    
    // Node vector (pointing to ascending node)
    glm::dvec3 k(0.0, 0.0, 1.0);  // Z-axis
    glm::dvec3 n = glm::cross(k, h);
    double nMag = glm::length(n);
    
    // Eccentricity vector
    glm::dvec3 eVec = ((v * v - mu / r) * position - glm::dot(position, velocity) * velocity) / mu;
    elements.eccentricity = glm::length(eVec);
    
    // Semi-major axis
    if (std::abs(elements.eccentricity - 1.0) > 1e-10) {
        elements.semiMajorAxis = -mu / (2.0 * elements.specificEnergy);
    } else {
        // Parabolic orbit
        elements.semiMajorAxis = std::numeric_limits<double>::infinity();
    }
    
    // Inclination
    if (hMag > 1e-10) {
        elements.inclination = std::acos(std::clamp(h.z / hMag, -1.0, 1.0));
    }
    
    // RAAN (Right Ascension of Ascending Node)
    if (nMag > 1e-10) {
        elements.raan = std::acos(std::clamp(n.x / nMag, -1.0, 1.0));
        if (n.y < 0) {
            elements.raan = Constants::TWO_PI - elements.raan;
        }
    }
    
    // Argument of periapsis
    if (nMag > 1e-10 && elements.eccentricity > 1e-10) {
        double eDotN = glm::dot(eVec, n);
        elements.argOfPeriapsis = std::acos(std::clamp(eDotN / (elements.eccentricity * nMag), -1.0, 1.0));
        if (eVec.z < 0) {
            elements.argOfPeriapsis = Constants::TWO_PI - elements.argOfPeriapsis;
        }
    }
    
    // True anomaly
    if (elements.eccentricity > 1e-10) {
        double eDotr = glm::dot(eVec, position);
        elements.trueAnomaly = std::acos(std::clamp(eDotr / (elements.eccentricity * r), -1.0, 1.0));
        if (glm::dot(position, velocity) < 0) {
            elements.trueAnomaly = Constants::TWO_PI - elements.trueAnomaly;
        }
    } else {
        // Circular orbit - measure from ascending node
        if (nMag > 1e-10) {
            double nDotr = glm::dot(n, position);
            elements.trueAnomaly = std::acos(std::clamp(nDotr / (nMag * r), -1.0, 1.0));
            if (position.z < 0) {
                elements.trueAnomaly = Constants::TWO_PI - elements.trueAnomaly;
            }
        }
    }
    
    // Periapsis and apoapsis altitudes
    if (elements.semiMajorAxis > 0 && elements.eccentricity < 1.0) {
        double periapsis = elements.semiMajorAxis * (1.0 - elements.eccentricity);
        double apoapsis = elements.semiMajorAxis * (1.0 + elements.eccentricity);
        elements.periapsisAltitude = periapsis - Constants::MOON_RADIUS;
        elements.apoapsisAltitude = apoapsis - Constants::MOON_RADIUS;
        elements.orbitalPeriod = computePeriod(elements.semiMajorAxis, mu);
    } else if (elements.eccentricity >= 1.0) {
        // Hyperbolic/parabolic orbit
        double periapsis = elements.semiMajorAxis * (1.0 - elements.eccentricity);
        if (elements.semiMajorAxis < 0) {
            periapsis = -elements.semiMajorAxis * (elements.eccentricity - 1.0);
        }
        elements.periapsisAltitude = periapsis - Constants::MOON_RADIUS;
        elements.apoapsisAltitude = std::numeric_limits<double>::infinity();
        elements.orbitalPeriod = std::numeric_limits<double>::infinity();
    }
    
    return elements;
}

void Orbit::computeStateFromElements(const OrbitalElements& elements,
                                    double mu,
                                    glm::dvec3& outPosition,
                                    glm::dvec3& outVelocity) {
    double a = elements.semiMajorAxis;
    double e = elements.eccentricity;
    double i = elements.inclination;
    double omega = elements.raan;
    double w = elements.argOfPeriapsis;
    double nu = elements.trueAnomaly;
    
    // Semi-latus rectum
    double p = a * (1.0 - e * e);
    
    // Position and velocity in perifocal frame
    double r = p / (1.0 + e * std::cos(nu));
    
    glm::dvec3 rPQW(r * std::cos(nu), r * std::sin(nu), 0.0);
    double h = std::sqrt(mu * p);
    glm::dvec3 vPQW(-std::sin(nu), e + std::cos(nu), 0.0);
    vPQW *= mu / h;
    
    // Rotation matrix from perifocal to inertial
    double cosOmega = std::cos(omega);
    double sinOmega = std::sin(omega);
    double cosW = std::cos(w);
    double sinW = std::sin(w);
    double cosI = std::cos(i);
    double sinI = std::sin(i);
    
    glm::dmat3 rotMat;
    rotMat[0][0] = cosOmega * cosW - sinOmega * sinW * cosI;
    rotMat[0][1] = -cosOmega * sinW - sinOmega * cosW * cosI;
    rotMat[0][2] = sinOmega * sinI;
    rotMat[1][0] = sinOmega * cosW + cosOmega * sinW * cosI;
    rotMat[1][1] = -sinOmega * sinW + cosOmega * cosW * cosI;
    rotMat[1][2] = -cosOmega * sinI;
    rotMat[2][0] = sinW * sinI;
    rotMat[2][1] = cosW * sinI;
    rotMat[2][2] = cosI;
    
    outPosition = rotMat * rPQW;
    outVelocity = rotMat * vPQW;
}

void Orbit::createCircularOrbit(double altitude, double inclination, 
                               double raan, double trueAnomaly,
                               double mu, double bodyRadius,
                               glm::dvec3& outPosition,
                               glm::dvec3& outVelocity) {
    double r = bodyRadius + altitude;
    
    OrbitalElements elements;
    elements.semiMajorAxis = r;
    elements.eccentricity = 0.0;
    elements.inclination = inclination;
    elements.raan = raan;
    elements.argOfPeriapsis = 0.0;
    elements.trueAnomaly = trueAnomaly;
    
    computeStateFromElements(elements, mu, outPosition, outVelocity);
}

void Orbit::createEllipticalOrbit(double periapsisAltitude, 
                                 double apoapsisAltitude,
                                 double inclination, double raan,
                                 double argOfPeriapsis, double trueAnomaly,
                                 double mu, double bodyRadius,
                                 glm::dvec3& outPosition,
                                 glm::dvec3& outVelocity) {
    double rp = bodyRadius + periapsisAltitude;
    double ra = bodyRadius + apoapsisAltitude;
    
    double a = (rp + ra) / 2.0;
    double e = (ra - rp) / (ra + rp);
    
    OrbitalElements elements;
    elements.semiMajorAxis = a;
    elements.eccentricity = e;
    elements.inclination = inclination;
    elements.raan = raan;
    elements.argOfPeriapsis = argOfPeriapsis;
    elements.trueAnomaly = trueAnomaly;
    
    computeStateFromElements(elements, mu, outPosition, outVelocity);
}

double Orbit::computeOrbitalVelocity(double radius, double mu, double semiMajorAxis) {
    return std::sqrt(mu * (2.0 / radius - 1.0 / semiMajorAxis));
}

double Orbit::computeCircularVelocity(double radius, double mu) {
    return std::sqrt(mu / radius);
}

double Orbit::computeEscapeVelocity(double radius, double mu) {
    return std::sqrt(2.0 * mu / radius);
}

double Orbit::computePeriod(double semiMajorAxis, double mu) {
    return Constants::TWO_PI * std::sqrt(semiMajorAxis * semiMajorAxis * semiMajorAxis / mu);
}

double Orbit::computeAltitude(const glm::dvec3& position, double bodyRadius) {
    return glm::length(position) - bodyRadius;
}
