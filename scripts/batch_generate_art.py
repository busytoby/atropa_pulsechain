import os
import glob
import json
import hashlib
import math
from PIL import Image, ImageDraw, ImageFilter

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def get_shade(color, factor):
    return tuple(min(255, int(c * factor)) for c in color)

def draw_isometric_cube(draw, cx, cy, x, y, z, size, base_color):
    # Isometric projection math (30 degrees: cos=0.866, sin=0.5)
    dx = size * 0.866
    dy = size * 0.5
    
    px = cx + (x - y) * dx
    py = cy + (x + y) * dy - z * size
    
    center = (px, py)
    top = (px, py - size)
    bottom = (px, py + size)
    left_top = (px - dx, py - dy)
    left_bottom = (px - dx, py + dy)
    right_top = (px + dx, py - dy)
    right_bottom = (px + dx, py + dy)
    
    top_color = get_shade(base_color, 1.2)
    left_color = get_shade(base_color, 0.9)
    right_color = get_shade(base_color, 0.7)
    
    draw.polygon([top, right_top, center, left_top], fill=top_color)
    draw.polygon([left_top, center, bottom, left_bottom], fill=left_color)
    draw.polygon([center, right_top, right_bottom, bottom], fill=right_color)
    
    outline_color = get_shade(base_color, 1.4)
    draw.line([top, right_top, bottom, left_top, top], fill=outline_color, width=1)
    draw.line([center, top], fill=outline_color, width=1)
    draw.line([center, bottom], fill=outline_color, width=1)
    draw.line([center, left_top], fill=outline_color, width=1)
    draw.line([center, right_top], fill=outline_color, width=1)

def generate_voxel_shape(desc):
    desc = desc.lower()
    voxels = []
    
    if "key" in desc:
        # Ring at top (z=2)
        for x in range(-2, 3):
            for y in range(-2, 3):
                if 2 <= x*x + y*y <= 5:
                    voxels.append((x, y, 2, 0))
        # Shaft
        for z in range(-3, 2):
            voxels.append((0, 0, z, 0))
        # Teeth
        voxels.append((1, 0, -2, 0))
        voxels.append((2, 0, -2, 1))
        voxels.append((1, 0, -1, 0))
        
    elif "sword" in desc or "blade" in desc or "dirk" in desc:
        # Blade
        for z in range(-1, 4):
            voxels.append((0, 0, z, 1))
        # Guard
        for x in range(-2, 3):
            voxels.append((x, 0, -2, 0))
        # Hilt
        voxels.append((0, 0, -3, 0))
        voxels.append((0, 0, -4, 0))
        
    elif "shield" in desc or "sentinel" in desc or "vault" in desc or "guard" in desc:
        for z in range(-2, 3):
            width = 2 if z >= 0 else 2 + z
            for x in range(-width, width + 1):
                voxels.append((x, 0, z, 0))
                if x == 0 and z == 0:
                    voxels.append((x, 0, z, 1))
                    
    elif "crown" in desc or "king" in desc or "princess" in desc or "queen" in desc:
        # Base Ring
        for x in range(-2, 3):
            for y in range(-2, 3):
                if 3 <= x*x + y*y <= 5:
                    voxels.append((x, y, -2, 0))
        # Crown peaks
        voxels.append((-2, 0, -1, 0))
        voxels.append((-2, 0, 0, 1))
        voxels.append((2, 0, -1, 0))
        voxels.append((2, 0, 0, 1))
        voxels.append((0, 2, -1, 0))
        voxels.append((0, 2, 0, 1))
        voxels.append((0, -2, -1, 0))
        voxels.append((0, -2, 0, 1))
        voxels.append((0, 0, 1, 1))
        
    elif "gas" in desc or "fuel" in desc or "fire" in desc or "burn" in desc or "flame" in desc:
        for z in range(-3, 4):
            max_r = 3 - (z + 3)//2
            for x in range(-max_r, max_r + 1):
                for y in range(-max_r, max_r + 1):
                    if x*x + y*y <= max_r*max_r:
                        is_inner = (x*x + y*y <= 1) and z < 2
                        voxels.append((x, y, z, 1 if is_inner else 0))
                        
    elif "heart" in desc or "love" in desc:
        for x in (-1, 1):
            for y in (-1, 1):
                voxels.append((x, y, 2, 0))
        for x in range(-2, 3):
            for y in range(-2, 3):
                if abs(x) + abs(y) <= 3:
                    voxels.append((x, y, 1, 0))
                if abs(x) + abs(y) <= 2:
                    voxels.append((x, y, 0, 1))
                if abs(x) + abs(y) <= 1:
                    voxels.append((x, y, -1, 0))
        voxels.append((0, 0, -2, 0))
        
    elif "drop" in desc or "water" in desc or "liquid" in desc or "pool" in desc:
        for z in range(-3, 4):
            r = 3 - (z + 3)//2
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 1 if (z == -2 and x == 0 and y == 0) else 0))
                        
    elif "star" in desc or "electric" in desc or "lightning" in desc or "energy" in desc or "spark" in desc:
        for x in range(-1, 2):
            for y in range(-1, 2):
                for z in range(-1, 2):
                    if abs(x)+abs(y)+abs(z) <= 1:
                        voxels.append((x, y, z, 1))
        for d in range(-3, 4):
            if abs(d) > 1:
                voxels.append((d, 0, 0, 0))
                voxels.append((0, d, 0, 0))
                voxels.append((0, 0, d, 0))
                
    elif "usd" in desc or "dai" in desc or "usdc" in desc or "usdt" in desc or "stable" in desc or "dollar" in desc:
        s_points = [
            (1, 0, 2), (0, 0, 2), (-1, 0, 2),
            (-1, 0, 1),
            (-1, 0, 0), (0, 0, 0), (1, 0, 0),
            (1, 0, -1),
            (1, 0, -2), (0, 0, -2), (-1, 0, -2)
        ]
        for x, y, z in s_points:
            voxels.append((x, y, z, 0))
        for z in range(-3, 4):
            voxels.append((0, 0, z, 1))
            
    else:  # Fallback to standard Coin/Token disc
        for z in range(-1, 2):
            for x in range(-3, 4):
                for y in range(-3, 4):
                    d = x*x + y*y
                    if d <= 9:
                        is_rim = d >= 7 or z == 1
                        is_core = d == 0
                        if is_rim:
                            voxels.append((x, y, z, 0))
                        elif is_core:
                            voxels.append((x, y, z, 1))
                            
    voxels.sort(key=lambda v: (v[0] + v[1] + v[2]))
    return voxels

