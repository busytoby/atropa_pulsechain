#!/bin/bash
# ==============================================================================
# Auncient WinchesterMQ Compositor Runner
# ==============================================================================
set -e

# Default file paths
ERIS_VIDEO="ambient_manifold_photorealistic.mp4"
FOMAL_VIDEO="ambient_manifold_photorealistic.mp4"
FORNAX_VIDEO="ambient_manifold_photorealistic.mp4"
LINEART_VIDEO="manifold_layer_lineart.mp4"
DEPTH_VIDEO="manifold_layer_depth.mp4"
NORMAL_VIDEO="manifold_layer_normal.mp4"
SEG_VIDEO="manifold_layer_segmentation.mp4"
AUDIO_FILE="cloudburst_ambient_resonance.wav"
OUTPUT_VIDEO="ambient_manifold_composed_interop.mp4"
RENDER_STYLE="photorealistic"

# Print usage instructions
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --eris <path>       Eris base video (default: $ERIS_VIDEO)"
    echo "  --fomalhaute <path> Fomalhaute base video (default: $FOMAL_VIDEO)"
    echo "  --fornax <path>     Fornax base video (default: $FORNAX_VIDEO)"
    echo "  --lineart <path>    LineArt outline video (default: $LINEART_VIDEO)"
    echo "  --depth <path>      Depth map video (default: $DEPTH_VIDEO)"
    echo "  --normal <path>     Normal vector video (default: $NORMAL_VIDEO)"
    echo "  --seg <path>        Segmentation mask video (default: $SEG_VIDEO)"
    echo "  --audio <path>      Soundtrack file (default: $AUDIO_FILE)"
    echo "  --output <path>     Output video destination (default: $OUTPUT_VIDEO)"
    echo "  --style <style>     Render aesthetic style: photorealistic|stylized|lineart|retro (default: $RENDER_STYLE)"
    echo "  -h, --help          Show this message"
    exit 1
}

# Parse command line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --eris) ERIS_VIDEO="$2"; shift ;;
        --fomalhaute) FOMAL_VIDEO="$2"; shift ;;
        --fornax) FORNAX_VIDEO="$2"; shift ;;
        --lineart) LINEART_VIDEO="$2"; shift ;;
        --depth) DEPTH_VIDEO="$2"; shift ;;
        --normal) NORMAL_VIDEO="$2"; shift ;;
        --seg) SEG_VIDEO="$2"; shift ;;
        --audio) AUDIO_FILE="$2"; shift ;;
        --output) OUTPUT_VIDEO="$2"; shift ;;
        --style) RENDER_STYLE="$2"; shift ;;
        -h|--help) usage ;;
        *) echo "Unknown parameter passed: $1"; usage ;;
    esac
    shift
done

# Ensure C compositor binary and shared interop library are compiled
if [ ! -f "scripts/libantigravity_interop.so" ]; then
    echo "[COMPILER] Compiling shared interop library..."
    gcc -O3 -shared -fPIC scripts/libantigravity_interop.c -o scripts/libantigravity_interop.so
fi

if [ -f "scripts/manifold_interop_renderer" ]; then
    # Always rebuild since binary argument footprint changed
    rm scripts/manifold_interop_renderer
fi

echo "[COMPILER] Compiling C interop compositor..."
gcc -O3 scripts/manifold_interop_renderer.c -o scripts/manifold_interop_renderer -Lscripts/ -lantigravity_interop -lm -Wl,-rpath,./scripts/

# Generate dynamic register state mapping from audio for beat modulation
REGISTRY_FILE="tmp_registry.bin"
if [ -f "$REGISTRY_FILE" ]; then
    rm "$REGISTRY_FILE"
fi

echo "[PIPELINE] Extrapolating beat modulation from $AUDIO_FILE..."
python3 -c "
import os, struct, wave, numpy as np
audio_path = '$AUDIO_FILE'
samples = []
if os.path.exists(audio_path):
    try:
        with wave.open(audio_path, 'rb') as wf:
            params = wf.getparams()
            frames = wf.readframes(wf.getnframes())
            count = wf.getnframes() * params.nchannels
            if params.sampwidth == 2:
                samples = [s / 32768.0 for s in struct.unpack(f'<{count}h', frames)]
            elif params.sampwidth == 1:
                samples = [(s - 128) / 128.0 for s in struct.unpack(f'<{count}B', frames)]
            if params.nchannels > 1:
                samples = samples[::params.nchannels]
    except Exception as e:
        print('Error reading audio for modulation:', e)

with open('$REGISTRY_FILE', 'wb') as f:
    total_frames = 960
    samples_per_frame = len(samples) // total_frames if samples else 0
    for frame in range(total_frames):
        mod = 0.5
        if samples_per_frame > 0:
            chunk = samples[frame * samples_per_frame : (frame + 1) * samples_per_frame]
            if chunk:
                rms = np.sqrt(np.mean(np.array(chunk)**2))
                mod = float(np.clip(rms * 5.0, 0.0, 1.0))
        vx = [0.0] * 16
        vy = [0.0] * 16
        vz = [0.0] * 16
        f.write(struct.pack('<11Q2If16f16f16fI4108s',
            0, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 500,
            0, frame, mod,
            *vx, *vy, *vz, 0, b'\x00' * 4108
        ))
"

echo "[PIPELINE] Launching fast C interop compositor..."
./scripts/manifold_interop_renderer \
  <(ffmpeg -i "$ERIS_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$FOMAL_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$FORNAX_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$LINEART_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$DEPTH_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$NORMAL_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  <(ffmpeg -i "$SEG_VIDEO" -f rawvideo -pix_fmt rgb24 - 2>/dev/null) \
  "$REGISTRY_FILE" "$RENDER_STYLE" | \
ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - \
  -stream_loop -1 -i "$AUDIO_FILE" -c:v libx264 -pix_fmt yuv420p \
  -c:a aac -b:a 192k -shortest "$OUTPUT_VIDEO"

# Clean up local dummy registry
if [ -f "$REGISTRY_FILE" ]; then
    rm "$REGISTRY_FILE"
fi

echo "[SUCCESS] Finished rendering composition to: $OUTPUT_VIDEO"
