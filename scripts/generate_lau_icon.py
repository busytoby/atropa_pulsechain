#!/usr/bin/env python3
# Generate a complete, idealized 3D Lissajous icon given any PulseChain token/LAU address.
# Supports passing direct live contract registers for exact on-chain mapping.
# Outputs a 512x512 PNG icon, an idealized JPEG AI container (.jpai) with 4:4:4 ab4h color, and hashes.

import sys
import os
import math
import struct
import hashlib
import numpy as np
from PIL import Image, ImageDraw

def hsl_to_rgb(h, s, l):
    c = (1.0 - abs(2.0 * l - 1.0)) * s
    x = c * (1.0 - abs((h / 60.0) % 2.0 - 1.0))
    m = l - c / 2.0
    
    if 0 <= h < 60:
        r, g, b = c, x, 0.0
    elif 60 <= h < 120:
        r, g, b = x, c, 0.0
    elif 120 <= h < 180:
        r, g, b = 0.0, c, x
    elif 180 <= h < 240:
        r, g, b = 0.0, x, c
    elif 240 <= h < 300:
        r, g, b = x, 0.0, c
    else:
        r, g, b = c, 0.0, x
        
    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

def draw_glow_line(draw_obj, p1, p2, color, width=1):
    r, g, b = color
    draw_obj.line([p1, p2], fill=(r, g, b, 25), width=width+4)
    draw_obj.line([p1, p2], fill=(r, g, b, 70), width=width+2)
    draw_obj.line([p1, p2], fill=(r, g, b, 255), width=width)

def project_lissajous(x, y, z, size):
    def get_raw_proj(px_val, py_val, pz_val):
        yaw = 0.5
        pitch = 0.4
        cam_x = math.cos(yaw) * 260
        cam_y = math.sin(yaw) * 260
        cam_z = 150
        zoom = 0.90
        
        dx = px_val - cam_x
        dy = py_val - cam_y
        dz = pz_val - cam_z
        
        cos_y, sin_y = math.cos(yaw), math.sin(yaw)
        rx = dx * cos_y - dy * sin_y
        ry = dx * sin_y + dy * cos_y
        rz = dz
        
        cos_p, sin_p = math.cos(pitch), math.sin(pitch)
        x_new = rx * cos_p + rz * sin_p
        y_new = ry
        z_new = -rx * sin_p + rz * cos_p
        
        focal = 350.0
        if z_new == 0: z_new = 1
        px = (x_new * focal) / (z_new + 500) * zoom
        py = (y_new * focal) / (z_new + 500) * zoom
        return px, py
        
    px_raw, py_raw = get_raw_proj(x, y, z)
    px_orig, py_orig = get_raw_proj(0, 0, 0)
    
    px_final = px_raw - px_orig + size / 2
    py_final = py_raw - py_orig + size / 2
    return int(px_final), int(py_final)

