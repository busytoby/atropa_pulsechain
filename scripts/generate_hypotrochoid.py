#!/usr/bin/env python3
import os
import sys
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

def project_3d(x, y, z, size):
    def get_raw_proj(px_val, py_val, pz_val):
        # Align exactly with the evolution demo final-frame camera settings
        # TOTAL_FRAMES = 2700. f_effective = TOTAL_FRAMES - 270 = 2430.
        # cam_yaw = 2430 * 0.015 = 36.45
        # cam_pitch = 0.3 + 0.05 * sin(2430 * 0.008) = 0.3 + 0.05 * sin(19.44) = 0.3155
        # cam_x = cos(36.45) * 360 = 276.5
        # cam_y = sin(36.45) * 360 = 230.7
        # cam_z = 240 + 40 * cos(2430 * 0.005) = 240 + 40 * cos(12.15) = 278.4
        yaw = 36.45
        pitch = 0.3155
        cam_x = 276.5
        cam_y = 230.7
        cam_z = 278.4
        zoom = 0.82
        
        dx = px_val - cam_x
        dy = py_val - cam_y
        dz = pz_val - cam_z
        
        cos_y, sin_y = math.cos(yaw), math.sin(yaw)
        rx = dx * cos_y - dz * sin_y
        ry = dy
        rz = dx * sin_y + dz * cos_y
        
        cos_p, sin_p = math.cos(pitch), math.sin(pitch)
        x_new = rx
        y_new = ry * cos_p - rz * sin_p
        z_new = ry * sin_p + rz * cos_p
        
        focal = 500.0
        if z_new == 0: z_new = 1
        px = (x_new * focal) / (z_new + 700) * zoom
        py = (y_new * focal) / (z_new + 700) * zoom
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
        print("Usage: python3 generate_hypotrochoid.py <LAU_ADDRESS>")
        sys.exit(1)
        
    address = sys.argv[1].lower().strip()
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    
    # Base HSL color mapped from the hash
    hue = (int(addr_hash[10:13], 16) % 360)
    color_rgb = hsl_to_rgb(hue, 0.90, 0.55)
    
    # Load cache registers if available, else fallback to hash mapping
    r_base, r_channel, r_dynamo, r_foundation = 0, 0, 0, 0
    cache_path = os.path.join(os.path.dirname(__file__), "pulsechain_register_cache.json")
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
        except Exception:
            pass
            
    if r_base == 0:
        # Generate pseudorandom VM mappings from hash
        r_base = int(addr_hash[0:4], 16)
        r_channel = int(addr_hash[4:8], 16)
        r_dynamo = int(addr_hash[8:12], 16)
        r_foundation = int(addr_hash[12:16], 16)
        
    # Scale VM register parameters to valid hypotrochoid sizes
    R = 100.0 + (r_foundation % 80)
    r = 20.0 + (r_channel % 40)
    d = 30.0 + (r_dynamo % 60)
    phi = (r_base % 100) / 100.0 * 2.0 * math.pi
    
    size = 512
    # Use pure black background for ControlNet depth map accuracy
    img = Image.new("RGB", (size, size), (0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    
    # Render coordinate path directly as pure white line features
    color_rgb = (255, 255, 255)
    
    num_points = 500
    # Layer multiple offsets of d to create concentric holographic rings
    d_offsets = [d * 0.7, d, d * 1.3]
    for idx, d_val in enumerate(d_offsets):
        proj_points = []
        for i in range(num_points):
            theta = i * 4.0 * math.pi / num_points # 2 full turns
            lx = (R - r) * math.cos(theta) + d_val * math.cos(((R - r) / r) * theta + phi)
            ly = (R - r) * math.sin(theta) - d_val * math.sin(((R - r) / r) * theta)
            lz = 80.0 * math.cos(5.0 * theta)  # depth component modulated by dynamic phase
            
            px, py = project_3d(lx, ly, lz, size)
            proj_points.append((px, py))
            
        # Draw segments with wider width for the central layer
        w = 2 if idx == 1 else 1
        for i in range(num_points):
            p1 = proj_points[i]
            p2 = proj_points[(i + 1) % num_points]
            draw_glow_line(draw, p1, p2, color_rgb, width=w)
        
    output_dir = "assets"
    os.makedirs(output_dir, exist_ok=True)
    
    png_path = os.path.join(output_dir, f"{address}_hypotrochoid.png")
    img.save(png_path)
    
    # Save as .jpai JPEG AI container
    jpai_path = os.path.join(output_dir, f"{address}_hypotrochoid.jpai")
    rgb_arr = np.array(img).astype(np.float32) / 255.0
    rgba_arr = np.ones((size, size, 4), dtype=np.float16)
    rgba_arr[:, :, :3] = rgb_arr.astype(np.float16)
    
    with open(jpai_path, "wb") as f:
        f.write(b"JPAI")
        f.write(struct.pack("<I", size))
        f.write(struct.pack("<I", size))
        f.write(struct.pack("<I", 4))
        f.write(b"ab4h")
        f.write(rgba_arr.tobytes())
        
    print(f"[Hypotrochoid Organism] Success!")
    print(f"  -> PNG saved: {png_path}")
    print(f"  -> JPAI saved: {jpai_path}")

if __name__ == "__main__":
    main()
