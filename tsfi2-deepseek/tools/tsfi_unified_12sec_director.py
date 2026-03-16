import os
import urllib.request
import json
import base64
import math
import time
import subprocess
from PIL import Image

def generate_dna_mask(frame, W=512, H=512, TOTAL_FRAMES=180):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    time_offset = frame * 0.1
    pulse = math.sin((frame / TOTAL_FRAMES) * math.pi * 20.0) 
    scale = 512 / 1536.0
    
    # Apply the exact VLM and DeepSeek extracted DNA parameters
    # Fur Color (76, 29, 14), Sickness (51, 65, 0)
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
            
            # The DeepSeek Exponential Shadow (Phase 4)
            dist_left = math.hypot(px - 0.400, py - 0.780)
            dist_right = math.hypot(px - 0.624, py - 0.780)
            k = 50.0
            shadow_val = math.exp(-k * dist_left) + math.exp(-k * dist_right)
            shadow_dist = -math.log(shadow_val) / k if shadow_val > 0 else 1.0
            
            if shadow_dist < 0.08:
                pixels[x, y] = (19, 19, 19) # VLM Shadow Floor
            elif dist_eye1 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif dist_eye2 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif math.hypot(px - 0.35, py - 0.25) < 0.075 or math.hypot(px - 0.65, py - 0.25) < 0.075:
                pixels[x, y] = (76, 29, 14) # Fur baseline
            elif math.hypot(px - 0.5, py - 0.5) < (0.2 + (pulse * 0.01)):
                if math.hypot(px - 0.45, py - 0.45) < 0.08:
                    pixels[x, y] = (252, 252, 252) # Specular Highlight
                else:
                    pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                if (x * y * frame) % 100 < 50:
                    pixels[x, y] = (51, 65, 0) # VLM Sickness Baseline
                else:
                    pixels[x, y] = (76, 29, 14)
                    
    return img

import concurrent.futures

def run_ultimate_12sec_director():
    print("=== TSFi Ultimate 12-Second AOT Photorealistic Matrix ===", flush=True)
    frames_dir = "assets/ultimate_12sec_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    FPS = 15
    TOTAL_FRAMES = 180 # 12 Seconds
    
    time.sleep(2)
    
    print(f"\\n[ACTIVE] Initiating 12-Second AOT Execution Loop ({TOTAL_FRAMES} frames) on {os.cpu_count()} CPU cores...", flush=True)
    
    # Pre-compute the abstract mathematical masks across ALL physical CPU cores
    executor = concurrent.futures.ProcessPoolExecutor(max_workers=os.cpu_count())
    
    # We submit the next frame to the background CPU pool so it's ready before the GPU needs it
    future_mask = executor.submit(generate_dna_mask, 0, W=512, H=512, TOTAL_FRAMES=TOTAL_FRAMES)
    
    for i in range(TOTAL_FRAMES):
        # 1. Yield the pre-computed mask from the multi-core pool
        abstract_img = future_mask.result()
        
        # Immediately kick off the next frame's mask generation on a free CPU core
        if i + 1 < TOTAL_FRAMES:
            future_mask = executor.submit(generate_dna_mask, i + 1, W=512, H=512, TOTAL_FRAMES=TOTAL_FRAMES)
            
        temp_path = f"{frames_dir}/abstract_tmp.png"
        abstract_img.save(temp_path)
        with open(temp_path, "rb") as f:
            b64_img = base64.b64encode(f.read()).decode('utf-8')
            
        # 2. Architect Directed Prompt
        sickness = int((i / TOTAL_FRAMES) * 100)
        prompt = f"Photorealistic sick brown teddy bear sitting, cinematic lighting, {sickness} percent mutated, dark background. Masterpiece, 8k resolution."
            
        # 3. Vulcan Painter API
        data = {
            "prompt": prompt, "width": 512, "height": 512, "steps": 6, "cfg_scale": 2.5, "seed": 42 + (i%5),
            "init_images": [b64_img], "denoising_strength": 0.85 # High strength paints photorealism over math
        }
        
        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
        req.add_header('Content-Type', 'application/json')
        
        try:
            response = urllib.request.urlopen(req)
            img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
            frame_path = f"{frames_dir}/frame_{i:04d}.png"
            with open(frame_path, "wb") as f: f.write(img_bytes)
            
            if i % 15 == 0:
                print(f"-> Painter generated Frame {i}/{TOTAL_FRAMES}", flush=True)
                
        except Exception as e:
            print(f"[FRACTURE] API failed frame {i}: {e}")

    print("\\n[ACTIVE] Compiling Heavily Compressed Video via FFmpeg...", flush=True)
    output_video = "assets/atropa_ultimate_12sec_photorealistic.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] 12-Second Neural Synthesis Complete: {output_video} ({os.path.getsize(output_video)/1024/1024:.2f} MB)")

if __name__ == "__main__": run_ultimate_12sec_director()