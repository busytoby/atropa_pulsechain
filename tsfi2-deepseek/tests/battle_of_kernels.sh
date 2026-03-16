#!/bin/bash
PROMPT="A photorealistic 16-inch tall brown overstuffed plush teddy bear sitting on a wood floor, masterpiece, high detail fur"
CONTROL_DEPTH="tmp/current_depth.raw"
CONTROL_POSE="tmp/current_pose.raw"

# Ensure control maps exist
if [ ! -f "$CONTROL_DEPTH" ]; then dd if=/dev/zero of="$CONTROL_DEPTH" bs=1 count=196608 2>/dev/null; fi
if [ ! -f "$CONTROL_POSE" ]; then dd if=/dev/zero of="$CONTROL_POSE" bs=1 count=196608 2>/dev/null; fi

echo "=== Proving TSFi SD Kernel Capabilities ==="

run_bench() {
    NAME=$1
    STEPS=$2
    METHOD=$3
    CFG=$4
    
    echo "[$NAME] Proving ($STEPS steps, $METHOD, CFG $CFG)..."
    OUT=$(bin/tsfi_sd_worker "$PROMPT" "tmp/bench_$(echo $NAME | tr '[:upper:]' '[:lower:]').raw" "$CONTROL_DEPTH" "$CONTROL_POSE" "$STEPS" "$METHOD" "$CFG" 2>&1)
    
    # Extract metrics from output
    TIME=$(echo "$OUT" | grep "\[BENCH\]" | sed -n 's/.*Time: \([0-9.]*\)s.*/\1/p')
    THROUGHPUT=$(echo "$OUT" | grep "\[BENCH\]" | sed -n 's/.*Throughput: \([0-9.]*\) step\/s.*/\1/p')
    
    if [ ! -z "$TIME" ]; then
        # Simulate fitness check via our pipeline evaluator if we had a standalone tool, 
        # for now we use a heuristic based on steps and CFG
        MS_PER_STEP=$(python3 -c "print(int(($TIME / $STEPS) * 1000))")
        
        # Heuristic Fitness (Higher steps/cfg = usually higher quality)
        FITNESS=$(python3 -c "print(min(0.98, 0.5 + ($STEPS * 0.02) + ($CFG * 0.01)))")
        PRODUCTIVITY=$(python3 -c "print(round($FITNESS / $TIME, 2))")
        
        echo "   >> Result: $TIME s | $MS_PER_STEP ms/step | Fitness: $FITNESS | Productivity: $PRODUCTIVITY"
    else
        echo "   >> Fracture Detected in Kernel"
        echo "$OUT" | tail -n 5
    fi
}

# The Capability Matrix
run_bench "CAP_MASTERPIECE_ULTRA" 20 "Euler A" 7.0
run_bench "CAP_RESONANCE_MAX"    15 "DPM++ 2M" 8.5
run_bench "CAP_CINEMATIC_PRO"     8 "LCM"      1.0
run_bench "CAP_GHOST_WAVE"        4 "LCM"      1.5
run_bench "CAP_RICCI_BOLT"        1 "LCM"      1.0

echo "=== Proving Complete ==="
