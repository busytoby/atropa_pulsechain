#!/usr/bin/env python3
import os
import re
import math
import numpy as np
from PIL import Image, ImageDraw

def render_frames():
    log_path = "chorus_simulation.log"
    output_dir = "rendered_frames"
    os.makedirs(output_dir, exist_ok=True)
    
    # Parse logs for positions and voltages
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
        "Skeptic": {"x": 220, "y": 120, "color": (200, 200, 50)},
        "Eerie":   {"x": 420, "y": 120, "color": (150, 50, 200)},
        "Coop":    {"x": 320, "y": 360, "color": (50, 200, 100)}
    }
    
    WIDTH, HEIGHT = 640, 480
    
    # Render exactly 313 frames corresponding to conversation steps
    for idx, step in enumerate(log_data):
        # Parallax scrolling plum scanlines background
        t = idx / 10.0
        scroll_x = int(t * 30.0) % WIDTH
        img = Image.new("RGB", (WIDTH, HEIGHT), color=(26, 20, 28))
        draw = ImageDraw.Draw(img)
        
        # Grid lines background
        for y_line in range(0, HEIGHT, 32):
            draw.line([(0, y_line), (WIDTH, y_line)], fill=(44, 32, 48), width=1)
        for x_line in range(-scroll_x, WIDTH + 32, 32):
            draw.line([(x_line, 0), (x_line, HEIGHT)], fill=(44, 32, 48), width=1)
            
        # Draw connections/conversational exchanges
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        draw.line([speaker_pos["x"], speaker_pos["y"], listener_pos["x"], listener_pos["y"]], fill=(255, 255, 255), width=3)
        
        # Draw each bear head sphere
        for name, bear in bears.items():
            base_radius = 40
            if name == step["listener"]:
                base_radius += int(min(step["voltage"] * 3, 60))
            
            x, y = bear["x"], bear["y"]
            
            # Draw Menorah-style vacuum tube backing glow
            for tube_idx in range(-3, 4):
                tx = x + (tube_idx * 12)
                ty = y + 50
                # Glow brightness modulated by voltage
                glow_val = int(min(100 + step["voltage"] * 15, 255))
                draw.line([tx, ty, tx, ty - 30], fill=(glow_val, int(glow_val * 0.5), 0), width=3)
            
            # Temporary mask to apply woven cloth fabric texture
            mask = Image.new("L", (WIDTH, HEIGHT), 0)
            mask_draw = ImageDraw.Draw(mask)
            mask_draw.ellipse([x - base_radius, y - base_radius, x + base_radius, y + base_radius], fill=255)
            
            # Generate procedural woven cloth texture
            img_arr = np.array(img)
            mask_arr = np.array(mask)
            y_coords, x_coords = np.where(mask_arr > 0)
            
            warp = np.sin(x_coords * 1.2) * 12.0
            weft = np.cos(y_coords * 1.2) * 12.0
            cloth_shading = warp + weft
            
            r = np.clip(bear["color"][0] + cloth_shading, 0, 255).astype(np.uint8)
            g = np.clip(bear["color"][1] + cloth_shading, 0, 255).astype(np.uint8)
            b = np.clip(bear["color"][2] + cloth_shading, 0, 255).astype(np.uint8)
            
            img_arr[y_coords, x_coords, 0] = r
            img_arr[y_coords, x_coords, 1] = g
            img_arr[y_coords, x_coords, 2] = b
            
            new_img = Image.fromarray(img_arr)
            img.paste(new_img, (0, 0))
            
            draw.ellipse([x - base_radius, y - base_radius, x + base_radius, y + base_radius], outline=(255, 255, 255), width=2)
            
            # Draw Lissajous projection orbit loops around the head (Tripartite VM manifestation)
            orbit_points = []
            steps_count = 30
            for pt in range(steps_count):
                angle = (pt / steps_count) * 2.0 * math.pi
                fx = 3.0
                fy = (step["pitch"] / 110.0)
                ox = x + int(base_radius * 1.3 * math.sin(fx * angle))
                oy = y + int(base_radius * 1.3 * math.cos(fy * angle + step["voltage"]))
                orbit_points.append((ox, oy))
            
            for pt in range(steps_count):
                p1 = orbit_points[pt]
                p2 = orbit_points[(pt + 1) % steps_count]
                draw.line([p1[0], p1[1], p2[0], p2[1]], fill=(0, 255, 200), width=1)
                
            draw.text((x - 20, y - 5), name, fill="black")
            
        # Draw status info
        draw.text((10, 10), "HUDSON BEAR CHORUS EMOTIONAL DIALOGUE", fill="white")
        draw.text((10, 25), f"Step: {idx + 1}/313", fill="white")
        draw.text((10, 40), f"Speaker: {step['speaker']}", fill="white")
        draw.text((10, 55), f"Listener: {step['listener']}", fill="white")
        draw.text((10, 70), f"Voltage: {step['voltage']:.3f} V", fill="white")
        draw.text((10, 85), f"Pitch: {step['pitch']:.1f} Hz", fill="white")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
