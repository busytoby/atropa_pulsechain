import os
import urllib.request
import json
import base64
import math
import time
import subprocess
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def generate_abstract_geometry(frame, W=512, H=512, TOTAL_FRAMES=2700):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    time_offset = frame * 0.1
    macro_evolution = frame / TOTAL_FRAMES
    pulse = math.sin(macro_evolution * math.pi * 50.0) 
    
    scale = 512 / 1536.0
    
    head_radius = (200 + pulse * 10) * scale
    left_ear_r = 75 * scale
    pupil_radius = (20 + abs(pulse) * 15) * scale
    
    g_x = 0.0
    g_y = 0.0
    body_stretch_y = 0.25
    
    STAND_TRIGGER = 165 * 15 # 2m45s at 15fps (2475)
    
    if frame > STAND_TRIGGER:
        active_frames = frame - STAND_TRIGGER
        if active_frames < 15: # 1 second to stand
            stand_progress = active_frames / 15.0
            g_y -= (stand_progress * 0.15)
            body_stretch_y += (stand_progress * 0.15)
        else:
            walk_frames = active_frames - 15
            walk_progress = float(walk_frames)
            g_y -= 0.15
            body_stretch_y = 0.40
            g_x -= (walk_progress * 0.006) 
            g_y += (math.sin(walk_progress * 0.5) * 0.02) 
            g_x += (math.cos(walk_progress * 0.25) * 0.01)
    
    for y in range(H):
        py = y / H
        if py < (0.1 + g_y) or py > (0.95 + g_y): continue
        for x in range(W):
            px = x / W
            if px < (0.1 + g_x) or px > (0.9 + g_x): continue
            
            ax = px - g_x
            ay = py - g_y
            
            dist_eye1 = math.hypot(ax - 0.5, ay - 0.4)
            dist_eye2 = math.hypot(ax - 0.5, ay - 0.6)
            pulse_radius = 0.02 + (pulse * 0.005)
            
            if dist_eye1 < pulse_radius or dist_eye2 < (0.045 + pulse * 0.01):
                pixels[x, y] = (0, 255, 0)
            elif math.hypot(ax - 0.362, ay - 0.25) < 0.075 or math.hypot(ax - 0.662, ay - 0.25) < 0.075:
                pixels[x, y] = (255, 0, 0)
            elif math.hypot(ax - 0.5, ay - 0.5) < (0.2 + (pulse * 0.01)):
                pixels[x, y] = (255, 0, 0)
            elif math.hypot(ax - 0.5, (ay - 0.75) / (body_stretch_y / 0.25)) < 0.25:
                if (x * y * frame) % 100 < 50:
                    pixels[x, y] = (84, 107, 45)
                else:
                    pixels[x, y] = (91, 63, 51)
                    
    return img

import concurrent.futures

def boot_moondream_vlm():
    print(f"[INFO] Booting Moondream2 Vision-Language Matrix across {os.cpu_count()} CPU cores...")
    try:
        # Force PyTorch to map the tensor math across all physical CPU cores
        torch.set_num_threads(os.cpu_count())
        
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        return model, processor
    except:
        return None, None

def run_unified_director():
    print("=== TSFi Unified Abstract-to-Real Director (3-Minute Matrix) ===", flush=True)
    frames_dir = "assets/unified_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    vlm_model, vlm_processor = boot_moondream_vlm()
    
    FPS = 15
    DURATION = 180
    TOTAL_FRAMES = FPS * DURATION
    STAND_TRIGGER = 165 * FPS
    
    time.sleep(5) # Give SD Server time to boot
    vlm_feedback = "Maintain structural photorealism."
    
    # Thread pool for generating masks on background CPU cores
    executor = concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count())
    future_mask = executor.submit(generate_abstract_geometry, 0, TOTAL_FRAMES=TOTAL_FRAMES)
    
    for i in range(TOTAL_FRAMES):
        # Wait for the background CPU thread to finish this frame's mask
        abstract_img = future_mask.result()
        
        # Immediately kick off the NEXT frame's mask calculation on another CPU core
        if i + 1 < TOTAL_FRAMES:
            future_mask = executor.submit(generate_abstract_geometry, i + 1, TOTAL_FRAMES=TOTAL_FRAMES)
            
        temp_path = f"{frames_dir}/abstract_tmp.png"
        abstract_img.save(temp_path)
        with open(temp_path, "rb") as f:
            b64_img = base64.b64encode(f.read()).decode('utf-8')
            
        # 2. VLM Directed Prompt
        if i < STAND_TRIGGER:
            prompt = f"Photorealistic sick brown teddy bear sitting, cinematic lighting, {int((i/STAND_TRIGGER)*100)} percent mutated. {vlm_feedback}"
        else:
            prompt = f"Photorealistic sick brown teddy bear walking away off stage left, {int(((i-STAND_TRIGGER)/(15*FPS))*100)} percent off screen. {vlm_feedback}"
            
        # 3. Vulcan Painter API
        data = {
            "prompt": prompt, "width": 512, "height": 512, "steps": 6, "cfg_scale": 2.5, "seed": 42 + i,
            "init_images": [b64_img], "denoising_strength": 0.82 
        }
        
        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
        req.add_header('Content-Type', 'application/json')
        
        try:
            response = urllib.request.urlopen(req)
            img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
            frame_path = f"{frames_dir}/frame_{i:04d}.png"
            with open(frame_path, "wb") as f: f.write(img_bytes)
            if i % 30 == 0: print(f"-> Generated {i}/{TOTAL_FRAMES} ({(i/TOTAL_FRAMES)*100:.1f}%)", flush=True)
            
            # VLM Analysis every 60 frames (4 seconds)
            if i > 0 and i % 60 == 0 and vlm_model:
                img = Image.open(frame_path).convert("RGB")
                q = "Is the bear standing up?" if i >= STAND_TRIGGER else "Is the bear sitting?"
                ans = vlm_model.answer_question(vlm_model.encode_image(img), q, vlm_processor)
                vlm_feedback = "Ensure bipedal stance." if "no" in ans.lower() else "Maintain alignment."
                
        except Exception as e:
            print(f"[FRACTURE] API failed frame {i}: {e}")

    print("\\n[ACTIVE] Compiling Heavily Compressed Video via FFmpeg...", flush=True)
    output_video = "assets/atropa_unified_3min.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] {output_video} ({os.path.getsize(output_video)/1024/1024:.2f} MB)")

if __name__ == "__main__": run_unified_director()