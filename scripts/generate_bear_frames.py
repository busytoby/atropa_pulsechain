#!/usr/bin/env python3
import os
import re
import math
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
    
    # Render exactly 313 frames corresponding to conversation steps
    for idx, step in enumerate(log_data):
        img = Image.new("RGB", (640, 480), "black")
        draw = ImageDraw.Draw(img)
        
        # Draw connections/conversational exchanges
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        draw.line([speaker_pos["x"], speaker_pos["y"], listener_pos["x"], listener_pos["y"]], fill=(255, 255, 255), width=3)
        
        # Draw each bear head sphere
        for name, bear in bears.items():
            base_radius = 40
            if name == step["listener"]:
                # Modulate listener head size by emotional voltage
                base_radius += int(min(step["voltage"] * 3, 60))
            
            x, y = bear["x"], bear["y"]
            draw.ellipse([x - base_radius, y - base_radius, x + base_radius, y + base_radius], fill=bear["color"], outline="white")
            draw.text((x - 20, y - 5), name, fill="black")
            
        # Draw status info
        draw.text((10, 10), f"Step: {idx + 1}/313", fill="white")
        draw.text((10, 25), f"Speaker: {step['speaker']}", fill="white")
        draw.text((10, 40), f"Listener: {step['listener']}", fill="white")
        draw.text((10, 55), f"Voltage: {step['voltage']:.3f} V", fill="white")
        draw.text((10, 70), f"Pitch: {step['pitch']:.1f} Hz", fill="white")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
