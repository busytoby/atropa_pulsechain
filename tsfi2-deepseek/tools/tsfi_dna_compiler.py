import os
import json
import struct
import math
import time
import urllib.request
import base64
import torch
import re
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor
from llama_cpp import Llama

def run_dna_compiler():
    print("=== TSFi Unified DNA Compiler (Tri-Model Architecture) ===", flush=True)
    os.makedirs("assets", exist_ok=True)
    
    # ---------------------------------------------------------
    # PHASE 1: The Imagination (Stable Diffusion Target Frame)
    # ---------------------------------------------------------
    print("\\n[PHASE 1] stable-diffusion: Synthesizing Photorealistic Target Asset...")
    sd_prompt = "A highly detailed photorealistic Sick Teddy Bear, matted brown fur, glowing green eyes, cinematic lighting, Atropa 999 aesthetic, dark background"
    
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    data = { "prompt": sd_prompt, "width": 512, "height": 512, "steps": 10, "cfg_scale": 4.0, "seed": 42 }
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
        with open("assets/sd_target_concept.png", "wb") as f:
            f.write(img_bytes)
        print("  -> [PASS] Photorealistic concept generated and saved.")
    except Exception as e:
        print(f"  -> [FRACTURE] SD Server failed. Ensure sd-server is running on port 8080. ({e})")
        return

    # ---------------------------------------------------------
    # PHASE 2: The Artistic Critique (Moondream VLM)
    # ---------------------------------------------------------
    print("\\n[PHASE 2] moondream2: Extracting Aesthetic Math from Concept Image...")
    try:
        torch.set_num_threads(os.cpu_count())
        vlm_model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        vlm_processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        
        img = Image.open("assets/sd_target_concept.png").convert("RGB")
        enc_image = vlm_model.encode_image(img)
        
        q = "Analyze the Sick Teddy Bear. Provide exactly three things: 1. The exact RGB hex color of its fur. 2. The exact RGB hex color of its eyes. 3. What percentage of its body is covered in sickness/mutations?"
        aesthetic_critique = vlm_model.answer_question(enc_image, q, vlm_processor)
        print(f"  -> [VLM OUTPUT] {aesthetic_critique}")
    except Exception as e:
        print(f"  -> [FRACTURE] Moondream VLM failed: {e}")
        return

    # ---------------------------------------------------------
    # PHASE 3: The Geometric Translation (DeepSeek Coder)
    # ---------------------------------------------------------
    print("\\n[PHASE 3] deepseek-coder: Translating VLM Critique into PM4 Math...")
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path): return
    
    llm = Llama(model_path=gguf_path, n_ctx=2048, n_threads=16, n_gpu_layers=-1, verbose=False)
    
    prompt = f"""You are the TSFi DNA Architect.
The VLM has analyzed the Photorealistic Target Image and provided this aesthetic critique:
"{aesthetic_critique}"

Translate this critique into the baseline integers for our PM4 C-Kernel Geometry.
If the VLM outputted array coordinates instead of Hex colors, just pick a reasonable RGB integer (0-255) for brown fur and green eyes. 
Output exactly a JSON object with these keys: 'fur_r', 'fur_g', 'fur_b', 'eye_r', 'eye_g', 'eye_b', 'sickness_threshold_percentage'.
Example: {{"fur_r": 92, "fur_g": 64, "fur_b": 51, "eye_r": 0, "eye_g": 255, "eye_b": 0, "sickness_threshold_percentage": 45}}
DO NOT WRITE ANY CODE. Do not explain. Output STRICTLY the JSON object."""

    output = llm(f"### Instruction:\\n{prompt}\\n### Response:\\n", max_tokens=128, temperature=0.1, stop=["### Instruction:"], echo=False)
    report = output['choices'][0]['text'].strip()
    
    try:
        # Strip markdown if hallucinates
        clean_json = re.search(r'\{.*?\}', report.replace('\\n', ' ')).group(0)
        dna_math = json.loads(clean_json)
        print(f"  -> [PASS] DeepSeek translated JSON: {dna_math}")
    except Exception as e:
        print(f"  -> [FRACTURE] JSON parse failed: {report} ({e})")
        return

    # ---------------------------------------------------------
    # PHASE 4: Fusing the DNA Vessel (Binary Serialization)
    # ---------------------------------------------------------
    print("\\n[PHASE 4] TSFi Runtime: Fusing the 10,800-frame DNA Binary...")
    
    TOTAL_FRAMES = 10800
    STAND_FRAME = 9900
    
    fur_r = int(dna_math.get("fur_r", 91))
    fur_g = int(dna_math.get("fur_g", 63))
    fur_b = int(dna_math.get("fur_b", 51))
    
    eye_r = int(dna_math.get("eye_r", 0))
    eye_g = int(dna_math.get("eye_g", 255))
    eye_b = int(dna_math.get("eye_b", 0))
    
    base_sick = int(dna_math.get("sickness_threshold_percentage", 50)) / 100.0
    
    dna_path = "assets/atropa.dna"
    with open(dna_path, "wb") as f:
        # Write DNA Header (Magic bytes + Total Frames)
        f.write(b'TSFI')
        f.write(struct.pack('=I', TOTAL_FRAMES))
        
        # Calculate kinematics for every frame
        g_x = 0.0
        g_y = 0.0
        body_y_stretch = 0.25
        
        for frame in range(TOTAL_FRAMES):
            macro = frame / float(TOTAL_FRAMES)
            pulse = math.sin(macro * math.pi * 50.0)
            
            if frame > STAND_FRAME:
                active = frame - STAND_FRAME
                if active < 60:
                    prog = active / 60.0
                    g_y = -(prog * 0.15)
                    body_y_stretch = 0.25 + (prog * 0.15)
                else:
                    walk = active - 60
                    g_y = -0.15 + (math.sin(walk * 0.5) * 0.02)
                    g_x = -(walk * 0.003) + (math.cos(walk * 0.25) * 0.01)
                    body_y_stretch = 0.40
            
            dynamic_sick = base_sick - (macro * 0.2)
            
            # Phase 0.5: Sickness Mutation (Missing Eye)
            eye_count = 2
            if dynamic_sick < 0.6: 
                eye_count = 1 # Suppress the right eye due to VLM anatomical fracture
                
            # Phase 2: Dynamic Specular Lighting
            # Base intensity is 0.5. As g_y decreases (stand up), intensity spikes to 1.0
            light_intensity = 0.5
            if g_y < 0.0:
                light_intensity += min(0.5, abs(g_y) * 2.0)
                
            # Pack exact geometric state: 
            # [g_x(f), g_y(f), body_stretch(f), pulse(f), sick_thresh(f), light_intensity(f), fur_r(B), fur_g(B), fur_b(B), eye_r(B), eye_g(B), eye_b(B), eye_count(B)]
            # 6 floats (24 bytes) + 7 bytes = 31 bytes per frame
            frame_data = struct.pack('=ffffffBBBBBBB', 
                                     g_x, g_y, body_y_stretch, pulse, dynamic_sick, light_intensity,
                                     fur_r, fur_g, fur_b, eye_r, eye_g, eye_b, eye_count)
            f.write(frame_data)
            
    file_size = os.path.getsize(dna_path)
    print(f"  -> [ABSOLUTE SUCCESS] atropa.dna vessel sealed. Mass: {file_size / 1024:.2f} KB.")
    print("  -> The PM4 C-Kernel can now read this file and render 10,800 frames instantly.")

if __name__ == "__main__":
    run_dna_compiler()