def create_radial_gradient(size):
    bg = Image.new("RGB", (size, size))
    pixels = bg.load()
    cx, cy = size // 2, size // 2
    max_d = math.sqrt(cx**2 + cy**2)
    for y in range(size):
        for x in range(size):
            dx = x - cx
            dy = y - cy
            d = math.sqrt(dx**2 + dy**2)
            f = d / max_d
            r = int(12 * (1.0 - f) + 4 * f)
            g = int(22 * (1.0 - f) + 6 * f)
            b = int(45 * (1.0 - f) + 12 * f)
            pixels[x, y] = (r, g, b)
    return bg

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 generate_lau_icon.py <LAU_ADDRESS> [r_base r_channel r_dynamo r_foundation c_base c_channel c_dynamo c_foundation]")
        sys.exit(1)
        
    address = sys.argv[1].lower().strip()
    if not address.startswith("0x") or len(address) != 42:
        print("Error: Invalid Ethereum/PulseChain address format.")
        sys.exit(1)
        
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    
    hue = (int(addr_hash[10:13], 16) % 360)
    color_rgb = hsl_to_rgb(hue, 0.90, 0.55)
    
    if len(sys.argv) >= 10:
        r_base = int(sys.argv[2])
        r_channel = int(sys.argv[3])
        r_dynamo = int(sys.argv[4])
        r_foundation = int(sys.argv[5])
        c_base = int(sys.argv[6])
        c_channel = int(sys.argv[7])
        c_dynamo = int(sys.argv[8])
        c_foundation = int(sys.argv[9])
        
        fx = 1.0 + (r_channel % 5)
        fy = 1.0 + (r_dynamo % 5)
        fz = 1.0 + (r_foundation % 5)
        phi = (r_base % 100) / 100.0 * 2.0 * math.pi
        multiplier = 1.0 + (r_channel % 5)
        
        cfx = 1.0 + (c_channel % 5)
        cfy = 1.0 + (c_dynamo % 5)
        cfz = 1.0 + (c_foundation % 5)
        cphi = (c_base % 100) / 100.0 * 2.0 * math.pi
        cmultiplier = 1.0 + (c_channel % 5)
        print("[Live Mode] Rendering icon using exact on-chain register values.")
    else:
        # Check local cache first for offline mode
        cache_path = os.path.join(os.path.dirname(__file__), "pulsechain_register_cache.json")
        loaded_from_cache = False
        if os.path.exists(cache_path):
            try:
                import json
                with open(cache_path, "r") as f:
                    cache = json.load(f)
                if address in cache:
                    entry = cache[address]
                    r_base = int(entry["r_base"])
                    r_channel = int(entry["r_channel"])
                    r_dynamo = int(entry["r_dynamo"])
                    r_foundation = int(entry["r_foundation"])
                    c_base = int(entry["c_base"])
                    c_channel = int(entry["c_channel"])
                    c_dynamo = int(entry["c_dynamo"])
                    c_foundation = int(entry["c_foundation"])
                    
                    fx = 1.0 + (r_channel % 5)
                    fy = 1.0 + (r_dynamo % 5)
                    fz = 1.0 + (r_foundation % 5)
                    phi = (r_base % 100) / 100.0 * 2.0 * math.pi
                    multiplier = 1.0 + (r_channel % 5)
                    
                    cfx = 1.0 + (c_channel % 5)
                    cfy = 1.0 + (c_dynamo % 5)
                    cfz = 1.0 + (c_foundation % 5)
                    cphi = (c_base % 100) / 100.0 * 2.0 * math.pi
                    cmultiplier = 1.0 + (c_channel % 5)
                    print("[Offline Cache Mode] Rendering icon using cached on-chain register values.")
                    loaded_from_cache = True
            except Exception as e:
                print(f"[Offline Cache Warning] Failed to load from cache: {e}")
                
        if not loaded_from_cache:
            fx = 1.0 + (int(addr_hash[0:2], 16) % 5)
            fy = 1.0 + (int(addr_hash[2:4], 16) % 5)
            fz = 1.0 + (int(addr_hash[4:6], 16) % 5)
            phi = (int(addr_hash[6:8], 16) % 100) / 100.0 * 2.0 * math.pi
            multiplier = 1.0 + (int(addr_hash[8:10], 16) % 5)
            
            cfx = fx + 0.5
            cfy = fy + 0.5
            cfz = fz + 0.5
            cphi = phi + math.pi
            cmultiplier = multiplier
            print("[Fallback Mode] Rendering icon using address hash values.")
    
    size = 512
    img = create_radial_gradient(size)
    draw = ImageDraw.Draw(img, "RGBA")
    
    # Blueprint grid
    cx_grid, cy_grid = size // 2, size // 2
    max_dist = math.sqrt(cx_grid**2 + cy_grid**2)
    for x in range(0, size, 20):
        dist = abs(x - cx_grid)
        alpha = int(max(2, 14 * (1.0 - dist / max_dist)))
        draw.line([(x, 0), (x, size)], fill=(0, 242, 254, alpha), width=1)
    for y in range(0, size, 20):
        dist = abs(y - cy_grid)
        alpha = int(max(2, 14 * (1.0 - dist / max_dist)))
        draw.line([(0, y), (size, y)], fill=(0, 242, 254, alpha), width=1)
        
    for r_val in [80, 160, 240]:
        draw.ellipse([size/2 - r_val, size/2 - r_val, size/2 + r_val, size/2 + r_val], outline=(0, 242, 254, 15), width=1)
        
    p1_x, p1_y = project_lissajous(0, 0, -120, size)
    p2_x, p2_y = project_lissajous(0, 0, 120, size)
    draw_glow_line(draw, (p1_x, p1_y), (p2_x, p2_y), (34, 197, 94), width=2)
        
    num_points = 180
    curves = [
        (fx, fy, fz, phi, multiplier),
        (cfx, cfy, cfz, cphi, cmultiplier)
    ]
    
    for f_x, f_y, f_z, ph, mult in curves:
        proj_points = []
        for i in range(num_points):
            theta = i * 2.0 * math.pi / num_points
            lx = 135.0 * math.sin(f_x * theta + ph)
            ly = 135.0 * math.sin(f_y * theta)
            lz = 135.0 * math.cos(f_z * theta)
            
            px, py = project_lissajous(lx, ly, lz, size)
            proj_points.append((px, py))
            
        for i in range(num_points):
            p1 = proj_points[i]
            p2 = proj_points[(i + 1) % num_points]
            draw_glow_line(draw, p1, p2, color_rgb, width=1)
            
        for i in range(num_points):
            target_idx = int((i * mult) % num_points)
            p1 = proj_points[i]
            p2 = proj_points[target_idx]
            r, g, b = color_rgb
            draw.line([p1, p2], fill=(r, g, b, 30), width=1)
            
    output_dir = "assets"
    os.makedirs(output_dir, exist_ok=True)
    
    # 8. Save PNG Fallback
    output_path = os.path.join(output_dir, f"{address}_icon.png")
    img.save(output_path)
    
    # 9. Save Idealized JPEG AI standard 4:4:4 ab4h Container File (.jpai)
    jpai_path = os.path.join(output_dir, f"{address}_icon.jpai")
    
    rgb_arr = np.array(img).astype(np.float32) / 255.0
    rgba_arr = np.ones((size, size, 4), dtype=np.float16)
    rgba_arr[:, :, :3] = rgb_arr.astype(np.float16)
    
    with open(jpai_path, "wb") as f:
        f.write(b"JPAI")                       # FourCC: JPAI (JPEG AI Container)
        f.write(struct.pack("<I", size))       # Width
        f.write(struct.pack("<I", size))       # Height
        f.write(struct.pack("<I", 4))          # Channels (4:4:4:4)
        f.write(b"ab4h")                       # Format FourCC: ab4h
        f.write(rgba_arr.tobytes())            # Raw half-float payload
    
    with open(jpai_path, "rb") as f:
        sha_hash = hashlib.sha256(f.read()).hexdigest()
        
    print(f"LAU:          {address}")
    print(f"Color (RGB):  {color_rgb}")
    print(f"Format:       JPEG AI Container (.jpai) with 4:4:4 ab4h colorspace")
    print(f"Icon Path:    {jpai_path}")
    print(f"SHA-256 Hash: {sha_hash}")

if __name__ == "__main__":
    main()
