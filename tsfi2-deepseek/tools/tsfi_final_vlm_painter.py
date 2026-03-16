import os
import urllib.request
import json
import base64
import time
import subprocess
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def generate_vulkan_frame(index, prompt, prev_frame_path, seed):
    # If there is no previous frame, we use txt2img to forge the Genesis Frame
    if not prev_frame_path:
        url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
        data = {
            "prompt": prompt,
            "width": 512,
            "height": 512,
            "steps": 6,
            "cfg_scale": 2.5,
            "seed": seed
        }
    else:
        # Organic Evolution Loop: img2img
        url = 'http://127.0.0.1:8080/sdapi/v1/img2img'
        with open(prev_frame_path, "rb") as img_file:
            b64_img = base64.b64encode(img_file.read()).decode('utf-8')
        data = {
            "prompt": prompt,
            "width": 512,
            "height": 512,
            "steps": 6,
            "cfg_scale": 2.5,
            "seed": seed,
            "init_images": [b64_img],
            "denoising_strength": 0.35 # Low strength keeps the bear stable but allows movement/sickness mutation
        }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        img_bytes = base64.b64decode(result['images'][0])
        with open(f"assets/vlm_frames/frame_{index:04d}.png", "wb") as f:
            f.write(img_bytes)
        return True
    except Exception as e:
        print(f"[FRACTURE] Painter API failed: {e}")
        return False

def boot_moondream_vlm():
    print("[INFO] Booting Moondream2 Vision-Language CPU Matrix...")
    try:
        torch.set_num_threads(os.cpu_count())
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        return model, processor
    except Exception as e:
        print(f"[WARN] VLM Boot Failed: {e}")
        return None, None

def run_final_vlm_director():
    print("=== TSFi Final VLM & Painter 3-Minute Choreography ===", flush=True)
    frames_dir = "assets/vlm_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    vlm_model, vlm_processor = boot_moondream_vlm()
    
    FPS = 15
    DURATION = 180
    TOTAL_FRAMES = FPS * DURATION
    STAND_TRIGGER = 165 * FPS # 2 minutes 45 seconds
    
    time.sleep(2)
    vlm_feedback = "Maintain structural photorealism."
    
    print(f"\\n[ACTIVE] Initiating 3-Minute VLM Execution Loop ({TOTAL_FRAMES} frames)...", flush=True)
    
    for i in range(TOTAL_FRAMES):
        sickness = int((i / TOTAL_FRAMES) * 100)
        
        # 1. Timeline Prompt Engineering
        if i < STAND_TRIGGER:
            base_prompt = f"Photorealistic sick brown teddy bear sitting in the center of the room, cinematic lighting, {sickness} percent mutated, dark background"
        elif i < STAND_TRIGGER + (5 * FPS):
            base_prompt = "Photorealistic sick brown teddy bear abruptly standing up straight on two legs, full body visible, cinematic lighting, dark background"
        else:
            base_prompt = "Photorealistic sick brown teddy bear walking away towards the far left side of the screen, leaving the room, cinematic lighting"
            
        final_prompt = f"{base_prompt}. {vlm_feedback}"
        seed = 42 + (i % 5) # Bouncing seed for organic bubbling
        prev_frame = f"{frames_dir}/frame_{i-1:04d}.png" if i > 0 else None
        
        # 2. Vulkan Painter Execution
        success = generate_vulkan_frame(i, final_prompt, prev_frame, seed)
        
        if i % 30 == 0:
            print(f"-> Painter executed Frame {i}/{TOTAL_FRAMES} ({(i/TOTAL_FRAMES)*100:.1f}%)", flush=True)
            
        # 3. VLM Optical Scan (Every Single Frame)
        if success and vlm_model:
            print(f"  -> [VLM SCAN {i}] Moondream2 evaluating structural trajectory...")
            img = Image.open(f"{frames_dir}/frame_{i:04d}.png").convert("RGB")
            
            q = "What should be changed to make the bear walk off the left side of the screen?" if i >= STAND_TRIGGER else "What should be changed to make the bear look sicker and more mutated?"
            ans = vlm_model.answer_question(vlm_model.encode_image(img), q, vlm_processor)
            print(f"  -> [VLM RESPONSE] {ans}")
            
            # Autonomously inject the VLM's literal critique back into the next frame's prompt
            # We take the first sentence of the VLM's critique to avoid prompt overflow
            vlm_feedback = ans.split('.')[0] + "."

    print("\\n[ACTIVE] Compiling Heavily Compressed Video via FFmpeg...", flush=True)
    output_video = "assets/atropa_vlm_painter_3min_final.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] VLM Director Video Synthesis Complete: {output_video} ({os.path.getsize(output_video)/1024/1024:.2f} MB)")

if __name__ == "__main__":
    run_final_vlm_director()
