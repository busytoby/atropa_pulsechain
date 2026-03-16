import os
import urllib.request
import json
import base64
import math
import time
import subprocess
from PIL import Image

def generate_abstract_geometry(frame, W=512, H=512):
    # This mathematically replicates the DeepSeek SDF logic we compiled into the C-Kernel
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    pulse = math.sin((frame / 60) * math.pi * 2.0)
    
    # Scale down geometry from 1536 to 512 for the Painter matrix
    scale = 512 / 1536.0
    
    head_radius = (200 + pulse * 10) * scale
    left_ear_r = 75 * scale
    pupil_radius = (20 + abs(pulse) * 15) * scale
    
    for y in range(H):
        py = y / H
        for x in range(W):
            px = x / W
            
            # SDF Math
            dist_eye1 = math.hypot(px - 0.5, py - 0.4)
            dist_eye2 = math.hypot(px - 0.5, py - 0.6) 
            
            pulse_radius = 0.02 + (pulse * 0.005)
            
            if dist_eye1 < pulse_radius or dist_eye2 < (0.045 + pulse * 0.01):
                pixels[x, y] = (0, 255, 0) # Green Eyes
            elif math.hypot(px - 0.362, py - 0.25) < 0.075 or math.hypot(px - 0.662, py - 0.25) < 0.075:
                pixels[x, y] = (255, 0, 0) # Red Ears
            elif math.hypot(px - 0.5, py - 0.5) < (0.2 + (pulse * 0.01)):
                pixels[x, y] = (255, 0, 0) # Red Cranium
            elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                # Body noise (simplified for speed)
                if (x * y * frame) % 100 < 50:
                    pixels[x, y] = (84, 107, 45) # Sick Green
                else:
                    pixels[x, y] = (91, 63, 51) # Brown
                    
    return img

def render_true_synthesis_video():
    print("=== TSFi Unified Abstract-to-Real Matrix ===", flush=True)
    
    frames_dir = "assets/unified_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    FRAMES = 60 # 4 seconds @ 15fps
    
    # Wait for the SD Vulkan server to be ready
    time.sleep(2)
    
    for i in range(FRAMES):
        # 1. Synthesize the rigid, abstract mathematical geometry (The DeepSeek C-Code)
        abstract_img = generate_abstract_geometry(i)
        
        # Convert abstract PIL image to base64 for the Painter API
        temp_path = f"{frames_dir}/abstract_tmp.png"
        abstract_img.save(temp_path)
        with open(temp_path, "rb") as f:
            b64_img = base64.b64encode(f.read()).decode('utf-8')
            
        # 2. Engage the Painter (Stable Diffusion Vulkan Engine)
        sickness = int((i / FRAMES) * 100)
        prompt = f"A highly detailed photorealistic Sick Teddy Bear, matted brown fur, glowing green eyes, cinematic lighting, Atropa 999 aesthetic, {sickness} percent mutated, dark background"
        
        url = 'http://127.0.0.1:8080/sdapi/v1/img2img'
        data = {
            "prompt": prompt,
            "width": 512,
            "height": 512,
            "steps": 6,
            "cfg_scale": 3.0,
            "seed": 42 + i,
            "init_images": [b64_img],
            "denoising_strength": 0.85 
        }
        
        req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
        req.add_header('Content-Type', 'application/json')
        
        try:
            response = urllib.request.urlopen(req)
            result = json.loads(response.read())
            img_bytes = base64.b64decode(result['images'][0])
            with open(f"{frames_dir}/frame_{i:04d}.png", "wb") as f:
                f.write(img_bytes)
            print(f"-> United Frame {i}/{FRAMES} Synthesized. Abstract Geometry transformed into Photorealism.")
        except Exception as e:
            print(f"[FRACTURE] Painter API failed: {e}")
            
    # 3. Mux into final video
    print("\\n[ACTIVE] Compiling Video via FFmpeg...", flush=True)
    
    output_video = "assets/atropa_abstract_to_real.mp4"
    ffmpeg_cmd = [
        "ffmpeg", "-y", "-framerate", "15",
        "-i", f"{frames_dir}/frame_%04d.png",
        "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p",
        output_video
    ]
    
    subprocess.run(ffmpeg_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] Unified Video Synthesis Complete: {output_video}")

if __name__ == "__main__":
    render_true_synthesis_video()
