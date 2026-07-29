import os
import sys
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont

SAMPLE_RATE = 44100
DURATION_SECS = 15
FPS = 30
WIDTH = 1280
HEIGHT = 720
TOTAL_FRAMES = DURATION_SECS * FPS

def generate_psg_synth_audio(filename):
    print("[AUDIO] Synthesizing retro FM spool soundtrack...")
    num_samples = SAMPLE_RATE * DURATION_SECS
    with wave.open(filename, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        samples = []
        for i in range(num_samples):
            t = i / SAMPLE_RATE
            # Heartbeat double pulse trigger
            beat_cycle = t % 1.0
            amp_envelope = 0.0
            if beat_cycle < 0.15:
                amp_envelope = math.exp(-15.0 * beat_cycle)
            elif beat_cycle >= 0.20 and beat_cycle < 0.35:
                amp_envelope = 0.7 * math.exp(-15.0 * (beat_cycle - 0.20))
            
            # Bass pulse frequency
            freq = 60.0
            mod = math.sin(2.0 * math.pi * 120.0 * t) * 1.5
            carrier = math.sin(2.0 * math.pi * freq * t + mod)
            bass = math.tanh(carrier * 2.0) * amp_envelope

            # Cute arpeggiator note sweep
            notes = [130.81, 164.81, 196.00, 220.00] # C3, E3, G3, A3
            step = int(t * 6) % 4
            melody = math.sin(2.0 * math.pi * notes[step] * t) * 0.15 * amp_envelope

            mixed = bass + melody
            sample_val = int(mixed * 16384.0)
            samples.append(struct.pack("<h", sample_val))
        wf.writeframes(b"".join(samples))
    print("[AUDIO] Synth completed.")

def main():
    print("=== STARTING SKELETON-HASP-BOOK RENDERER ===")
    audio_path = "temp_spool_melody.wav"
    generate_psg_synth_audio(audio_path)
    
    artifact_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/dd413f30-1457-4127-8d11-415844cbbc2d"
    os.makedirs(artifact_dir, exist_ok=True)
    video_output = os.path.join(artifact_dir, "skeleton_hasp_book_demo.mp4")
    
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    for frame_idx in range(TOTAL_FRAMES):
        t = frame_idx / FPS
        img = Image.new("RGB", (WIDTH, HEIGHT), color=(10, 15, 24)) # Dark sci-fi cyan hue
        draw = ImageDraw.Draw(img)
        
        # Parallax background grid representing virtual spool memory blocks
        scroll_y = int(t * 40.0) % 64
        for y in range(-scroll_y, HEIGHT + 64, 64):
            draw.line([(0, y), (WIDTH, y)], fill=(16, 28, 48), width=1)
        for x in range(0, WIDTH, 64):
            draw.line([(x, 0), (x, HEIGHT)], fill=(16, 28, 48), width=1)
            
        # Draw 6-bit Fieldata word boxes pulsing
        for i in range(12):
            box_x = 80 + i * 96
            box_y = 120 + int(math.sin(t * 3.0 + i) * 15.0)
            draw.rectangle([box_x, box_y, box_x + 80, box_y + 40], outline=(0, 255, 255), width=2)
            # Render a simulated 6-bit code string
            draw.text((box_x + 10, box_y + 12), f"FLD{i:02X}", fill=(200, 255, 255))
            
        # Render a wireframe quaternion geometry mapping DisplacementShader constraints
        # Rotation angles updated by time
        angle = t * 1.2
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        
        # Wireframe vertices (cube mapping)
        vertices = []
        raw_coords = [
            (-1, -1, -1), (1, -1, -1), (1, 1, -1), (-1, 1, -1),
            (-1, -1, 1), (1, -1, 1), (1, 1, 1), (-1, 1, 1)
        ]
        
        # Scale and project vertices under wrap register boundary constraints
        scale_val = 140.0 + math.sin(t * 4.0) * 20.0
        for x, y, z in raw_coords:
            # 3D Rotation along Y and X axes
            x_rot = x * cos_a - z * sin_a
            z_rot = x * sin_a + z * cos_a
            y_rot = y * cos_a - z_rot * sin_a
            
            # Perspective projection
            proj_x = int(WIDTH // 2 + x_rot * scale_val)
            proj_y = int(HEIGHT // 2 + y_rot * scale_val)
            vertices.append((proj_x, proj_y))
            
        # Render wireframe connections (solid cyan when program debug halt conditions exist)
        edges = [
            (0,1), (1,2), (2,3), (3,0),
            (4,5), (5,6), (6,7), (7,4),
            (0,4), (1,5), (2,6), (3,7)
        ]
        
        edge_color = (0, 255, 255) # Solid cyan
        for u, v in edges:
            draw.line([vertices[u], vertices[v]], fill=edge_color, width=2)
            
        # Draw highlighted vertex spheres mirroring RiInterface calls
        for px, py in vertices:
            draw.ellipse([px - 6, py - 6, px + 6, py + 6], fill=(0, 255, 255), outline=(255, 255, 255))
            
        # HUD overlays listing the active compilation stages
        draw.text((50, 520), "PIXAR RENDERMAN COMPLIANCE PREVIEWER", fill=(255, 255, 255))
        draw.text((50, 540), "PIPELINE   : SKELETON-HASP-BOOK (CONCURRENT SPOOL)", fill=(0, 255, 255))
        draw.text((50, 560), "ENCODING   : 6-BIT FIELDATA ALIGNED", fill=(200, 200, 255))
        draw.text((50, 580), "CALIBRATION: DisplacementShader REG_DISP_X (SYNCED)", fill=(0, 255, 100))
        draw.text((50, 600), f"DURATION   : {t:.2f}s / {DURATION_SECS}s", fill=(150, 150, 150))
        
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    if os.path.exists(audio_path):
        os.remove(audio_path)
    print("=== VIDEO GENERATION COMPLETE ===")
    print(f"[SUCCESS] Saved at: {video_output}")

if __name__ == "__main__":
    main()
