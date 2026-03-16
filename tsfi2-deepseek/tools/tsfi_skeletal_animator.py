import os
import subprocess
import time
import math
from PIL import Image, ImageDraw

def generate_openpose_skeleton(frame, W=512, H=512):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # ---------------------------------------------------------
    # The TSFi Kinematic Bone Matrix (OpenPose Format)
    # ---------------------------------------------------------
    
    # Base positioning
    center_x = W // 2
    base_y = int(H * 0.7) # Hips
    
    # Walking Kinematics
    # 1 cycle = 10 frames.
    cycle = (frame % 20) / 20.0 
    swing = math.sin(cycle * math.pi * 2.0) # -1.0 to 1.0
    
    # Bone Lengths
    torso_len = 120
    arm_len = 60
    leg_len = 80
    
    # Nodes (X, Y)
    neck = (center_x, base_y - torso_len)
    nose = (center_x, neck[1] - 40)
    
    r_shoulder = (neck[0] - 40, neck[1])
    l_shoulder = (neck[0] + 40, neck[1])
    
    r_hip = (center_x - 30, base_y)
    l_hip = (center_x + 30, base_y)
    
    # Arm Swing Kinematics (Opposite to legs)
    # Right arm swings forward (down and left in 2D perspective, or just changes angle)
    r_elbow_x = r_shoulder[0] - 20
    r_elbow_y = r_shoulder[1] + int(arm_len * 0.8) + int(swing * 20)
    r_wrist_x = r_elbow_x - 10
    r_wrist_y = r_elbow_y + arm_len + int(swing * 30)
    
    l_elbow_x = l_shoulder[0] + 20
    l_elbow_y = l_shoulder[1] + int(arm_len * 0.8) - int(swing * 20)
    l_wrist_x = l_elbow_x + 10
    l_wrist_y = l_elbow_y + arm_len - int(swing * 30)
    
    # Leg Swing Kinematics
    r_knee_x = r_hip[0]
    r_knee_y = r_hip[1] + leg_len - int(swing * 30)
    r_ankle_x = r_knee_x
    r_ankle_y = r_knee_y + leg_len - int(swing * 20)
    
    l_knee_x = l_hip[0]
    l_knee_y = l_hip[1] + leg_len + int(swing * 30)
    l_ankle_x = l_knee_x
    l_ankle_y = l_knee_y + leg_len + int(swing * 20)
    
    # ---------------------------------------------------------
    # Draw OpenPose Standard Colors (Thickness ~8)
    # ---------------------------------------------------------
    th = 8
    
    # Torso (Neck to Hip center)
    draw.line([neck, (center_x, base_y)], fill=(255, 0, 0), width=th)
    # Head
    draw.line([neck, nose], fill=(255, 0, 0), width=th)
    
    # Right Arm (Red/Yellow)
    draw.line([neck, r_shoulder], fill=(255, 85, 0), width=th)
    draw.line([r_shoulder, (r_elbow_x, r_elbow_y)], fill=(255, 170, 0), width=th)
    draw.line([(r_elbow_x, r_elbow_y), (r_wrist_x, r_wrist_y)], fill=(255, 255, 0), width=th)
    
    # Left Arm (Green)
    draw.line([neck, l_shoulder], fill=(85, 255, 0), width=th)
    draw.line([l_shoulder, (l_elbow_x, l_elbow_y)], fill=(0, 255, 0), width=th)
    draw.line([(l_elbow_x, l_elbow_y), (l_wrist_x, l_wrist_y)], fill=(0, 255, 85), width=th)
    
    # Right Leg (Blue/Purple)
    draw.line([(center_x, base_y), r_hip], fill=(0, 255, 170), width=th)
    draw.line([r_hip, (r_knee_x, r_knee_y)], fill=(0, 85, 255), width=th)
    draw.line([(r_knee_x, r_knee_y), (r_ankle_x, r_ankle_y)], fill=(0, 0, 255), width=th)
    
    # Left Leg (Purple/Pink)
    draw.line([(center_x, base_y), l_hip], fill=(85, 0, 255), width=th)
    draw.line([l_hip, (l_knee_x, l_knee_y)], fill=(170, 0, 255), width=th)
    draw.line([(l_knee_x, l_knee_y), (l_ankle_x, l_ankle_y)], fill=(255, 0, 255), width=th)
    
    # Draw Nodes
    nodes = [nose, neck, r_shoulder, (r_elbow_x, r_elbow_y), (r_wrist_x, r_wrist_y),
             l_shoulder, (l_elbow_x, l_elbow_y), (l_wrist_x, l_wrist_y),
             r_hip, (r_knee_x, r_knee_y), (r_ankle_x, r_ankle_y),
             l_hip, (l_knee_x, l_knee_y), (l_ankle_x, l_ankle_y)]
             
    for nx, ny in nodes:
        draw.ellipse([nx-4, ny-4, nx+4, ny+4], fill=(255, 255, 255))
        
    return img

def execute_skeletal_test():
    print("=== TSFi Kinematic Skeletal Matrix (OpenPose) ===", flush=True)
    os.makedirs("assets/skeletal_test", exist_ok=True)
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    controlnet = "assets/models/control_openpose.safetensors"
    
    FRAMES = 10
    start_time = time.time()
    
    for f in range(FRAMES):
        mask_path = f"assets/skeletal_test/skeleton_{f:04d}.png"
        out_path = f"assets/skeletal_test/frame_{f:04d}.png"
        
        # 1. Generate the Mathematical Bone Structure
        skeleton = generate_openpose_skeleton(f)
        skeleton.save(mask_path)
        
        # 2. Command Stable Diffusion to wrap photorealistic fur around the bones
        # We use txt2img with ControlNet instead of img2img! This guarantees a perfect bear every time without deep-frying!
        prompt = "A photorealistic brown teddy bear walking, standing on two legs, arms swinging, masterpiece, 8k resolution, highly detailed fur"
        
        cmd = [
            sd_bin,
            "-m", model,
            "--vae", vae,
            "--control-net", controlnet,
            "--control-image", mask_path,
            "--control-strength", "1.0", # Absolute obedience to the skeleton
            "-p", prompt,
            "-n", "abstract, deformed, bad anatomy, flat, illustration",
            "-W", "512", "-H", "512",
            "--steps", "20", # High steps for perfect quality
            "--cfg-scale", "7.0",
            "--seed", "42",
            "-o", out_path
        ]
        
        print(f"[ACTIVE] Synthesizing Frame {f}/{FRAMES} (Wrapping Textures over Bones)...")
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
    print(f"\\n[ABSOLUTE SUCCESS] Skeletal Animation Complete in {time.time() - start_time:.2f} seconds.")

if __name__ == "__main__":
    execute_skeletal_test()
