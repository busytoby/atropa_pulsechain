#!/usr/bin/env python3
# Generate a clean, idealized 3D Lissajous icon given any PulseChain token/LAU address.
# Outputs a 512x512 PNG icon and its SHA-256 reproducibility checksum.

import sys
import os
import math
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
    # Standard perspective matrix
    yaw = 0.5
    pitch = 0.4
    cam_x = math.cos(yaw) * 260
    cam_y = math.sin(yaw) * 260
    cam_z = 150
    zoom = 0.90
    
    dx = x - cam_x
    dy = y - cam_y
    dz = z - cam_z
    
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
    px = (x_new * focal) / (z_new + 500) * zoom + size / 2
    py = (y_new * focal) / (z_new + 500) * zoom + size / 2
    return int(px), int(py)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 generate_lau_icon.py <LAU_ADDRESS>")
        print("Example: python3 generate_lau_icon.py 0xfede210090acc84f3db9c8c7bc8ffbed544b07e4")
        sys.exit(1)
        
    address = sys.argv[1].lower().strip()
    if not address.startswith("0x") or len(address) != 42:
        print("Error: Invalid Ethereum/PulseChain address format.")
        sys.exit(1)
        
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    
    # 1. Deterministically derive vibrant HSL color from address hash
    hue = (int(addr_hash[10:13], 16) % 360)
    color_rgb = hsl_to_rgb(hue, 0.90, 0.55)
    
    # 2. Derive Lissajous parameters from address hash
    fx = 1.0 + (int(addr_hash[0:2], 16) % 5)
    fy = 1.0 + (int(addr_hash[2:4], 16) % 5)
    fz = 1.0 + (int(addr_hash[4:6], 16) % 5)
    phi = (int(addr_hash[6:8], 16) % 100) / 100.0 * 2.0 * math.pi
    multiplier = 1.0 + (int(addr_hash[8:10], 16) % 5)
    
    # 3. Create canvas (512x512)
    size = 512
    img = Image.new("RGBA", (size, size), (6, 9, 18, 255))
    draw = ImageDraw.Draw(img, "RGBA")
    
    # Optional grid rings in background
    for r_val in [80, 160, 240]:
        draw.ellipse([size/2 - r_val, size/2 - r_val, size/2 + r_val, size/2 + r_val], outline=(0, 242, 254, 15), width=1)
        
    # Render two interlaced Lissajous curves
    num_points = 180
    for is_cone in [False, True]:
        fx_c = fx + (0.5 if is_cone else 0.0)
        fy_c = fy + (0.5 if is_cone else 0.0)
        fz_c = fz + (0.5 if is_cone else 0.0)
        phi_c = phi + (math.pi if is_cone else 0.0)
        
        proj_points = []
        for i in range(num_points):
            theta = i * 2.0 * math.pi / num_points
            lx = 135.0 * math.sin(fx_c * theta + phi_c)
            ly = 135.0 * math.sin(fy_c * theta)
            lz = 135.0 * math.cos(fz_c * theta)
            
            px, py = project_lissajous(lx, ly, lz, size)
            proj_points.append((px, py))
            
        # Draw connections
        for i in range(num_points):
            p1 = proj_points[i]
            p2 = proj_points[(i + 1) % num_points]
            draw_glow_line(draw, p1, p2, color_rgb, width=1)
            
        # Draw modulo chords
        for i in range(num_points):
            target_idx = int((i * multiplier) % num_points)
            p1 = proj_points[i]
            p2 = proj_points[target_idx]
            # Draw chord with lower opacity
            r, g, b = color_rgb
            draw.line([p1, p2], fill=(r, g, b, 30), width=1)
            
    # Save output
    output_dir = "assets"
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"{address}_icon.png")
    img.save(output_path)
    
    # Compute verification hash
    with open(output_path, "rb") as f:
        sha_hash = hashlib.sha256(f.read()).hexdigest()
        
    print(f"LAU:          {address}")
    print(f"Color (RGB):  {color_rgb}")
    print(f"Icon Path:    {output_path}")
    print(f"SHA-256 Hash: {sha_hash}")

if __name__ == "__main__":
    main()
