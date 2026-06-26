#!/usr/bin/env python3
import os
import sys
import math
import hashlib
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont

def generate_sd_texture(prompt, output_raw):
    os.makedirs("tmp", exist_ok=True)
    worker_path = "./bin/tsfi_sd_worker"
    
    cmd = [
        worker_path,
        prompt,
        output_raw,
        "0",      # no shm
        "turbo",
        "4",      # steps
        "euler_a",
        "1.5"     # cfg
    ]
    
    cwd = "tsfi2-deepseek" if os.path.exists("tsfi2-deepseek") else "."
    if not os.path.exists(os.path.join(cwd, worker_path)):
        if os.path.exists("./bin/tsfi_sd_worker"):
            cmd[0] = "./bin/tsfi_sd_worker"
            cwd = "."
            
    print(f"[SD Texture] Generating texture: \"{prompt}\"...")
    try:
        subprocess.run(cmd, cwd=cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    except Exception as e:
        print(f"[SD Texture Warning] Failed to run SD worker, creating procedurally generated texture instead: {e}")
        # Procedural fallback texture
        width, height = 512, 512
        raw_data = np.zeros((height, width, 3), dtype=np.uint8)
        for y in range(height):
            for x in range(width):
                val = int(127 + 127 * math.sin(x * 0.05) * math.cos(y * 0.05))
                raw_data[y, x] = [val, int(val * 0.8), int(val * 0.9)]
        os.makedirs(os.path.dirname(os.path.join(cwd, output_raw)), exist_ok=True)
        with open(os.path.join(cwd, output_raw), "wb") as f:
            f.write(raw_data.tobytes())

def project_3d(x, y, z, size):
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
        return px, py, z_new
        
    px_raw, py_raw, z_new = get_raw_proj(x, y, z)
    px_orig, py_orig, _ = get_raw_proj(0, 0, 0)
    
    px_final = px_raw - px_orig + size / 2
    py_final = py_raw - py_orig + size / 2
    return px_final, py_final, z_new

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 texture_lau_skeleton.py <LAU_ADDRESS>")
        sys.exit(1)
        
    address = sys.argv[1].lower().strip()
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    
    # Check local cache first for registers
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
        r_base = int(addr_hash[0:4], 16)
        r_channel = int(addr_hash[4:8], 16)
        r_dynamo = int(addr_hash[8:12], 16)
        r_foundation = int(addr_hash[12:16], 16)
        
    # Mapping coordinates
    fx = 1.0 + (r_channel % 5)
    fy = 1.0 + (r_dynamo % 5)
    fz = 1.0 + (r_foundation % 5)
    phi = (r_base % 100) / 100.0 * 2.0 * math.pi
    
    # EDO-22 hypotrochoid dimensions
    R_hyp = 100.0 + (r_foundation % 80)
    r_hyp = 20.0 + (r_channel % 40)
    d_hyp = 30.0 + (r_dynamo % 60)
    
    # ----------------------------------------------------
    # RENDER 1: YI Lissajous Skeleton (Texture A)
    # ----------------------------------------------------
    # Prompt illustrating: WinchesterMQ Signal, phase twists, neon light bloom filaments
    prompt_yi = (
        "WinchesterMQ signal carrier wave, crystalline obsidian and neon turquoise light bloom filaments, "
        "cosmic phase twist geometry, MotzkinPrime modular field scaling patterns, 8k resolution, realistic reflections"
    )
    raw_out_yi = "tmp/sd_texture_yi.raw"
    generate_sd_texture(prompt_yi, raw_out_yi)
    
    cwd = "tsfi2-deepseek" if os.path.exists("tsfi2-deepseek") else "."
    raw_path_yi = os.path.join(cwd, raw_out_yi)
    if os.path.exists(raw_path_yi):
        with open(raw_path_yi, "rb") as f:
            tex_yi_data = f.read()
        tex_yi = Image.frombytes("RGB", (512, 512), tex_yi_data)
    else:
        tex_yi = Image.new("RGB", (512, 512), (0, 242, 254))
    
    pixels_yi = tex_yi.load()
    size = 512
    img_yi_out = Image.new("RGB", (size, size), (12, 22, 45))
    draw_yi = ImageDraw.Draw(img_yi_out, "RGBA")
    
    # Load CJK font for 倫 Signature
    try:
        font_cjk = ImageFont.truetype("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", 180)
    except Exception:
        try:
            font_cjk = ImageFont.truetype("/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf", 180)
        except Exception:
            font_cjk = None

    if font_cjk:
        text = "倫"
        tx_pos = size // 2 - 90
        ty_pos = size // 2 - 110
        # Draw 3D embossed signature
        draw_yi.text((tx_pos + 3, ty_pos + 3), text, font=font_cjk, fill=(0, 0, 0, 160))
        draw_yi.text((tx_pos - 3, ty_pos - 3), text, font=font_cjk, fill=(255, 255, 255, 100))
        draw_yi.text((tx_pos, ty_pos), text, font=font_cjk, fill=(180, 140, 60, 210))
    
    num_points = 360
    proj_points_yi = []
    normals_yi = []
    
    for i in range(num_points):
        theta = i * 2.0 * math.pi / num_points
        lx = 135.0 * math.sin(fx * theta + phi)
        ly = 135.0 * math.sin(fy * theta)
        lz = 135.0 * math.cos(fz * theta)
        
        px, py, depth = project_3d(lx, ly, lz, size)
        proj_points_yi.append((px, py))
        
        theta_next = (i + 1) * 2.0 * math.pi / num_points
        lx_n = 135.0 * math.sin(fx * theta_next + phi)
        ly_n = 135.0 * math.sin(fy * theta_next)
        lz_n = 135.0 * math.cos(fz * theta_next)
        
        tx = lx_n - lx
        ty = ly_n - ly
        tz = lz_n - lz
        length = math.sqrt(tx**2 + ty**2 + tz**2) or 1.0
        normals_yi.append((-ty/length, tx/length))
        
    for i in range(num_points):
        p1 = proj_points_yi[i]
        p2 = proj_points_yi[(i + 1) % num_points]
        nx, ny = normals_yi[i]
        
        tx_coord = int(p1[0]) % 512
        ty_coord = int(p1[1]) % 512
        r_t, g_t, b_t = pixels_yi[tx_coord, ty_coord]
        
        dot = max(0.2, nx * 0.707 + ny * 0.707)
        r_s, g_s, b_s = int(r_t * dot), int(g_t * dot), int(b_t * dot)
        
        # Emulated 3D embossing/beveling highlights & shadows
        p1_sh = (p1[0] + 1.2 * nx, p1[1] + 1.2 * ny)
        p2_sh = (p2[0] + 1.2 * nx, p2[1] + 1.2 * ny)
        p1_hl = (p1[0] - 1.2 * nx, p1[1] - 1.2 * ny)
        p2_hl = (p2[0] - 1.2 * nx, p2[1] - 1.2 * ny)
        
        # Draw shadow side (black with alpha)
        draw_yi.line([p1_sh, p2_sh], fill=(0, 0, 0, 140), width=4)
        # Draw highlight side (white/light with alpha)
        draw_yi.line([p1_hl, p2_hl], fill=(255, 255, 255, 90), width=4)
        
        # Draw textured center
        draw_yi.line([p1, p2], fill=(r_s, g_s, b_s, 255), width=4)
        draw_yi.line([p1, p2], fill=(min(255, r_s + 40), min(255, g_s + 40), min(255, b_s + 40), 255), width=2)
        
    output_dir = "assets"
    os.makedirs(output_dir, exist_ok=True)
    yi_out_path = os.path.join(output_dir, f"{address}_textured.png")
    img_yi_out.save(yi_out_path)
    print(f"[Textured Output] Successfully rendered YI textured skeleton: {yi_out_path}")
    
    # ----------------------------------------------------
    # RENDER 2: LAU Hypotrochoid Skeleton (Texture B)
    # ----------------------------------------------------
    # Prompt illustrating: EDO-22 frequency octaves, foundation gates, WinchesterMQ SCSI loops
    prompt_lau = (
        "EDO-22 octave frequency circuit board, metallic gold and micro-etched brass plating, "
        "WinchesterMQ SCSI handshake loops, foundation gates, realistic reflections, 8k resolution, masterpiece"
    )
    raw_out_lau = "tmp/sd_texture_lau.raw"
    generate_sd_texture(prompt_lau, raw_out_lau)
    
    raw_path_lau = os.path.join(cwd, raw_out_lau)
    if os.path.exists(raw_path_lau):
        with open(raw_path_lau, "rb") as f:
            tex_lau_data = f.read()
        tex_lau = Image.frombytes("RGB", (512, 512), tex_lau_data)
    else:
        tex_lau = Image.new("RGB", (512, 512), (243, 36, 171))
        
    pixels_lau = tex_lau.load()
    img_lau_out = Image.new("RGB", (size, size), (12, 22, 45))
    draw_lau = ImageDraw.Draw(img_lau_out, "RGBA")
    
    if font_cjk:
        text = "倫"
        tx_pos = size // 2 - 90
        ty_pos = size // 2 - 110
        # Draw 3D embossed signature
        draw_lau.text((tx_pos + 3, ty_pos + 3), text, font=font_cjk, fill=(0, 0, 0, 160))
        draw_lau.text((tx_pos - 3, ty_pos - 3), text, font=font_cjk, fill=(255, 255, 255, 100))
        draw_lau.text((tx_pos, ty_pos), text, font=font_cjk, fill=(180, 140, 60, 210))
    
    # Layer multiple offsets of d to create concentric holographic rings
    d_offsets = [d_hyp * 0.7, d_hyp, d_hyp * 1.3]
    for idx, d_val in enumerate(d_offsets):
        proj_points_lau = []
        normals_lau = []
        
        for i in range(num_points):
            theta = i * 4.0 * math.pi / num_points
            lx = (R_hyp - r_hyp) * math.cos(theta) + d_val * math.cos(((R_hyp - r_hyp) / r_hyp) * theta + phi)
            ly = (R_hyp - r_hyp) * math.sin(theta) - d_val * math.sin(((R_hyp - r_hyp) / r_hyp) * theta)
            lz = 80.0 * math.cos(5.0 * theta)
            
            px, py, depth = project_3d(lx, ly, lz, size)
            proj_points_lau.append((px, py))
            
            theta_next = (i + 1) * 4.0 * math.pi / num_points
            lx_n = (R_hyp - r_hyp) * math.cos(theta_next) + d_val * math.cos(((R_hyp - r_hyp) / r_hyp) * theta_next + phi)
            ly_n = (R_hyp - r_hyp) * math.sin(theta_next) - d_val * math.sin(((R_hyp - r_hyp) / r_hyp) * theta_next)
            lz_n = 80.0 * math.cos(5.0 * theta_next)
            
            tx = lx_n - lx
            ty = ly_n - ly
            tz = lz_n - lz
            length = math.sqrt(tx**2 + ty**2 + tz**2) or 1.0
            normals_lau.append((-ty/length, tx/length))
            
        # Draw this offset layer with appropriate line widths
        w = 4 if idx == 1 else 2
        for i in range(num_points):
            p1 = proj_points_lau[i]
            p2 = proj_points_lau[(i + 1) % num_points]
            nx, ny = normals_lau[i]
            
            tx_coord = int(p1[0]) % 512
            ty_coord = int(p1[1]) % 512
            r_t, g_t, b_t = pixels_lau[tx_coord, ty_coord]
            
            dot = max(0.2, nx * 0.707 + ny * 0.707)
            r_s, g_s, b_s = int(r_t * dot), int(g_t * dot), int(b_t * dot)
            
            # Emulated 3D embossing highlights & shadows
            p1_sh = (p1[0] + 1.2 * nx, p1[1] + 1.2 * ny)
            p2_sh = (p2[0] + 1.2 * nx, p2[1] + 1.2 * ny)
            p1_hl = (p1[0] - 1.2 * nx, p1[1] - 1.2 * ny)
            p2_hl = (p2[0] - 1.2 * nx, p2[1] - 1.2 * ny)
            
            # Draw shadow
            draw_lau.line([p1_sh, p2_sh], fill=(0, 0, 0, 140), width=w)
            # Draw highlight
            draw_lau.line([p1_hl, p2_hl], fill=(255, 255, 255, 90), width=w)
            
            # Draw core textured line
            draw_lau.line([p1, p2], fill=(r_s, g_s, b_s, 255), width=w)
            draw_lau.line([p1, p2], fill=(min(255, r_s + 40), min(255, g_s + 40), min(255, b_s + 40), 255), width=w // 2)
        
    lau_out_path = os.path.join(output_dir, f"{address}_hypotrochoid_textured.png")
    img_lau_out.save(lau_out_path)
    print(f"[Textured Output] Successfully rendered LAU textured hypotrochoid: {lau_out_path}")

if __name__ == "__main__":
    main()
