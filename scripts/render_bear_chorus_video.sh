#!/bin/bash
set -e

echo "[Video Pipeline] Compiling bear chorus C renderer executable..."
gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinclude -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_bear_chorus.c tsfi2-deepseek/src/auncient_teddy_bear_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_displacementshader.c -o tsfi2-deepseek/src/auncient_bear_chorus -lm -lrt

echo "[Video Pipeline] Executing C DSP synthesis and PPM frame rendering..."
./tsfi2-deepseek/src/auncient_bear_chorus

echo "[Video Pipeline] Compiling rendered PPM frames and audio to final mp4..."
ffmpeg -y -r 30 -i rendered_frames/frame_%04d.ppm -i bear_chorus_track.wav -c:v libx264 -pix_fmt yuv420p -c:a aac -shortest bear_chorus_demo.mp4

echo "[Video Pipeline] Completed successfully! Saved as bear_chorus_demo.mp4"
