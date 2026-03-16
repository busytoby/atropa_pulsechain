#!/bin/bash
set -o pipefail

# Visual Alignment Verification Test Suite
# Verifies:
# 1. Left-Aligned (Implied LTR) -> Prompt "> " Left
# 2. Right-Aligned (Implied LTR) -> Prompt " <" Right
# 3. Left-Aligned RTL -> Prompt "> " Left
# 4. Right-Aligned RTL -> Prompt " <" Right

LOG_FILE="test_align.log"
rm -f "$LOG_FILE" *.ppm

# Create LSAN Suppression File if needed (reusing visual one)
cat <<EOF > lsan_visual.supp
leak:libwayland-client
leak:ld-linux
leak:libtsfi_ai.so
leak:<unknown module>
EOF

echo "--- Starting Alignment Verification ---"

# Ensure Timing Tool exists
if [ ! -f "bin/tsfi_time" ]; then
    echo "Building Timing Tool..."
    make bin/tsfi_time
fi

TIME_TOOL="bin/tsfi_time"

generate_input() {
    echo "LOAD plugins/tsfi_vulkan"
    echo "1.0 OPEN_WINDOW"
    echo "1.0 REC_INIT 3"
    
    # Wait for init
    $TIME_TOOL 3.0
    echo "1.0 TEST_INPUT 119" # Toggle Edit (Show UI)
    echo "1.0 TEST_INPUT 23"  # Toggle Input Bar ('i')
    
    # 1. Default: Left Aligned, LTR
    echo "1.0 SET_ALIGN_LEFT"
    echo "1.0 SET_FLOW_LTR"
    $TIME_TOOL 0.5
    echo "1.0 SCREENSHOT"
    $TIME_TOOL 1.0
    
    # 2. Right Aligned, LTR
    echo "1.0 SET_ALIGN_RIGHT"
    echo "1.0 SET_FLOW_LTR"
    $TIME_TOOL 0.5
    echo "1.0 SCREENSHOT"
    $TIME_TOOL 1.0
    
    # 3. Left Aligned, RTL
    echo "1.0 SET_ALIGN_LEFT"
    echo "1.0 SET_FLOW_RTL"
    $TIME_TOOL 0.5
    echo "1.0 SCREENSHOT"
    $TIME_TOOL 1.0
    
    # 4. Right Aligned, RTL
    echo "1.0 SET_ALIGN_RIGHT"
    echo "1.0 SET_FLOW_RTL"
    $TIME_TOOL 0.5
    echo "1.0 SCREENSHOT"
    $TIME_TOOL 1.0
    
    echo "0.0 EXIT"
}

# Run TSFi2
generate_input |  :detect_odr_violation=0 ./tsfi2 > "$LOG_FILE" 2>&1

# Rename files based on order
# We assume the engine starts counting at 0 or 1. We check what exists.
# Usually static int starts at 0.
if [ -f "manual_0.ppm" ]; then
    mv manual_0.ppm align_left_ltr.ppm
    mv manual_1.ppm align_right_ltr.ppm
    mv manual_2.ppm align_left_rtl.ppm
    mv manual_3.ppm align_right_rtl.ppm
elif [ -f "manual_1.ppm" ]; then
    mv manual_1.ppm align_left_ltr.ppm
    mv manual_2.ppm align_right_ltr.ppm
    mv manual_3.ppm align_left_rtl.ppm
    mv manual_4.ppm align_right_rtl.ppm
fi

# --- ANALYSIS ---
FAIL=0

verify_image() {
    IMG=$1
    EXPECT_SIDE=$2 # LEFT or RIGHT
    
    echo "Analyzing $IMG (Expect $EXPECT_SIDE)..."
    
    if [ ! -f "$IMG" ]; then
        echo "[FAIL] Image $IMG missing!"
        FAIL=1
        return
    fi
    
    OUTPUT=$(./tests/test_visual_alignment "$IMG")
    SIDE=$(echo "$OUTPUT" | grep "PROMPT_SIDE" | cut -d= -f2)
    DETECT=$(echo "$OUTPUT" | grep "PROMPT_DETECTED" | cut -d= -f2)
    
    if [ "$DETECT" -eq 1 ]; then
        if [ "$SIDE" == "$EXPECT_SIDE" ]; then
            echo "[PASS] $IMG Correct: $SIDE"
        else
            echo "[FAIL] $IMG Incorrect Side: Got $SIDE, Expected $EXPECT_SIDE"
            FAIL=1
        fi
    else
        echo "[FAIL] $IMG Prompt NOT Detected"
        # Dump output for debug
        echo "$OUTPUT"
        FAIL=1
    fi
}

verify_image "align_left_ltr.ppm" "LEFT"
verify_image "align_right_ltr.ppm" "RIGHT"
verify_image "align_left_rtl.ppm" "LEFT"
verify_image "align_right_rtl.ppm" "RIGHT"

if [ $FAIL -eq 0 ]; then
    echo "=== ALIGNMENT TEST PASSED ==="
    exit 0
else
    echo "=== ALIGNMENT TEST FAILED ==="
    echo "--- LOG DUMP (Last 50 lines) ---"
    tail -n 50 "$LOG_FILE"
    exit 1
fi
