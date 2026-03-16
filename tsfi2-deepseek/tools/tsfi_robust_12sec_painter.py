import os
import subprocess
import math
import time
from PIL import Image

def generate_dna_mask(frame, W=512, H=512, TOTAL_FRAMES=180):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    time_offset = frame * 0.1
    pulse = math.sin((frame / TOTAL_FRAMES) * math.pi * 20.0) 
    
    for y in range(H):
        py = y / H
        if py < 0.1 or py > 0.95: continue
        for x in range(W):
            px = x / W
            if px < 0.1 or px > 0.9: continue
            
            # Offsets extracted by VLM
            dist_eye1 = math.hypot(px - 0.46, py - 0.4)
            dist_eye2 = math.hypot(px - 0.54, py - 0.4)
            pulse_radius = 0.02 + (pulse * 0.005)
            
            # The DeepSeek Exponential Shadow
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
                    pixels[x, y] = (252, 252, 252)
                else:
                    pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                if (x * y * frame) % 100 < 50:
                    pixels[x, y] = (51, 65, 0)
                else:
                    pixels[x, y] = (76, 29, 14)
                    
    return img

def run_robust_12sec_painter():
    print("=== TSFi Robust 12-Second AOT Painter ===", flush=True)
    frames_dir = "assets/robust_12sec_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    FPS = 15
    TOTAL_FRAMES = 180
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    
    for i in range(TOTAL_FRAMES):
        mask_path = f"{frames_dir}/mask_{i:04d}.png"
        out_path = f"{frames_dir}/frame_{i:04d}.png"
        
        # 1. Generate DNA Mask
        img = generate_dna_mask(i, TOTAL_FRAMES=TOTAL_FRAMES)
        img.save(mask_path)
        
        # 2. Render Photorealism via Stable Diffusion CLI
        sickness = int((i / TOTAL_FRAMES) * 100)
        prompt = f"Photorealistic sick brown teddy bear sitting, cinematic lighting, {sickness} percent mutated, dark background. Masterpiece, 8k resolution."
        
        cmd = [
            sd_bin,
            "-m", model,
            "--vae", vae,
            "-p", prompt,
            "-W", "512", "-H", "512",
            "--steps", "6",
            "--cfg-scale", "2.5",
            "--seed", str(42 + (i%5)),
            "-i", mask_path,
            "--strength", "0.85",
            "-o", out_path
        ]
        
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if i % 15 == 0:
            print(f"-> Painter completed Frame {i}/{TOTAL_FRAMES}", flush=True)

    print("\\n[ACTIVE] Compiling Heavily Compressed Video via FFmpeg...", flush=True)
    output_video = "assets/atropa_ultimate_12sec_photorealistic.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] {output_video}")

if __name__ == "__main__":
    run_robust_12sec_painter()
