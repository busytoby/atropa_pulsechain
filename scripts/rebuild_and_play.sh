#!/bin/bash
# Rebuild and play the fresh Auncient ballet demo cleanly

set -e

echo "[REBUILD] Purging old video files..."
rm -f bin/teddy_ballet_demo.mp4

echo "[REBUILD] Compiling render binary..."
gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Isrc -Itsfi2-deepseek/inc \
    tests/demo_ballet_render.c src/auncient_ballet_animator.c src/auncient_cloth_material_bridge.c \
    tsfi2-deepseek/src/tsfi_cade_imf_nato.c src/auncient_sdk.c src/auncient_edsac_firewall.c \
    src/auncient_timeline_autodin.c src/auncient_vulkan_materials.c src/cloth_simulator.c \
    tsfi2-deepseek/src/tsfi_usdshade.c src/auncient_usd_stl_bridge.c src/auncient_stl_loader.c \
    -o tests/demo_ballet_render -lm -lrt -lssl -lcrypto -lpthread

echo "[REBUILD] Generating clean progressive ballet animation..."
./tests/demo_ballet_render

echo "[REBUILD] Clean build complete. Output size:"
ls -lh bin/teddy_ballet_demo.mp4
