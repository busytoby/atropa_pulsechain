import subprocess
import time
import re
import os
import datetime

BENCHMARKS = [
    {"name": "MLA Kernel", "cmd": "bin/bench_deepseek_mla", "metric_regex": r"Compute Bandwidth:\s+([0-9.]+)\s+GFLOPS/sec"},
    {"name": "MLA Dense Context", "cmd": "bin/bench_deepseek_mla_dense", "metric_regex": r"Computational Velocity:\s+([0-9.]+)\s+GFLOPS/sec"},
    {"name": "GEMV Scan", "cmd": "bin/bench_deepseek_gemv", "metric_regex": r"Computational Velocity:\s+([0-9.]+)\s+GFLOPS/sec"}
]

MANIFOLD_PATH = "audit_lore.pos"
POLL_INTERVAL = 3600  # Run every hour by default, or as a one-shot loop

def run_cmd(cmd):
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        return True, result.stdout
    except subprocess.CalledProcessError as e:
        return False, e.stdout + "\\n" + e.stderr

def post_to_manifold(log_text):
    print(f"-> Solidifying into manifold: {log_text}")
    # Escape quotes
    safe_text = log_text.replace('"', "'").replace('\\n', ' ')
    
    # Generate deterministic pseudo-random offset based on timestamp to avoid collision
    offset = int(time.time()) % 1024
    cmd = f"./tsfi_helmholtz_cli post_lore {MANIFOLD_PATH} {offset} \"{safe_text}\""
    success, output = run_cmd(cmd)
    if success:
        print(f"   [ANCHOR] Posted to Leaf {offset}")
    else:
        print(f"   [FRACTURE] Failed to post: {output}")

def main():
    print("=== DeepSeek Architectural Autonomous Benchmark Loop ===")
    
    while True:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"\\n--- Benchmark Cycle: {timestamp} ---")
        
        # Ensure binaries are built
        run_cmd("make all")
        
        for bench in BENCHMARKS:
            print(f"-> Executing {bench['name']}...")
            success, out = run_cmd(bench["cmd"])
            
            if success:
                match = re.search(bench["metric_regex"], out)
                if match:
                    metric = match.group(1)
                    log_text = f"BENCHMARK [{timestamp}] {bench['name']}: {metric} GFLOPS/sec verified."
                    print(f"   [SUCCESS] {metric} GFLOPS/sec")
                    post_to_manifold(log_text)
                else:
                    print(f"   [WARN] Metric regex failed to parse output.")
            else:
                print(f"   [FRACTURE] Benchmark {bench['name']} failed execution.")
        
        print(f"\\n--- Cycle Complete. Sleeping for {POLL_INTERVAL} seconds. ---")
        time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    main()
