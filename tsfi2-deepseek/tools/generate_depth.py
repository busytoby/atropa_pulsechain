import torch
import cv2
import numpy as np
import sys
import os

# Load MiDaS for Depth Estimation
print("[INFO] Loading MiDaS Depth Model...")
midas_model_type = "DPT_Hybrid" # Use DPT_Large for maximum quality, DPT_Hybrid for balance
try:
    midas = torch.hub.load("intel-isl/MiDaS", midas_model_type)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    midas.to(device)
    midas.eval()

    midas_transforms = torch.hub.load("intel-isl/MiDaS", "transforms")
    transform = midas_transforms.dpt_transform
except Exception as e:
    print(f"[ERROR] Failed to load MiDaS: {e}")
    sys.exit(1)

def generate_depth_map(input_image_path, output_image_path):
    print(f"[INFO] Generating Depth Map for {input_image_path}")
    
    img = cv2.imread(input_image_path)
    if img is None:
        print("[ERROR] Input image not found.")
        return

    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    input_batch = transform(img).to(device)

    with torch.no_grad():
        prediction = midas(input_batch)
        
        # Resize the prediction to match the original image resolution
        prediction = torch.nn.functional.interpolate(
            prediction.unsqueeze(1),
            size=img.shape[:2],
            mode="bicubic",
            align_corners=False,
        ).squeeze()

    output = prediction.cpu().numpy()

    # Normalize depth output to 0-255 for image saving
    output_normalized = cv2.normalize(output, None, 0, 255, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_8U)
    
    # Save the depth map
    cv2.imwrite(output_image_path, output_normalized)
    print(f"[SUCCESS] Depth map saved to {output_image_path}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 tools/generate_depth.py <input.png> <output_depth.png>")
    else:
        generate_depth_map(sys.argv[1], sys.argv[2])
