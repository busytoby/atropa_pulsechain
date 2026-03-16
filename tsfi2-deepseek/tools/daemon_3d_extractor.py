import time
import os
import subprocess
import glob
import sys
import torch
import cv2
import numpy as np

sys.stdout.reconfigure(line_buffering=True)

def create_obj(rgb_path, depth_path, obj_path):
    print(f"[{int(time.time())}] Compiling 3D Mesh: {obj_path}")
    img = cv2.imread(rgb_path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    depth = cv2.imread(depth_path, cv2.IMREAD_GRAYSCALE)
    target_size = 256
    img = cv2.resize(img, (target_size, target_size))
    depth = cv2.resize(depth, (target_size, target_size))
    h, w = img.shape[:2]
    vertices = []
    colors = []
    depth_scale = 0.5 
    for y in range(h):
        for x in range(w):
            z = depth[y, x] / 255.0 * depth_scale
            vertices.append(( (x/float(w)) - 0.5, 0.5 - (y/float(h)), z ))
            r, g, b = img[y, x]
            colors.append((r/255.0, g/255.0, b/255.0))
            
    faces = []
    for y in range(h - 1):
        for x in range(w - 1):
            idx1 = y * w + x + 1          
            idx2 = y * w + (x + 1) + 1
            idx3 = (y + 1) * w + x + 1
            idx4 = (y + 1) * w + (x + 1) + 1
            z1 = vertices[idx1-1][2]
            z2 = vertices[idx2-1][2]
            z3 = vertices[idx3-1][2]
            z4 = vertices[idx4-1][2]
            if abs(z1-z2) < 0.05 and abs(z1-z3) < 0.05 and abs(z1-z4) < 0.05:
                faces.append((idx1, idx2, idx3))
                faces.append((idx2, idx4, idx3))
                
    with open(obj_path, 'w') as f:
        f.write("# TSFi Procedural 3D Mesh\n")
        f.write("# Generated from RGB-D via Genie Orchestration\n")
        for i in range(len(vertices)):
            v = vertices[i]
            c = colors[i]
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f} {c[0]:.4f} {c[1]:.4f} {c[2]:.4f}\n")
        for face in faces:
            f.write(f"f {face[0]} {face[1]} {face[2]}\n")

# Pre-load MiDaS ONCE when the daemon starts, so we don't pay the 15-second 
# PyTorch initialization penalty on every single frame.
print("[INFO] Loading MiDaS Depth Model...")
midas_model_type = "DPT_Hybrid"
try:
    midas = torch.hub.load("intel-isl/MiDaS", midas_model_type)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    midas.to(device)
    midas.eval()
    midas_transforms = torch.hub.load("intel-isl/MiDaS", "transforms")
    transform = midas_transforms.dpt_transform
    print("[SUCCESS] MiDaS Neural Engine Online.")
except Exception as e:
    print(f"[ERROR] Failed to load MiDaS: {e}")
    sys.exit(1)

def run_3d_extractor():
    print("[INFO] 3D Extractor Daemon Online. Waiting for stable frames...")
    
    last_mtime = 0
    while True:
        target_frame = "assets/mpx_out.png"
        if os.path.exists(target_frame):
            current_mtime = os.path.getmtime(target_frame)
            if current_mtime > last_mtime + 5.0:  # Increased sample rate to every 5 seconds
                
                timestamp = int(time.time())
                depth_out = f"assets/depth_maps/depth_{timestamp}.png"
                rgb_out = f"assets/depth_maps/rgb_{timestamp}.png"
                
                os.makedirs("assets/depth_maps", exist_ok=True)
                
                import shutil
                shutil.copy(target_frame, rgb_out)
                
                # Inline generation to avoid subprocess overhead
                img = cv2.imread(target_frame)
                if img is not None:
                    print(f"[{timestamp}] Extracting Z-Depth Matrix...")
                    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
                    input_batch = transform(img_rgb).to(device)

                    with torch.no_grad():
                        prediction = midas(input_batch)
                        prediction = torch.nn.functional.interpolate(
                            prediction.unsqueeze(1),
                            size=img_rgb.shape[:2],
                            mode="bicubic",
                            align_corners=False,
                        ).squeeze()

                    output = prediction.cpu().numpy()
                    output_normalized = cv2.normalize(output, None, 0, 255, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_8U)
                    cv2.imwrite(depth_out, output_normalized)
                    
                    obj_out = f"assets/depth_maps/mesh_{timestamp}.obj"
                    create_obj(rgb_out, depth_out, obj_out)
                    
                    last_mtime = current_mtime
                
        time.sleep(1)

if __name__ == "__main__":
    run_3d_extractor()
