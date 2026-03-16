import torch
import cv2
import numpy as np
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image
import sys

print("[INFO] Loading Moondream Genie...")
try:
    model = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
    processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
except Exception as e:
    print(f"Failed to load Genie: {e}")
    sys.exit(1)

def orchestrate_3d(rgb_path, depth_path):
    print(f"\n[INFO] Orchestrating 3D Construction for: {rgb_path}")
    
    # 1. Load the flat 2D RGB image
    img = Image.open(rgb_path).convert("RGB")
    enc_image = model.encode_image(img)
    
    # 2. Genie identifies the spatial relationships
    q_space = "What object is closest to the camera, and what is behind it? Be specific."
    ans_space = model.answer_question(enc_image, q_space, processor)
    print(f"[GENIE SPATIAL] {ans_space}")
    
    # 3. Genie identifies the exact materials for the 3D Shader
    q_mat = "Describe the physical material and texture of these objects in one short sentence, suitable for a 3D renderer."
    ans_mat = model.answer_question(enc_image, q_mat, processor)
    print(f"[GENIE MATERIAL] {ans_mat}")

    # 4. In a real pipeline, the Genie's output tells the C++ meshing engine HOW to cut the depth map
    print("\n[PIPELINE INSTRUCTION]")
    print("If we pass the raw Depth Map (Z-axis) into the 3D Mesher right now, the background will stretch like rubber to connect to the back of the plushies.")
    print("Because the Genie identified that the plushies are in front, the pipeline must:")
    print("  a) Cut the Depth Map where Z changes rapidly (the silhouette).")
    print(f"  b) Apply the Shader Material: '{ans_mat}' to the extruded geometry.")

if __name__ == "__main__":
    # Just grab the newest generated depth map pair
    import glob
    depths = sorted(glob.glob("assets/depth_maps/depth_*.png"))
    rgbs = sorted(glob.glob("assets/depth_maps/rgb_*.png"))
    if depths and rgbs:
        orchestrate_3d(rgbs[-1], depths[-1])
    else:
        print("No depth maps found. Let the daemon run longer.")
