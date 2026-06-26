#!/usr/bin/env python3
import os
import sys
import subprocess

def main():
    if len(sys.argv) < 2:
        address = "0xd32c39fee49391c7952d1b30b15921b0d3b42e69"
        print(f"[Orchestrator] No address provided. Defaulting to LAU: {address}")
    else:
        address = sys.argv[1].lower().strip()
        
    print("=== LAU & YI UNIFIED ICON GENERATION PIPELINE ===")
    
    # 1. Generate YI Lissajous Icon
    print("\n[Step 1/3] Generating YI Lissajous Icon...")
    cmd1 = ["python3", "scripts/generate_lau_icon.py", address]
    try:
        subprocess.run(cmd1, check=True)
    except Exception as e:
        print(f"[Error] Step 1 failed: {e}")
        
    # 2. Generate Stable Diffusion textured LAU Icon
    print("\n[Step 2/3] Generating Stable Diffusion Textured Icons...")
    cmd2 = ["python3", "scripts/texture_lau_skeleton.py", address]
    try:
        subprocess.run(cmd2, check=True)
    except Exception as e:
        print(f"[Error] Step 2 failed: {e}")
        
    # 3. Generate Layered Hypotrochoid Organism Icon
    print("\n[Step 3/4] Generating Layered Hypotrochoid LAU Icon...")
    cmd3 = ["python3", "scripts/generate_hypotrochoid.py", address]
    try:
        subprocess.run(cmd3, check=True)
    except Exception as e:
        print(f"[Error] Step 3 failed: {e}")
        
    # 4. Generate Photorealistic Humanoid/Bear/Angel/Butterfly Figure Icon
    print("\n[Step 4/4] Generating Photorealistic Humanoid Figure...")
    cmd4 = ["python3", "scripts/generate_lau_humanoid.py", address]
    try:
        subprocess.run(cmd4, check=True)
    except Exception as e:
        print(f"[Error] Step 4 failed: {e}")
        
    print("\n=== PIPELINE EXECUTION COMPLETED SUCCESSFULLY ===")
    print(f"Generated assets for address {address}:")
    print(f"  1. YI Icon:                   assets/{address}_icon.png / .jpai")
    print(f"  2. YI Textured:               assets/{address}_textured.png")
    print(f"  3. LAU Textured Hypotrochoid: assets/{address}_hypotrochoid_textured.png")
    print(f"  4. Hypotrochoid LAU (Vector): assets/{address}_hypotrochoid.png / .jpai")
    print(f"  5. Humanoid Photorealistic:   assets/{address}_humanoid_photorealistic.png")

if __name__ == "__main__":
    main()
