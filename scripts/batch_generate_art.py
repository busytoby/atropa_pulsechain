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
    desc_lower = desc.lower()
    voxels = []
    
    # 0. BOZO Clown Face
    if "bozo" in desc_lower or "clown" in desc_lower:
        voxels.append((0, 0, 0, 1)) # Nose
        for z in range(-1, 2):
            for x in range(-1, 2):
                for y in range(-1, 2):
                    if (x, y, z) != (0, 0, 0) and (x, y, z) != (0, 0, 1):
                        voxels.append((x, y, z, 1))
        for z in range(-1, 3):
            for y in range(-1, 2):
                voxels.append((-2, y, z, 0))
                voxels.append((2, y, z, 0))
        voxels.append((-1, -1, -2, 0))
        voxels.append((0, -1, -2, 0))
        voxels.append((1, -1, -2, 0))
        voxels.append((-2, -1, -1, 0))
        voxels.append((2, -1, -1, 0))

    # 1. Animal / Dog / Inu
    elif "inu" in desc_lower or "dog" in desc_lower or "pinu" in desc_lower or "puppy" in desc_lower:
        for x in range(-1, 2):
            for y in range(-1, 2):
                voxels.append((x, y, -1, 0))
        voxels.append((-2, -2, 0, 0))
        voxels.append((-1, -3, 0, 1))
        voxels.append((1, -3, 0, 1))
        voxels.append((2, -2, 0, 0))
        
    # 2. Yin Yang
    elif "yinyang" in desc_lower or "yyang" in desc_lower or ("yin" in desc_lower and "yang" in desc_lower):
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
    elif "poop" in desc_lower or "poo" in desc_lower or "p0op" in desc_lower or "po0p" in desc_lower:
        for z in range(-2, 3):
            r = 3 - (z + 2)
            if r < 0: r = 0
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 0 if z % 2 == 0 else 1))
                        
    # 4. Dinosaur / Dino
    elif "dino" in desc_lower or "dinosaur" in desc_lower:
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
    elif "wildlife" in desc_lower or "refuge" in desc_lower or "tree" in desc_lower or "forest" in desc_lower or "🌲" in desc_lower or "wood" in desc_lower or "nature" in desc_lower:
        for z in range(-3, 0):
            voxels.append((0, 0, z, 0))
        for z in range(0, 4):
            r = 3 - z
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 1))
                        
    # 6. Star Gazer
    elif "stargazer" in desc_lower or "gazer" in desc_lower or "star" in desc_lower or "spark" in desc_lower:
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
    elif "tits" in desc_lower or "doubledd" in desc_lower:
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
    elif "printer" in desc_lower or "press" in desc_lower:
        for x in range(-2, 3):
            for y in range(-2, 3):
                for z in range(-3, 1):
                    voxels.append((x, y, z, 0))
        for y in range(-1, 2):
            voxels.append((0, y, 1, 1))
            voxels.append((0, y, 2, 1))
            
    # 9. Ghost / Shadow
    elif "shadow" in desc_lower or "ghost" in desc_lower or "phantom" in desc_lower or "specter" in desc_lower:
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
    elif "firefly" in desc_lower or "fly" in desc_lower or "bug" in desc_lower or "insect" in desc_lower:
        for z in range(-1, 2):
            voxels.append((0, 0, z, 0))
        voxels.append((0, 0, -2, 1))
        voxels.append((-1, 0, 0, 1))
        voxels.append((1, 0, 0, 1))
        voxels.append((-2, 0, 1, 1))
        voxels.append((2, 0, 1, 1))
        
    # 11. Key
    elif "key" in desc_lower:
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
    elif "sword" in desc_lower or "blade" in desc_lower or "dirk" in desc_lower or "dagger" in desc_lower or "weapon" in desc_lower:
        for z in range(-1, 4):
            voxels.append((0, 0, z, 1))
        for x in range(-2, 3):
            voxels.append((x, 0, -2, 0))
        voxels.append((0, 0, -3, 0))
        voxels.append((0, 0, -4, 0))
        
    # 13. Shield
    elif "shield" in desc_lower or "sentinel" in desc_lower or "vault" in desc_lower or "guard" in desc_lower or "defense" in desc_lower or "protect" in desc_lower:
        for z in range(-2, 3):
            width = 2 if z >= 0 else 2 + z
            for x in range(-width, width + 1):
                voxels.append((x, 0, z, 0))
                if x == 0 and z == 0:
                    voxels.append((x, 0, z, 1))
                    
    # 14. Crown
    elif "crown" in desc_lower or "king" in desc_lower or "princess" in desc_lower or "queen" in desc_lower or "emperor" in desc_lower:
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
    elif "gas" in desc_lower or "fuel" in desc_lower or "fire" in desc_lower or "burn" in desc_lower or "flame" in desc_lower or "plasma" in desc_lower:
        for z in range(-3, 4):
            max_r = 3 - (z + 3)//2
            for x in range(-max_r, max_r + 1):
                for y in range(-max_r, max_r + 1):
                    if x*x + y*y <= max_r*max_r:
                        is_inner = (x*x + y*y <= 1) and z < 2
                        voxels.append((x, y, z, 1 if is_inner else 0))
                        
    # 16. Heart
    elif "heart" in desc_lower or "love" in desc_lower or "peace" in desc_lower or "humanity" in desc_lower:
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
    elif "drop" in desc_lower or "water" in desc_lower or "liquid" in desc_lower or "pool" in desc_lower or "fluid" in desc_lower:
        for z in range(-3, 4):
            r = 3 - (z + 3)//2
            for x in range(-r, r+1):
                for y in range(-r, r+1):
                    if x*x + y*y <= r*r:
                        voxels.append((x, y, z, 1 if (z == -2 and x == 0 and y == 0) else 0))
                        
    # 18. Stablecoin/USD/Dollar
    elif "usd" in desc_lower or "dai" in desc_lower or "usdc" in desc_lower or "usdt" in desc_lower or "stable" in desc_lower or "dollar" in desc_lower or "cash" in desc_lower or "money" in desc_lower:
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

    # 19. Crypto / Bitcoin / Ethereum / Coin / Token Icon
    elif "btc" in desc_lower or "bitcoin" in desc_lower or "eth" in desc_lower or "ethereum" in desc_lower or "bnb" in desc_lower or "pls" in desc_lower or "pulse" in desc_lower or "coin" in desc_lower:
        for z in range(-1, 2):
            for x in range(-3, 4):
                for y in range(-3, 4):
                    d = x*x + y*y
                    if d <= 9:
                        is_glyph = (x == -1 and -2 <= y <= 2) or (x >= 0 and y in (-2, 0) and x*x <= 4) or (x == 2 and y in (-1,))
                        if is_glyph:
                            voxels.append((x, y, z, 1))
                        else:
                            voxels.append((x, y, z, 0))

    # 20. Bear
    elif "bear" in desc_lower:
        for z in range(-1, 2):
            for x in range(-2, 3):
                for y in range(-2, 3):
                    if x*x + y*y <= 4:
                        voxels.append((x, y, z, 0))
        voxels.append((0, -2, -1, 1))
        voxels.append((0, -3, -1, 1))
        voxels.append((-2, 0, 2, 0))
        voxels.append((2, 0, 2, 0))

    # 21. UFO / Flying Saucer
    elif "ufo" in desc_lower or "alien" in desc_lower or "space" in desc_lower or "cosmos" in desc_lower or "galaxy" in desc_lower:
        for x in range(-3, 4):
            for y in range(-3, 4):
                if x*x + y*y <= 9:
                    voxels.append((x, y, 0, 0))
        voxels.append((0, 0, 1, 1))
        voxels.append((0, 1, 1, 1))
        voxels.append((0, -1, 1, 1))
        voxels.append((1, 0, 1, 1))
        voxels.append((-1, 0, 1, 1))
        voxels.append((0, 0, 2, 1))

    # 22. Pirate / Skull
    elif "pirate" in desc_lower or "skull" in desc_lower or "caw" in desc_lower or "death" in desc_lower:
        for z in range(0, 3):
            for x in range(-2, 3):
                for y in range(-2, 3):
                    if x*x + y*y <= 4:
                        voxels.append((x, y, z, 0))
        for x in range(-1, 2):
            voxels.append((x, -1, -1, 0))
            voxels.append((x, -1, -2, 0))
        voxels.append((-1, -2, 0, 1))
        voxels.append((1, -2, 0, 1))

    # 23. Food / Pizza
    elif "food" in desc_lower or "pizza" in desc_lower or "burger" in desc_lower:
        for z in range(-1, 1):
            for y in range(-3, 4):
                max_x = y + 3
                for x in range(-max_x, max_x + 1):
                    is_crust = y == 3
                    voxels.append((x, y, z, 1 if is_crust else 0))

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
