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
    
    # Read wave file audio samples
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
                # Limit to match the first 12 transitions
                if len(log_data) < 12:
                    log_data.append({
                        "speaker": match.group(1),
                        "listener": match.group(2),
                        "voltage": float(match.group(4)),
                        "pitch": float(match.group(5))
                    })
                
    if not log_data:
        log_data = [{"speaker": "Trusty", "listener": "Coop", "voltage": 1.0, "pitch": 220.0}] * 12
        
    bears = {
        "Trusty":  {"x": 120, "y": 240, "color": (50, 150, 255)},
        "Aggro":   {"x": 520, "y": 240, "color": (255, 50, 50)},
        "Skeptic": {"x": 220, "y": 120, "color": (200, 200, 5)},
        "Eerie":   {"x": 420, "y": 120, "color": (150, 50, 200)},
        "Coop":    {"x": 320, "y": 360, "color": (50, 200, 100)}
    }
    
    bear_addresses = {
        "Trusty":  "0x1111111111111111111111111111111111111111",
        "Aggro":   "0x2222222222222222222222222222222222222222",
        "Skeptic": "0x3333333333333333333333333333333333333333",
        "Eerie":   "0x4444444444444444444444444444444444444444",
        "Coop":    "0x5555555555555555555555555555555555555555"
    }
    
    WIDTH, HEIGHT = 640, 480
    FPS = 10
    
    duration_per_turn = 2.5
    pause_duration = 0.5
    total_duration = (duration_per_turn + pause_duration) * len(log_data)
    total_frames = int(total_duration * FPS)
    
    step_samples = int(sample_rate * duration_per_turn)
    pause_samples = int(sample_rate * pause_duration)
    turn_samples = step_samples + pause_samples
    
    for frame_idx in range(total_frames):
        time_sec = frame_idx / float(FPS)
        
        # Determine current active conversation index
        turn_idx = int(time_sec // (duration_per_turn + pause_duration))
        if turn_idx >= len(log_data):
            turn_idx = len(log_data) - 1
            
        step = log_data[turn_idx]
        
        # Check if currently in speaking window or cognitive pause window
        time_in_turn = time_sec % (duration_per_turn + pause_duration)
        is_speaking = (time_in_turn < duration_per_turn)
        
        img = Image.new("RGB", (WIDTH, HEIGHT), (12, 14, 20))
        draw = ImageDraw.Draw(img, "RGBA")
        
        # Grid overlay
        for x_line in range(0, WIDTH, 40):
            draw.line([(x_line, 0), (x_line, HEIGHT)], fill=(18, 20, 28), width=1)
        for y_line in range(0, HEIGHT, 40):
            draw.line([(0, y_line), (WIDTH, y_line)], fill=(18, 20, 28), width=1)
            
        # Extract active audio samples for the current frame to drive connection oscilloscope wave
        frame_start_sample = int(time_sec * sample_rate)
        frame_end_sample = frame_start_sample + int(sample_rate / FPS)
        sig_frame = audio_samples[frame_start_sample:frame_end_sample] if frame_end_sample <= len(audio_samples) else np.zeros(int(sample_rate / FPS))
        
        # Draw active dialogue connection line as an audio oscilloscope wave
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        
        if is_speaking and len(sig_frame) > 0:
            osc_points = []
            num_points = 40
            for pt in range(num_points):
                ratio = pt / float(num_points - 1)
                amp = sig_frame[int(ratio * (len(sig_frame) - 1))] * 60.0 # amplitude scale
                # Normal vector for perpendicular oscilloscope wave displacment
                dx = listener_pos["x"] - speaker_pos["x"]
                dy = listener_pos["y"] - speaker_pos["y"]
                dist = math.sqrt(dx*dx + dy*dy)
                if dist > 0.1:
                    nx = -dy / dist
                    ny = dx / dist
                else:
                    nx, ny = 0, 1
                    
                rx = speaker_pos["x"] + dx * ratio + nx * amp
                ry = speaker_pos["y"] + dy * ratio + ny * amp
                osc_points.append((rx, ry))
            draw.line(osc_points, fill=(255, 255, 255, 180), width=2)
            
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
            
            # Draw quality LAU Lissajous rendering orbiting active speaker during turn
            if is_speaking and name == step["speaker"] and len(sig_frame) > 0:
                address = bear_addresses.get(name, "0x1111111111111111111111111111111111111111")
                addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
                
                fx = 1.0 + (int(addr_hash[0:2], 16) % 5)
                fy = 1.0 + (int(addr_hash[2:4], 16) % 5)
                fz = 1.0 + (int(addr_hash[4:6], 16) % 5)
                phi = (int(addr_hash[6:8], 16) % 100) / 100.0 * 2.0 * math.pi
                
                steps_count = 100
                delay = int(8 * (fx + 1))
                proj_points = []
                
                # Fetch local sound segment matching the frame step offset
                sample_offset = turn_idx * turn_samples + int(time_in_turn * sample_rate)
                sig_segment = audio_samples[sample_offset : sample_offset + steps_count] if sample_offset + steps_count <= len(audio_samples) else np.zeros(steps_count)
                
                for i in range(steps_count):
                    theta = (i * 2.0 * math.pi) / steps_count
                    sig_now = sig_segment[i] * 1.5 if i < len(sig_segment) else 0.0
                    sig_delayed = sig_segment[i - delay] * 1.5 if (i >= delay and i - delay < len(sig_segment)) else 0.0
                    
                    eta = ((theta * fx + time_sec * 0.5 + sig_now * 0.5) * 0.5) % (math.pi / 2.0)
                    xi1 = ((theta * fy + time_sec * 0.6 + sig_delayed * 0.5) * 2) % (2.0 * math.pi)
                    xi2 = ((theta * fz + time_sec * 0.7) * 3) % (2.0 * math.pi)
                    
                    qx_g = math.cos(eta) * math.sin(xi1)
                    qy_g = math.sin(eta) * math.cos(xi2)
                    
                    lx = (qx_g * sig_now + qy_g * sig_delayed) * 80.0
                    ly = (-qy_g * sig_now + qx_g * sig_delayed) * 80.0
                    lz = 20.0 * math.cos(fz * theta)
                    
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
        draw.text((20, 35), f"Step: {turn_idx + 1}/12 | Active: {step['speaker']} -> {step['listener']}", fill="#00f2fe")
        draw.text((20, 50), f"Frequency Mod: {step['pitch']:.1f} Hz | Voltage: {step['voltage']:.3f} V | State: {'TALKING' if is_speaking else 'COGNITIVE_PAUSE'}", fill="#ffd700")
        
        frame_path = os.path.join(output_dir, f"frame_{frame_idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {total_frames} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
