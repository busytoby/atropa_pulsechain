#!/usr/bin/env python3
import json
import os
import sys

def load_json(filepath):
    if not os.path.exists(filepath):
        return None
    try:
        with open(filepath, 'r') as f:
            return json.load(f)
    except Exception:
        return None

def extract_flat_metrics(data):
    flat = {}
    if not data:
        return flat
    
    # Extract summary metrics
    if "summary" in data and isinstance(data["summary"], dict):
        for k, v in data["summary"].items():
            if isinstance(v, (int, float)):
                flat[f"summary.{k}"] = (v, "Headless Vulkan Teddy Bear FPS" if k == "average_fps" else k)
                
    # Extract subsystem metrics
    for section, metrics in data.items():
        if section in ["summary", "frame_times_seconds", "audio_latency_records"]:
            continue
        if isinstance(metrics, dict):
            for k, v in metrics.items():
                if isinstance(v, (int, float)):
                    # Provide friendly description mapped from performance_guard or key name
                    desc = f"{section} {k}".replace("_", " ").title()
                    flat[f"{section}.{k}"] = (v, desc)
    return flat

def main():
    if len(sys.argv) < 2:
        print("Usage: compare_benchmarks.py <baseline_results.json> [current_results.json]")
        sys.exit(1)
        
    baseline_path = sys.argv[1]
    current_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), "benchmark_results.json")
    
    baseline_data = load_json(baseline_path)
    current_data = load_json(current_path)
    
    if not baseline_data:
        print(f"[ERROR] Baseline file not found or invalid: {baseline_path}")
        sys.exit(1)
    if not current_data:
        print(f"[ERROR] Current results file not found or invalid: {current_path}")
        sys.exit(1)
        
    baseline_metrics = extract_flat_metrics(baseline_data)
    current_metrics = extract_flat_metrics(current_data)
    
    print("\n=== Auncient System Benchmark Comparison (benchcmp) ===")
    print(f"Baseline: {os.path.basename(baseline_path)}")
    print(f"Current:  {os.path.basename(current_path)}")
    print("-" * 90)
    print(f"{'Metric / Subsystem Description':<45} | {'Baseline':<12} | {'Current':<12} | {'Delta':<10}")
    print("-" * 90)
    
    regressions_detected = False
    
    for key, (base_val, desc) in baseline_metrics.items():
        if key not in current_metrics:
            continue
        
        curr_val, _ = current_metrics[key]
        if base_val == 0:
            delta_pct = 0.0
        else:
            delta_pct = ((curr_val - base_val) / base_val) * 100.0
            
        # Determine if regression (lower is better for latency/memory, higher is better for throughput)
        is_lower_better = "latency" in key or "bytes" in key or "duration" in key or "ms" in key or "ns" in key or "us" in key
        
        if is_lower_better:
            is_regression = delta_pct > 5.0 # Regressed by more than 5%
            is_improvement = delta_pct < -5.0
        else:
            is_regression = delta_pct < -5.0 # Regressed by more than 5%
            is_improvement = delta_pct > 5.0
            
        # Color coding
        if is_regression:
            delta_str = f"\033[31m{delta_pct:+.2f}%\033[0m (REGRESSION)"
            regressions_detected = True
        elif is_improvement:
            delta_str = f"\033[32m{delta_pct:+.2f}%\033[0m (SPEEDUP)"
        else:
            delta_str = f"{delta_pct:+.2f}%"
            
        print(f"{desc:<45} | {base_val:<12,.2f} | {curr_val:<12,.2f} | {delta_str:<10}")
        
    print("-" * 90)
    if regressions_detected:
        print("\033[31m[WARNING] Performance regressions detected compared to baseline results.\033[0m")
        sys.exit(1)
    else:
        print("\033[32m[SUCCESS] No significant performance regressions detected. All operations stable.\033[0m")
        sys.exit(0)

if __name__ == "__main__":
    main()
