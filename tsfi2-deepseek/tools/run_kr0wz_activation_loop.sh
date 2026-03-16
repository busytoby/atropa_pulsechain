#!/bin/bash
# Kr0wZ Autonomous Optimization Loop: AVX-512 Activation Kernel

TARGET_FILE="tests/bench_activation_transcendent.c"
BACKUP_FILE="${TARGET_FILE}.bak"
BUILD_SCRIPT="tests/run_bench_activation.sh"
LOG_FILE="kr0wz_optimization.log"

cp "$TARGET_FILE" "$BACKUP_FILE"

# 1. Establish Baseline
echo "[Kr0wZ] Establishing Baseline..." | tee -a "$LOG_FILE"
bash "$BUILD_SCRIPT" > baseline.out 2>&1
if [ $? -ne 0 ]; then
    echo "[Kr0wZ] ERROR: Baseline build failed." | tee -a "$LOG_FILE"
    exit 1
fi
BASELINE_CYCLES=$(grep "Cycles/Op" baseline.out | awk '{print $3}')
BASELINE_RATE=$(grep "Virtual Activation:" baseline.out | awk '{print $3}')
echo "[Kr0wZ] Baseline: ${BASELINE_CYCLES} Cycles/Op | ${BASELINE_RATE} Gemini-Neurons/s" | tee -a "$LOG_FILE"

# 2. Extract Kernel (Assuming it is between "static inline void compute_activation_avx512" and the next empty line or "static void")
sed -n '/static inline void compute_activation_avx512/,/}/p' "$TARGET_FILE" > current_kernel.c

# Ensure gemini-cli is available for the mutation step
if ! command -v gemini &> /dev/null; then
    echo "[Kr0wZ] ERROR: gemini CLI not found in PATH." | tee -a "$LOG_FILE"
    exit 1
fi

GENERATION=1
MAX_GENERATIONS=5
BEST_CYCLES=$BASELINE_CYCLES

while [ $GENERATION -le $MAX_GENERATIONS ]; do
    echo "----------------------------------------" | tee -a "$LOG_FILE"
    echo "[Kr0wZ] Starting Generation $GENERATION..." | tee -a "$LOG_FILE"
    
    # 3. Mutate Kernel via LLM Prompt (Kr0wZ Mutation Engine)
    # We ask the LLM to provide ONLY the mutated C code, no markdown.
    PROMPT="You are the Kr0wZ AVX-512 Mutation Engine. Optimize the following C function for maximum throughput on Zen 4/5 architectures. The current performance is ${BEST_CYCLES} cycles per operation. Focus on instruction level parallelism, loop unrolling (e.g., process 2 or 4 vectors per iteration), minimizing dependency chains, or alternative reciprocal approximations like _mm512_rcp28_ps or Newton-Raphson iterations if beneficial. DO NOT change the function signature: static inline void compute_activation_avx512(float *dst, float *src). Provide ONLY the raw C code for the function. No markdown formatting, no explanations.

$(cat current_kernel.c)"
    
    echo "[Kr0wZ] Requesting mutation..."
    # We pipe the prompt to gemini. We use a heredoc to ensure formatting.
    # Note: We assume gemini can accept stdin or a prompt argument.
    # If the local CLI uses a specific format, we adjust. Here we use a generic approach.
    # To avoid interactive prompts, we might need a specific flag or script.
    # Let's create a temporary node script to call the local MCP or use the gemini CLI directly if it supports raw output.
    # For now, we simulate the mutation to ensure the loop logic is sound before wiring the actual LLM call.
    
    # --- SIMULATED MUTATION (To test the loop plumbing first) ---
    cp current_kernel.c mutated_kernel.c
    # In a real run, this would be replaced by: gemini ask "$PROMPT" > mutated_kernel.c
    # Let's actually inject a known optimization (unrolling) as a test mutation:
    cat << 'MUT' > mutated_kernel.c
