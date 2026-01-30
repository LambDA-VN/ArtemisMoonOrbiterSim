# ArtemisMoonOrbiterSim

An interactive C++ simulation of an Artemis-style spacecraft orbiting the Moon, with real-time rendering, UI controls, and basic mission-style features including orbit insertion, burns, camera modes, and telemetry.

## Quick Start (Linux Standalone Package)

Download the latest pre-built package from the [Releases](https://github.com/LambDA-VN/ArtemisMoonOrbiterSim/releases) page.

```bash
# Download and extract
tar -xzf ArtemisMoonOrbiterSim-*-linux-x86_64.tar.gz
cd ArtemisMoonOrbiterSim-*-linux-x86_64

# Run the simulator
./bin/run-artemis.sh
```

**Requirements**: OpenGL 3.3+ compatible graphics driver (Mesa or proprietary drivers).

## Features

- **Two-body orbital mechanics** with RK4 integration for accurate simulation
- **Real-time 3D rendering** of the Moon and spacecraft using OpenGL 3.3
- **Interactive UI** with Dear ImGui for telemetry, maneuver planning, and camera controls
- **Multiple orbital scenarios**: circular, elliptical, and near-surface orbits
- **Thrust and maneuver system** with prograde/retrograde/normal burn modes
- **Trajectory prediction** showing future orbit path
- **Time warp** functionality (1x to 100x)
- **Multiple camera modes**: Free fly, Chase, Orbit around Moon, Top-down

## Build Requirements

- **C++20** compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **CMake** 3.20 or higher
- **OpenGL** 3.3+ compatible graphics driver
- Linux: X11 or Wayland development libraries

### Linux Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Linux Dependencies (Fedora)

```bash
sudo dnf install gcc-c++ cmake git
sudo dnf install mesa-libGL-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
```

## Building

```bash
# Clone the repository
git clone https://github.com/LambDA-VN/ArtemisMoonOrbiterSim.git
cd ArtemisMoonOrbiterSim

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . -j$(nproc)

# Run
./ArtemisMoonOrbiterSim
```

### Windows (Visual Studio)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Controls

| Key | Action |
|-----|--------|
| Space | Toggle pause |
| [ / ] | Decrease/Increase time warp |
| O | Toggle orbit path |
| R | Reset simulation |
| Right Mouse + Drag | Rotate camera |
| Scroll | Zoom in/out |
| W/A/S/D/Q/E | Move camera (free fly mode) |

## Scenarios

1. **Circular Low Lunar Orbit** - 100 km altitude circular orbit
2. **Elliptical Capture Orbit** - 100 km periapsis, 5000 km apoapsis polar orbit
3. **Near Surface Skimming** - 15 km periapsis for testing collision detection

## Physics Model

- **Gravity**: Two-body point mass model (a = -μr/|r|³)
- **Moon μ**: 4902.8 km³/s²
- **Moon Radius**: 1737.4 km
- **Integrator**: RK4 (default), Semi-implicit Euler, Euler
- **Fixed timestep**: 20 ms (50 Hz physics)

## Architecture

```
src/
├── main.cpp           # Entry point
├── core/
│   ├── Application    # Main loop, event handling
│   ├── Time           # Time management, time warp
│   └── Constants      # Physical and simulation constants
├── physics/
│   ├── Spacecraft     # State vector, thrust system
│   ├── Orbit          # Orbital elements computation
│   └── Integrator     # RK4 and other numerical integrators
├── render/
│   ├── Renderer       # OpenGL rendering, meshes, shaders
│   ├── Camera         # Multiple camera modes
│   ├── Shader         # Shader loading and uniforms
│   └── Mesh           # Geometry generation and rendering
└── ui/
    └── Ui             # ImGui panels and controls
```

## License

MIT License - See LICENSE file for details.

## Acknowledgments

- Moon gravitational parameters from NASA
- Uses GLFW, glad, glm, and Dear ImGui libraries