def render_card_art(card, output_path):
    width, height = 1280, 720
    bg_img = Image.new("RGB", (width, height), (8, 5, 15))
    draw = ImageDraw.Draw(bg_img)
    
    color_hex = card.get('color', '#00f0ff')
    color_rgb = hex_to_rgb(color_hex)
    accent_rgb = (255, 255, 255)
    
    glow_mask = Image.new("L", (width, height), 0)
    glow_draw = ImageDraw.Draw(glow_mask)
    glow_draw.ellipse([640 - 300, 360 - 300, 640 + 300, 360 + 300], fill=60)
    glow_color_img = Image.new("RGB", (width, height), color_rgb)
    bg_img = Image.composite(glow_color_img, bg_img, glow_mask)
    
    draw = ImageDraw.Draw(bg_img)
    grid_color = get_shade(color_rgb, 0.15)
    for i in range(0, width, 40):
        draw.line([i, 0, i, height], fill=grid_color, width=1)
    for j in range(0, height, 40):
        draw.line([0, j, width, j], fill=grid_color, width=1)
        
    cx, cy = 640, 360
    hud_color = get_shade(color_rgb, 0.4)
    draw.ellipse([cx - 280, cy - 280, cx + 280, cy + 280], outline=hud_color, width=2)
    draw.ellipse([cx - 240, cy - 240, cx + 240, cy + 240], outline=get_shade(color_rgb, 0.6), width=1)
    draw.line([cx - 300, cy, cx - 260, cy], fill=hud_color, width=3)
    draw.line([cx + 260, cy, cx + 300, cy], fill=hud_color, width=3)
    draw.line([cx, cy - 300, cx, cy - 260], fill=hud_color, width=3)
    draw.line([cx, cy + 260, cx, cy + 300], fill=hud_color, width=3)
    
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        x1 = cx + 230 * math.cos(rad)
        y1 = cy + 230 * math.sin(rad)
        x2 = cx + 250 * math.cos(rad)
        y2 = cy + 250 * math.sin(rad)
        draw.line([x1, y1, x2, y2], fill=hud_color, width=2)

    desc = card.get('desc', '')
    voxels = generate_voxel_shape(desc)
    voxel_size = 22
    
    for vx, vy, vz, color_type in voxels:
        v_color = accent_rgb if color_type == 1 else color_rgb
        draw_isometric_cube(draw, cx, cy, vx, vy, vz, voxel_size, v_color)
        
    vignette = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    v_draw = ImageDraw.Draw(vignette)
    v_draw.rectangle([0, 0, width, height], outline=(0, 0, 0, 255), width=24)
    vignette = vignette.filter(ImageFilter.GaussianBlur(15))
    bg_img.paste(vignette, (0, 0), vignette)
    
    bg_img.save(output_path)

def main():
    data_dir = 'solidity/dysnomia/domain/data'
    assets_dir = 'assets'
    os.makedirs(assets_dir, exist_ok=True)
    
    card_files = glob.glob(os.path.join(data_dir, '0x*.json'))
    print(f"Found {len(card_files)} individual token card JSON files to render.")
    
    rendered_count = 0
    for file_path in card_files:
        try:
            with open(file_path, 'r') as f:
                card = json.load(f)
            addr = card.get('address', '').lower()
            if not addr:
                continue
            output_path = os.path.join(assets_dir, f"{addr}.png")
            render_card_art(card, output_path)
            rendered_count += 1
        except Exception as e:
            print(f"Error rendering {file_path}: {e}")
            
    print(f"Batch generation complete. Successfully rendered {rendered_count} card art PNGs in {assets_dir}/.")

if __name__ == '__main__':
    main()
