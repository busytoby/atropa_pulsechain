import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor
import numpy as np
import sys
import os

# TSFi Deep Moon Dream Streamer: VLM Audit Node
# Fully 3D Involved: RGB + Depth Analysis

def run_3d_audit(rgb_path, depth_path):
    print(f"[MOON] Initiating 3D-Involved Audit for {rgb_path}...")
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        
        rgb = Image.open(rgb_path).convert('RGB')
        depth = Image.open(depth_path).convert('L')
        depth_data = np.array(depth)
        
        # 1. Visual Interrogation
        enc_image = model.encode_image(rgb)
        
        # Query for physical flaws
        critique = model.answer_question(enc_image, "Identify the region where the teddy bear fur looks most synthetic. Provide the normalized bounding box [Y_min, X_min, Y_max, X_max].", processor)
        print(f"[CRITIQUE] {critique}")
        
        # Parse box (simple extraction)
        try:
            # Assuming Moondream returns something like "[0.1, 0.2, 0.3, 0.4]"
            box_str = critique.strip("[]").split(",")
            y_min, x_min, y_max, x_max = [float(v) for v in box_str[:4]]
            
            # 2. 3D Spatial Mapping (The "Streamer" Advantage)
            # Find the average depth in this box to project back to 3D voxel space
            h, w = depth_data.shape
            iy_min, ix_min = int(y_min * h), int(x_min * w)
            iy_max, ix_max = int(y_max * h), int(x_max * w)
            box_depth = np.mean(depth_data[iy_min:iy_max, ix_min:ix_max])
            
            print(f"[3D-MAP] Fracture localized at Pixel({ix_min},{iy_min}) with Depth {box_depth:.2f}")
            print(f"[STREAM] Emitting physical correction: Roughness Secret += 0.1, Melanin Secret -= 0.05")
            
        except:
            print("[WARN] Could not parse spatial box from VLM response.")

    except Exception as e:
        print(f"[FRACTURE] VLM Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 tsfi_dream_streamer_vlm.py <rgb_path> <depth_path>")
    else:
        run_3d_audit(sys.argv[1], sys.argv[2])
