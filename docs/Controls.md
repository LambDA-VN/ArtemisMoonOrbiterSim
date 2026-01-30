# Controls

## Keyboard Controls

### Simulation

| Key | Action |
|-----|--------|
| `Space` | Toggle pause/play |
| `[` | Decrease time warp |
| `]` | Increase time warp |
| `R` | Reset to current scenario |
| `O` | Toggle orbit path display |
| `Escape` | Exit application |

### Camera (Free Fly Mode)

| Key | Action |
|-----|--------|
| `W` | Move forward |
| `S` | Move backward |
| `A` | Move left |
| `D` | Move right |
| `E` | Move up |
| `Q` | Move down |

## Mouse Controls

| Input | Action |
|-------|--------|
| Right Mouse Button + Drag | Rotate camera view |
| Scroll Wheel | Zoom in/out |
| Left Click | Interact with UI |

## Time Warp Levels

The simulation supports the following time warp levels:

- 1x (real-time)
- 2x
- 5x
- 10x
- 50x
- 100x

**Note:** At higher time warps, physics accuracy may decrease slightly. Use lower time warps for precise maneuvers.

## Camera Modes

### Free Fly
- Full freedom to fly anywhere in the scene
- Use WASD/QE for movement
- Right-click and drag to look around

### Chase
- Camera follows behind the spacecraft
- Automatically orients along velocity vector
- Scroll to adjust follow distance

### Orbit Around Moon
- Camera orbits around the Moon center
- Right-click and drag to orbit the camera
- Scroll to change orbit distance

### Top Down
- Orthographic view from above
- Useful for seeing the full orbit shape
- Scroll to change view scale

## Maneuver Controls (UI Panel)

1. Select burn direction (Prograde, Retrograde, etc.)
2. Set throttle level (0-100%)
3. Set burn duration
4. Click "Execute Burn" to start

### Burn Directions

- **Prograde**: Burn in direction of velocity (raises orbit)
- **Retrograde**: Burn opposite to velocity (lowers orbit)
- **Radial In**: Burn toward Moon center
- **Radial Out**: Burn away from Moon center
- **Normal**: Burn perpendicular to orbital plane (one direction)
- **Anti-Normal**: Burn perpendicular to orbital plane (opposite direction)
