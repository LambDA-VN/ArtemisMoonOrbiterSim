# Physics Model

This document describes the physics models and equations used in the Artemis Moon Orbiter Simulation.

## Reference Frame

The simulation uses a **Moon-Centered Inertial (MCI)** reference frame:
- Origin at Moon's center of mass
- The frame does not rotate with the Moon
- Z-axis points toward lunar north pole
- X and Y axes in the equatorial plane

## Gravity Model

### Two-Body Point Mass

The primary gravity model is a simple two-body point mass approximation:

```
a = -μ * r / |r|³
```

Where:
- `a` = gravitational acceleration vector (m/s²)
- `μ` = Moon's gravitational parameter = 4,902,800,066,000 m³/s² (4902.8 km³/s²)
- `r` = position vector from Moon center to spacecraft (m)
- `|r|` = magnitude of position vector (m)

### Moon Parameters

| Parameter | Value | Unit |
|-----------|-------|------|
| Gravitational parameter (μ) | 4.9028 × 10¹² | m³/s² |
| Mean radius | 1,737,400 | m |
| J2 coefficient | 0 (not implemented) | - |

## State Vector

The spacecraft state is represented as:

```cpp
struct SpacecraftState {
    glm::dvec3 position;      // meters, MCI frame
    glm::dvec3 velocity;      // m/s, MCI frame
    glm::dquat attitude;      // quaternion (w,x,y,z)
    glm::dvec3 angularVelocity; // rad/s
    double mass;              // kg
};
```

## Numerical Integration

### RK4 (Runge-Kutta 4th Order) - Default

The primary integrator uses the classic 4th-order Runge-Kutta method:

```
k1 = f(t, y)
k2 = f(t + dt/2, y + dt*k1/2)
k3 = f(t + dt/2, y + dt*k2/2)
k4 = f(t + dt, y + dt*k3)

y(t+dt) = y(t) + (dt/6) * (k1 + 2*k2 + 2*k3 + k4)
```

This method provides 4th-order accuracy with good stability for orbital mechanics.

### Semi-Implicit Euler

Available as an alternative, faster but less accurate:

```
v(t+dt) = v(t) + a(t) * dt
x(t+dt) = x(t) + v(t+dt) * dt
```

Better energy conservation than standard Euler.

### Standard Euler

The simplest method, available for comparison:

```
x(t+dt) = x(t) + v(t) * dt
v(t+dt) = v(t) + a(t) * dt
```

Not recommended for accurate orbital simulation.

### Fixed Timestep

Physics uses a fixed timestep of **20 milliseconds (0.02 s)** or 50 Hz.

This ensures deterministic and stable simulation regardless of frame rate.

## Orbital Elements

Classical Keplerian orbital elements are computed from state vectors:

| Element | Symbol | Description |
|---------|--------|-------------|
| Semi-major axis | a | Half the longest diameter of ellipse |
| Eccentricity | e | Measure of orbit elongation (0=circular) |
| Inclination | i | Angle from equatorial plane |
| RAAN | Ω | Right ascension of ascending node |
| Argument of periapsis | ω | Angle from ascending node to periapsis |
| True anomaly | ν | Current position along orbit |

### Computed Quantities

- **Periapsis altitude** = a(1-e) - R_moon
- **Apoapsis altitude** = a(1+e) - R_moon
- **Orbital period** = 2π√(a³/μ)
- **Specific orbital energy** = v²/2 - μ/r
- **Angular momentum** = |r × v|

## Thrust Model

### Finite Burn Model

Thrust is applied continuously with:

```
F = throttle × max_thrust × direction
a_thrust = F / m

mass_flow_rate = thrust / (Isp × g0)
m(t+dt) = m(t) - mass_flow_rate × dt
```

Where:
- `throttle` = 0 to 1
- `max_thrust` = 25,000 N (default)
- `Isp` = 320 s (default)
- `g0` = 9.80665 m/s²

### Thrust Directions

| Mode | Direction |
|------|-----------|
| Prograde | Along velocity vector |
| Retrograde | Opposite to velocity |
| Radial In | Toward Moon center |
| Radial Out | Away from Moon center |
| Normal | Cross product of radial × prograde |
| Anti-Normal | Opposite to normal |

## Spacecraft Parameters (Defaults)

| Parameter | Value | Unit |
|-----------|-------|------|
| Total mass | 26,000 | kg |
| Dry mass | 18,000 | kg |
| Max thrust | 25,000 | N |
| Specific impulse | 320 | s |

*Note: These are placeholder values for educational purposes, not actual Artemis spacecraft parameters.*

## Collision Detection

Surface impact is detected when:

```
|r| ≤ R_moon
```

Or equivalently, when altitude ≤ 0.

Upon impact:
1. Simulation freezes
2. Impact notification displayed
3. User can reset to continue

## Energy Conservation

For accurate RK4 integration with a circular orbit:
- Specific orbital energy should remain nearly constant
- Expected drift over 30 minutes: < 0.01%

For validation, compare:
- Analytical orbital period vs simulated period
- Initial vs final specific orbital energy

## Future Extensions (Not Implemented)

- J2 gravitational perturbation
- Solar radiation pressure
- Third-body effects (Earth, Sun)
- Atmospheric drag (very minor for Moon)
- Maneuver node planning with conic projections
