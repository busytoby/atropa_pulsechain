#!/usr/bin/env python3
import os
import sys
import time
import json
import re
import subprocess
import argparse

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../.."))
WORKER_PATH = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/bin/tsfi_sd_worker")

def run_worker_collect_logs(prompt, steps, profile):
    # Run text-only render
    cmd = [
        WORKER_PATH,
        prompt,
        "tmp/subproc_bench.raw",
        "0", # use_shm = 0
        profile,
        str(steps),
        "euler_a",
        "1.5"
    ]
    
    proc = subprocess.run(
        cmd,
        cwd=os.path.join(WORKSPACE_DIR, "tsfi2-deepseek"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # Clean up output raw file
    raw_path = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/tmp/subproc_bench.raw")
    if os.path.exists(raw_path):
        try: os.remove(raw_path)
        except: pass
        
    return proc.stdout, proc.returncode

def parse_subprocess_timings(stdout_str):
    metrics = {
        "clip_encode_ms": 0.0,
        "unet_sampling_sec": 0.0,
        "vae_decode_sec": 0.0,
        "total_generate_sec": 0.0
    }
    
    # Look for: get_learned_condition completed, taking XX ms
    clip_match = re.search(r"get_learned_condition completed, taking (\d+) ms", stdout_str)
    if clip_match:
        metrics["clip_encode_ms"] = float(clip_match.group(1))
        
    # Look for: sampling completed, taking XXs
    unet_match = re.search(r"sampling completed, taking ([\d.]+)s", stdout_str)
    if unet_match:
        metrics["unet_sampling_sec"] = float(unet_match.group(1))
        
    # Look for: decode_first_stage completed, taking XXs
    vae_match = re.search(r"decode_first_stage completed, taking ([\d.]+)s", stdout_str)
    if vae_match:
        metrics["vae_decode_sec"] = float(vae_match.group(1))
        
    # Look for: generate_image completed in XXs
    total_match = re.search(r"generate_image completed in ([\d.]+)s", stdout_str)
    if total_match:
        metrics["total_generate_sec"] = float(total_match.group(1))
        
    return metrics

def main():
    parser = argparse.ArgumentParser(description="TSFi Stable Diffusion Sub-Processes Benchmarker")
    parser.add_argument("--profile", type=str, default="dream", choices=["sd15", "turbo", "dream"])
    args = parser.parse_args()
    
    if not os.path.exists(WORKER_PATH):
        print(f"[ERROR] Worker binary not found at: {WORKER_PATH}")
        sys.exit(1)
        
    print("=========================================================")
    print("     TSFi Stable Diffusion Sub-Processes Profiler")
    print("=========================================================")
    print(f"Model Profile: {args.profile}")
    print("=========================================================")
    
    prompts = {
        "short": "Simple coin art",
        "medium": "Vibrant cyberpunk trading card game vector art, neon outlines, high detail",
        "long": "Masterpiece retro-futuristic trading card game vector art, glowing neon laser grids, isometric voxel frame showing a detailed sentinel shield, gouache painted colors, maximum fidelity sci-fi aesthetic, 1980s retro cyber theme, high contrast"
    }
    
    steps_list = [4, 8, 15]
    
    prompt_runs = []
    step_runs = []
    
    # 1. Profile CLIP Text Encoding across different prompt lengths
    print("\n[STAGE 1] Profiling CLIP Text Encoding vs Prompt Length...")
    for p_name, p_text in prompts.items():
        print(f"  - Running with {p_name} prompt (len={len(p_text)} chars)...")
        stdout, code = run_worker_collect_logs(p_text, 4, args.profile)
        if code != 0:
            print(f"    [ERROR] Execution failed for prompt: {p_name}")
            continue
        metrics = parse_subprocess_timings(stdout)
        prompt_runs.append({
            "prompt_type": p_name,
            "prompt_length": len(p_text),
            "clip_encode_ms": metrics["clip_encode_ms"]
        })
        print(f"    >> CLIP Encode Time: {metrics['clip_encode_ms']:.1f} ms")
        
    # 2. Profile UNet Sampling across different step counts
    print("\n[STAGE 2] Profiling UNet Sampling vs Step Count...")
    base_prompt = prompts["medium"]
    for steps in steps_list:
        print(f"  - Running with step_count={steps}...")
        stdout, code = run_worker_collect_logs(base_prompt, steps, args.profile)
        if code != 0:
            print(f"    [ERROR] Execution failed for steps: {steps}")
            continue
        metrics = parse_subprocess_timings(stdout)
        step_runs.append({
            "steps": steps,
            "unet_sampling_sec": metrics["unet_sampling_sec"],
            "vae_decode_sec": metrics["vae_decode_sec"],
            "total_generate_sec": metrics["total_generate_sec"]
        })
        sec_per_step = metrics["unet_sampling_sec"] / steps
        print(f"    >> UNet Sampling: {metrics['unet_sampling_sec']:.3f} s ({sec_per_step:.3f} s/step)")
        print(f"    >> VAE Decode:     {metrics['vae_decode_sec']:.3f} s")
        print(f"    >> Total Gen:      {metrics['total_generate_sec']:.3f} s")
        
    # Output to JSON
    output_path = os.path.join(SCRIPT_DIR, "subprocess_benchmark_results.json")
    with open(output_path, "w") as f:
        json.dump({
            "model_profile": args.profile,
            "clip_prompt_runs": prompt_runs,
            "unet_step_runs": step_runs,
            "timestamp": time.time()
        }, f, indent=2)
        
    print("\n=========================================================")
    print(f"[SUCCESS] Sub-process benchmarking complete.")
    print(f"[SUCCESS] Results written to: {output_path}")
    print("=========================================================")

if __name__ == "__main__":
    main()
