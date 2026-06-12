import os
import argparse
import json
import urllib.request
import base64
from PIL import Image, ImageDraw, ImageFilter

def load_dna_record(dna_path, frame_idx):
    import struct
    if not os.path.exists(dna_path):
        return None
    with open(dna_path, "rb") as f:
        magic = f.read(4)
        if magic != b'TSFI':
            return None
        total_frames = struct.unpack('=I', f.read(4))[0]
        f.seek(8 + frame_idx * 31)
        frame_data = f.read(31)
        if len(frame_data) < 31:
            return None
        g_x, g_y, stretch, pulse, fire, light, r, g, b, er, eg, eb, ec = struct.unpack(
            '=ffffffBBBBBBB', frame_data
        )
        return {
            'g_x': g_x, 'g_y': g_y, 'stretch': stretch, 'pulse': pulse,
            'fire': fire, 'light': light, 'r': r, 'g': g, 'b': b,
            'er': er, 'eg': eg, 'eb': eb, 'ec': ec
        }

def render_vlm_synthesized_frame(frame_idx, steps=4, cfg=1.5, prompt_override=None):
    print("=== TSFi Autonomous VLM Synthesizer Frame Director ===")
    
    # Load attributes
    dna_path = "tsfi2-deepseek/assets/dragon.dna"
    dna = load_dna_record(dna_path, frame_idx)
    if not dna:
        dna = {
            'g_x': 0.0, 'g_y': 0.0, 'stretch': 1.0, 'pulse': 0.0, 'fire': 0.0, 'light': 1.0,
            'r': 200, 'g': 30, 'b': 45, 'er': 255, 'eg': 215, 'eb': 0, 'ec': 2
        }

    print(f"[Synthesizer] Context variables -> scale RGB({dna['r']},{dna['g']},{dna['b']}) | fire: {dna['fire']:.2f} | eyes: {dna['ec']}")
    
    # 1. Base scene background loading
    bg_img = None
    bg_path = "tsfi2-deepseek/assets/castle_bg.png"
    if frame_idx >= 600 and frame_idx < 900:
        bg_path = "tsfi2-deepseek/assets/cavern_bg.png"
        
    if os.path.exists(bg_path):
        bg_img = Image.open(bg_path).convert("RGB")
        # Resize to standard widescreen HD
        bg_img = bg_img.resize((1280, 720))
    else:
        # Create solid canvas fallback
        bg_img = Image.new("RGB", (1280, 720), (12, 7, 20))

    # 2. Procedural high-fidelity character / vector outline rendering
    overlay = Image.new("RGBA", (1280, 720), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    
    # Coordinates mapping center
    cx = 640 + int(dna['g_x'] * 150)
    cy = 400 + int((dna['g_y'] + dna['pulse'] * 0.04) * 150)
    
    scale_w = int(180 * dna['stretch'])
    scale_h = int(180 * (2.0 - dna['stretch']))
    
    scale_color = (dna['r'], dna['g'], dna['b'], 255)
    eye_color = (dna['er'], dna['eg'], dna['eb'], 255)
    
    # Draw character vector outlines (High-fidelity custom shapes)
    if dna['r'] == 255 and dna['g'] == 170 and dna['b'] == 0:
        # Render Dirk the Daring Knight overlay
        draw.ellipse([cx - scale_w, cy - scale_h, cx + scale_w, cy + scale_h], outline=scale_color, width=8)
        draw.rectangle([cx - scale_w//2, cy - scale_h//4, cx + scale_w//2, cy + scale_h//4], fill=(150, 150, 150, 200))
        # Plume
        draw.chord([cx - scale_w, cy - scale_h - 40, cx, cy - scale_h//2], 180, 360, fill=(255, 0, 0, 255))
    elif dna['r'] == 255 and dna['g'] == 0 and dna['b'] == 187:
        # Render Princess Daphne crown overlay
        draw.ellipse([cx - scale_w//2, cy - scale_h//2, cx + scale_w//2, cy + scale_h//2], outline=scale_color, width=6)
        # Crown points
        crown_points = [
            (cx - 40, cy - 80), (cx - 50, cy - 120), (cx - 20, cy - 100),
            (cx, cy - 140), (cx + 20, cy - 100), (cx + 50, cy - 120), (cx + 40, cy - 80)
        ]
        draw.polygon(crown_points, fill=(255, 215, 0, 255), outline=(255, 255, 255, 255))
    elif dna['r'] == 0 and dna['g'] == 255 and dna['b'] == 102:
        # Lizard King (#00ff66) - Draw scaly round body and scepter
        draw.ellipse([cx - scale_w, cy - scale_h, cx + scale_w, cy + scale_h], outline=scale_color, width=8)
        draw.line([cx - scale_w, cy, cx - scale_w - 30, cy - 60], fill=(255, 215, 0, 255), width=6) # Scepter
        draw.ellipse([cx - scale_w - 40, cy - 80, cx - scale_w - 20, cy - 60], fill=(255, 215, 0, 255)) # Scepter top
    elif dna['r'] == 119 and dna['g'] == 255 and dna['b'] == 0:
        # Giddy Goon (#77ff00) - Draw pointed ears and chaotic shape
        draw.polygon([
            (cx - scale_w, cy), (cx - scale_w - 40, cy - 40), (cx - scale_w + 10, cy - 20),
            (cx, cy - scale_h), (cx + scale_w - 10, cy - 20), (cx + scale_w + 40, cy - 40), (cx + scale_w, cy)
        ], outline=scale_color, width=6)
    elif dna['r'] == 0 and dna['g'] == 255 and dna['b'] == 255:
        # Electric Knight (#00ffff) - Draw robotic shield armor and lightning bolts
        draw.rectangle([cx - scale_w, cy - scale_h, cx + scale_w, cy + scale_h], outline=scale_color, width=8)
        # Draw lightning aesthetic lines
        draw.line([cx - 10, cy - 40, cx - 30, cy + 10], fill=(255, 255, 0, 255), width=4)
        draw.line([cx - 30, cy + 10, cx - 10, cy + 40], fill=(255, 255, 0, 255), width=4)
    else:
        # Render Singe the Dragon outline (#ff3c3c)
        # Main Head
        draw.chord([cx - scale_w, cy - scale_h, cx + scale_w, cy + scale_h], 0, 360, outline=scale_color, width=10)
        # Snout
        draw.polygon([
            (cx - scale_w, cy - scale_h//4), (cx - scale_w*1.5, cy - scale_h//6),
            (cx - scale_w*1.5, cy + scale_h//6), (cx - scale_w, cy + scale_h//4)
        ], outline=scale_color, fill=(dna['r']//2, dna['g']//2, dna['b']//2, 180))

    # Draw eyes
    for i in range(dna['ec']):
        ex = cx - 40 + (i * 30)
        ey = cy - 30
        if dna['r'] == 255 and (dna['g'] == 170 or dna['g'] == 0):
            ex = cx - 15 + (i * 30)
            ey = cy - 10
        draw.ellipse([ex - 12, ey - 12, ex + 12, ey + 12], fill=eye_color)
        draw.ellipse([ex - 4, ey - 4, ex + 4, ey + 4], fill=(255, 255, 255, 255))

    # Fire eruption layer
    if dna['fire'] > 0.05:
        fire_layer = Image.new("RGBA", (1280, 720), (0, 0, 0, 0))
        fire_draw = ImageDraw.Draw(fire_layer)
        fire_w = int(dna['fire'] * 300)
        fire_points = [
            (cx - scale_w, cy), (cx - scale_w - fire_w, cy - 60),
            (cx - scale_w - fire_w - 50, cy), (cx - scale_w - fire_w, cy + 60)
        ]
        fire_draw.polygon(fire_points, fill=(255, 120, 0, 180))
        # Inner flame core
        fire_draw.polygon([
            (cx - scale_w, cy), (cx - scale_w - fire_w//2, cy - 30),
            (cx - scale_w - fire_w//2 - 20, cy), (cx - scale_w - fire_w//2, cy + 30)
        ], fill=(255, 230, 0, 220))
        
        # Apply slight blur to fire layer
        fire_layer = fire_layer.filter(ImageFilter.GaussianBlur(5))
        overlay.alpha_composite(fire_layer)

    # 3. Composite overlays
    bg_img.paste(overlay, (0, 0), overlay)
    
    png_out = "assets/storybook/page_dragon_dna.png"
    bg_img.save(png_out)
    print(f"[Synthesizer] Composite page frame written to: {png_out}")

    # 4. Initiate VLM Moondream interrogation check
    print("[Synthesizer] Waking Moondream VLM node check...")
    VLM_URL = "http://127.0.0.1:11435/api/generate"
    try:
        with open(png_out, "rb") as img_file:
            b64_data = base64.b64encode(img_file.read()).decode('utf-8')
            
        payload = {
            "model": "moondream",
            "prompt": "Verify this composite video still frame generated from local vector thunks. Does it represent the target 1980s cel aesthetic?",
            "images": [b64_data],
            "stream": False
        }
        
        req = urllib.request.Request(
            VLM_URL,
            data=json.dumps(payload).encode('utf-8'),
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        with urllib.request.urlopen(req, timeout=8) as response:
            res_data = response.read().decode('utf-8')
            res_json = json.loads(res_data)
            feedback = res_json.get("response", "")
            print("\n=== VLM Synthesizer Verification Critique ===")
            print(feedback)
            print("=============================================\n")
    except Exception as e:
        print(f"[Synthesizer] VLM verification offline fallback (Ollama not found on 11435): {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="TSFi VLM Synthesizer Frame Creator")
    parser.add_argument("frame", type=int, nargs="?", default=700, help="Frame index")
    parser.add_argument("--steps", type=int, default=4)
    parser.add_argument("--cfg", type=float, default=1.5)
    parser.add_argument("--prompt", type=str, default=None)
    args = parser.parse_args()
    
    render_vlm_synthesized_frame(args.frame, args.steps, args.cfg, args.prompt)
