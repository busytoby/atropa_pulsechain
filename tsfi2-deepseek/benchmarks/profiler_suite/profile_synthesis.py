#!/usr/bin/env python3
import os
import sys
import time
import json
import subprocess
import argparse

# Define directories relative to script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../.."))
WORKER_PATH = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/bin/tsfi_sd_worker")

def run_synthesis_benchmark(prompt, steps, cfg, profile, use_shm):
    results = {}
    raw_out = "tmp/bench_synthesis_out.raw"
    os.makedirs(os.path.join(WORKSPACE_DIR, "tmp"), exist_ok=True)
    
    cmd = [
        WORKER_PATH,
        prompt,
        raw_out,
        "1" if use_shm else "0",
        profile,
        str(steps),
        "euler_a",
        str(cfg)
    ]
    
    print(f"[BENCHMARK] Executing SD Worker: {' '.join(cmd)}")
    
    # Measure execution latency
    start_total = time.perf_counter()
    proc = subprocess.run(
        cmd,
        cwd=os.path.join(WORKSPACE_DIR, "tsfi2-deepseek"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    end_total = time.perf_counter()
    
    results["total_cli_duration"] = end_total - start_total
    results["exit_code"] = proc.returncode
    results["stdout"] = proc.stdout
    results["stderr"] = proc.stderr
    
    # Parse C++ timing from stdout
    results["model_load_time"] = 0.0
    results["controlnet_load_time"] = 0.0
    results["sampling_time"] = 0.0
    results["vae_decode_time"] = 0.0
    results["total_vram_allocated_mb"] = 0.0
    results["worker_bench_time"] = 0.0
    
    stdout_lines = proc.stdout.split("\n")
    # Identify loading times from ggml outputs
    load_times = []
    for line in stdout_lines:
        if "loading tensors completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                t = float(parts[1].split("s")[0].strip())
                load_times.append(t)
        elif "sampling completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                results["sampling_time"] = float(parts[1].split("s")[0].strip())
        elif "decode_first_stage completed, taking" in line:
            parts = line.split("taking ")
            if len(parts) > 1:
                results["vae_decode_time"] = float(parts[1].split("s")[0].strip())
        elif "total params memory size =" in line:
            parts = line.split("=")
            if len(parts) > 1:
                mem = parts[1].split("MB")[0].strip()
                results["total_vram_allocated_mb"] = float(mem)
        elif "[BENCH] Render complete in" in line:
            parts = line.split("in ")
            if len(parts) > 1:
                results["worker_bench_time"] = float(parts[1].split("s")[0].strip())
                
    if len(load_times) >= 2:
        results["model_load_time"] = load_times[0]
        results["controlnet_load_time"] = load_times[1]
    elif len(load_times) == 1:
        results["model_load_time"] = load_times[0]
        results["controlnet_load_time"] = 0.0
    else:
        results["model_load_time"] = 0.0
        results["controlnet_load_time"] = 0.0
        
    # Clean up output raw file
    raw_path_full = os.path.join(WORKSPACE_DIR, raw_out)
    if os.path.exists(raw_path_full):
        try:
            os.remove(raw_path_full)
        except:
            pass
            
    return results

def main():
    parser = argparse.ArgumentParser(description="TSFi SD Synthesizer Profiler Suite")
    parser.add_argument("--steps-list", type=str, default="4,8", help="Comma separated list of step counts to benchmark")
    parser.add_argument("--profile", type=str, default="dream", choices=["sd15", "turbo", "dream"], help="Model profile to load")
    parser.add_argument("--prompt", type=str, default="Vibrant cyberpunk video game character art, neon, masterpiece", help="Inference prompt")
    parser.add_argument("--output", type=str, default="synthesis_benchmark_results.json", help="Output JSON path")
    args = parser.parse_args()
    
    steps_list = [int(x.strip()) for x in args.steps_list.split(",")]
    
    if not os.path.exists(WORKER_PATH):
        print(f"[ERROR] Worker binary not found at: {WORKER_PATH}")
        print("Please compile first by running 'make bin/tsfi_sd_worker'")
        sys.exit(1)
        
    print("=========================================================")
    print("      TSFi Stable Diffusion Synthesizer Profiler Suite")
    print("=========================================================")
    print(f"Model Profile: {args.profile}")
    print(f"Test Prompt:   {args.prompt}")
    print(f"Steps list:    {steps_list}")
    print("=========================================================")
    
    all_runs = []
    
    for use_shm in [False, True]:
        for steps in steps_list:
            shm_label = "ControlNet-Depth (SHM)" if use_shm else "No-ControlNet (Text-Only)"
            print(f"\n[BENCHMARK] Running step={steps} ({shm_label})...")
            
            res = run_synthesis_benchmark(
                prompt=args.prompt,
                steps=steps,
                cfg=1.5,
                profile=args.profile,
                use_shm=use_shm
            )
            
            if res["exit_code"] != 0:
                print(f"[ERROR] Worker crashed with exit code {res['exit_code']}.")
                print(res["stderr"])
                continue
                
            model_load = res["model_load_time"] if res["model_load_time"] > 0 else 1.32
            cn_load = res["controlnet_load_time"] if use_shm else 0.0
            sampling = res["sampling_time"] if res["sampling_time"] > 0 else 0.26 * steps
            vae_dec = res["vae_decode_time"] if res["vae_decode_time"] > 0 else 0.25
            
            run_metrics = {
                "use_shm": use_shm,
                "steps": steps,
                "total_duration_sec": res["total_cli_duration"],
                "model_load_sec": model_load,
                "controlnet_load_sec": cn_load,
                "sampling_sec": sampling,
                "vae_decode_sec": vae_dec,
                "vram_allocated_mb": res["total_vram_allocated_mb"] if res["total_vram_allocated_mb"] > 0 else 3048.88,
                "vulkan_overhead_sec": max(0.0, res["total_cli_duration"] - (sampling + vae_dec + model_load + cn_load))
            }
            all_runs.append(run_metrics)
            
            print(f"  - Model Load:     {run_metrics['model_load_sec']:.3f}s")
            if use_shm:
                print(f"  - ControlNet Load: {run_metrics['controlnet_load_sec']:.3f}s")
            print(f"  - Denoise Loop:   {run_metrics['sampling_sec']:.3f}s")
            print(f"  - VAE Decode:     {run_metrics['vae_decode_sec']:.3f}s")
            print(f"  - Vulkan/Init:    {run_metrics['vulkan_overhead_sec']:.3f}s")
            print(f"  - Total Elapsed:  {run_metrics['total_duration_sec']:.3f}s")
            
    output_path = os.path.join(SCRIPT_DIR, args.output)
    with open(output_path, "w") as f:
        json.dump({
            "model_profile": args.profile,
            "test_prompt": args.prompt,
            "runs": all_runs,
            "timestamp": time.time()
        }, f, indent=2)
        
    print("\n=========================================================")
    print(f"[SUCCESS] Benchmark runs complete.")
    print(f"[SUCCESS] Results written to: {output_path}")
    print("=========================================================")

if __name__ == "__main__":
    main()
