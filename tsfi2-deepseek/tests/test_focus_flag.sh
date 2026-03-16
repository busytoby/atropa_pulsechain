#!/bin/bash
# ==============================================================================
# TSFi Sovereign Wayland Windowing Geometry & Asynchronous Execution Matrix
# ==============================================================================
#
# ARCHITECTURAL PURPOSE:
# This temporal orchestration script mathematically validates the `TSFI_NO_FOCUS`
# environmental constraint. It explicitly proves that the primary `tsfi2` executable
# can geometrically spawn and manage a Vulkan-backed graphical window natively 
# across a localized inter-process communication pipe without stealing sovereign
# terminal input focus from the encompassing execution layer.
#
# EXECUTION GEOMETRY:
# 1. State Isolation: The `TSFI_NO_FOCUS=1` parameter is physically exported to
#    forcibly lock the window into a non-intrusive `PASSIVE` geometrical topology.
# 2. Asynchronous IPC: A dedicated temporary FIFO pipe (`mktemp -u`) is structurally
#    instantiated to sequence asynchronous standard input commands into the isolated
#    background binary.
# 3. Temporal Directives: Strict sleep boundaries separate the architectural 
#    `LOAD plugins/tsfi_vulkan` and `OPEN_WINDOW` triggers to mathematically ensure
#    the asynchronous rendering context completely resolves its topological states.
# 4. Teardown Loop: The matrix forces a hardware termination sequence (`10.0 EXIT`)
#    and mathematically checks the resultant `output.log` buffer to confirm the 
#    `Mode: PASSIVE` status correctly bypassed active focus assignment.
# ==============================================================================

export TSFI_NO_FOCUS=1
export AMD_VULKAN_ICD=RADV
export ASAN_OPTIONS=detect_odr_violation=0

# Create a pipe for communication
PIPE=$(mktemp -u)
mkfifo $PIPE

# Run tsfi2 in background, reading from pipe
./tsfi2 < $PIPE > output.log 2>&1 &
PID=$!

# Send commands
{
    echo "LOAD plugins/tsfi_vulkan"
    sleep 1
    echo "1.0 OPEN_WINDOW"
    sleep 2
    echo "10.0 EXIT"
} > $PIPE

# Wait a bit
sleep 1

# Kill if still running
kill $PID 2>/dev/null
wait $PID 2>/dev/null

# Clean up
rm $PIPE

# Check log for focus messages
if grep -q "Mode: PASSIVE" output.log; then
    echo "[PASS] Passive Mode Detected."
    exit 0
else
    echo "[FAIL] Passive Mode NOT Detected."
    cat output.log
    exit 1
fi
