# Assets Directory

This directory contains textures and optional model files for the simulation.

## Required Files

- `textures/moon_albedo.png` - Moon surface texture (optional, will fall back to procedural gray if missing)

## Optional Files

- `models/spacecraft.glb` - Custom spacecraft model (optional, uses procedural arrow mesh if missing)

## Notes

If the moon texture is missing, the application will use a simple gray color for the Moon's surface.
You can obtain a moon texture from NASA's public domain image library or similar sources.

Recommended moon texture resolution: 2048x1024 or higher for good quality.
