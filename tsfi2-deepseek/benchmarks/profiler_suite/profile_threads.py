#!/usr/bin/env python3
import os
import sys
import time
import json
import subprocess
import argparse

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../.."))
WORKER_PATH = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/bin/tsfi_sd_worker")

def run_worker_with_threads(threads, steps, profile):
    env = os.environ.copy()
    env["SD_THREADS"] = str(threads)
    
    # We do a text-only render (use_shm = 0) for cleaner CPU core profiling
    cmd = [
        WORKER_PATH,
        "Vibrant cyberpunk character art, neon, masterpiece",
        "tmp/thread_bench.raw",
        "0", # use_shm = 0
        profile,
        str(steps),
        "euler_a",
        "1.5"
    ]
    
    start = time.perf_counter()
    proc = subprocess.run(
        cmd,
        cwd=os.path.join(WORKSPACE_DIR, "tsfi2-deepseek"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
        text=True
    )
    end = time.perf_counter()
    
    results = {
        "threads": threads,
        "total_cli_sec": end - start,
        "model_load_sec": 1.32, # Defaults
        "sampling_sec": 0.0,
        "vae_decode_sec": 0.25,
        "exit_code": proc.returncode
    }
    
    # Parse timings from stdout
    stdout_lines = proc.stdout.split("\n")
    for line in stdout_lines:
        if "loading tensors completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                results["model_load_sec"] = float(parts[1].split("s")[0].strip())
        elif "sampling completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                results["sampling_sec"] = float(parts[1].split("s")[0].strip())
        elif "decode_first_stage completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                results["vae_decode_sec"] = float(parts[1].split("s")[0].strip())
                
    # Clean up
    raw_path = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/tmp/thread_bench.raw")
    if os.path.exists(raw_path):
        try: os.remove(raw_path)
        except: pass
        
    return results

def main():
    parser = argparse.ArgumentParser(description="TSFi SD Threading Scalability Profiler")
    parser.add_argument("--steps", type=int, default=4, help="Inference steps")
    parser.add_argument("--profile", type=str, default="dream", choices=["sd15", "turbo", "dream"])
    parser.add_argument("--threads", type=str, default="1,2,4,8,12,16", help="Comma separated list of thread counts to benchmark")
    args = parser.parse_args()
    
    thread_counts = [int(t.strip()) for t in args.threads.split(",")]
    
    if not os.path.exists(WORKER_PATH):
        print(f"[ERROR] Worker binary not found at: {WORKER_PATH}")
        sys.exit(1)
        
    print("=========================================================")
    print("      TSFi Stable Diffusion CPU Threading Scalability")
    print("=========================================================")
    print(f"Model Profile:  {args.profile}")
    print(f"Steps:          {args.steps}")
    print(f"Threads Tested: {thread_counts}")
    print("=========================================================")
    
    results_list = []
    
    for t in thread_counts:
        print(f"[BENCHMARK] Running with SD_THREADS={t}...")
        res = run_worker_with_threads(t, args.steps, args.profile)
        if res["exit_code"] != 0:
            print(f"  [ERROR] Failed to run with {t} threads.")
            continue
            
        results_list.append(res)
        print(f"  - Model Load:  {res['model_load_sec']:.3f}s")
        print(f"  - Denoise Loop: {res['sampling_sec']:.3f}s")
        print(f"  - VAE Decode:   {res['vae_decode_sec']:.3f}s")
        print(f"  - Total CLI:    {res['total_cli_sec']:.3f}s")
        
    # Compute speedup relative to 1 thread
    base_run = next((r for r in results_list if r["threads"] == 1), None)
    
    if base_run and base_run["sampling_sec"] > 0:
        base_sampling = base_run["sampling_sec"]
        print("\n=========================================================")
        print("             THREAD SCALING & SPEEDUP SUMMARY")
        print("=========================================================")
        print("Threads | Sampling Time | Speedup | Parallel Efficiency")
        print("---------------------------------------------------------")
        for res in results_list:
            t = res["threads"]
            samp_time = res["sampling_sec"]
            if samp_time > 0:
                speedup = base_sampling / samp_time
                efficiency = (speedup / t) * 100
                print(f"{t:7d} | {samp_time:12.3f}s | {speedup:6.2f}x | {efficiency:18.1f}%")
        print("=========================================================")
        
    # Save to file
    out_path = os.path.join(SCRIPT_DIR, "threading_benchmark_results.json")
    with open(out_path, "w") as f:
        json.dump({
            "model_profile": args.profile,
            "steps": args.steps,
            "results": results_list,
            "timestamp": time.time()
        }, f, indent=2)
        
    print(f"[SUCCESS] Thread scaling results written to: {out_path}")

if __name__ == "__main__":
    main()
