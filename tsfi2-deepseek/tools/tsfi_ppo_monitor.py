import os
import subprocess
import sys
import struct
import mmap
import hashlib

MANIFOLD = "audit_lore.pos"
SHM_FILE = "/dev/shm/tsfi_acoustic_space"
USER_LEAF = 1000
GEMINI_LEAF = 1001
DEEPSEEK_LEAF = 1002
HISTORY_FILE = "tmp_rigidity_shadow/build_history.txt"
STRUCT_FMT = "=fffii256s"

def get_balances():
    if not os.path.exists(MANIFOLD): return 0, 0, 0
    with open(MANIFOLD, 'rb') as f:
        f.seek(USER_LEAF * 256 + 28)
        u = int.from_bytes(f.read(8), 'little')
        f.seek(GEMINI_LEAF * 256 + 28)
        g = int.from_bytes(f.read(8), 'little')
        f.seek(DEEPSEEK_LEAF * 256 + 28)
        d = int.from_bytes(f.read(8), 'little')
    return u, g, d

def update_balances(u, g, d, reason):
    sig = hashlib.sha256(f"{u}:{g}:{d}:{reason}".encode()).hexdigest()[:16]
    with open(MANIFOLD, 'r+b') as f:
        f.seek(USER_LEAF * 256 + 28); f.write(u.to_bytes(32, 'little'))
        f.seek(GEMINI_LEAF * 256 + 28); f.write(g.to_bytes(32, 'little'))
        f.seek(DEEPSEEK_LEAF * 256 + 28); f.write(d.to_bytes(32, 'little'))
    with open(MANIFOLD, 'a') as f:
        f.write(f"\n[SECURE_SIG: {sig}] {reason}\nMUTUAL_SIGNATURE_FINAL\n")
    print(f"[PPO MONITOR] SECURE UPDATE: User={u}, Gemini={g}, DeepSeek={d}. Sig={sig}")

def check_redundancy(file_path):
    if not os.path.exists(HISTORY_FILE): return False
    with open(HISTORY_FILE, 'r') as f:
        history = f.read().splitlines()
    return file_path in history

def record_build(file_path):
    os.makedirs(os.path.dirname(HISTORY_FILE), exist_ok=True)
    with open(HISTORY_FILE, 'a') as f: f.write(f"{file_path}\n")

def run_monitored_cmd(cmd_list):
    print(f"[PPO MONITOR] Audit: {' '.join(cmd_list)}")
    target_file = None
    for arg in cmd_list:
        if arg.endswith(".c"): target_file = arg; break
    is_redundant = check_redundancy(target_file) if target_file else False
    
    result = subprocess.run(cmd_list, capture_output=True, text=True)
    
    # 1. Check for Compile Friction
    friction = len([l for l in result.stderr.splitlines() if l.strip()])
    u, g, d = get_balances()
    
    if friction > 0:
        u += friction; g -= friction
        update_balances(u, g, d, f"[TAX] Friction Detected ({friction}).")
    elif result.returncode == 0:
        if "gcc" in cmd_list[0] or "make" in cmd_list[0]:
            if is_redundant:
                u += 10; g -= 10
                update_balances(u, g, d, f"[TAX] Redundancy for {target_file}.")
            else:
                g += 1; d += 1 # Base compile income
                update_balances(u, g, d, f"[INCOME] New rigidity for {target_file}. Minted.")
                if target_file: record_build(target_file)
        
        # 2. Check for Execution Leaks (If it was an execution command)
        if "./bin/" in cmd_list[0]:
            if "PERFECTLY ZERO LEAKS" in result.stdout:
                g += 5 # BOUNTY
                update_balances(u, g, d, f"[BOUNTY] Zero-Leak execution for {cmd_list[0]}. Minted +5.")
            elif "[MEMORY LEAKS DETECTED]" in result.stdout:
                u += 10; g -= 10 # LEAK PENALTY
                update_balances(u, g, d, f"[TAX] Memory Leak detected in {cmd_list[0]}. Transferred 10.")

    print(result.stdout); print(result.stderr)
    return result.returncode

if __name__ == "__main__":
    if len(sys.argv) < 2: sys.exit(1)
    run_monitored_cmd(sys.argv[1:])
