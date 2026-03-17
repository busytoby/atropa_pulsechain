import os
from PIL import Image
import numpy as np

def generate_packed_manifold(src_base, dst_path):
    # Standard resolution for optimized shading
    RES = 256
    manifold = np.zeros((RES, RES, 4), dtype=np.uint8)

    # 1. Load Normal Map (Channel R, G)
    norm_path = os.path.join(src_base, "aot_normal_map.png")
    if os.path.exists(norm_path):
        img = Image.open(norm_path).convert('RGB').resize((RES, RES))
        data = np.array(img)
        manifold[..., 0] = data[..., 0] # Normal X
        manifold[..., 1] = data[..., 1] # Normal Y
        print(f"[PASS] Normal Map packed (Ch R,G)")

    # 2. Load AO Map (Channel B)
    ao_path = os.path.join(src_base, "aot_ambient_occlusion.png")
    if os.path.exists(ao_path):
        img = Image.open(ao_path).convert('L').resize((RES, RES))
        manifold[..., 2] = np.array(img)
        print(f"[PASS] AO Map packed (Ch B)")

    # 3. Load Rim Map (Channel A)
    rim_path = os.path.join(src_base, "aot_rim_light.png")
    if os.path.exists(rim_path):
        img = Image.open(rim_path).convert('L').resize((RES, RES))
        manifold[..., 3] = np.array(img)
        print(f"[PASS] Rim Map packed (Ch A)")

    with open(dst_path, 'wb') as f:
        f.write(manifold.tobytes())
    print(f"=== [OPTIMIZED MANIFOLD CREATED] === size: {os.path.getsize(dst_path)} bytes")

if __name__ == '__main__':
    src_base = "../../assets/"
    dst_path = "assets/shading_manifold.bin"
    generate_packed_manifold(src_base, dst_path)
