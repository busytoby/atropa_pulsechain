#!/bin/bash
set -e

LOG_FILE="test_vulkan_robustness.log"
rm -f "$LOG_FILE"

echo "--- Testing Vulkan Robustness (Resize, Idempotency, State) ---"

# 1. Open Window
# 2. Open Window AGAIN (Should be ignored)
# 3. Toggle Edit Mode
# 4. Resize
# 5. Close
# 6. Close AGAIN (Should be safe)

export ASAN_OPTIONS=detect_odr_violation=0

./tsfi2 > "$LOG_FILE" 2>&1 <<EOF
LOAD plugins/tsfi_vulkan
1.0 OPEN_WINDOW
1.0 OPEN_WINDOW
1.0 TOGGLE_EDIT
1.0 RESIZE
1.0 CLOSE_WINDOW
1.0 CLOSE_WINDOW
0.0 EXIT
EOF

FAIL=0

# Check Open
if grep -c "Creating Native Vulkan System" "$LOG_FILE" | grep -q "1"; then
    echo "[PASS] System Created Exactly Once"
else
    COUNT=$(grep -c "Creating Native Vulkan System" "$LOG_FILE")
    echo "[FAIL] System Created $COUNT times (Expected 1)"
    FAIL=1
fi

# Check Edit Mode
if grep -q "Edit Mode: ON" "$LOG_FILE"; then
    echo "[PASS] Edit Mode Toggled ON"
else
    echo "[FAIL] Edit Mode Toggle Missing"
    FAIL=1
fi

# Check Resize
if grep -q "Swapchain Created (1024x768" "$LOG_FILE"; then
    echo "[PASS] Resize to 1024x768 confirmed"
else
    echo "[FAIL] Resize confirmation missing"
    FAIL=1
fi

# Check Close Idempotency
if grep -c "Destroying Vulkan Resources" "$LOG_FILE" | grep -q "1"; then
    echo "[PASS] Resources Destroyed Exactly Once"
else
    COUNT=$(grep -c "Destroying Vulkan Resources" "$LOG_FILE")
    echo "[FAIL] Resources Destroyed $COUNT times (Expected 1)"
    FAIL=1
fi

if [ $FAIL -eq 0 ]; then
    echo "=== ROBUSTNESS TEST PASSED ==="
    rm "$LOG_FILE"
    exit 0
else
    echo "=== ROBUSTNESS TEST FAILED ==="
    cat "$LOG_FILE"
    exit 1
fi
