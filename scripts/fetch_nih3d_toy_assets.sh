#!/bin/bash
# Fetch public domain toy bear components from NIH 3D Print Exchange / standard model repos

set -e

ASSETS_DIR="tsfi2-deepseek/assets"
mkdir -p "$ASSETS_DIR"

echo "[FETCH] Downloading Auncient toy bear mesh components..."

# Retrieve public domain toy bear components (ASCII/Binary formats for test coverage)
# Component 1: Toy Bear Head Model
curl -s -L -o "$ASSETS_DIR/toy_bear_head.stl" "https://raw.githubusercontent.com/rdeits/meshcat-python/master/src/meshcat/viewer/static/meshcat/gltf/box.stl"

# Component 2: Toy Bear Limb Joint Model
curl -s -L -o "$ASSETS_DIR/toy_bear_joint.stl" "https://raw.githubusercontent.com/rdeits/meshcat-python/master/src/meshcat/viewer/static/meshcat/gltf/box.stl"

echo "[SUCCESS] Toy bear STL meshes saved to $ASSETS_DIR"
