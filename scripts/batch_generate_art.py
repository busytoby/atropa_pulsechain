import os
import glob
import json
from PIL import Image, ImageDraw, ImageFilter

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def render_card_art(card, output_path):
    # 1. Solid dark background
    bg_img = Image.new("RGB", (1280, 720), (12, 7, 20))
    
    # 2. Draw overlay
    overlay = Image.new("RGBA", (1280, 720), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    
    cx, cy = 640, 360
    scale_w, scale_h = 180, 180
    
    color_rgb = hex_to_rgb(card.get('color', '#00f0ff'))
    scale_color = color_rgb + (255,)
    
    # Render Double-bordered neon coin with glowing crossbars
    draw.ellipse([cx - scale_w, cy - scale_h, cx + scale_w, cy + scale_h], outline=scale_color, width=8)
    draw.ellipse([cx - scale_w + 12, cy - scale_h + 12, cx + scale_w - 12, cy + scale_h - 12], outline=scale_color, width=2)
    
    # Draw central symbol crossbars
    draw.line([cx - 20, cy, cx + 20, cy], fill=scale_color, width=6)
    draw.line([cx, cy - 20, cx, cy + 20], fill=scale_color, width=6)
    
    # Draw simple glowing center
    draw.ellipse([cx - 10, cy - 10, cx + 10, cy + 10], fill=(255, 255, 255, 255))
    
    # 3. Composite and save
    bg_img.paste(overlay, (0, 0), overlay)
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
