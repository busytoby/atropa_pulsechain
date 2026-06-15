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

def generate_voxel_shape(desc, seed_str=None):
    import re
    tokens = re.findall(r'[a-z0-9]+', desc.lower())
    voxels = []
    
    # 0. BOZO Clown Face
    if "bozo" in tokens or "clown" in tokens:
        # Red Nose (Accent)
        voxels.append((0, 0, 0, 1))
        # White face makeup (Accent)
        for z in range(-1, 2):
            for x in range(-1, 2):
                for y in range(-1, 2):
                    if (x, y, z) != (0, 0, 0) and (x, y, z) != (0, 0, 1):
                        voxels.append((x, y, z, 1))
        # Hair (Theme color)
        for z in range(-1, 3):
            for y in range(-1, 2):
                voxels.append((-2, y, z, 0))
                voxels.append((2, y, z, 0))
        # Smile (Theme color)
        voxels.append((-1, -1, -2, 0))
        voxels.append((0, -1, -2, 0))
        voxels.append((1, -1, -2, 0))
        voxels.append((-2, -1, -1, 0))
        voxels.append((2, -1, -1, 0))

    # 1. Animal / Dog / Inu
    elif "inu" in tokens or "dog" in tokens or "pinu" in tokens:
        for x in range(-1, 2):
            for y in range(-1, 2):
                voxels.append((x, y, -1, 0))
        voxels.append((-2, -2, 0, 0))
        voxels.append((-1, -3, 0, 1))
        voxels.append((1, -3, 0, 1))
        voxels.append((2, -2, 0, 0))
        
    # 2. Yin Yang
    elif "yinyang" in tokens or "yyang" in tokens or "yin" in tokens or "yang" in tokens:
        for z in range(-1, 2):
            for x in range(-3, 4):
                for y in range(-3, 4):
                    if x*x + y*y <= 9:
                        is_white = y > 0 or (y == 0 and x > 0)
                        if x == 0 and y == 2:
                            is_white = False
                        elif x == 0 and y == -2:
                            is_white = True
                        voxels.append((x, y, z, 1 if is_white else 0))
                        
    # 3. Poop / Poo
    elif "poop" in tokens or "poo" in tokens or "p0op" in tokens or "po0p" in tokens:
        for z in range(-2, 3):
            r = 3 - (z + 2)
            if r < 0: r = 0
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 0 if z % 2 == 0 else 1))
                        
    # 4. Dinosaur / Dino
    elif "dino" in tokens or "dinosaur" in tokens:
        for z in range(-2, 3):
            for x in range(-2, 3):
                for y in range(-2, 3):
                    voxels.append((x, y, z, 0))
        for z in range(-1, 2):
            for x in range(-2, 3):
                voxels.append((x, 3, z, 0))
        voxels.append((-1, 2, 2, 1))
        voxels.append((1, 2, 2, 1))
        
    # 5. Tree / Forest / Wildlife / Refuge
    elif "wildlife" in tokens or "refuge" in tokens or "tree" in tokens or "forest" in tokens or "🌲" in tokens or "fox" in tokens or "duck" in tokens:
        for z in range(-3, 0):
            voxels.append((0, 0, z, 0))
        for z in range(0, 4):
            r = 3 - z
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 1))
                        
    # 6. Star Gazer
    elif "stargazer" in tokens or "gazer" in tokens or "star" in tokens:
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
                
    # 7. Tits
    elif "tits" in tokens or "doubledd" in tokens:
        for z in range(-1, 2):
            for x in range(-3, 0):
                for y in range(-2, 3):
                    if (x+1.5)**2 + y**2 + z**2 <= 2.25:
                        voxels.append((x, y, z, 0))
            for x in range(1, 4):
                for y in range(-2, 3):
                    if (x-1.5)**2 + y**2 + z**2 <= 2.25:
                        voxels.append((x, y, z, 0))
        voxels.append((-2, 0, 1, 1))
        voxels.append((2, 0, 1, 1))
        
    # 8. Printer / Press
    elif "printer" in tokens or "press" in tokens:
        for x in range(-2, 3):
            for y in range(-2, 3):
                for z in range(-3, 1):
                    voxels.append((x, y, z, 0))
        for y in range(-1, 2):
            voxels.append((0, y, 1, 1))
            voxels.append((0, y, 2, 1))
            
    # 9. Ghost / Shadow
    elif "shadow" in tokens or "ghost" in tokens:
        for z in range(-2, 3):
            r = 2 if z >= 0 else 1
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 0))
        voxels.append((-1, 0, -3, 0))
        voxels.append((1, 0, -3, 0))
        voxels.append((-1, 1, 1, 1))
        voxels.append((1, 1, 1, 1))
        
    # 10. Firefly / Fly
    elif "firefly" in tokens or "fly" in tokens:
        for z in range(-1, 2):
            voxels.append((0, 0, z, 0))
        voxels.append((0, 0, -2, 1))
        voxels.append((-1, 0, 0, 1))
        voxels.append((1, 0, 0, 1))
        voxels.append((-2, 0, 1, 1))
        voxels.append((2, 0, 1, 1))
        
    # 11. Key
    elif "key" in tokens:
        for x in range(-2, 3):
            for y in range(-2, 3):
                if 2 <= x*x + y*y <= 5:
                    voxels.append((x, y, 2, 0))
        for z in range(-3, 2):
            voxels.append((0, 0, z, 0))
        voxels.append((1, 0, -2, 0))
        voxels.append((2, 0, -2, 1))
        voxels.append((1, 0, -1, 0))
        
    # 12. Sword
    elif "sword" in tokens or "blade" in tokens or "dirk" in tokens:
        for z in range(-1, 4):
            voxels.append((0, 0, z, 1))
        for x in range(-2, 3):
            voxels.append((x, 0, -2, 0))
        voxels.append((0, 0, -3, 0))
        voxels.append((0, 0, -4, 0))
        
    # 13. Shield
    elif "shield" in tokens or "sentinel" in tokens or "vault" in tokens or "guard" in tokens:
        for z in range(-2, 3):
            width = 2 if z >= 0 else 2 + z
            for x in range(-width, width + 1):
                voxels.append((x, 0, z, 0))
                if x == 0 and z == 0:
                    voxels.append((x, 0, z, 1))
                    
    # 14. Crown
    elif "crown" in tokens or "king" in tokens or "princess" in tokens or "queen" in tokens:
        for x in range(-2, 3):
            for y in range(-2, 3):
                if 3 <= x*x + y*y <= 5:
                    voxels.append((x, y, -2, 0))
        voxels.append((-2, 0, -1, 0))
        voxels.append((-2, 0, 0, 1))
        voxels.append((2, 0, -1, 0))
        voxels.append((2, 0, 0, 1))
        voxels.append((0, 2, -1, 0))
        voxels.append((0, 2, 0, 1))
        voxels.append((0, -2, -1, 0))
        voxels.append((0, -2, 0, 1))
        voxels.append((0, 0, 1, 1))
        
    # 15. Gas/Flame
    elif "gas" in tokens or "fuel" in tokens or "fire" in tokens or "burn" in tokens or "flame" in tokens:
        for z in range(-3, 4):
            max_r = 3 - (z + 3)//2
            for x in range(-max_r, max_r + 1):
                for y in range(-max_r, max_r + 1):
                    if x*x + y*y <= max_r*max_r:
                        is_inner = (x*x + y*y <= 1) and z < 2
                        voxels.append((x, y, z, 1 if is_inner else 0))
                        
    # 16. Heart
    elif "heart" in tokens or "love" in tokens:
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
        
    # 17. Droplet
    elif "drop" in tokens or "water" in tokens or "liquid" in tokens or "pool" in tokens:
        for z in range(-3, 4):
            r = 3 - (z + 3)//2
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 1 if (z == -2 and x == 0 and y == 0) else 0))
                        
    # 18. Stablecoin/USD/Dollar
    elif "usd" in tokens or "dai" in tokens or "usdc" in tokens or "usdt" in tokens or "stable" in tokens or "dollar" in tokens:
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
            
    else:  # Procedural unique geometric shape based on seed_str/address hash
        import hashlib
        h = hashlib.md5((seed_str if seed_str else desc).encode('utf-8')).hexdigest()
        val = int(h, 16)
        
        # 4-way rotational symmetry ensures high-tech glyph/rune shape
        bit_index = 0
        for z in range(-2, 3):
            for x in range(0, 3):
                for y in range(0, 3):
                    if x == 0 and y == 0:
                        continue
                    bit = (val >> bit_index) & 1
                    bit_index = (bit_index + 1) % 128
                    if bit:
                        color_bit = (val >> ((bit_index + 5) % 128)) & 1
                        for mx, my in ((x, y), (-x, y), (x, -y), (-x, -y)):
                            pt = (mx, my, z, color_bit)
                            if pt not in voxels:
                                voxels.append(pt)
        # Add central core column for structural connectivity
        core_color_type = (val >> 2) & 1
        for z in range(-2, 3):
            voxels.append((0, 0, z, core_color_type))
                            
    voxels.sort(key=lambda v: (v[0] + v[1] + v[2]))
    return voxels

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
    font_path = "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"
    if not os.path.exists(font_path):
        font_path = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"
    try:
        font_title = ImageFont.truetype(font_path, 34)
        font_body = ImageFont.truetype(font_path, 20)
        font_small = ImageFont.truetype(font_path, 15)
        font_mono = ImageFont.truetype("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 15)
    except:
        font_title = ImageFont.load_default()
        font_body = ImageFont.load_default()
        font_small = ImageFont.load_default()
        font_mono = ImageFont.load_default()
        
    # Title
    token_name = card.get('name', 'UNKNOWN').split(' (')[0].upper()
    draw.text((750, 70), token_name, fill=accent_rgb, font=font_title)
    
    # Type Badge
    card_type = card.get('type', 'TOKEN').upper()
    badge_w = len(card_type) * 11 + 20
    draw.rectangle([750, 130, 750 + badge_w, 160], fill=get_shade(color_rgb, 0.7), outline=color_rgb, width=1)
    draw.text((760, 137), card_type, fill=accent_rgb, font=font_small)
    
    # Symbol Badge
    symbol = card.get('symbol', 'TKN').upper()
    badge_sym_w = len(symbol) * 12 + 20
    draw.rectangle([760 + badge_w, 130, 760 + badge_w + badge_sym_w, 160], fill=(24, 15, 36), outline=accent_rgb, width=1)
    draw.text((770 + badge_w, 137), symbol, fill=accent_rgb, font=font_small)
    
    # Description
    draw.text((750, 190), "CHARACTERISTICS / DEFI GENOME:", fill=get_shade(color_rgb, 0.8), font=font_small)
    desc_lines = wrap_text(desc, 38)
    curr_y = 225
    for line in desc_lines:
        draw.text((750, curr_y), line, fill=accent_rgb, font=font_body)
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
