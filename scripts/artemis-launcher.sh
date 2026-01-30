#!/bin/bash
# Launcher script for Artemis Moon Orbiter Simulator
# This script sets up the environment and runs the simulator

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="$(dirname "$SCRIPT_DIR")"

# Set assets path relative to installation
export ARTEMIS_ASSETS_DIR="${INSTALL_DIR}/share/ArtemisMoonOrbiterSim/assets"

# Change to the installation directory
cd "$INSTALL_DIR" || exit 1

# Run the simulator
exec "${SCRIPT_DIR}/ArtemisMoonOrbiterSim" "$@"
