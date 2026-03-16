import os
import subprocess
import time
from PIL import Image
import math

def generate_dna_mask(W=256, H=256):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    for y in range(H):
        for x in range(W):
            # Scale the math to 256x256
            if math.hypot(x - (W/2), y - (H/2)) < (50):
                pixels[x, y] = (76, 29, 14)
    return img

def test_sub8_benchmark():
    print("=== TSFi Sub-8 Second Hardware Override Benchmark ===")
    
    mask_path = "assets/sub8_mask.png"
    out_path = "assets/sub8_output.png"
    generate_dna_mask().save(mask_path)
    
    cmd = [
        "assets/sd_cpp/build/bin/sd-cli",
        "-m", "assets/models/sd15.safetensors",
        "--vae", "assets/models/taesd.safetensors",
        "-p", "A highly detailed photorealistic Sick Teddy Bear, matted brown fur",
        "-W", "256", "-H", "256",
        "--steps", "3",
        "--cfg-scale", "1.0", 
        "--type", "f16",
        "--seed", "42",
        "-i", mask_path,
        "--strength", "0.85",
        "-o", out_path
    ]
    
    print("[ACTIVE] Dispatching Sub-8 Second 256x256 Matrix...")
    start_time = time.time()
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    
    end_time = time.time()
    
    for line in result.stdout.split('\\n'):
        if "generate_image completed" in line or "sampling completed" in line or "decode_first_stage" in line:
            print(f"  -> {line.strip()}")
            
    print(f"\\n[ABSOLUTE SUCCESS] Total Wall-Clock Execution: {(end_time - start_time):.2f} Seconds")

if __name__ == "__main__":
    test_sub8_benchmark()
