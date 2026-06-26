#!/usr/bin/env python3
import os
import sys
import math
import hashlib
import subprocess
import json
from PIL import Image

def hsl_to_rgb(h, s, l):
    c = (1.0 - abs(2.0 * l - 1.0)) * s
    x = c * (1.0 - abs((h / 60.0) % 2.0 - 1.0))
    m = l - c / 2.0
    if 0 <= h < 60:
        r, g, b = c, x, 0.0
    elif 60 <= h < 120:
        r, g, b = x, c, 0.0
    elif 120 <= h < 180:
        r, g, b = 0.0, c, x
    elif 180 <= h < 240:
        r, g, b = 0.0, x, c
    elif 240 <= h < 300:
        r, g, b = x, 0.0, c
    else:
        r, g, b = c, 0.0, x
    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

def main():
    if len(sys.argv) < 2:
        address = "0xd32c39fee49391c7952d1b30b15921b0d3b42e69"
        print(f"[Humanoid Director] No address provided. Defaulting to: {address}")
    else:
        address = sys.argv[1].lower().strip()
        
    # Load registers from cache (LAU registers: r_base etc., and SHA/SHIO/YI registers: c_base etc.)
    r_base, r_channel, r_dynamo, r_foundation = 0, 0, 0, 0
    c_base, c_channel, c_dynamo, c_foundation = 0, 0, 0, 0
    cache_path = os.path.join(os.path.dirname(__file__), "pulsechain_register_cache.json")
    if os.path.exists(cache_path):
        try:
            with open(cache_path, "r") as f:
                cache = json.load(f)
            if address in cache:
                entry = cache[address]
                r_base = int(entry["r_base"])
                r_channel = int(entry["r_channel"])
                r_dynamo = int(entry["r_dynamo"])
                r_foundation = int(entry["r_foundation"])
                c_base = int(entry["c_base"])
                c_channel = int(entry["c_channel"])
                c_dynamo = int(entry["c_dynamo"])
                c_foundation = int(entry["c_foundation"])
        except Exception:
            pass
            
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    if r_base == 0:
        r_base = int(addr_hash[0:4], 16)
        r_channel = int(addr_hash[4:8], 16)
        r_dynamo = int(addr_hash[8:12], 16)
        r_foundation = int(addr_hash[12:16], 16)
        
    if c_base == 0:
        c_base = int(addr_hash[16:20], 16)
        c_channel = int(addr_hash[20:24], 16)
        c_dynamo = int(addr_hash[24:28], 16)
        c_foundation = int(addr_hash[28:32], 16)
        
    # Derive colors entirely from SHA/SHIO/YI numbers (c_base, c_channel)
    hue = (c_base % 360)
    saturation = 0.5 + ((c_channel % 50) / 100.0) # 50% to 100%
    lightness = 0.4 + ((c_foundation % 30) / 100.0) # 40% to 70%
    color_rgb = hsl_to_rgb(hue, saturation, lightness)
        
    # Scale elements for details
    lobes = 5 + (r_channel % 3)
    hardness = "raw polished bamboo finish" if (r_foundation % 2 == 0) else "weathered natural wood grain finish"
    
    # Soft descriptive properties that guide Stable Diffusion to naturally discover the figure
    # rather than hardcoding names. Maps details like wing hints, body shapes, or monster characteristics.
    form_descriptors = []
    
    # Base structure range (organism at least, humanoid/super-humanoid or monster at most)
    struct_mod = r_base % 4
    if struct_mod == 0:
        form_descriptors.append("organic living plant creature structure")
    elif struct_mod == 1:
        form_descriptors.append("humanoid figure grown from interlocking branches and shoots")
    elif struct_mod == 2:
        form_descriptors.append("super-humanoid wooden figure with leaf and wing-like extensions")
    else:
        form_descriptors.append("exotic forest monster entity with multi-faceted roots and limbs")
        
    # Append visual qualities mapped from dynamic registers
    if r_channel % 2 == 0:
        form_descriptors.append("delicate butterfly-like leaf wing structures")
    if r_dynamo % 2 == 0:
        form_descriptors.append("bear-like organic posture characteristics")
        
    guides = ", ".join(form_descriptors)
    
    # Formulate SD prompt using vocabulary and mapped registers
    prompt = (
        f"A highly detailed photorealistic organism sculpted from natural materials, sitting in meditation, "
        f"where the body is physically constructed and grown from the concentric hypotrochoid vector rings and Lissajous loops of the LAU token, "
        f"featuring a {guides}, body colored RGB({color_rgb[0]},{color_rgb[1]},{color_rgb[2]}), "
        f"glowing green eyes, intricate EDO-22 frequency patterns of micro-chords and wood lines, "
        f"integrated natural bamboo fiber textures and organic hypotrochoid shoot patterns, "
        f"embossed wood grain details, {lobes}-cusped symmetry geometry, "
        f"{hardness}, cinematic studio lighting, dark background, 8k resolution, masterpiece"
    )
    
    raw_out = "tmp/humanoid_render.raw"
    png_out = f"assets/{address}_humanoid_photorealistic.png"
    
    os.makedirs("tmp", exist_ok=True)
    os.makedirs("assets", exist_ok=True)
    
    worker_path = "./bin/tsfi_sd_worker"
    cmd = [
        worker_path,
        prompt,
        raw_out,
        "0", # no shm
        "turbo",
        "4", # steps
        "euler_a",
        "1.5" # cfg
    ]
    
    print(f"\n[Humanoid Director] Generating photorealistic humanoid for: {address}")
    print(f"  -> Prompt: \"{prompt}\"")
    
    cwd = "tsfi2-deepseek" if os.path.exists("tsfi2-deepseek") else "."
    if not os.path.exists(os.path.join(cwd, worker_path)):
        if os.path.exists("./bin/tsfi_sd_worker"):
            cmd[0] = "./bin/tsfi_sd_worker"
            cwd = "."
            
    try:
        subprocess.run(cmd, cwd=cwd, check=True)
        raw_path_adj = os.path.join(cwd, raw_out)
        if os.path.exists(raw_path_adj):
            with open(raw_path_adj, 'rb') as f:
                raw_data = f.read()
            # SD worker outputs 512x512 raw bytes
            img = Image.frombytes('RGB', (512, 512), raw_data)
            img.save(png_out)
            print(f"[Humanoid Director] Success! Saved to: {png_out}")
        else:
            print("[Error] Raw output file from SD worker not found.")
    except Exception as e:
        print(f"[Error] Failed to run SD worker: {e}")

if __name__ == "__main__":
    main()
