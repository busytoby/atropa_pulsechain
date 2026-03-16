#!/bin/bash

TARGET="./tests/mock_target_app"
COCKPIT="./bin/tsfi_cockpit"
LOG="cockpit_debug.log"

echo "=== Cockpit Debugger Test ==="
echo "Building target..."
make tests/mock_target_app > /dev/null

echo "Running Cockpit with Target..."
# Run cockpit, capturing output. It will run until target exits.
# We redirect stderr to /dev/null because cockpit writes usage there if args wrong, 
# but here we pass args.
# Note: Cockpit uses shm. Target sets TSFI_TELEMETRY_ID=cockpit internally if spawned by cockpit.
# But we need to ensure the shared memory is read.

# We run cockpit, which spawns the target.
$COCKPIT $TARGET > $LOG 2>&1 &
PID=$!

# Wait for it to finish (Target sleeps for ~1s total)
sleep 2
kill $PID 2>/dev/null

echo "Analyzing Log..."

# 1. Check for Init
if grep -q "BOOT_SEQUENCE" $LOG; then
    echo "  [PASS] Saw BOOT_SEQUENCE"
else
    echo "  [FAIL] Missed BOOT_SEQUENCE"
    echo "Log Tail:"
    tail -n 20 $LOG
    exit 1
fi

# 2. Check for Allocation
if grep -q "ALLOC_PHASE_1" $LOG; then
    echo "  [PASS] Saw ALLOC_PHASE_1"
else
    echo "  [FAIL] Missed ALLOC_PHASE_1"
    exit 1
fi

# 3. Check for Compute Steps
if grep -q "COMPUTE_STEP_3" $LOG; then
    echo "  [PASS] Saw COMPUTE_STEP_3"
else
    echo "  [FAIL] Missed COMPUTE_STEP_3"
    exit 1
fi

# 4. Check Execution Count
# We expect at least: Boot, Alloc1, Alloc2, 5x Compute, Free1, Free2, Shutdown = 12 steps.
# grep "Exec Steps" and get the last one.
LAST_STEPS=$(grep "Exec Steps" $LOG | tail -n 1 | awk '{print $4}')
echo "  [INFO] Final Exec Steps: $LAST_STEPS"

if [ "$LAST_STEPS" -ge 10 ]; then
    echo "  [PASS] Step count reasonable ($LAST_STEPS >= 10)"
else
    echo "  [FAIL] Step count too low ($LAST_STEPS < 10)"
    exit 1
fi

echo "=== Cockpit Verification Successful ==="
rm $LOG
exit 0