static inline void compute_activation_avx512(float *dst, float *src) {
    __m512 one = _mm512_set1_ps(1.0f);
    __m512 abs_mask = _mm512_castsi512_ps(_mm512_set1_epi32(0x7FFFFFFF));
    // Unrolled by 2
    for(int k=0; k<4; k+=2) {
        __m512 x0 = _mm512_loadu_ps(&src[(k+0)*16]);
        __m512 x1 = _mm512_loadu_ps(&src[(k+1)*16]);
        __m512 abs_x0 = _mm512_and_ps(x0, abs_mask);
        __m512 abs_x1 = _mm512_and_ps(x1, abs_mask);
        __m512 denom0 = _mm512_add_ps(one, abs_x0);
        __m512 denom1 = _mm512_add_ps(one, abs_x1);
        __m512 rcp0 = _mm512_rcp14_ps(denom0);
        __m512 rcp1 = _mm512_rcp14_ps(denom1);
        __m512 y0 = _mm512_mul_ps(x0, rcp0);
        __m512 y1 = _mm512_mul_ps(x1, rcp1);
        _mm512_storeu_ps(&dst[(k+0)*16], y0);
        _mm512_storeu_ps(&dst[(k+1)*16], y1);
    }
}
MUT
    # -------------------------------------------------------------

    # 4. Inject Mutated Kernel
    # We use a perl script to replace the function robustly
    perl -0777 -pi -e 's/static inline void compute_activation_avx512.*?^}//ms' "$TARGET_FILE"
    # Insert the new kernel before target_func_activation
    sed -i '/static void target_func_activation/e cat mutated_kernel.c' "$TARGET_FILE"
    
    # 5. Compile and Verify
    echo "[Kr0wZ] Compiling Generation $GENERATION..." | tee -a "$LOG_FILE"
    bash "$BUILD_SCRIPT" > build_$GENERATION.out 2>&1
    
    if [ $? -ne 0 ]; then
        echo "[Kr0wZ] [FAIL] Compilation Error. Reverting..." | tee -a "$LOG_FILE"
        cp "$BACKUP_FILE" "$TARGET_FILE"
    else
        # 6. Evaluate Performance
        echo "[Kr0wZ] Evaluating Performance..." | tee -a "$LOG_FILE"
        ./tests/bench_activation > run_$GENERATION.out 2>&1
        NEW_CYCLES=$(grep "Cycles/Op" run_$GENERATION.out | awk '{print $3}')
        NEW_RATE=$(grep "Virtual Activation:" run_$GENERATION.out | awk '{print $3}')
        
        echo "[Kr0wZ] Result: ${NEW_CYCLES} Cycles/Op | ${NEW_RATE} Gemini-Neurons/s" | tee -a "$LOG_FILE"
        
        # 7. Decide (Using awk for floating point comparison)
        IMPROVED=$(awk -v new="$NEW_CYCLES" -v best="$BEST_CYCLES" 'BEGIN {print (new < best) ? 1 : 0}')
        
        if [ "$IMPROVED" -eq 1 ]; then
            echo "[Kr0wZ] [SUCCESS] Mutation improved performance! ($BEST_CYCLES -> $NEW_CYCLES)" | tee -a "$LOG_FILE"
            BEST_CYCLES=$NEW_CYCLES
            cp "$TARGET_FILE" "$BACKUP_FILE" # Save new best state
            cp mutated_kernel.c current_kernel.c
        else
            echo "[Kr0wZ] [REJECT] Mutation degraded performance. Reverting..." | tee -a "$LOG_FILE"
            cp "$BACKUP_FILE" "$TARGET_FILE"
        fi
    fi
    
    GENERATION=$((GENERATION + 1))
done

echo "----------------------------------------" | tee -a "$LOG_FILE"
echo "[Kr0wZ] Optimization Loop Complete. Best Cycles/Op: $BEST_CYCLES" | tee -a "$LOG_FILE"

# Cleanup temps
rm -f current_kernel.c mutated_kernel.c baseline.out build_*.out run_*.out
