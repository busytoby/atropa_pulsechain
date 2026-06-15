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

from render_vlm_synthesizer import generate_voxel_shape, write_to_shm_depth
import subprocess

def sanitize_text_for_font(text, fallback="TOKEN"):
    # Keep standard printable characters, including emojis and special unicode symbols (ord >= 32)
    sanitized = "".join(c for c in text if ord(c) >= 32)
    sanitized = " ".join(sanitized.split())
    if not sanitized:
        return fallback
    return sanitized

def draw_text_with_fallback(draw, position, text, fill, font_primary, font_fallback):
    x, y = position
    for c in text:
        # Standard ASCII and Enclosed Alphanumerics go to primary font (FreeSans)
        # Everything else (emojis, peace, fuel, wave, wood etc.) goes to Symbola
        if (32 <= ord(c) <= 126) or (9312 <= ord(c) <= 9450):
            font = font_primary
        else:
            font = font_fallback
        draw.text((x, y), c, fill=fill, font=font)
        x += draw.textlength(c, font=font)

def wrap_text(text, width_chars):
    words = text.split()
    lines = []
    current_line = []
    current_length = 0
    for word in words:
        if current_length + len(word) + 1 > width_chars:
            lines.append(" ".join(current_line))
            current_line = [word]
            current_length = len(word)
        else:
            current_line.append(word)
            current_length += len(word) + 1
    if current_line:
        lines.append(" ".join(current_line))
    return lines

