import os
import time
import subprocess
import random
import mmap
import ctypes
from PIL import Image

# SHM Constants
TSFI_CN_SHM_DGUI = "tsfi_cn_dgui"
DGUI_SIZE = 32 

def get_dgui_shm():
    try:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR)
        return mmap.mmap(fd, DGUI_SIZE)
    except FileNotFoundError:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR | os.O_CREAT)
        os.ftruncate(fd, DGUI_SIZE)
        m = mmap.mmap(fd, DGUI_SIZE)
        m[0:4] = b'DGUI'
        return m

def update_guidance(m, depth=0.8, pose=0.6, cfg=7.5, steps=4):
    m[4:8] = bytearray(ctypes.c_float(depth))
    m[8:12] = bytearray(ctypes.c_float(pose))
    m[12:16] = bytearray(ctypes.c_float(cfg))
    m[16:20] = (int(steps)).to_bytes(4, 'little')

def generate_crow_lineart(posture, taste_name, taste_prompt, output_dir):
    print(f"[LINEART] {posture} | Taste: {taste_name}")
    
    # Stylized Line Art Prompt
    base_prompt = "clean minimalist line art of a stuffed animal crow plush, elegant ink drawing, bold black outlines on pure white background, vector graphics style, technical illustration, sharp edges, no shading, high contrast"
    full_prompt = f"{base_prompt}, {taste_prompt}"
    
    output_raw = "tmp/crow_lineart.raw"
    output_png = f"{output_dir}/{posture.replace(' ', '_').lower()}_{taste_name.lower()}_lineart.png"
    
    steps = 20
    cfg = 9.0 # Higher CFG for cleaner lines and background
    
    # 1. Update SHM Guidance
    dgui = get_dgui_shm()
    # High depth strength to maintain the "form" of the plushie in lineart
    update_guidance(dgui, depth=0.95, pose=0.8, cfg=cfg, steps=steps)
    
    cmd = [
        "bin/tsfi_sd_worker",
        full_prompt,
        output_raw,
        "1", # use_shm ENABLED
        "sd15",
        str(steps),
        "euler_a",
        str(cfg)
    ]
    
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        print(f"[ERROR] SD Worker failed: {result.stderr.decode()}")
        return False
        
    if os.path.exists(output_raw):
        with open(output_raw, "rb") as f:
            raw_data = f.read()
        
        expected_size = 512 * 512 * 3
        if len(raw_data) >= expected_size:
            img = Image.frombytes("RGB", (512, 512), raw_data[:expected_size])
            img.save(output_png)
            # Update broadcast for visual feedback
            img.save("/tmp/tsfi_broadcast.jpg", "JPEG", quality=95)
            print(f"[SUCCESS] Saved {output_png}")
            return True
    return False

if __name__ == "__main__":
    out_dir = "assets/crow_ballet_lineart"
    os.makedirs(out_dir, exist_ok=True)
    os.makedirs("tmp", exist_ok=True)
    
    postures = [
        "First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position",
        "Plier", "Etendre", "Relever", "Reverence"
    ]
    
    # Tastes adapted for line art expression
    tastes = [
        ("INK", "fine nib pen detail, cross-hatching, stipple art accents"),
        ("MINIMAL", "single continuous line style, abstract contours, negative space"),
        ("TECHNICAL", "blueprint style, schematic lines, architectural precision")
    ]
    
    print("=== TSFi Crow Ballet Lineart Matrix Starting ===")
    print("-> Ensure tools/tsfi_crow_skeleton_writer.py is running!")
    
    for posture in postures:
        print(f"\n[WAIT] Waiting for Skeleton Provider to sync to {posture}...")
        time.sleep(6) 
        
        for name, prompt in tastes:
            generate_crow_lineart(posture, name, prompt, out_dir)
            time.sleep(1)
            
    print("\n[COMPLETE] Crow Ballet Lineart Matrix Finished.")
