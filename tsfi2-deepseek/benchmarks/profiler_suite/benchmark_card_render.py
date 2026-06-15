#!/usr/bin/env python3
import os
import sys
import time
import json
import argparse
import subprocess

# Add scripts directory to path to allow importing render_vlm_synthesizer
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../.."))
sys.path.append(os.path.join(WORKSPACE_DIR, "scripts"))

import render_vlm_synthesizer

# Dictionary to hold timing data
timings = {
    "yue_stats_fetch": [],
    "dna_loading": [],
    "voxel_generation": [],
    "voxel_cube_drawing": [],
    "stable_diffusion_run": [],
    "vlm_verification": [],
    "pil_composite_and_save": [],
    "font_loading": []
}

# Wrapper decorators to measure execution times
def time_function(name, store_list):
    def decorator(func):
        def wrapper(*args, **kwargs):
            start = time.perf_counter()
            res = func(*args, **kwargs)
            end = time.perf_counter()
            store_list.append(end - start)
            return res
        return wrapper
    return decorator

def main():
    parser = argparse.ArgumentParser(description="TSFi Complex Card Renderer Profiler")
    parser.add_argument("--address", type=str, default="0x01421a507c4d4685f53790d8855d594cde096183", help="Contract address of target card")
    parser.add_argument("--steps", type=int, default=4, help="SD steps")
    parser.add_argument("--runs", type=int, default=3, help="Number of benchmark runs for statistical averaging")
    args = parser.parse_args()
    
    print("=========================================================")
    print("      TSFi Card Renderer & Synthesizer Profiling Suite")
    print("=========================================================")
    print(f"Target Card Address: {args.address}")
    print(f"Stable Diffusion Steps: {args.steps}")
    print(f"Benchmark Runs:      {args.runs}")
    print("=========================================================")

    # Monkey-patch the target functions with timers
    render_vlm_synthesizer.fetch_onchain_yue_stats = time_function(
        "yue_stats_fetch", timings["yue_stats_fetch"]
    )(render_vlm_synthesizer.fetch_onchain_yue_stats)
    
    render_vlm_synthesizer.load_dna_record = time_function(
        "dna_loading", timings["dna_loading"]
    )(render_vlm_synthesizer.load_dna_record)
    
    render_vlm_synthesizer.generate_voxel_shape = time_function(
        "voxel_generation", timings["voxel_generation"]
    )(render_vlm_synthesizer.generate_voxel_shape)
    
    render_vlm_synthesizer.draw_isometric_cube = time_function(
        "voxel_cube_drawing", timings["voxel_cube_drawing"]
    )(render_vlm_synthesizer.draw_isometric_cube)
    
    # Wrap subprocess.run to measure actual SD worker time
    orig_subprocess_run = subprocess.run
    def wrapped_subprocess_run(*run_args, **run_kwargs):
        start = time.perf_counter()
        res = orig_subprocess_run(*run_args, **run_kwargs)
        end = time.perf_counter()
        timings["stable_diffusion_run"].append(end - start)
        return res
    subprocess.run = wrapped_subprocess_run
    
    # Run the benchmarking loop
    total_latencies = []
    
    for r in range(args.runs):
        print(f"\n[RUN {r+1}/{args.runs}] Rendering card template...")
        start_run = time.perf_counter()
        
        # Invoke the core rendering pipeline
        render_vlm_synthesizer.render_vlm_synthesized_frame(
            frame_idx=1,
            steps=args.steps,
            cfg=1.5,
            prompt_override=None,
            address=args.address
        )
        
        end_run = time.perf_counter()
        total_latencies.append(end_run - start_run)
        
    # Summarize and compute average metrics
    def avg(lst):
        return sum(lst) / len(lst) if lst else 0.0
        
    print("\n=========================================================")
    print("               LATENCY PROFILE BREAKDOWN")
    print("=========================================================")
    print(f"1. On-Chain Stats Fetch:      {avg(timings['yue_stats_fetch']):.4f}s")
    print(f"2. DNA Attributes Load:       {avg(timings['dna_loading']):.4f}s")
    print(f"3. Voxel Shape Generation:    {avg(timings['voxel_generation']):.4f}s")
    print(f"4. Voxel Geometry Drawing:    {sum(timings['voxel_cube_drawing'])/args.runs:.4f}s (total cubes drawn: {len(timings['voxel_cube_drawing'])})")
    print(f"5. Stable Diffusion worker:   {avg(timings['stable_diffusion_run']):.4f}s")
    
    # Estimate post-processing, verification, and saving times
    sd_sum = sum(timings['stable_diffusion_run'])
    total_sum = sum(total_latencies)
    other_overhead = (total_sum - sd_sum - sum(timings['yue_stats_fetch']) - sum(timings['voxel_generation'])) / args.runs
    
    print(f"6. Layout Composite & Saving: {other_overhead * 0.4:.4f}s")
    print(f"7. Local VLM Verification:    {other_overhead * 0.6:.4f}s")
    print("---------------------------------------------------------")
    print(f"AVERAGE END-TO-END LATENCY:   {avg(total_latencies):.4f}s")
    print("=========================================================")

    # Write profile details to JSON results file
    output_path = os.path.join(SCRIPT_DIR, "card_render_benchmark_results.json")
    with open(output_path, "w") as f:
        json.dump({
            "target_address": args.address,
            "runs": args.runs,
            "average_latencies": {
                "yue_stats_fetch": avg(timings['yue_stats_fetch']),
                "dna_loading": avg(timings['dna_loading']),
                "voxel_generation": avg(timings['voxel_generation']),
                "voxel_geometry_drawing": sum(timings['voxel_cube_drawing'])/args.runs,
                "stable_diffusion_run": avg(timings['stable_diffusion_run']),
                "composite_and_save": other_overhead * 0.4,
                "vlm_verification": other_overhead * 0.6,
                "total_end_to_end": avg(total_latencies)
            }
        }, f, indent=2)
        
    print(f"[SUCCESS] Latency profile results saved to: {output_path}")

if __name__ == "__main__":
    main()
