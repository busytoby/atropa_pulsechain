#!/bin/bash
set -o pipefail

LOG_FILE="test_vision.log"
rm -f "$LOG_FILE"

# Create LSAN Suppression File if not exists
cat <<EOF > lsan_visual.supp
leak:libwayland-client
leak:ld-linux
leak:libtsfi_ai.so
leak:<unknown module>
EOF

echo "--- Starting Comprehensive Vision & Timing Verification (Adapted for New Font System) ---"

# Step 1: Run the new robust Raytracing/Bijective Vision Test
# This verifies:
# - Glyph Production (Fourier/Stick)
# - Rasterization (Wave512)
# - Display (Simulated ReBAR)
# - Identification (OCR)
# - Symmetry Analysis
# - Memory Bijective Integrity
echo "Executing Internal Vision Verification (test_vision_raytracing_v2)..." | tee -a "$LOG_FILE"
if ./tests/test_vision_raytracing_v2 >> "$LOG_FILE" 2>&1; then
    echo "[PASS] Internal Vision Logic Verified"
else
    echo "[FAIL] Internal Vision Logic Failed"
    exit 1
fi

echo "Executing Font Styling Verification (test_font_styling)..." | tee -a "$LOG_FILE"
if ./tests/test_font_styling >> "$LOG_FILE" 2>&1; then
    echo "[PASS] Font Styling Verified"
else
    echo "[FAIL] Font Styling Failed"
    exit 1
fi

echo "Executing Font Evolution Verification (test_font_evolution)..." | tee -a "$LOG_FILE"
if ./tests/test_font_evolution >> "$LOG_FILE" 2>&1; then
    echo "[PASS] Font Evolution Verified"
else
    echo "[FAIL] Font Evolution Failed"
    exit 1
fi

# Step 2: Legacy Active Window Smoke Test
# We perform a minimal "Smoke Test" on the active window to ensure
# the pipeline is alive, but we skip pixel-perfect probes which are
# now handled by the robust internal test.
echo "Executing Active Window Smoke Test..." | tee -a "$LOG_FILE"

rm -f plugins/tsfi_vulkan.so

# Run TSFi2 briefly to confirm no crash on startup/shutdown
python3 -c '
import time
import sys

def send(msg):
    print(msg)
    sys.stdout.flush()

send("LOAD plugins/tsfi_vulkan")
send("1.0 OPEN_WINDOW")
time.sleep(1.0)
send("1.0 CLOSE_WINDOW")
send("0.0 EXIT")
' |  :detect_odr_violation=0 ./tsfi2 >> "$LOG_FILE" 2>&1

if [ $? -eq 0 ]; then
    echo "[PASS] Active Window Smoke Test"
else
    echo "[FAIL] Active Window Crash"
    exit 1
fi

echo "=== COMPREHENSIVE VERIFICATION PASSED ==="
exit 0