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
    # Isometric projection math
    # Angle is 30 degrees (cos=0.866, sin=0.5)
    dx = size * 0.866
    dy = size * 0.5
    
    # Project cube center relative to grid coordinates
    px = cx + (x - y) * dx
    py = cy + (x + y) * dy - z * size
    
    # Vertices
    center = (px, py)
    top = (px, py - size)
    bottom = (px, py + size)
    left_top = (px - dx, py - dy)
    left_bottom = (px - dx, py + dy)
    right_top = (px + dx, py - dy)
    right_bottom = (px + dx, py + dy)
    
    # Face colors
    top_color = get_shade(base_color, 1.2)
    left_color = get_shade(base_color, 0.9)
    right_color = get_shade(base_color, 0.7)
    
    # Render faces
    draw.polygon([top, right_top, center, left_top], fill=top_color)
    draw.polygon([left_top, center, bottom, left_bottom], fill=left_color)
    draw.polygon([center, right_top, right_bottom, bottom], fill=right_color)
    
    # Render outlines for clean definition
    outline_color = get_shade(base_color, 1.4)
    draw.line([top, right_top, bottom, left_top, top], fill=outline_color, width=1)
    draw.line([center, top], fill=outline_color, width=1)
    draw.line([center, bottom], fill=outline_color, width=1)
    draw.line([center, left_top], fill=outline_color, width=1)
    draw.line([center, right_top], fill=outline_color, width=1)

def generate_voxel_shape(seed_str, shape_type):
    # Deterministic generation based on md5 hash
    h = hashlib.md5(seed_str.encode('utf-8')).hexdigest()
    voxels = []
    
    # Center of grid is at (0,0,0). We build within [-3, 3] range
    if shape_type == 0:  # Diamond / Crystal
        for z in range(-3, 4):
            r = 3 - abs(z)
            for x in range(-r, r + 1):
                for y in range(-r, r + 1):
                    if abs(x) + abs(y) <= r:
                        voxels.append((x, y, z, 0)) # 0 for base color, 1 for accent
                        
    elif shape_type == 1:  # Token / Disc
        for z in range(-1, 2):
            for x in range(-3, 4):
                for y in range(-3, 4):
                    d = x*x + y*y
                    if d <= 9:
                        # Draw hollow center or coin rim
                        is_rim = d >= 7 or z == 1
                        is_core = d == 0
                        if is_rim:
                            voxels.append((x, y, z, 0))
                        elif is_core:
                            voxels.append((x, y, z, 1)) # White center
                            
    elif shape_type == 2:  # Atomic Core
        # Central sphere
        for x in range(-1, 2):
            for y in range(-1, 2):
                for z in range(-1, 2):
                    if abs(x)+abs(y)+abs(z) <= 2:
                        voxels.append((x, y, z, 1))
        # Orbiting rings
        for i in range(-3, 4):
            if i != 0:
                voxels.append((i, 0, 0, 0))
                voxels.append((0, i, 0, 0))
                voxels.append((0, 0, i, 0))
                
    elif shape_type == 3:  # Shield / Monolith
        for x in range(-2, 3):
            for y in range(-1, 2):
                for z in range(-3, 4):
                    # Taper the top
                    if abs(x) <= 3 - abs(z)//2:
                        is_core = (x == 0 and z == 0)
                        voxels.append((x, y, z, 1 if is_core else 0))
                        
    else:  # Open Ring / Gear
        for z in range(-1, 2):
            for x in range(-3, 4):
                for y in range(-3, 4):
                    d = x*x + y*y
                    if 4 <= d <= 10:
                        voxels.append((x, y, z, 0))
                    elif d == 0:
                        voxels.append((x, y, z, 1))
                        
    # Depth sort voxels by x + y + z ascending for correct isometric overlap drawing
    voxels.sort(key=lambda v: (v[0] + v[1] + v[2]))
    return voxels

def render_card_art(card, output_path):
    # Image size
    width, height = 1280, 720
    
    # 1. Base Dark Cyberpunk Ambient Background
    bg_img = Image.new("RGB", (width, height), (8, 5, 15))
    draw = ImageDraw.Draw(bg_img)
    
    color_hex = card.get('color', '#00f0ff')
    color_rgb = hex_to_rgb(color_hex)
    accent_rgb = (255, 255, 255) # White core
    
    # Draw soft ambient glow behind the model
    glow_mask = Image.new("L", (width, height), 0)
    glow_draw = ImageDraw.Draw(glow_mask)
    glow_draw.ellipse([640 - 300, 360 - 300, 640 + 300, 360 + 300], fill=60)
    glow_color_img = Image.new("RGB", (width, height), color_rgb)
    bg_img = Image.composite(glow_color_img, bg_img, glow_mask)
    
    # Re-fetch Draw object after compositing
    draw = ImageDraw.Draw(bg_img)
    
    # 2. Draw Holographic background grid lines
    grid_color = get_shade(color_rgb, 0.15)
    for i in range(0, width, 40):
        draw.line([i, 0, i, height], fill=grid_color, width=1)
    for j in range(0, height, 40):
        draw.line([0, j, width, j], fill=grid_color, width=1)
        
    # Draw futuristic HUD rings
    cx, cy = 640, 360
    hud_color = get_shade(color_rgb, 0.4)
    # Outer ring
    draw.ellipse([cx - 280, cy - 280, cx + 280, cy + 280], outline=hud_color, width=2)
    # Inner dashed-style ring
    draw.ellipse([cx - 240, cy - 240, cx + 240, cy + 240], outline=get_shade(color_rgb, 0.6), width=1)
    # Crosshairs / Target markers
    draw.line([cx - 300, cy, cx - 260, cy], fill=hud_color, width=3)
    draw.line([cx + 260, cy, cx + 300, cy], fill=hud_color, width=3)
    draw.line([cx, cy - 300, cx, cy - 260], fill=hud_color, width=3)
    draw.line([cx, cy + 260, cx, cy + 300], fill=hud_color, width=3)
    
    # Angle ticks
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        x1 = cx + 230 * math.cos(rad)
        y1 = cy + 230 * math.sin(rad)
        x2 = cx + 250 * math.cos(rad)
        y2 = cy + 250 * math.sin(rad)
        draw.line([x1, y1, x2, y2], fill=hud_color, width=2)

    # 3. Generate and Render the 3D Voxel Model
    addr = card.get('address', '')
    # Determine shape type deterministically using address bytes
    addr_hash = hashlib.md5(addr.encode('utf-8')).hexdigest()
    shape_type = int(addr_hash[:2], 16) % 5
    
    voxel_size = 22
    voxels = generate_voxel_shape(addr, shape_type)
    
    # Render each voxel
    for vx, vy, vz, color_type in voxels:
        v_color = accent_rgb if color_type == 1 else color_rgb
        draw_isometric_cube(draw, cx, cy, vx, vy, vz, voxel_size, v_color)
        
    # 4. Add Vignette Border
    vignette = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    v_draw = ImageDraw.Draw(vignette)
    v_draw.rectangle([0, 0, width, height], outline=(0, 0, 0, 255), width=24)
    # Apply blur to vignette
    vignette = vignette.filter(ImageFilter.GaussianBlur(15))
    bg_img.paste(vignette, (0, 0), vignette)
    
    # Save the composite high fidelity art asset
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
