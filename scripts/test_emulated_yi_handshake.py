#!/usr/bin/env python3
import subprocess
import sys
import os

def main():
    print("=== Auncient Emulated YI Handshake Alpha Test Runner ===")
    
    # Paths
    src_file = "tsfi2-deepseek/tests/test_heltec_ook_kermit.c"
    bin_file = "tsfi2-deepseek/tests/test_heltec_ook_kermit"
    
    if not os.path.exists(src_file):
        print(f"[FAIL] Emulation test source file not found at: {src_file}")
        sys.exit(1)
        
    # Compile the emulation test bed using GCC
    print(f"[BUILD] Compiling emulated test bed source {src_file}...")
    compile_cmd = ["gcc", "-O2", "-Wall", src_file, "-o", bin_file]
    
    proc = subprocess.run(compile_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.returncode != 0:
        print(f"[FAIL] Compilation failed:\n{proc.stderr}")
        sys.exit(1)
        
    print("[BUILD] Compilation successful.")
    
    # Run the emulated test bed
    print("[RUN] Executing emulated C test loop...")
    proc_run = subprocess.run([f"./{bin_file}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    print("\n--- Emulation Output Stream ---")
    print(proc_run.stdout.strip())
    print("--------------------------------\n")
    
    if proc_run.returncode != 0:
        print(f"[FAIL] Emulated execution crashed with code {proc_run.returncode}:\n{proc_run.stderr}")
        sys.exit(1)
        
    # Verify key milestones in output
    print("[VERIFY] Validating key milestones in output stream...")
    output = proc_run.stdout
    
    milestones = [
        "Auncient Emulated Helmholtz 9-Epoch Handshake (Rod A <=> Cone B)",
        "Swapping Contour registers...",
        "Swapping Pole registers...",
        "Swapping Foundation registers...",
        "Helmholtz convergence established! YI =",
        "Generating YI.react Nonce Signatures:",
        "Nonce 0 | Ichidai:",
        "Nonce 1 | Ichidai:",
        "Nonce 2 | Ichidai:",
        "Building Internal APOGEE YI (Private Rod & Cone state on Device A)",
        "Private Device APOGEE YI established! YI =",
        "Generating Private APOGEE YI Nonce Signatures:",
        "APOGEE Nonce 0 | Ichidai:"
    ]
    
    all_passed = True
    for milestone in milestones:
        if milestone in output:
            print(f"  [PASS] Found milestone: \"{milestone}\"")
        else:
            print(f"  [FAIL] Missing milestone: \"{milestone}\"")
            all_passed = False
            
    if all_passed:
        print("\n[RESULT] [SUCCESS] All emulated YI handshake and nonce verification checks PASSED!")
        sys.exit(0)
    else:
        print("\n[RESULT] [FAILED] One or more verification milestones failed in emulation.")
        sys.exit(1)

if __name__ == '__main__':
    main()
