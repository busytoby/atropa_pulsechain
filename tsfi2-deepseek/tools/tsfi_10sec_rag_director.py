import os
import urllib.request
import json
import base64
import time
import subprocess
import re
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def retrieve_rag_lore():
    print("[RAG] Retrieving Atropa/999 Lore bounds...")
    try:
        # Instead of scanning C-code geometry, we RAG the lore text for the Painter
        lore = "Atropa/999 Sovereign Choreography involving a Sick Teddy Bear."
        with open("inc/tsfi_atropa_geometry.h", "r") as f:
            c_header = f.read()
            match = re.search(r'static const TsfiTeddyEye TSFI_BASELINE_EYE[^;]*;', c_header, re.DOTALL)
            if match: lore += " Baseline Eye is Green."
        return lore
    except:
        return "Sick Teddy Bear, dark background."

def generate_painted_frame(index, prompt, prev_frame_path, seed):
    url = 'http://127.0.0.1:8080/txt2img'
    
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "step": 6,
        "cfg_scale": 3.0,
        "seed": seed
    }
    
    if prev_frame_path and os.path.exists(prev_frame_path):
        url = 'http://127.0.0.1:8080/img2img'
        with open(prev_frame_path, "rb") as img_file:
            b64_img = base64.b64encode(img_file.read()).decode('utf-8')
        data["image_data"] = [{"data": b64_img, "id": 1}]
        data["strength"] = 0.55 # Lower strength for smoother evolution
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        img_bytes = base64.b64decode(result[0]['data'])
        with open(f"assets/rag_frames/frame_{index:03d}.png", "wb") as f:
            f.write(img_bytes)
        return True
    except Exception as e:
        print(f"[FRACTURE] Painter API failed on frame {index}: {e}")
        return False

def boot_moondream_vlm():
    print("[INFO] Booting Moondream2 Vision-Language CPU Matrix...")
    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        return model, processor
    except:
        return None, None

def run_10sec_rag_director():
    print("=== TSFi 10-Second RAG-VLM-Painter Matrix ===", flush=True)
    frames_dir = "assets/rag_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    # 1. RAG Extractor
    lore_context = retrieve_rag_lore()
    
    vlm_model, vlm_processor = boot_moondream_vlm()
    
    FPS = 15
    DURATION = 10
    TOTAL_FRAMES = FPS * DURATION
    
    print(f"\\n[ACTIVE] Initiating 10-Second RAG Execution Loop ({TOTAL_FRAMES} frames)...", flush=True)
    
    vlm_feedback_modifier = ""
    
    for i in range(TOTAL_FRAMES):
        sickness = int((i / TOTAL_FRAMES) * 100)
        
        # We inject the RAG Lore directly into the generative prompt
        prompt = f"Photorealistic 8k, {lore_context} Geometry mutated {sickness} percent. {vlm_feedback_modifier}"
        seed = 42 + i
        prev_frame = f"{frames_dir}/frame_{i-1:03d}.png" if i > 0 else None
        
        success = generate_painted_frame(i, prompt, prev_frame, seed)
        
        if i % 15 == 0:
            print(f"-> Painter executed Frame {i}/{TOTAL_FRAMES} ({(i/TOTAL_FRAMES)*100:.1f}%)", flush=True)
            
        # VLM Optical Scan (Every 2 seconds)
        if success and vlm_model and (i > 0) and (i % 30 == 0):
            print("  -> [VLM SCAN] Moondream2 evaluating physical geometry...")
            img = Image.open(f"{frames_dir}/frame_{i:03d}.png").convert("RGB")
            
            # The VLM questions the execution based on the RAG lore
            q = f"Does this image accurately depict a {lore_context}?"
            ans = vlm_model.answer_question(vlm_model.encode_image(img), q, vlm_processor)
            print(f"  -> [VLM RESPONSE] {ans}")
            
            if "no" in ans.lower() or "not" in ans.lower():
                vlm_feedback_modifier = "Aggressively enforce bear anatomy and dark aesthetic."
            else:
                vlm_feedback_modifier = "Maintain structural photorealism."

    print("\\n[ACTIVE] Compiling Video via FFmpeg...", flush=True)
    output_video = "assets/atropa_10sec_rag_painter.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%03d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "28", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] {output_video} ({os.path.getsize(output_video)/1024/1024:.2f} MB)")

if __name__ == "__main__": run_10sec_rag_director()