def render_card_art(card, output_path):
    from PIL import ImageFont
    
    width, height = 1280, 720
    bg_img = Image.new("RGB", (width, height), (8, 5, 15))
    draw = ImageDraw.Draw(bg_img)
    
    color_hex = card.get('color', '#00f0ff')
    color_rgb = hex_to_rgb(color_hex)
    accent_rgb = (255, 255, 255)
    
    addr = card.get('address', '').lower()
    addr_hash = hashlib.md5(addr.encode('utf-8')).hexdigest()
    
    # 1. Outer chamfered high-tech card frame
    draw.polygon([(40, 15), (1240, 15), (1265, 40), (1265, 680), (1240, 705), (40, 705), (15, 680), (15, 40)], outline=get_shade(color_rgb, 0.8), width=4)
    
    # 2. Left side Artwork Panel (640x640)
    # Background panel glow
    glow_mask = Image.new("L", (width, height), 0)
    glow_draw = ImageDraw.Draw(glow_mask)
    glow_draw.ellipse([360 - 260, 360 - 260, 360 + 260, 360 + 260], fill=70)
    glow_color_img = Image.new("RGB", (width, height), color_rgb)
    bg_img = Image.composite(glow_color_img, bg_img, glow_mask)
    draw = ImageDraw.Draw(bg_img)
    
    # Chamfered artwork panel border
    draw.polygon([(60, 40), (660, 40), (680, 60), (680, 660), (660, 680), (60, 680), (40, 660), (40, 60)], outline=get_shade(color_rgb, 0.4), width=2)
    
    # Holographic space grid inside artwork panel
    grid_color = get_shade(color_rgb, 0.12)
    for x in range(40, 681, 40):
        draw.line([x, 40, x, 680], fill=grid_color, width=1)
    for y in range(40, 681, 40):
        draw.line([40, y, 680, y], fill=grid_color, width=1)
        
    # Floating particles inside artwork panel
    h_val = int(addr_hash, 16)
    for i in range(15):
        px = 40 + ((h_val >> (i * 2)) % 600 + 20)
        py = 40 + ((h_val >> (i * 3)) % 600 + 20)
        p_sz = (h_val >> (i * 4)) % 3 + 1
        p_color = accent_rgb if i % 3 == 0 else get_shade(color_rgb, 1.2)
        draw.ellipse([px - p_sz, py - p_sz, px + p_sz, py + p_sz], fill=p_color)
        
    # Center targeting HUD rings in the artwork box
    cx, cy = 360, 360
    hud_color = get_shade(color_rgb, 0.35)
    draw.ellipse([cx - 240, cy - 240, cx + 240, cy + 240], outline=hud_color, width=2)
    draw.ellipse([cx - 210, cy - 210, cx + 210, cy + 210], outline=get_shade(color_rgb, 0.5), width=1)
    draw.line([cx - 260, cy, cx - 220, cy], fill=hud_color, width=3)
    draw.line([cx + 220, cy, cx + 260, cy], fill=hud_color, width=3)
    draw.line([cx, cy - 260, cx, cy - 220], fill=hud_color, width=3)
    draw.line([cx, cy + 220, cx, cy + 260], fill=hud_color, width=3)
    
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        x1 = cx + 200 * math.cos(rad)
        y1 = cy + 200 * math.sin(rad)
        x2 = cx + 215 * math.cos(rad)
        y2 = cy + 215 * math.sin(rad)
        draw.line([x1, y1, x2, y2], fill=hud_color, width=2)
        
    # 3. Draw Pedestal Base
    pedestal_color = get_shade(color_rgb, 0.5)
    for px in range(-2, 3):
        for py in range(-2, 3):
            if abs(px) + abs(py) <= 3:
                draw_isometric_cube(draw, cx, cy, px, py, -4, 30, pedestal_color)

    # 4. Generate & render the Voxel Shape inside HUD rings
    desc = card.get('desc', '')
    voxels = generate_voxel_shape(desc, seed_str=addr)
    voxel_size = 30
    
    for vx, vy, vz, color_type in voxels:
        v_color = accent_rgb if color_type == 1 else color_rgb
        draw_isometric_cube(draw, cx, cy, vx, vy, vz, voxel_size, v_color)
        
    # 5. Right side Info Panel (720x40 to 1240x680)
    draw.polygon([(740, 40), (1220, 40), (1240, 60), (1240, 660), (1220, 680), (740, 680), (720, 660), (720, 60)], outline=get_shade(color_rgb, 0.4), fill=(10, 6, 18), width=2)
    
    # Load fonts
    font_path = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf"
    if not os.path.exists(font_path):
        font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
    if not os.path.exists(font_path):
        font_path = "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
    if not os.path.exists(font_path):
        font_path = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"

    font_mono_path = "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
    if not os.path.exists(font_mono_path):
        font_mono_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
    if not os.path.exists(font_mono_path):
        font_mono_path = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"

    font_fallback_path = "/usr/share/fonts/truetype/ancient-scripts/Symbola_hint.ttf"

    try:
        font_title = ImageFont.truetype(font_path, 34)
        font_body = ImageFont.truetype(font_path, 20)
        font_small = ImageFont.truetype(font_path, 15)
        font_mono = ImageFont.truetype(font_mono_path, 15)
    except:
        font_title = ImageFont.load_default()
        font_body = ImageFont.load_default()
        font_small = ImageFont.load_default()
        font_mono = ImageFont.load_default()

    if os.path.exists(font_fallback_path):
        font_title_fb = ImageFont.truetype(font_fallback_path, 34)
        font_body_fb = ImageFont.truetype(font_fallback_path, 20)
        font_small_fb = ImageFont.truetype(font_fallback_path, 15)
        font_mono_fb = ImageFont.truetype(font_fallback_path, 15)
    else:
        font_title_fb = font_title
        font_body_fb = font_body
        font_small_fb = font_small
        font_mono_fb = font_mono
        
    # Title (Sanitized for unrenderable characters)
    token_name = sanitize_text_for_font(card.get('name', 'UNKNOWN').split(' (')[0].upper(), fallback="PARTNER TOKEN")
    draw_text_with_fallback(draw, (750, 70), token_name, accent_rgb, font_title, font_title_fb)
    
    # Type Badge
    card_type = card.get('type', 'TOKEN').upper()
    badge_w = int(draw.textlength(card_type, font=font_small)) + 20
    draw.rectangle([750, 130, 750 + badge_w, 160], fill=get_shade(color_rgb, 0.7), outline=color_rgb, width=1)
    draw_text_with_fallback(draw, (760, 137), card_type, accent_rgb, font_small, font_small_fb)
    
    # Symbol Badge (Sanitized)
    symbol = sanitize_text_for_font(card.get('symbol', 'TKN').upper(), fallback="TKN")
    badge_sym_w = int(draw.textlength(symbol, font=font_small)) + 20
    draw.rectangle([760 + badge_w, 130, 760 + badge_w + badge_sym_w, 160], fill=(24, 15, 36), outline=accent_rgb, width=1)
    draw_text_with_fallback(draw, (770 + badge_w, 137), symbol, accent_rgb, font_small, font_small_fb)
    
    # Description (Sanitized)
    draw.text((750, 190), "CHARACTERISTICS / DEFI GENOME:", fill=get_shade(color_rgb, 0.8), font=font_small)
    desc_sanitized = sanitize_text_for_font(desc, fallback="DIGITAL ASSET DATA RECORD")
    desc_lines = wrap_text(desc_sanitized, 38)
    curr_y = 225
    for line in desc_lines:
        draw_text_with_fallback(draw, (750, curr_y), line, accent_rgb, font_body, font_body_fb)
        curr_y += 26
        
    # Stats
    draw.text((750, 420), "CORE POWER INDEX:", fill=get_shade(color_rgb, 0.8), font=font_small)
    atk_val = (h_val % 8) + 3
    def_val = ((h_val >> 4) % 8) + 3
    nrg_val = ((h_val >> 8) % 8) + 3
    
    stats = [
        ("ATK/BURST", atk_val),
        ("DEF/SHIELD", def_val),
        ("LIQUIDITY", nrg_val)
    ]
    stat_y = 455
    for label, val in stats:
        draw.text((750, stat_y), label, fill=accent_rgb, font=font_small)
        for i in range(10):
            bx = 890 + i * 22
            fill_color = color_rgb if i < val else get_shade(color_rgb, 0.15)
            draw.rectangle([bx, stat_y + 1, bx + 16, stat_y + 13], fill=fill_color)
        stat_y += 32
        
    # Address
    draw.text((750, 610), "CRYPTOGRAPHIC SIGNATURE:", fill=get_shade(color_rgb, 0.8), font=font_small)
    draw.text((750, 635), f"ADDR: {addr.upper()}", fill=color_rgb, font=font_mono)
    
    # Vignette
    vignette = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    v_draw = ImageDraw.Draw(vignette)
    v_draw.rectangle([0, 0, width, height], outline=(0, 0, 0, 255), width=24)
    vignette = vignette.filter(ImageFilter.GaussianBlur(15))
    bg_img.paste(vignette, (0, 0), vignette)
    
    # Run SD stylization pipeline
    write_to_shm_depth(bg_img)
    
    sd_prompt = (
        f"Vibrant high-fidelity sci-fi trading card game art, cel-shaded neon vector style, representing '{desc}', "
        f"bold retro 1980s futuristic cyber aesthetic, neon glows, clean vector outlines, hand-painted gouache coloration, masterpiece"
    )
    
    raw_out = f"tmp/batch_sd_out_{addr_hash}.raw"
    os.makedirs("tsfi2-deepseek/tmp", exist_ok=True)
    
    worker_path = "./bin/tsfi_sd_worker"
    cmd = [
        worker_path,
        sd_prompt,
        raw_out,
        "1", # use_shm = 1
        "sd15", # profile
        "4", # steps
        "euler_a", # sampler
        "1.5" # cfg
    ]
    
    print(f"[Batch Generator] Executing Stable Diffusion worker for {card.get('name')}: {' '.join(cmd)}")
    try:
        subprocess.run(cmd, cwd="tsfi2-deepseek", check=True)
        raw_path_adj = "tsfi2-deepseek/" + raw_out
        if os.path.exists(raw_path_adj):
            with open(raw_path_adj, 'rb') as f:
                raw_data = f.read()
            if len(raw_data) == 1280 * 704 * 3:
                bg_img = Image.frombytes('RGB', (1280, 704), raw_data).resize((1280, 720))
            elif len(raw_data) == 512 * 512 * 3:
                bg_img = Image.frombytes('RGB', (512, 512), raw_data).resize((1280, 720))
            
            try:
                os.remove(raw_path_adj)
            except:
                pass
    except Exception as e:
        print(f"[Batch Generator] SD worker failed, falling back to raw render: {e}")
        
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
