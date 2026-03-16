import os
import base64
import json
import urllib.request
import time
import io
from PIL import Image, ImageDraw
import math
import subprocess

def build_openpose_skeleton(f_idx, total_frames):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    center_x = W // 2
    base_y = int(H * 0.75) 
    scale_y = 1.0
    
    # Smooth continuous sine wave over the exact frame count
    sit_cycle = f_idx / float(total_frames)
    sit_arm_wave = math.sin(sit_cycle * math.pi * 2.0)
            
    cx = center_x
    cy = base_y
    
    torso_len = int(120 * scale_y)
    arm_len = int(80 * scale_y)
    
    neck = (cx, cy - torso_len)
    nose = (cx, neck[1] - 40)
    
    r_shoulder = (neck[0] - 40, neck[1])
    l_shoulder = (neck[0] + 40, neck[1])
    r_hip = (cx - 30, cy)
    l_hip = (cx + 30, cy)
    
    # Natural sitting arm position with a gentle sine wave waving motion
    r_elbow_x = r_shoulder[0] - 30
    r_elbow_y = r_shoulder[1] + int(arm_len * 0.6) - int(sit_arm_wave * 20)
    r_wrist_x = r_elbow_x - 20
    r_wrist_y = r_elbow_y + int(arm_len * 0.5) - int(sit_arm_wave * 30)
    
    l_elbow_x = l_shoulder[0] + 30
    l_elbow_y = l_shoulder[1] + int(arm_len * 0.6) + int(sit_arm_wave * 20)
    l_wrist_x = l_elbow_x + 20
    l_wrist_y = l_elbow_y + int(arm_len * 0.5) + int(sit_arm_wave * 30)
    
    # Spread the legs out and forward to represent a classic sitting plush bear
    r_knee = (r_hip[0] - 40, r_hip[1] + 20)
    r_ankle = (r_hip[0] - 70, r_hip[1] + 50)
    l_knee = (l_hip[0] + 40, l_hip[1] + 20)
    l_ankle = (l_hip[0] + 70, l_hip[1] + 50)
        
    th = 8
    draw.line([neck, (cx, cy)], fill=(255, 0, 0), width=th)
    draw.line([neck, nose], fill=(255, 0, 0), width=th)
    draw.line([neck, r_shoulder], fill=(255, 85, 0), width=th)
    draw.line([r_shoulder, (r_elbow_x, r_elbow_y)], fill=(255, 170, 0), width=th)
    draw.line([(r_elbow_x, r_elbow_y), (r_wrist_x, r_wrist_y)], fill=(255, 255, 0), width=th)
    draw.line([neck, l_shoulder], fill=(85, 255, 0), width=th)
    draw.line([l_shoulder, (l_elbow_x, l_elbow_y)], fill=(0, 255, 0), width=th)
    draw.line([(l_elbow_x, l_elbow_y), (l_wrist_x, l_wrist_y)], fill=(0, 255, 85), width=th)
    
    draw.line([(cx, cy), r_hip], fill=(0, 255, 170), width=th)
    draw.line([r_hip, r_knee], fill=(0, 85, 255), width=th)
    draw.line([r_knee, r_ankle], fill=(0, 0, 255), width=th)
    draw.line([(cx, cy), l_hip], fill=(85, 0, 255), width=th)
    draw.line([l_hip, l_knee], fill=(170, 0, 255), width=th)
    draw.line([l_knee, l_ankle], fill=(255, 0, 255), width=th)
    
    return img

os.makedirs("assets", exist_ok=True)
prompt = "A photorealistic overstuffed plush 16 inch tall brown teddy bear sitting still like a companion pet, masterpiece, 8k resolution, cinematic lighting, highly detailed anatomy"
neg_prompt = "muscular, human-like, abstract, deformed, bad anatomy, missing limbs, blurry"

TOTAL_FRAMES = 17 # Frame 0, Frame 16 (the two ends), plus 15 intermediary frames

for i in range(TOTAL_FRAMES):
    print(f"Generating Smooth Frame {i}/{TOTAL_FRAMES-1}...")
    skel = build_openpose_skeleton(i, TOTAL_FRAMES)
    mask_path = f"assets/smooth_skel_{i}.png"
    skel.save(mask_path)
    
    out_path = f"smooth_frame_{i}.png"
    cmd = [
        "assets/sd_cpp/build/bin/sd-cli",
        "-m", "assets/models/sd15.safetensors",
        "--vae", "assets/models/taesd.safetensors",
        "--control-net", "assets/models/control_openpose.safetensors",
        "--control-image", mask_path,
        "--control-strength", "1.0",
        "-p", prompt,
        "-n", neg_prompt,
        "-W", "512", "-H", "512",
        "--steps", "4",
        "--cfg-scale", "1.2",
        "--seed", "42", # Lock seed for absolute temporal consistency
        "-o", out_path
    ]
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"  -> Generated {out_path}")
    
    img = Image.open(out_path).convert("RGB")
    img.save(f"smooth_frame_{i:04d}.ppm", "PPM")
    print(f"  -> Converted to smooth_frame_{i:04d}.ppm")

print("\n[INFO] Compiling MP4 video to prove zero-jitter temporal smoothing...")
subprocess.run(["ffmpeg", "-y", "-framerate", "10", "-i", "smooth_frame_%04d.ppm", "-c:v", "libx264", "-pix_fmt", "yuv420p", "smooth_wave_demo.mp4"])
print("[SUCCESS] Video saved as smooth_wave_demo.mp4")
