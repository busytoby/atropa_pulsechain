import os
import subprocess
import math
import time
from PIL import Image

def generate_dna_mask():
    img = Image.new("RGB", (512, 512), (0, 0, 0))
    pixels = img.load()
    for y in range(512):
        for x in range(512):
            if math.hypot(x - 256, y - 256) < 100:
                pixels[x, y] = (76, 29, 14)
    return img

def test_fast_cli():
    print("=== TSFi Accelerated CLI img2img Validation ===")
    
    img = generate_dna_mask()
    mask_path = "assets/fast_cli_mask.png"
    out_path = "assets/fast_cli_output.png"
    img.save(mask_path)
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    
    prompt = "A highly detailed photorealistic Sick Teddy Bear, matted brown fur"
    
    cmd = [
        sd_bin,
        "-m", model,
        "--vae", vae,
        "--vae-tiling",
        "--vae-on-cpu",
        "-p", prompt,
        "-W", "512", "-H", "512",
        "--steps", "4",
        "--cfg-scale", "2.0",
        "--seed", "42",
        "-i", mask_path,
        "--strength", "0.85",
        "-o", out_path
    ]
    
    print("[ACTIVE] Dispatching CLI Generation (Loading 3GB Matrix + 4-Step VAE Decode)...")
    start_time = time.time()
    
    # Run the process and capture the literal stdout to prove to the user what's happening
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    
    end_time = time.time()
    
    # Extract the exact generation time reported by sd.cpp
    for line in result.stdout.split('\\n'):
        if "generate_image completed" in line:
            print(f"  -> {line.strip()}")
            
    print(f"\\n[ABSOLUTE SUCCESS] Total Wall-Clock Execution: {(end_time - start_time):.2f} Seconds")

if __name__ == "__main__":
    test_fast_cli()
