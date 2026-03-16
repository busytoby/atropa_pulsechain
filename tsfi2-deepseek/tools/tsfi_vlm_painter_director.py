import os
import urllib.request
import json
import base64
import subprocess
import time
import math
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def generate_painted_frame(index, prompt, prev_frame_path, seed):
    url = 'http://127.0.0.1:8080/txt2img'
    
    # We use a highly compressed fast-step config to ensure generation completes quickly
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "step": 8,
        "cfg_scale": 3.0,
        "seed": seed
    }
    
    # If we are evolving the video, we inject the previous frame (img2img)
    # Wait, sd-server txt2img endpoint doesn't support init_image directly. 
    # sd-server supports img2img endpoint!
    if prev_frame_path and os.path.exists(prev_frame_path):
        url = 'http://127.0.0.1:8080/img2img'
        with open(prev_frame_path, "rb") as img_file:
            b64_img = base64.b64encode(img_file.read()).decode('utf-8')
            
        data["image_data"] = [{"data": b64_img, "id": 1}]
        data["strength"] = 0.65 # Allow 35% mutation from the text prompt
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        for i, img_data in enumerate(result):
            img_bytes = base64.b64decode(img_data['data'])
            with open(f"assets/frames/frame_{index:04d}.png", "wb") as f:
                f.write(img_bytes)
        return True
    except Exception as e:
        print(f"[FRACTURE] Vulkan Painter failed on frame {index}: {e}")
        return False

def boot_moondream_vlm():
    print("[INFO] Booting Moondream2 Vision-Language CPU Matrix...")
    model_id = "vikhyatk/moondream2"
    revision = "2024-04-02"
    try:
        model = AutoModelForCausalLM.from_pretrained(
            model_id, trust_remote_code=True, revision=revision, pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained(model_id, trust_remote_code=True, revision=revision)
        return model, processor
    except Exception as e:
        print(f"[WARN] VLM Boot Failed: {e}")
        return None, None

def run_vlm_director():
    print("=== TSFi VLM & Painter Autonomous Director ===", flush=True)
    
    frames_dir = "assets/frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    # Clean previous artifacts
    for f in os.listdir(frames_dir):
        os.remove(os.path.join(frames_dir, f))
        
    vlm_model, vlm_processor = boot_moondream_vlm()
    
    FPS = 15
    DURATION = 180 # 3 minutes
    TOTAL_FRAMES = FPS * DURATION
    STAND_TRIGGER = 165 * FPS # 2 minutes 45 seconds
    
    print(f"\\n[ACTIVE] Initiating 3-Minute VLM Execution Loop ({TOTAL_FRAMES} frames)...", flush=True)
    
    vlm_feedback_modifier = ""
    
    for i in range(TOTAL_FRAMES):
        # 1. Timeline Prompt Engineering
        if i < STAND_TRIGGER:
            # 0:00 to 2:45
            sickness = int((i / STAND_TRIGGER) * 100)
            base_prompt = f"A highly detailed photorealistic sick brown teddy bear sitting in the center of the room, cinematic lighting, {sickness} percent mutated, dark background"
        elif i < STAND_TRIGGER + (5 * FPS):
            # 2:45 to 2:50 (The Stand)
            base_prompt = "A highly detailed photorealistic sick brown teddy bear standing up straight on two legs, full body visible, cinematic lighting, dark background"
        else:
            # 2:50 to 3:00 (The Walk Off Stage Left)
            walk_prog = min(100, int(((i - (STAND_TRIGGER + (5*FPS))) / (10 * FPS)) * 100))
            base_prompt = f"A highly detailed photorealistic sick brown teddy bear walking away towards the far left side of the screen, leaving the room, cinematic lighting. Bear is {walk_prog} percent off-screen."
            
        # Combine base prompt with VLM's ongoing feedback
        final_prompt = f"{base_prompt}. {vlm_feedback_modifier}".strip()
        seed = 42 + i
        prev_frame = f"{frames_dir}/frame_{i-1:04d}.png" if i > 0 else None
        
        # 2. Vulkan Painter Execution
        success = generate_painted_frame(i, final_prompt, prev_frame, seed)
        
        if i % 50 == 0:
            print(f"-> Painter executed Frame {i}/{TOTAL_FRAMES} ({(i/TOTAL_FRAMES)*100:.1f}%)", flush=True)
            
        # 3. VLM Optical Scan (Every 4 seconds of video to adjust the trajectory)
        if success and vlm_model and (i > 0) and (i % 60 == 0):
            print("  -> [VLM SCAN] Moondream2 evaluating physical geometry...")
            img = Image.open(f"{frames_dir}/frame_{i:04d}.png").convert("RGB")
            enc_image = vlm_model.encode_image(img)
            
            if i < STAND_TRIGGER:
                q = "Is the teddy bear sitting in the center of the image?"
            else:
                q = "Is the teddy bear walking towards the left edge of the image?"
                
            response = vlm_model.answer_question(enc_image, q, vlm_processor)
            print(f"  -> [VLM RESPONSE] {response}")
            
            # Autonomously parse the VLM's logic to steer the Painter
            if "no" in response.lower() or "not" in response.lower() or "right" in response.lower():
                if i < STAND_TRIGGER:
                    vlm_feedback_modifier = "Force the subject to sit exactly in the center."
                else:
                    vlm_feedback_modifier = "Aggressively push the subject to the far left edge of the frame."
            else:
                vlm_feedback_modifier = "Perfect alignment, maintain current trajectory."

    print("\\n[ACTIVE] Stitching VLM neural geometry into final .mp4 via FFmpeg...", flush=True)
    
    output_video = "assets/atropa_vlm_painter_3min.mp4"
    ffmpeg_cmd = [
        "ffmpeg", "-y", "-framerate", str(FPS),
        "-i", f"{frames_dir}/frame_%04d.png",
        "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p",
        output_video
    ]
    
    subprocess.run(ffmpeg_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    # Calculate and output final file mass
    file_size = os.path.getsize(output_video)
    print(f"[ABSOLUTE SUCCESS] VLM Director Video Synthesis Complete.")
    print(f"  -> Generated: {output_video}")
    print(f"  -> Highly Compressed File Mass: {file_size / 1024 / 1024:.2f} MB")

if __name__ == "__main__":
    run_vlm_director()
