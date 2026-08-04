#!/bin/bash
set -e

echo "[Video Pipeline] Compiling bear chorus executable..."
gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinclude -Itsfi2-deepseek/inc src/auncient_bear_chorus.c src/auncient_teddy_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o src/auncient_bear_chorus -lm -lrt

echo "[Video Pipeline] Running 313 stress-test cycles..."
./src/auncient_bear_chorus > chorus_simulation.log

echo "[Video Pipeline] Generating audio soundtrack..."
./scripts/generate_chorus_soundtrack.py

echo "[Video Pipeline] Rendering frames using PIL compositor..."
./scripts/generate_bear_frames.py

echo "[Video Pipeline] Compiling rendered frames and audio to final mp4..."
ffmpeg -y -r 10 -i rendered_frames/frame_%04d.png -i bear_chorus_track.wav -c:v libx264 -pix_fmt yuv420p -c:a aac -shortest bear_chorus_demo.mp4

echo "[Video Pipeline] Completed successfully! Saved as bear_chorus_demo.mp4"
