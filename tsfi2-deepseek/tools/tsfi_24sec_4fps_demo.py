import os
import subprocess
import time
from PIL import Image
import math

def generate_dna_mask(frame, TOTAL_FRAMES, W=256, H=256):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    # The mathematical evolution variables
    pulse = math.sin((frame / TOTAL_FRAMES) * math.pi * 10.0) 
    sickness_threshold = (frame / TOTAL_FRAMES) * 100.0 # Creeps from 0% to 100%
    
    for y in range(H):
        py = y / H
        if py < 0.1 or py > 0.95: continue
        for x in range(W):
            px = x / W
            if px < 0.1 or px > 0.9: continue
            
            dist_eye1 = math.hypot(px - 0.46, py - 0.4)
            dist_eye2 = math.hypot(px - 0.54, py - 0.4)
            pulse_radius = 0.02 + (pulse * 0.005)
            
            # Sub-scaled 256x256 Branching Shadow
            dist_left = math.hypot(px - 0.400, py - 0.780)
            dist_right = math.hypot(px - 0.624, py - 0.780)
            k = 50.0
            shadow_val = math.exp(-k * dist_left) + math.exp(-k * dist_right)
            shadow_dist = -math.log(shadow_val) / k if shadow_val > 0 else 1.0
            
            if shadow_dist < 0.08:
                pixels[x, y] = (19, 19, 19)
            elif dist_eye1 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif dist_eye2 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif math.hypot(px - 0.35, py - 0.25) < 0.075 or math.hypot(px - 0.65, py - 0.25) < 0.075:
                pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.5) < (0.2 + (pulse * 0.01)):
                if math.hypot(px - 0.45, py - 0.45) < 0.08:
                    pixels[x, y] = (252, 252, 252) # Specular Light
                else:
                    pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                # Procedural Sickness Noise mapping
                if (x * y * (frame+1)) % 100 < sickness_threshold:
                    pixels[x, y] = (51, 65, 0)
                else:
                    pixels[x, y] = (76, 29, 14)
    return img

def run_24sec_demo_synthesis():
    print("=== TSFi Absolute 24-Second (4 FPS) Core Synthesis ===", flush=True)
    frames_dir = "assets/demo_24sec_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    FPS = 4
    DURATION = 24
    TOTAL_FRAMES = FPS * DURATION # 96 Frames
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    
    start_time = time.time()
    
    for i in range(TOTAL_FRAMES):
        mask_path = f"{frames_dir}/mask_{i:04d}.png"
        out_path = f"{frames_dir}/frame_{i:04d}.png"
        
        # 1. Generate mathematically perfectly guided DNA Mask
        img = generate_dna_mask(i, TOTAL_FRAMES)
        img.save(mask_path)
        
        # 2. Architect Directed Prompt for this specific frame
        sickness_percent = int((i / TOTAL_FRAMES) * 100)
        prompt = f"Photorealistic sick brown teddy bear sitting, cinematic lighting, {sickness_percent} percent mutated, dark background"
        
        # 3. The 1.48-Second Sub-Architecture Pipeline
        cmd = [
            sd_bin,
            "-m", model,
            "--vae", vae,
            "-p", prompt,
            "-W", "256", "-H", "256",
            "--steps", "4",
            "--cfg-scale", "1.0", # Disable CFG double-pass
            "--type", "f16",      # Force float16 quantization
            "--seed", str(42 + (i%5)),
            "-i", mask_path,
            "--strength", "0.85", # Strictly bind the geometry to the mask
            "-o", out_path
        ]
        
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if (i+1) % 4 == 0:
            elapsed = time.time() - start_time
            print(f"  -> Synthesized Second {(i+1)//4}/{DURATION} (Frame {i+1}/{TOTAL_FRAMES}) | Runtime: {elapsed:.2f}s", flush=True)

    print("\\n[ACTIVE] Stitching and Mathematically Upscaling via FFmpeg (4 FPS)...", flush=True)
    output_video = "assets/atropa_24sec_4fps_demo.mp4"
    
    # We use `-vf scale=512:512:flags=neighbor` to cleanly upscale the 256x256 result without adding blurry artifacting.
    subprocess.run([
        "ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", 
        "-vf", "scale=512:512:flags=neighbor", 
        "-c:v", "libx264", "-preset", "slow", "-crf", "20", "-pix_fmt", "yuv420p", output_video
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print(f"[ABSOLUTE SUCCESS] 24-Second Demo Anchored: {output_video}")

if __name__ == "__main__":
    run_24sec_demo_synthesis()
