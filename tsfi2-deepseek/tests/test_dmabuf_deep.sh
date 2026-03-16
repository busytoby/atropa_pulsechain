#!/bin/bash
# Test Suite for Vulkan DMABUF Integration (Log-based Verification)
set -ex

export ASAN_OPTIONS=detect_odr_violation=0

LOG_FILE="test_dmabuf_deep.log"
rm -f "$LOG_FILE"

echo "--- Running Deep DMABUF Verification ---"

# Launch TSFi with Vulkan plugin and trigger window opening
cat <<CMD > input.tsfi
LOAD plugins/tsfi_vulkan
1.0 OPEN_WINDOW
1.0 STEP
0.0 SCREENSHOT
1.0 CLOSE_WINDOW
0.0 EXIT
CMD

DEBUG_VULKAN=1 ./tsfi2 < input.tsfi > "$LOG_FILE" 2>&1
rm input.tsfi

# Assertions
FAIL=0

assert_log() {
    if grep -q "$1" "$LOG_FILE"; then
        echo "[PASS] $2"
    else
        echo "[FAIL] $2 (Missing: '$1')"
        FAIL=1
    fi
}

# 1. Successful initialization
assert_log "Creating Native Vulkan System" "Vulkan System Creation"
assert_log "Negotiated DMABUF: v" "DMABUF Interface Negotiation"

# 2. Feedback Reception
assert_log "DMABUF Feedback Listener Attached" "Feedback Listener Attachment"

# 3. Buffer Creation (Conditional)
if grep -q "DMA-BUF Export Available" "$LOG_FILE"; then
    echo "[INFO] DMABUF Supported by Driver. Verifying creation..."
    assert_log "DMABUF Buffer Created" "DMABUF wl_buffer creation"
else
    echo "[WARN] DMABUF Export NOT supported by Driver. Skipping buffer check."
    if grep -q "Retrying with Swapchain only" "$LOG_FILE"; then
        echo "[INFO] Correctly fell back to Swapchain."
    fi
fi

# 4. Vulkan Side
assert_log "Vulkan Wayland Surface Created" "Vulkan Wayland WSI"
assert_log "Device Created" "Vulkan Device Creation"

# 5. Cleanup
assert_log "Destroying Vulkan Resources" "Resource Cleanup Initiation"
assert_log "\[TSFI_VULKAN\] Destroyed" "System Cleanup Completion"

if [ $FAIL -eq 0 ]; then
    echo "=== DMABUF DEEP TEST PASSED ==="
    rm "$LOG_FILE"
    exit 0
else
    echo "=== DMABUF DEEP TEST FAILED ==="
    cat "$LOG_FILE"
    exit 1
fi