#!/usr/bin/env python3
import json
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
RESULTS_JSON = os.path.join(WORKSPACE_DIR, "benchmarks/profiler_suite/benchmark_results.json")

# Define target thresholds for Auncient system compliance
THRESHOLDS = {
    "aho_corasick": {
        "memory_bytes": {"max": 256 * 1024 * 1024, "desc": "Wavelet Arena Memory (Sheaf Standard)"}
    },
    "aho_corasick_compositor": {
        "lookup_latency_ns": {"max": 1000.0, "desc": "Aho-Corasick Compositor Latency"}
    },
    "yang_synthesis": {
        "throughput_m_waveforms_sec": {"min": 20000.0, "desc": "AVX-512 Yang Synthesis Rate"}
    },
    "merkle_reduction": {
        "throughput_reductions_sec": {"min": 4000.0, "desc": "Helmholtz Reduction Throughput"}
    },
    "deepseek_mla": {
        "throughput_million_passes_sec": {"min": 50.0, "desc": "DeepSeek MLA Kernels Rate"}
    },
    "architectural_supremacy": {
        "efficiency_gain_x": {"min": 1.2, "desc": "Field vs Pointer Efficiency Gain"}
    },
    "zero_overhead": {
        "zero_polling_latency_us": {"max": 100.0, "desc": "Zero-Polling Hardware Latency"}
    },
    "summary": {
        "average_fps": {"min": 3000.0, "desc": "Headless Vulkan Teddy Bear FPS"}
    },
    "winchester_mq": {
        "execution_latency_ns": {"max": 15000.0, "desc": "WinchesterMQ Yul Thunk Latency"}
    },
    "rdbms_operator": {
        "verification_latency_ns": {"max": 50000.0, "desc": "RDBMS Operator Verification Latency"}
    },
    "agentic_dispatch": {
        "cache_hit_latency_ns": {"max": 100.0, "desc": "Agentic Dispatch Cache Hit Latency"},
        "cache_miss_latency_ns": {"max": 500.0, "desc": "Agentic Dispatch Cache Miss Latency"}
    },
    "knowledge_graph": {
        "propagate_latency_ns": {"max": 1000.0, "desc": "Knowledge Graph Propagation Latency"},
        "prune_latency_ns": {"max": 1000.0, "desc": "Knowledge Graph Pruning Latency"},
        "ntm_latency_ns": {"max": 15000.0, "desc": "Knowledge Graph NTM Optimization Latency"}
    }
}

def load_results():
    if not os.path.exists(RESULTS_JSON):
        print(f"[ERROR] Benchmark results file not found at: {RESULTS_JSON}")
        sys.exit(1)
    try:
        with open(RESULTS_JSON, 'r') as f:
            return json.load(f)
    except Exception as e:
        print(f"[ERROR] Failed to parse benchmark results JSON: {e}")
        sys.exit(1)

def run_performance_guard():
    print("=== Auncient Performance Guard Gates & regression Checks ===")
    data = load_results()
    all_passed = True
    
    print(f"{'Subsystem Metric / Target Description':<50} | {'Current':<15} | {'Limit':<15} | {'Status':<10}")
    print("-" * 100)
    
    for category, metrics in THRESHOLDS.items():
        if category not in data:
            print(f"[WARN] Category '{category}' missing from benchmark results. Skipping.")
            continue
        
        category_data = data[category]
        for metric_name, target in metrics.items():
            if metric_name not in category_data:
                print(f"[WARN] Metric '{metric_name}' missing from '{category}'. Skipping.")
                continue
            
            value = category_data[metric_name]
            desc = target["desc"]
            
            status = "PASS"
            limit_str = ""
            
            if "max" in target:
                limit = target["max"]
                limit_str = f"<= {limit}"
                if value > limit:
                    status = "FAIL"
                    all_passed = False
            elif "min" in target:
                limit = target["min"]
                limit_str = f">= {limit}"
                if value < limit:
                    status = "FAIL"
                    all_passed = False
            
            # Formatting values for clean output display
            if "bytes" in metric_name:
                val_display = f"{value / (1024*1024):.2f} MB"
                lim_display = f"{limit / (1024*1024):.0f} MB"
            elif "ns" in metric_name:
                val_display = f"{value:.2f} ns"
                lim_display = f"{limit:.0f} ns"
            elif "latency" in metric_name or "us" in metric_name:
                val_display = f"{value:.2f} us"
                lim_display = f"{limit:.0f} us"
            elif "gain" in metric_name or "_x" in metric_name:
                val_display = f"{value:.2f}x"
                lim_display = f"{limit:.2f}x"
            else:
                val_display = f"{value:,.2f}"
                lim_display = f"{limit:,.0f}"
                
            status_color = "\033[32m[PASS]\033[0m" if status == "PASS" else "\033[31m[FAIL]\033[0m"
            print(f"{desc:<50} | {val_display:<15} | {limit_str.split()[0]} {lim_display:<11} | {status_color}")
            
    print("-" * 100)
    if all_passed:
        print("\033[32m[SUCCESS] All Auncient performance checks passed. No regressions detected.\033[0m")
        sys.exit(0)
    else:
        print("\033[31m[FAILURE] Performance regressions or limit breaches detected in Auncient systems.\033[0m")
        sys.exit(1)

if __name__ == "__main__":
    run_performance_guard()
