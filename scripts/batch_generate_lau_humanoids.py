#!/usr/bin/env python3
import os
import sys
import json
import glob
import subprocess

def main():
    print("[Batch Generator] Scanning split registries for active treasury tokens...")
    
    # 1. Load active (non-ignored) addresses
    active_tokens = {}
    for path in glob.glob("assets/treasury_tokens_*.json"):
        try:
            with open(path, "r") as f:
                data = json.load(f)
                for addr, info in data.items():
                    addr_lower = addr.lower().strip()
                    if not info.get("ignored"):
                        active_tokens[addr_lower] = info
        except Exception as e:
            print(f"[Warning] Error reading {path}: {e}")
            
    print(f"[Batch Generator] Found {len(active_tokens)} active (non-ignored) tokens.")
    
    # Limit number of tokens processed if requested (e.g., test run)
    limit = None
    if len(sys.argv) > 1:
        try:
            limit = int(sys.argv[1])
            print(f"[Batch Generator] Limiting process to first {limit} tokens.")
        except ValueError:
            pass

    os.makedirs("assets", exist_ok=True)
    
    count = 0
    for addr, info in active_tokens.items():
        if limit is not None and count >= limit:
            break
            
        symbol = info.get("symbol", "UNKNOWN")
        print(f"\n[{count + 1}/{len(active_tokens)}] Processing {symbol} ({addr})...")
        
        # Paths
        hypotrochoid_path = f"assets/{addr}_hypotrochoid.png"
        humanoid_path = f"assets/{addr}_humanoid_photorealistic.png"
        
        # Step A: Generate Hypotrochoid if not exists
        if not os.path.exists(hypotrochoid_path):
            print(f"  -> Generating hypotrochoid signature...")
            try:
                subprocess.run(["python3", "scripts/generate_hypotrochoid.py", addr], check=True)
            except Exception as e:
                print(f"  [Error] Failed to generate hypotrochoid: {e}")
                continue
        else:
            print(f"  -> Hypotrochoid already exists: {hypotrochoid_path}")
            
        # Step B: Generate Humanoid Organism if not exists
        if not os.path.exists(humanoid_path):
            print(f"  -> Generating photorealistic humanoid/animal...")
            try:
                # This will resize the hypotrochoid and write it to shared memory for ControlNet depth map
                subprocess.run(["python3", "scripts/generate_lau_humanoid.py", addr], check=True)
            except Exception as e:
                print(f"  [Error] Failed to generate humanoid: {e}")
                continue
        else:
            print(f"  -> Humanoid already exists: {humanoid_path}")
            
        count += 1
        
    print(f"\n[Batch Generator] Completed processing. Successfully verified/generated assets for {count} tokens.")

if __name__ == "__main__":
    main()
