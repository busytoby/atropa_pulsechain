#!/usr/bin/env python3
import os
import sys
import math
import hashlib
import subprocess
import json
import struct
import mmap
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
    
    # Pack register values and calculated properties as a strictly formatted numeric matrix prompt.
    # The prompt explicitly directs the latent solver to interpret the structural loops as anatomical targets:
    # head, ears, nose, eyes, body, limbs (leaning forward, hands together, legs apart), and maybe tail.
    prompt = (
        f"humanoid maybe animal leaning forward hands together legs apart head ears eyes nose body limbs tail, "
        f"0x{r_base:016X} 0x{r_channel:016X} 0x{r_dynamo:016X} 0x{r_foundation:016X} "
        f"0x{c_base:016X} 0x{c_channel:016X} 0x{c_dynamo:016X} 0x{c_foundation:016X} "
        f"RGB({color_rgb[0]},{color_rgb[1]},{color_rgb[2]}) {lobes} 0x{int(addr_hash, 16):032X}"
    )
    
    raw_out = "tmp/humanoid_render.raw"
    png_out = f"assets/{address}_humanoid_photorealistic.png"
    hypotrochoid_path = f"assets/{address}_hypotrochoid.png"
    
    os.makedirs("tmp", exist_ok=True)
    os.makedirs("assets", exist_ok=True)
    
    # Write the hypotrochoid image into shared memory segment if it exists
    use_shm = "0"
    if os.path.exists(hypotrochoid_path):
        try:
            # We open the generated hypotrochoid image, resize to 512x512, and write it to the /tsfi_cn_depth SHM file
            hyp_img = Image.open(hypotrochoid_path).convert("RGB").resize((512, 512))
            raw_pixels = hyp_img.tobytes()
            
            # Map size matching sizeof(TsfiControlNetMap)
            # Struct format: uint32 magic, uint32 version, uint32 width, uint32 height, uint32 channels, uint64 timestamp, uint8 data[1280 * 720 * 3]
            # /dev/shm/tsfi_cn_depth maps to "/tsfi_cn_depth"
            shm_path = "/dev/shm/tsfi_cn_depth"
            shm_fd = os.open(shm_path, os.O_RDWR | os.O_CREAT, 0o666)
            # Map structural size
            struct_size = 4 + 4 + 4 + 4 + 4 + 8 + (1280 * 720 * 3)
            os.ftruncate(shm_fd, struct_size)
            shm_buf = mmap.mmap(shm_fd, struct_size, mmap.MAP_SHARED, mmap.PROT_WRITE)
            
            # Write header: magic 0x54434E4D, version 1, width 512, height 512, channels 3, timestamp 0
            header = struct.pack("<IIIIIQ", 0x54434E4D, 1, 512, 512, 3, 0)
            shm_buf.seek(0)
            shm_buf.write(header)
            shm_buf.write(raw_pixels)
            shm_buf.close()
            os.close(shm_fd)
            use_shm = "1"
            print(f"[Humanoid Director] Mapped {hypotrochoid_path} to ControlNet depth SHM.")
        except Exception as e:
            print(f"[Humanoid Director Warning] Failed to write hypotrochoid to SHM: {e}")
            
    worker_path = "./bin/tsfi_sd_worker"
    
    # ControlNet weights are trained on SD 1.5. If SHM is active, we must use the SD 1.5 profile
    model_profile = "sd15" if use_shm == "1" else "turbo"
    steps = "20" if use_shm == "1" else "4"
    method = "euler_a"
    cfg = "7.0" if use_shm == "1" else "1.5"
    
    cmd = [
        worker_path,
        prompt,
        raw_out,
        use_shm, # use shm
        model_profile,
        steps,
        method,
        cfg,
        "0.85" # strength / controlnet weight
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
