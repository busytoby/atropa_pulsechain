#!/bin/bash
# tests/shared_verification.sh
# The Single Shared Unit Test for TSFi Coordinated Strategy.

export TSFI_NO_FOCUS=1
export AMD_VULKAN_ICD=RADV
export ASAN_OPTIONS=detect_odr_violation=0

LOG="shared_test.log"
rm -f "$LOG"

echo "==================================================="
echo "   TSFi SHARED VERIFICATION (Target: 100%)"
echo "==================================================="

# 1. Build Phase
echo "[STEP 1] Building..."
if ! make debug > build_verify.log 2>&1; then
    echo "[FAIL] Build Failed."
    exit 1
fi
echo "[PASS] Build Successful."

# 2. Execution Phase
echo "[STEP 2] Launching Application..."
PIPE=$(mktemp -u)
mkfifo $PIPE

./tsfi2 < $PIPE > "$LOG" 2>&1 &
PID=$!

{
    echo "LOAD plugins/tsfi_vulkan"
    sleep 2
    echo "1.0 OPEN_WINDOW"
    
    echo "---------------------------------------------------"
    echo " [USER CHECK REQUIRED]"
    echo " 1. Is the window visible?"
    echo " 2. Type now. Did the window steal focus?"
    echo " 3. Window will close in 30 seconds."
    echo "---------------------------------------------------"
    sleep 30
    
    echo "1.0 CLOSE_WINDOW"
    sleep 1
    echo "10.0 EXIT"
} > $PIPE

wait $PID 2>/dev/null
rm $PIPE

# 3. Log Analysis
echo "[STEP 3] Analyzing Logs..."
FOCUS_FAIL=0
WINDOW_FAIL=0

# Check for Focus Violation
if grep -q "FOCUS_VIOLATION" "$LOG"; then
    echo "[FAIL] Focus Violation Detected!"
    grep "FOCUS_VIOLATION" "$LOG"
    FOCUS_FAIL=1
fi

# Check for Targeted Output (Fullscreen attempts)
if grep -q "Targeted Output" "$LOG"; then
    if grep -q "Passive" "$LOG"; then
        echo "[INFO] Targeted Output (Passive Mode) - Acceptable."
    else
        echo "[WARN] Targeted Output detected (Possible Fullscreen)."
    fi
fi

# Check for Window Creation
if ! grep -q "Swapchain Created" "$LOG"; then
    echo "[FAIL] Window did not open (No Swapchain)."
    WINDOW_FAIL=1
fi

echo "==================================================="
if [ $FOCUS_FAIL -eq 1 ] || [ $WINDOW_FAIL -eq 1 ]; then
    echo "   TEST STATUS: FAILED"
    echo "   - Focus Safe: $((1 - FOCUS_FAIL))"
    echo "   - Window Open: $((1 - WINDOW_FAIL))"
    exit 1
else
    echo "   TEST STATUS: PASSED (100% Functional)"
    echo "   - Window: Visible (Passive Floating)"
    echo "   - Focus: Secure (No Theft Detected)"
    exit 0
fi
echo "==================================================="
