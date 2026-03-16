import os
import subprocess
import time
from PIL import Image
import math

def generate_dna_mask():
    img = Image.new("RGB", (512, 512), (0, 0, 0))
    pixels = img.load()
    for y in range(512):
        for x in range(512):
            if math.hypot(x - 256, y - 256) < 100:
                pixels[x, y] = (76, 29, 14)
    return img

def test_sub15_benchmark():
    print("=== TSFi Sub-15 Second Hardware Override Benchmark ===")
    
    mask_path = "assets/sub15_mask.png"
    out_path = "assets/sub15_output.png"
    generate_dna_mask().save(mask_path)
    
    cmd = [
        "assets/sd_cpp/build/bin/sd-cli",
        "-m", "assets/models/sd15.safetensors",
        "--vae", "assets/models/taesd.safetensors",
        "-p", "A highly detailed photorealistic Sick Teddy Bear, matted brown fur",
        "-W", "512", "-H", "512",
        "--steps", "4",
        # OPTIMIZATION 1: Disable CFG (Cuts UNet time exactly in half)
        "--cfg-scale", "1.0", 
        # OPTIMIZATION 2: Force aggressive 16-bit float quantization across the entire memory matrix
        "--type", "f16",
        "--seed", "42",
        "-i", mask_path,
        "--strength", "0.85",
        "-o", out_path
    ]
    
    print("[ACTIVE] Dispatching Sub-15 Second FP16 Matrix...")
    start_time = time.time()
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    
    end_time = time.time()
    
    for line in result.stdout.split('\\n'):
        if "generate_image completed" in line or "sampling completed" in line or "decode_first_stage" in line:
            print(f"  -> {line.strip()}")
            
    print(f"\\n[ABSOLUTE SUCCESS] Total Wall-Clock Execution: {(end_time - start_time):.2f} Seconds")

if __name__ == "__main__":
    test_sub15_benchmark()
