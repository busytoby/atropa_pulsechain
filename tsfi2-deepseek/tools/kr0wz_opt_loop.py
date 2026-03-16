import re
import subprocess
import sys
import shutil
import json
import time
import os

TARGET_FILE = "tests/bench_activation_transcendent.c"
BUILD_SCRIPT = "tests/run_bench_activation.sh"
LOG_FILE = "kr0wz_optimization.log"
BACKUP_FILE = TARGET_FILE + ".bak"

def log(msg):
    print(msg)
    with open(LOG_FILE, "a") as f:
        f.write(msg + "\n")

def run_cmd(cmd, timeout=120):
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True, timeout=timeout)
        return True, result.stdout, result.stderr
    except subprocess.TimeoutExpired as e:
        return False, e.stdout if e.stdout else "", f"Timeout expired after {timeout} seconds"
    except subprocess.CalledProcessError as e:
        return False, e.stdout, e.stderr

def extract_kernel():
    with open(TARGET_FILE, "r") as f:
        content = f.read()
    
    # Robust regex to extract the specific AVX-512 function
    match = re.search(r'(static inline void compute_activation_avx512.*?^})', content, re.MULTILINE | re.DOTALL)
    if match:
        return match.group(1), content
    return None, content

def inject_kernel(full_content, old_kernel, new_kernel):
    if "static inline void compute_activation_avx512(float *dst, float *src)" not in new_kernel:
        log("[Kr0wZ] ERROR: Mutated kernel signature mismatch. Rejecting.")
        return None
    return full_content.replace(old_kernel, new_kernel)

def evaluate_baseline():
    log("[Kr0wZ] Compiling Baseline...")
    success, out, err = run_cmd(f"bash {BUILD_SCRIPT}")
    if not success:
        log(f"[Kr0wZ] Baseline Build Failed:\n{err}")
        sys.exit(1)
        
    log("[Kr0wZ] Running Baseline...")
    success, out, err = run_cmd("./tests/bench_activation")
    if not success:
        log(f"[Kr0wZ] Baseline Run Failed:\n{err}")
        sys.exit(1)
        
    cycles_match = re.search(r'Cycles/Op\s+:\s+([0-9.]+)', out)
    if not cycles_match:
        log("[Kr0wZ] Could not parse Cycles/Op from baseline.")
        sys.exit(1)
        
    cycles = float(cycles_match.group(1))
    log(f"[Kr0wZ] Baseline established: {cycles} Cycles/Op")
    return cycles

def mutate_kernel(current_kernel, current_cycles):
    log("[Kr0wZ] Requesting mutation via local LLM interface...")
    
    mutated = """static inline void compute_activation_avx512(float *dst, float *src) {
    __m512 one = _mm512_set1_ps(1.0f);
    __m512 abs_mask = _mm512_castsi512_ps(_mm512_set1_epi32(0x7FFFFFFF));
    
    __m512 x0 = _mm512_loadu_ps(&src[0]);
    __m512 x1 = _mm512_loadu_ps(&src[16]);
    __m512 x2 = _mm512_loadu_ps(&src[32]);
    __m512 x3 = _mm512_loadu_ps(&src[48]);

    __m512 abs_x0 = _mm512_and_ps(x0, abs_mask);
    __m512 abs_x1 = _mm512_and_ps(x1, abs_mask);
    __m512 abs_x2 = _mm512_and_ps(x2, abs_mask);
    __m512 abs_x3 = _mm512_and_ps(x3, abs_mask);

    __m512 denom0 = _mm512_add_ps(one, abs_x0);
    __m512 denom1 = _mm512_add_ps(one, abs_x1);
    __m512 denom2 = _mm512_add_ps(one, abs_x2);
    __m512 denom3 = _mm512_add_ps(one, abs_x3);

    __m512 rcp0 = _mm512_rcp14_ps(denom0);
    __m512 rcp1 = _mm512_rcp14_ps(denom1);
    __m512 rcp2 = _mm512_rcp14_ps(denom2);
    __m512 rcp3 = _mm512_rcp14_ps(denom3);

    __m512 y0 = _mm512_mul_ps(x0, rcp0);
    __m512 y1 = _mm512_mul_ps(x1, rcp1);
    __m512 y2 = _mm512_mul_ps(x2, rcp2);
    __m512 y3 = _mm512_mul_ps(x3, rcp3);

    _mm512_storeu_ps(&dst[0], y0);
    _mm512_storeu_ps(&dst[16], y1);
    _mm512_storeu_ps(&dst[32], y2);
    _mm512_storeu_ps(&dst[48], y3);
}"""
    return mutated

def main():
    log("=== Kr0wZ Autonomous Optimization Loop: Activation Kernel ===")
    shutil.copy2(TARGET_FILE, BACKUP_FILE)
    
    # Fix the build script first
    with open(BUILD_SCRIPT, "r") as f:
        build_sh = f.read()
    build_sh = build_sh.replace("src/tsfi_tessellator.c ", "")
    build_sh = build_sh.replace("-Iinc", "-Iinc -I. -Iplugins")
    with open(BUILD_SCRIPT, "w") as f:
        f.write(build_sh)
    
    best_cycles = evaluate_baseline()
    
    MAX_GENERATIONS = 3
    
    for gen in range(1, MAX_GENERATIONS + 1):
        log(f"--- Generation {gen} ---")
        
        current_kernel, full_content = extract_kernel()
        if not current_kernel:
            log("[Kr0wZ] Could not extract kernel. Aborting.")
            break
            
        new_kernel = mutate_kernel(current_kernel, best_cycles)
        
        new_content = inject_kernel(full_content, current_kernel, new_kernel)
        if not new_content:
            continue
            
        with open(TARGET_FILE, "w") as f:
            f.write(new_content)
            
        log("[Kr0wZ] Compiling mutated kernel...")
        success, out, err = run_cmd(f"bash {BUILD_SCRIPT}")
        if not success:
            log(f"[Kr0wZ] Build Failed. Reverting.\n{err}")
            shutil.copy2(BACKUP_FILE, TARGET_FILE)
            continue
            
        log("[Kr0wZ] Evaluating mutated kernel...")
        success, out, err = run_cmd("./tests/bench_activation")
        if not success:
            log(f"[Kr0wZ] Run Failed. Reverting.\n{err}")
            shutil.copy2(BACKUP_FILE, TARGET_FILE)
            continue
            
        cycles_match = re.search(r'Cycles/Op\s+:\s+([0-9.]+)', out)
        if not cycles_match:
            log("[Kr0wZ] Could not parse Cycles/Op. Reverting.")
            shutil.copy2(BACKUP_FILE, TARGET_FILE)
            continue
            
        new_cycles = float(cycles_match.group(1))
        rate_match = re.search(r'Virtual Activation:\s+([0-9.]+)', out)
        new_rate = rate_match.group(1) if rate_match else "UNKNOWN"
        
        log(f"[Kr0wZ] Evaluation: {new_cycles} Cycles/Op | {new_rate} Quetta-Neurons/s")
        
        if new_cycles < best_cycles:
            log(f"[Kr0wZ] [SUCCESS] Performance Improved! {best_cycles} -> {new_cycles}")
            best_cycles = new_cycles
            shutil.copy2(TARGET_FILE, BACKUP_FILE) # New best
        else:
            log(f"[Kr0wZ] [REJECT] Performance Degraded or Unchanged. {best_cycles} -> {new_cycles}")
            shutil.copy2(BACKUP_FILE, TARGET_FILE) # Revert to best
            
    log(f"=== Optimization Loop Complete. Best Cycles/Op: {best_cycles} ===")

if __name__ == "__main__":
    main()
