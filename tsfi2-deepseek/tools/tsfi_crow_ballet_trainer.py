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
    # struct TsfiDynamicGuidance: float depth, float pose, float cfg, uint32 steps
    # We use bytearray conversion for ctypes floats
    m[4:8] = bytearray(ctypes.c_float(depth))
    m[8:12] = bytearray(ctypes.c_float(pose))
    m[12:16] = bytearray(ctypes.c_float(cfg))
    m[16:20] = (int(steps)).to_bytes(4, 'little')

def generate_crow_training(posture, taste_name, taste_prompt, output_dir):
    print(f"[TRAINING] {posture} | Taste: {taste_name}")
    
    # Base prompt with extreme focus on synthetic feathers
    base_prompt = "extreme macro photography of a stuffed animal crow plush, hyper-detailed individual synthetic feathers, matted black faux-plumage textures, visible stitching along feather edges, soft plush under-down, individual synthetic feather barbs, studio lighting catching iridescent sheen, neutral background, 16k resolution masterpiece"
    full_prompt = f"{base_prompt}, {taste_prompt}"
    
    output_raw = "tmp/crow_train.raw"
    output_png = f"{output_dir}/{posture.replace(' ', '_').lower()}_{taste_name.lower()}.png"
    
    # In training mode, we use high steps for maximum quality
    steps = 20
    cfg = 7.5
    
    # 1. Update SHM Guidance
    dgui = get_dgui_shm()
    update_guidance(dgui, depth=0.9, pose=0.8, cfg=cfg, steps=steps)
    
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
        
        # Check size to be safe
        expected_size = 512 * 512 * 3
        if len(raw_data) >= expected_size:
            img = Image.frombytes("RGB", (512, 512), raw_data[:expected_size])
            img.save(output_png)
            img.save("/tmp/tsfi_broadcast.jpg", "JPEG", quality=95)
            print(f"[SUCCESS] Saved {output_png}")
            return True
        else:
            print(f"[ERROR] raw_data too small: {len(raw_data)} vs {expected_size}")
            return False
    return False

if __name__ == "__main__":
    out_dir = "assets/crow_ballet_training"
    os.makedirs(out_dir, exist_ok=True)
    os.makedirs("tmp", exist_ok=True)
    
    postures = [
        "First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position",
        "Plier", "Etendre", "Relever", "Reverence"
    ]
    
    tastes = [
        ("QUOTH", "beak open in a cawing expression, throat feathers ruffled, expressive face, synthetic plumage texture detail"),
        ("SYMPATHY", "slightly tilted head, soft melting synthetic feathers, empathetic synthetic eyes, gentle expression, matted faux fur depth"),
        ("FLIGHT", "dynamic wing extension, individual synthetic flight feathers catching light, overlapping plush feather scales, kinetic energy, powerful silhouette")
    ]
    
    print("=== TSFi Crow Ballet Training Matrix Starting (FEATHER FOCUS) ===")
    print("-> Ensure tools/tsfi_crow_skeleton_writer.py is running in another terminal!")
    
    for posture in postures:
        print(f"\n[WAIT] Waiting for Skeleton Provider to sync to {posture}...")
        time.sleep(6) 
        
        for name, prompt in tastes:
            generate_crow_training(posture, name, prompt, out_dir)
            time.sleep(1)
            
    print("\n[COMPLETE] Crow Ballet Training Matrix Finished.")
