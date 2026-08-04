#!/usr/bin/env python3
import os
import re
import math
import hashlib
from PIL import Image, ImageDraw

def render_frames():
    log_path = "chorus_simulation.log"
    output_dir = "rendered_frames"
    os.makedirs(output_dir, exist_ok=True)
    
    log_data = []
    pattern = re.compile(r"Bear (\w+) spoke to Bear (\w+) -> (\w+) current voltage: ([\d\.]+) V.*?Pitch: ([\d\.]+) Hz")
    with open(log_path, "r") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                log_data.append({
                    "speaker": match.group(1),
                    "listener": match.group(2),
                    "voltage": float(match.group(4)),
                    "pitch": float(match.group(5))
                })
                
    if not log_data:
        log_data = [{"speaker": "Trusty", "listener": "Coop", "voltage": 1.0, "pitch": 220.0}] * 313
        
    bears = {
        "Trusty":  {"x": 120, "y": 240, "color": (50, 150, 255)},
        "Aggro":   {"x": 520, "y": 240, "color": (255, 50, 50)},
        "Skeptic": {"x": 220, "y": 120, "color": (200, 200, 5)},
        "Eerie":   {"x": 420, "y": 120, "color": (150, 50, 200)},
        "Coop":    {"x": 320, "y": 360, "color": (50, 200, 100)}
    }
    
    # Mock token address hashes to derive unique LAU signatures for the 5 bears
    bear_addresses = {
        "Trusty":  "0x1111111111111111111111111111111111111111",
        "Aggro":   "0x2222222222222222222222222222222222222222",
        "Skeptic": "0x3333333333333333333333333333333333333333",
        "Eerie":   "0x4444444444444444444444444444444444444444",
        "Coop":    "0x5555555555555555555555555555555555555555"
    }
    
    WIDTH, HEIGHT = 640, 480
    
    for idx, step in enumerate(log_data):
        img = Image.new("RGB", (WIDTH, HEIGHT), (12, 14, 20)) # Dark blueprint background
        draw = ImageDraw.Draw(img, "RGBA")
        
        # Grid overlay
        for x_line in range(0, WIDTH, 40):
            draw.line([(x_line, 0), (x_line, HEIGHT)], fill=(18, 20, 28), width=1)
        for y_line in range(0, HEIGHT, 40):
            draw.line([(0, y_line), (WIDTH, y_line)], fill=(18, 20, 28), width=1)
            
        # Draw active connection line
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        draw.line([speaker_pos["x"], speaker_pos["y"], listener_pos["x"], listener_pos["y"]], fill=(255, 255, 255, 100), width=2)
        
        # Draw 5 Teddy Bears
        for name, bear in bears.items():
            cx, cy = bear["x"], bear["y"]
            color = bear["color"]
            
            # Procedural 2D Teddy Bear Drawing outlines (from standard art director fallback)
            # Body
            draw.ellipse([cx - 30, cy + 10, cx + 30, cy + 50], fill=color, outline=(255, 255, 255), width=1)
            # Head
            draw.ellipse([cx - 25, cy - 30, cx + 25, cy + 20], fill=color, outline=(255, 255, 255), width=1)
            # Ears
            draw.ellipse([cx - 28, cy - 35, cx - 12, cy - 19], fill=color, outline=(255, 255, 255), width=1)
            draw.ellipse([cx + 12, cy - 35, cx + 28, cy - 19], fill=color, outline=(255, 255, 255), width=1)
            # Eyes
            draw.ellipse([cx - 10, cy - 10, cx - 6, cy - 6], fill=(0, 0, 0))
            draw.ellipse([cx + 6, cy - 10, cx + 10, cy - 6], fill=(0, 0, 0))
            # Nose
            draw.ellipse([cx - 4, cy, cx + 4, cy + 6], fill=(30, 30, 30))
            
            draw.text((cx - 18, cy + 55), name, fill="white")
            
            # If active speaker, draw quality LAU Lissajous rendering orbiting their drawing
            if name == step["speaker"]:
                address = bear_addresses.get(name, "0x1111111111111111111111111111111111111111")
                addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
                
                # Derive f_x, f_y, f_z from token address hash
                fx = 1.0 + (int(addr_hash[0:2], 16) % 5)
                fy = 1.0 + (int(addr_hash[2:4], 16) % 5)
                fz = 1.0 + (int(addr_hash[4:6], 16) % 5)
                phi = (int(addr_hash[6:8], 16) % 100) / 100.0 * 2.0 * math.pi
                
                # Render the Lissajous points
                num_points = 100
                proj_points = []
                for pt_idx in range(num_points):
                    theta = pt_idx * 2.0 * math.pi / num_points
                    lx = 60.0 * math.sin(fx * theta + phi + idx * 0.1) # slow rotation animate
                    ly = 60.0 * math.sin(fy * theta)
                    lz = 60.0 * math.cos(fz * theta)
                    
                    # 3D perspective projection
                    zoom = 0.90
                    z_new = lz + 200.0
                    px = cx + int((lx * 200.0) / z_new * zoom)
                    py = cy + int((ly * 200.0) / z_new * zoom)
                    proj_points.append((px, py))
                    
                for pt_idx in range(num_points):
                    p1 = proj_points[pt_idx]
                    p2 = proj_points[(pt_idx + 1) % num_points]
                    draw.line([p1, p2], fill=(0, 255, 200, 180), width=2)
                    
        # Overlay Status labels
        draw.text((20, 20), "AUNCIENT LAU TONEWHEEL CHORUS", fill="#ff007f")
        draw.text((20, 35), f"Step: {idx + 1}/313 | Active: {step['speaker']} -> {step['listener']}", fill="#00f2fe")
        draw.text((20, 50), f"Frequency Mod: {step['pitch']:.1f} Hz | Voltage: {step['voltage']:.3f} V", fill="#ffd700")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
