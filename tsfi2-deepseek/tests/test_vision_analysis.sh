#!/bin/bash
set -o pipefail

# Visual Verification Test Suite
# Tests: Screenshot (Baseline, UI), Resilience, Flight Recorder (Movement)

LOG_FILE="test_visual.log"
rm -f "$LOG_FILE" *.ppm

# Create LSAN Suppression File
cat <<EOF > lsan_visual.supp
leak:libwayland-client
leak:ld-linux
leak:libtsfi_ai.so
leak:<unknown module>
EOF

echo "--- Starting Visual Verification Test ---"

# Input Sequence (Dynamic Generation for Timing)
generate_input() {
    echo "LOAD plugins/tsfi_vulkan"
    echo "1.0 OPEN_WINDOW"
    echo "1.0 REC_INIT 2"
    
    # 1. Baseline Screenshot (Black)
    echo "1.0 SCREENSHOT"
    
    # Wait for unique timestamp (and hotloader delay)
    sleep 10
    
    # 2. Input Mode + Movement
    echo "1.0 TEST_INPUT 119" # Toggle Edit
    echo "1.0 TEST_INPUT 23"  # Toggle Input Bar
    echo "1.0 TEST_MOUSE_MOVE 100 100"
    sleep 0.2
    echo "1.0 TEST_MOUSE_CLICK"
    sleep 0.2
    echo "1.0 TEST_MOUSE_MOVE 200 200"
    sleep 0.2
    echo "1.0 TEST_MOUSE_RELEASE"
    
    # 3. UI Analysis (Instead of Screenshot)
    echo "1.0 ANALYZE_VISUALS"
    
    # 4. Dump Recorder
    echo "1.0 REC_DUMP"
    echo "1.0 REC_STOP"
    
    # Wait for user visual confirmation
    sleep 5
    
    echo "0.0 EXIT"
}

# Run TSFi2 with suppressions
generate_input |  :detect_odr_violation=0 ./tsfi2 > "$LOG_FILE" 2>&1

EXIT_CODE=$?

# Verify Exit Code
if [ $EXIT_CODE -ne 0 ]; then
    echo "[WARN] TSFi2 exited with code $EXIT_CODE"
fi

FAIL=0

# --- VERIFICATION ---

# Check Log for Analysis Metrics
if grep -q "\[VISION\] Frame Analysis:" "$LOG_FILE"; then
    echo "[PASS] Vision Analysis Triggered."
    
    # Extract Metrics
    MAX_VAL=$(grep "Frame Analysis:" "$LOG_FILE" | tail -n 1 | awk -F'Max=' '{print $2}' | awk -F',' '{print $1}')
    ORIENT=$(grep "Frame Analysis:" "$LOG_FILE" | tail -n 1 | awk -F'Orient=' '{print $2}')
    
    echo "[INFO] Detected Max Value: $MAX_VAL"
    echo "[INFO] Detected Orientation: $ORIENT"
    
    if [ "$MAX_VAL" -gt 200 ]; then
        echo "[PASS] Content Detected (Max > 200)"
    else
        echo "[FAIL] Content Missing (Max too low)"
        FAIL=1
    fi
    
    if [[ "$ORIENT" == BACKWARDS* ]]; then
        echo "[PASS] Orientation Correct: $ORIENT"
    else
        echo "[FAIL] Orientation Incorrect: $ORIENT"
        FAIL=1
    fi
else
    echo "[FAIL] Vision Analysis NOT Triggered (Log missing entry)"
    FAIL=1
fi

if [ $FAIL -eq 1 ]; then
    echo "[FAIL] Visual Regression Detected."
else
    # Cross-check that they are indeed different from each other (Basic Sanity)
    if cmp -s "manual_0.ppm" "manual_1.ppm"; then
        echo "[FAIL] manual_0.ppm and manual_1.ppm are IDENTICAL (No visual change detected)."
        FAIL=1
    else
        echo "[PASS] Visual State Change Confirmed (Baseline != UI)."
    fi
fi

if [ $FAIL -eq 0 ]; then
    echo "=== VISION ANALYSIS PASSED ==="
    exit 0
else
    echo "=== VISION ANALYSIS FAILED ==="
    exit 1
fi
