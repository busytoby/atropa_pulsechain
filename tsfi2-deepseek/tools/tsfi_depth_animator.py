import os
import subprocess
import time
import math
from PIL import Image

def generate_volumetric_depth_mask(frame, sickness, W=512, H=512):
    # Depth map: 0 is infinitely far away (background), 255 is physically touching the camera
    img = Image.new("L", (W, H), 0)
    pixels = img.load()
    
    pulse = math.sin(frame * 0.2)
    center_x = W / 2
    center_y = H * 0.6
    
    for y in range(H):
        py = y / H
        for x in range(W):
            px = x / W
            
            # The Base Body Sphere (3D Depth)
            # The closer to the center, the closer to the camera (higher value)
            dist_to_body = math.hypot(px - 0.5, py - 0.6)
            body_radius = 0.25
            
            if dist_to_body < body_radius:
                # Calculate a perfect 3D hemisphere using the Pythagorean theorem
                z = math.sqrt((body_radius**2) - (dist_to_body**2))
                depth_val = 100 + int((z / body_radius) * 100) # Base body depth is ~200
                
                # Sickness Extrusion (Tumors)
                # If sickness is active, we generate high-frequency 3D bumps on the stomach
                if sickness > 0:
                    # Procedural noise to create organic cellular clusters
                    noise = (math.sin(px * 50.0 + frame * 0.1) * math.cos(py * 50.0 + frame * 0.1))
                    if noise > 0.5:
                        tumor_height = int((noise * 55) * (sickness / 100.0))
                        depth_val = min(255, depth_val + tumor_height)
                
                pixels[x, y] = depth_val
                continue
                
            # The Head Sphere
            dist_to_head = math.hypot(px - 0.5, py - 0.3)
            head_radius = 0.18 + (pulse * 0.01)
            
            if dist_to_head < head_radius:
                z = math.sqrt((head_radius**2) - (dist_to_head**2))
                # Head is slightly closer to camera than body edges
                depth_val = 120 + int((z / head_radius) * 110)
                
                # Snout extrusion (sticks out even further)
                dist_to_snout = math.hypot(px - 0.5, py - 0.35)
                if dist_to_snout < 0.08:
                    depth_val += 25
                    
                pixels[x, y] = min(255, depth_val)
                continue
                
            # Ears (Flat depth, further back)
            dist_ear1 = math.hypot(px - 0.35, py - 0.15)
            dist_ear2 = math.hypot(px - 0.65, py - 0.15)
            if dist_ear1 < 0.07 or dist_ear2 < 0.07:
                pixels[x, y] = 100
                continue
                
            # Ground Plane Depth (Slanted floor)
            if py > 0.8:
                # The floor gets closer to the camera as Y increases
                floor_depth = int(((py - 0.8) / 0.2) * 80)
                pixels[x, y] = 20 + floor_depth
                
    return img

def execute_volumetric_animator():
    print("=== TSFi Volumetric Depth Matrix (ControlNet 3D Extrusion) ===", flush=True)
    os.makedirs("assets/volumetric_test", exist_ok=True)
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    controlnet = "assets/models/control_depth.safetensors"
    
    FRAMES = 10
    start_time = time.time()
    
    for f in range(FRAMES):
        mask_path = f"assets/volumetric_test/depth_{f:04d}.png"
        out_path = f"assets/volumetric_test/frame_{f:04d}.png"
        
        sickness = int((f / float(FRAMES)) * 100.0)
        
        # 1. Generate the Mathematical 3D Depth Map
        depth_mask = generate_volumetric_depth_mask(f, sickness)
        depth_mask.save(mask_path)
        
        prompt = f"A photorealistic brown teddy bear sitting, masterpiece, {sickness} percent mutated, massive bulging green crystalline tumors on stomach, 3d render, cinematic lighting"
        
        cmd = [
            sd_bin,
            "-m", model,
            "--vae", vae,
            "--control-net", controlnet,
            "--control-image", mask_path,
            "--control-strength", "1.0", # Absolute obedience to the 3D depth
            "-p", prompt,
            "-n", "abstract, deformed, bad anatomy, flat, 2d, illustration",
            "-W", "512", "-H", "512",
            "--steps", "20",
            "--cfg-scale", "7.0",
            "--seed", "42",
            "-o", out_path
        ]
        
        print(f"[ACTIVE] Extruding 3D Fur and Tumors | Frame {f}/{FRAMES} | Sickness: {sickness}%")
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
    print(f"\\n[ABSOLUTE SUCCESS] Volumetric Extrusion Complete in {time.time() - start_time:.2f} seconds.")

if __name__ == "__main__":
    execute_volumetric_animator()
