#!/usr/bin/env python3
import json
import os
import subprocess
import time

def get_git_commit():
    try:
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'], stderr=subprocess.DEVNULL).decode('utf-8').strip()
    except Exception:
        return "unknown"

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    results_path = os.path.join(script_dir, "benchmark_results.json")
    manifest_path = os.path.join(script_dir, "agy_benchmark_manifest.json")
    
    if not os.path.exists(results_path):
        print(f"[ERROR] Results file not found at: {results_path}")
        return
        
    try:
        with open(results_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        print(f"[ERROR] Failed to load results: {e}")
        return

    # Extract metrics
    vk_fps = data.get("summary", {}).get("average_fps", 0.0)
    ac_mem = data.get("aho_corasick", {}).get("memory_bytes", 0.0) / (1024*1024)
    ac_comp_latency = data.get("aho_corasick_compositor", {}).get("lookup_latency_ns", 0.0)
    yang_rate = data.get("yang_synthesis", {}).get("throughput_m_waveforms_sec", 0.0)
    merkle_rate = data.get("merkle_reduction", {}).get("throughput_reductions_sec", 0.0)
    mla_rate = data.get("deepseek_mla", {}).get("throughput_million_passes_sec", 0.0)
    supremacy_gain = data.get("architectural_supremacy", {}).get("efficiency_gain_x", 1.0)
    zero_poll_latency = data.get("zero_overhead", {}).get("zero_polling_latency_us", 0.0)

    # Format the standardized agy manifest structure
    manifest = {
        "manifest_format": "agy-benchmark-v1.0",
        "product_suite": "TSFi2 RDNA4 Simulation Engine",
        "engine_version": "4.1",
        "provenance": {
            "git_commit": get_git_commit(),
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "licensing": "ATROPA/DYSNOMIA"
        },
        "system_policy_guards": {
            "vulkan_teddy_bear": {
                "metric": "Headless Vulkan Teddy Bear FPS",
                "target": ">= 3000.0 FPS",
                "verified_actual": f"{vk_fps:,.2f} FPS",
                "status": "PASS" if vk_fps >= 3000.0 else "FAIL"
            },
            "wavelet_arena_memory": {
                "metric": "Wavelet Arena Memory (Sheaf Standard)",
                "target": "<= 256.00 MB",
                "verified_actual": f"{ac_mem:.2f} MB",
                "status": "PASS" if ac_mem <= 256.0 else "FAIL"
            },
            "aho_corasick_compositor": {
                "metric": "Aho-Corasick Compositor Latency",
                "target": "<= 1000.0 ns",
                "verified_actual": f"{ac_comp_latency:.2f} ns",
                "status": "PASS" if ac_comp_latency <= 1000.0 else "FAIL"
            },
            "yang_synthesis": {
                "metric": "AVX-512 Yang Synthesis Rate",
                "target": ">= 20000.0 Waveforms/sec",
                "verified_actual": f"{yang_rate:,.2f} Waveforms/sec",
                "status": "PASS" if yang_rate >= 20000.0 else "FAIL"
            },
            "merkle_reduction": {
                "metric": "Helmholtz Reduction Throughput",
                "target": ">= 4000.0 ns",
                "verified_actual": f"{merkle_rate:.2f} ns",
                "status": "PASS" if merkle_rate >= 4000.0 else "FAIL"
            },
            "deepseek_mla": {
                "metric": "DeepSeek MLA Kernels Rate",
                "target": ">= 90.0 MPasses/sec",
                "verified_actual": f"{mla_rate:.2f} MPasses/sec",
                "status": "PASS" if mla_rate >= 90.0 else "FAIL"
            },
            "architectural_supremacy": {
                "metric": "Field vs Pointer Efficiency Gain",
                "target": ">= 1.20x",
                "verified_actual": f"{supremacy_gain:.2f}x",
                "status": "PASS" if supremacy_gain >= 1.20 else "FAIL"
            },
            "zero_polling": {
                "metric": "Zero-Polling Hardware Latency",
                "target": "<= 100.0 us",
                "verified_actual": f"{zero_poll_latency:.2f} us",
                "status": "PASS" if zero_poll_latency <= 100.0 else "FAIL"
            },
            "winchester_mq": {
                "metric": "WinchesterMQ Yul Thunk Latency",
                "target": "<= 15000.0 ns",
                "verified_actual": f"{data.get('winchester_mq', {}).get('execution_latency_ns', 0.0):.2f} ns",
                "status": "PASS" if data.get('winchester_mq', {}).get('execution_latency_ns', 0.0) <= 15000.0 else "FAIL"
            },
            "rdbms_operator": {
                "metric": "RDBMS Operator Verification Latency",
                "target": "<= 50000.0 ns",
                "verified_actual": f"{data.get('rdbms_operator', {}).get('verification_latency_ns', 0.0):.2f} ns",
                "status": "PASS" if data.get('rdbms_operator', {}).get('verification_latency_ns', 0.0) <= 50000.0 else "FAIL"
            }
        }
    }
    
    try:
        with open(manifest_path, 'w') as f:
            json.dump(manifest, f, indent=2)
        print(f"[SUCCESS] Standardized agy benchmark manifest generated at: {manifest_path}")
    except Exception as e:
        print(f"[ERROR] Failed to write manifest: {e}")

if __name__ == "__main__":
    main()
