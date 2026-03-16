import urllib.request
import json
import base64
import os
import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def generate_sd_asset(prompt, output_file):
    print(f"  -> Demanding specialized asset: '{prompt}'")
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "steps": 12,
        "cfg_scale": 4.5,
        "seed": 42
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        img_bytes = base64.b64decode(result['images'][0])
        with open(output_file, "wb") as f:
            f.write(img_bytes)
        print(f"  -> [PASS] Asset locked: {output_file}")
        return True
    except Exception as e:
        print(f"  -> [FRACTURE] Vulkan Painter failed: {e}")
        return False

def boot_moondream_vlm():
    print("[INFO] Booting Moondream2 CPU Matrix for Structural Critique...")
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

def interrogate_moon_angle(model, processor, img_path, queries):
    img = Image.open(img_path).convert('RGB')
    enc_image = model.encode_image(img)
    results = []
    for q in queries:
        ans = model.answer_question(enc_image, q, processor)
        results.append((q, ans))
    return results

def run_deep_kinematic_aot():
    print("=== TSFi Advanced SD/VLM Extraction Matrix ===", flush=True)
    os.makedirs("assets", exist_ok=True)
    
    vlm_model, vlm_processor = boot_moondream_vlm()
    if not vlm_model: return
    
    # ---------------------------------------------------------
    # LAYER 1: The Sickness Propagation Mask (Normal Map)
    # ---------------------------------------------------------
    print("\\n[ACTIVE] Extracting Sickness Normal Map...")
    normal_prompt = "A pure RGB Normal map of a teddy bear, blue and purple normals facing the camera, photorealistic 3D vector map, no background"
    normal_path = "assets/aot_normal_map.png"
    if generate_sd_asset(normal_prompt, normal_path):
        queries = [
            "Analyze the vector directions. Do the normal map vectors push outwards aggressively on the stomach, or are they flat?",
            "Look at the edges of the normal map. Do the vectors imply a sharp, rigid boundary, or a soft, fuzzy boundary?"
        ]
        answers = interrogate_moon_angle(vlm_model, vlm_processor, normal_path, queries)
        for q, a in answers:
            print(f"  -> [MOON-ANGLE] Q: {q}\\n    A: {a}")

    # ---------------------------------------------------------
    # LAYER 2: The Floor Contact Shadow
    # ---------------------------------------------------------
    print("\\n[ACTIVE] Extracting Floor Contact Shadow Mask...")
    shadow_prompt = "A pure white void, with only the dark black contact shadow of a sitting teddy bear cast directly onto the floor beneath it."
    shadow_path = "assets/aot_contact_shadow.png"
    if generate_sd_asset(shadow_prompt, shadow_path):
        queries = [
            "Analyze the shape of the black shadow on the white floor. Is it a perfect oval, or does it branch out where the legs touch the ground?",
            "Provide the exact spatial bounding box of the black shadow on the floor."
        ]
        answers = interrogate_moon_angle(vlm_model, vlm_processor, shadow_path, queries)
        for q, a in answers:
            print(f"  -> [MOON-ANGLE] Q: {q}\\n    A: {a}")

if __name__ == "__main__":
    run_deep_kinematic_aot()
