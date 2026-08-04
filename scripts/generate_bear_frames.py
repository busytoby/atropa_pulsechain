#!/usr/bin/env python3
import os
import re
import math
import hashlib
import wave
import numpy as np
from PIL import Image, ImageDraw

def render_frames():
    log_path = "chorus_simulation.log"
    audio_path = "bear_chorus_track.wav"
    output_dir = "rendered_frames"
    os.makedirs(output_dir, exist_ok=True)
    
    # Read wave file audio segments to convolve the Lissajous curves
    wf = wave.open(audio_path, 'rb')
    sample_rate = wf.getframerate()
    num_frames = wf.getnframes()
    audio_bytes = wf.readframes(num_frames)
    audio_samples = np.frombuffer(audio_bytes, dtype=np.int16).astype(np.float32) / 32767.0
    wf.close()
    
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
    
    WIDTH, HEIGHT = 640, 480
    duration_per_step = 0.5
    step_samples = int(sample_rate * duration_per_step)
    
    for idx, step in enumerate(log_data):
        img = Image.new("RGB", (WIDTH, HEIGHT), (12, 14, 20))
        draw = ImageDraw.Draw(img, "RGBA")
        
        # Grid overlay
        for x_line in range(0, WIDTH, 40):
            draw.line([(x_line, 0), (x_line, HEIGHT)], fill=(18, 20, 28), width=1)
        for y_line in range(0, HEIGHT, 40):
            draw.line([(0, y_line), (WIDTH, y_line)], fill=(18, 20, 28), width=1)
            
        # Extract active audio segment for convolution (Lissajous excitation)
        start_s = idx * step_samples
        end_s = start_s + step_samples
        sig_segment = audio_samples[start_s:end_s] if end_s <= len(audio_samples) else np.zeros(step_samples)
        
        # Draw active connection line
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        draw.line([speaker_pos["x"], speaker_pos["y"], listener_pos["x"], listener_pos["y"]], fill=(255, 255, 255, 100), width=2)
        
        # Draw 5 Teddy Bears
        for name, bear in bears.items():
            cx, cy = bear["x"], bear["y"]
            color = bear["color"]
            
            # Procedural 2D Teddy Bear outlines
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
            
            # Compute convolved Lissajous shape if active speaker (19D matrix approximation)
            if name == step["speaker"] and len(sig_segment) > 100:
                # Derive mock state registers from bear identity hash
                h_val = int(hashlib.md5(name.encode('utf-8')).hexdigest()[:8], 16)
                f_w = (h_val % 4) + 1
                f_x = ((h_val >> 4) % 5) + 1
                f_y = ((h_val >> 8) % 4) + 1
                f_z = ((h_val >> 12) % 5) + 1
                f_e = ((h_val >> 16) % 3) + 1
                
                # Phase shift parameters
                phi_base = 0.5
                spin_velocity = (step["pitch"] / 220.0) * (idx * 0.1)
                phase_w = spin_velocity * 0.05 + phi_base
                phase_x = spin_velocity * 0.06 + phi_base
                phase_y = spin_velocity * 0.07 + phi_base
                
                steps_count = 100
                delay = int(8 * f_e)
                
                proj_points = []
                for i in range(steps_count):
                    theta = (i * 2.0 * math.pi) / steps_count
                    
                    # Read segment amplitudes (sig_now and sig_delayed)
                    sig_now = sig_segment[i] * 1.5 if i < len(sig_segment) else 0.0
                    sig_delayed = sig_segment[i - delay] * 1.5 if (i >= delay and i - delay < len(sig_segment)) else 0.0
                    
                    eta = ((theta * f_w + phase_w + sig_now * 0.5) * 0.5) % (math.pi / 2.0)
                    xi1 = ((theta * f_x + phase_x + sig_delayed * 0.5) * 2) % (2.0 * math.pi)
                    xi2 = ((theta * f_y + phase_y) * 3) % (2.0 * math.pi)
                    
                    qx_g = math.cos(eta) * math.sin(xi1)
                    qy_g = math.sin(eta) * math.cos(xi2)
                    
                    # 19D Convolution mapping math (Hopf fibration projections)
                    lx = (qx_g * sig_now + qy_g * sig_delayed) * 80.0
                    ly = (-qy_g * sig_now + qx_g * sig_delayed) * 80.0
                    lz = lz_val = 20.0 * math.cos(f_z * theta)
                    
                    # Project coordinates
                    zoom = 0.90
                    z_new = lz + 200.0
                    px = cx + int((lx * 200.0) / z_new * zoom)
                    py = cy + int((ly * 200.0) / z_new * zoom)
                    proj_points.append((px, py))
                    
                for i in range(steps_count):
                    p1 = proj_points[i]
                    p2 = proj_points[(i + 1) % steps_count]
                    draw.line([p1, p2], fill=(0, 255, 200, 180), width=2)
                    
        # Overlay Status labels
        draw.text((20, 20), "AUNCIENT LAU 19D CONVOLVED LISSAJOUS MATRIX", fill="#ff007f")
        draw.text((20, 35), f"Step: {idx + 1}/313 | Active: {step['speaker']} -> {step['listener']}", fill="#00f2fe")
        draw.text((20, 50), f"Frequency Mod: {step['pitch']:.1f} Hz | Voltage: {step['voltage']:.3f} V", fill="#ffd700")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
