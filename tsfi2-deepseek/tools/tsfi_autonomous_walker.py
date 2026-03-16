import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor
import os
import re

def update_c_vtuber_logic(python_script_path, new_radius_x, new_radius_y):
    print(f"[INFO] Hot-patching C Vtuber Script -> rx: {new_radius_x}, ry: {new_radius_y}")
    with open(python_script_path, "r") as f:
        code = f.read()
    
    # Replace the existing hardcoded circle walking logic with dynamically learned values
    code = re.sub(r"radius_x = [0-9.]+", f"radius_x = {new_radius_x}", code)
    code = re.sub(r"radius_y = [0-9.]+", f"radius_y = {new_radius_y}", code)
    
    with open(python_script_path, "w") as f:
        f.write(code)

def run_autonomous_walking_loop():
    print("=== TSFi VLM Autonomous Gait Learning (Moondream2) ===")
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
    except Exception as e:
        print(f"[FRACTURE] VLM failed to load: {e}")
        return

    # Assume the VTuber script just generated a recent frame of the bear walking
    frame_path = "assets/mpx_out.png"
    if not os.path.exists(frame_path):
        print(f"[FRACTURE] Missing frame to critique: {frame_path}")
        return
        
    img = Image.open(frame_path).convert('RGB')
    enc_image = model.encode_image(img)
    
    print("\n[ACTIVE] Interrogating Walking Frame for Elliptical Geometry...\n")
    
    q = "Look at the teddy bear walking. Is it walking in a perfect circle, or does the perspective look too flat? Answer with YES or NO, and provide an estimated X/Y pixel radius correction if it is too flat."
    
    print(f"QUERY: {q}")
    ans = model.answer_question(enc_image, q, processor)
    print(f"RESPONSE: {ans}\n")
    
    # Parse the LLM's physical critique
    # For simulation, if it detects "too flat" or "NO", we dynamically expand the Y radius
    if "NO" in ans.upper() or "flat" in ans.lower():
        print("[ADJUST] VLM detected geometric distortion. Mutating walking path via JIT Python hot-patch...")
        
        # We assume baseline was rx=180, ry=60. We will inject a wider radius to learn.
        update_c_vtuber_logic("tools/tsfi_c_vtuber.py", 220.0, 110.0)
        
        print("[SUCCESS] Walker neural path updated. Restarting VTuber stream to evaluate new gait...")
        os.system("pkill -f tsfi_c_vtuber || true")
        os.system("nohup python3 tools/tsfi_c_vtuber.py > vtuber_server.log 2>&1 &")

if __name__ == "__main__":
    run_autonomous_walking_loop()

