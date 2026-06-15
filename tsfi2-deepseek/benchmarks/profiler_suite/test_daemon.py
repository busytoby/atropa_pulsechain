#!/usr/bin/env python3
import os
import sys
import time
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../.."))
WORKER_PATH = os.path.join(WORKSPACE_DIR, "tsfi2-deepseek/bin/tsfi_sd_worker")

def read_until_token(stream, tokens):
    while True:
        line = stream.readline()
        if not line:
            return ""
        stripped = line.strip()
        if any(stripped.startswith(t) for t in tokens):
            return stripped
        # Print progress bars and other debug lines to stderr
        if stripped:
            sys.stderr.write(f"  [worker] {line}")

def main():
    print("=========================================================")
    print("    Stable Diffusion Resident Daemon (Stdin Mode) Test")
    print("=========================================================")
    
    cmd = [
        WORKER_PATH,
        "stdin",
        "tmp/daemon_test_out.raw",
        "0", # use_shm = 0 (text-only for test)
        "dream",
        "4",
        "euler_a",
        "1.5"
    ]
    
    env = os.environ.copy()
    env["SD_THREADS"] = "8"
    
    print("[DAEMON] Starting Stable Diffusion worker daemon...")
    start_init = time.perf_counter()
    proc = subprocess.Popen(
        cmd,
        cwd=os.path.join(WORKSPACE_DIR, "tsfi2-deepseek"),
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
        text=True
    )
    
    # Wait for READY token from stdout
    ready_line = read_until_token(proc.stdout, ["READY"])
    end_init = time.perf_counter()
    print(f"[DAEMON] Daemon status: {ready_line}")
    print(f"[DAEMON] Startup/Weight load time: {end_init - start_init:.3f} seconds\n")
    
    prompts = [
        "Gold shiny coin, retro futuristic vector style, masterpiece",
        "Sentinel shield neon vector logo, futuristic sci-fi game card, clean borders"
    ]
    
    for idx, prompt in enumerate(prompts):
        print(f"[TEST {idx+1}] Sending prompt: {prompt}")
        start_gen = time.perf_counter()
        
        proc.stdin.write(prompt + "\n")
        proc.stdin.flush()
        
        # Read response
        response_line = read_until_token(proc.stdout, ["SUCCESS", "ERROR"])
        end_gen = time.perf_counter()
        
        print(f"  - Output: {response_line}")
        print(f"  - Generation Time: {end_gen - start_gen:.3f} seconds (0s model loading!)")
        
    print("\n[DAEMON] Shutting down daemon...")
    proc.stdin.write("EXIT\n")
    proc.stdin.flush()
    proc.wait()
    print("[SUCCESS] Daemon test complete.")

if __name__ == "__main__":
    main()
