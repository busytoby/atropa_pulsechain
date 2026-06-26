#!/usr/bin/env python3
import os
import sys
import math
import hashlib
import numpy as np
import wave
import struct
import subprocess
from PIL import Image, ImageDraw

def hsl_to_rgb(h, s, l):
    c = (1.0 - abs(2.0 * l - 1.0)) * s
    x = c * (1.0 - abs((h / 60.0) % 2.0 - 1.0))
    m = l - c / 2.0
    if 0 <= h < 60:
        r, g, b = c, x, 0.0
    elif 60 <= h < 120:
        r, g, b = x, c, 0.0
    elif 120 <= h < 180:
        r, g, b = 0.0, c, x
    elif 180 <= h < 240:
        r, g, b = 0.0, x, c
    elif 240 <= h < 300:
        r, g, b = x, 0.0, c
    else:
        r, g, b = c, 0.0, x
    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

def project_3d(x, y, z, cam_x, cam_y, cam_z, yaw, pitch, zoom, width, height):
    dx = x - cam_x
    dy = y - cam_y
    dz = z - cam_z
    
    cos_y, sin_y = math.cos(yaw), math.sin(yaw)
    rx = dx * cos_y - dz * sin_y
    ry = dy
    rz = dx * sin_y + dz * cos_y
    
    cos_p, sin_p = math.cos(pitch), math.sin(pitch)
    x_new = rx
    y_new = ry * cos_p - rz * sin_p
    z_new = ry * sin_p + rz * cos_p
    
    focal = 500.0
    if z_new == 0: z_new = 1
    px = (x_new * focal) / (z_new + 700) * zoom + width / 2
    py = (y_new * focal) / (z_new + 700) * zoom + height / 2
    return int(px), int(py)

def main():
    address = "0xd32c39fee49391c7952d1b30b15921b0d3b42e69"
    addr_hash = hashlib.md5(address.encode('utf-8')).hexdigest()
    hue = (int(addr_hash[10:13], 16) % 360)
    color_rgb = hsl_to_rgb(hue, 0.90, 0.55)
    
    # Load cache registers
    r_base, r_channel, r_dynamo, r_foundation = 444832986301548, 370237741492560, 427925246710971, 750914752357745
    
    fx = 1.0 + (r_channel % 5)
    fy = 1.0 + (r_dynamo % 5)
    fz = 1.0 + (r_foundation % 5)
    phi = (r_base % 100) / 100.0 * 2.0 * math.pi
    
    R_hyp = 100.0 + (r_foundation % 80)
    r_hyp = 20.0 + (r_channel % 40)
    d_hyp = 30.0 + (r_dynamo % 60)
    
    WIDTH, HEIGHT = 1280, 720
    FPS = 30
    DURATION = 90
    TOTAL_FRAMES = FPS * DURATION
    SAMPLE_RATE = 44100
    
    print("[Pipeline] Generating EDO-22 audio track...")
    audio_path = "tmp/yi_lau_evolution.wav"
    os.makedirs("tmp", exist_ok=True)
    
    # Generate 90s audio
    total_samples = SAMPLE_RATE * DURATION
    audio = np.zeros(total_samples, dtype=np.float32)
    t_arr = np.linspace(0, DURATION, total_samples, endpoint=False)
    
    # Microtonal EDO-22 sweeping carrier
    root_freq = 110.0 # A2 frequency base
    edo_freq = root_freq * (2 ** (12 / 22)) # EDO-22 octave step
    
    # Phase modulating carrier based on YI -> LAU transition stages
    modulation = 0.5 + 2.5 * np.clip((t_arr - 30) / 30, 0, 1)
    audio = 0.15 * np.sin(2 * np.pi * edo_freq * t_arr + modulation * np.sin(2 * np.pi * (root_freq / 2) * t_arr))
    
    # Write audio
    with wave.open(audio_path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        packed = struct.pack(f"<{len(audio)}h", *(int(s * 32767) for s in audio))
        wf.writeframes(packed)
        
    print("[Pipeline] Encoding video frames directly to FFmpeg...")
    output_video = "yi_lau_evolution_demo.mp4"
    
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "rawvideo",
        "-pix_fmt", "rgb24",
        "-s", f"{WIDTH}x{HEIGHT}",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-shortest",
        output_video
    ]
    
    pipe = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    for f in range(TOTAL_FRAMES):
        t = f / TOTAL_FRAMES
        # Morph phases: 
        # t = 0.0 -> 0.33: YI Icon pure
        # t = 0.33 -> 0.66: YI Morphs into hypotrochoid LAU
        # t = 0.66 -> 1.0: LAU solidifies with textured bamboo segments
        
        morph_factor = 0.0
        if t > 0.30:
            morph_factor = min(1.0, (t - 0.30) / 0.35)
            
        bg = Image.new("RGB", (WIDTH, HEIGHT), (12, 16, 28))
        draw = ImageDraw.Draw(bg, "RGBA")
        
        # Camera orbit
        cam_yaw = f * 0.02
        cam_pitch = 0.3 + 0.1 * math.sin(f * 0.01)
        cam_x = math.cos(cam_yaw) * 320
        cam_y = math.sin(cam_yaw) * 320
        cam_z = 200 + 50 * math.cos(f * 0.005)
        zoom = 1.0
        
        num_points = 400
        proj_points = []
        normals = []
        
        # Define layered offsets for d
        d_offsets = [d_hyp * 0.7, d_hyp, d_hyp * 1.3]
        
        for idx, d_val in enumerate(d_offsets):
            points_layer = []
            for i in range(num_points):
                theta = i * 4.0 * math.pi / num_points
                
                # YI coordinate path (Lissajous)
                lx_yi = 135.0 * math.sin(fx * theta + phi)
                ly_yi = 135.0 * math.sin(fy * theta)
                lz_yi = 135.0 * math.cos(fz * theta)
                
                # LAU coordinate path (Hypotrochoid)
                lx_lau = (R_hyp - r_hyp) * math.cos(theta) + d_val * math.cos(((R_hyp - r_hyp) / r_hyp) * theta + phi)
                ly_lau = (R_hyp - r_hyp) * math.sin(theta) - d_val * math.sin(((R_hyp - r_hyp) / r_hyp) * theta)
                lz_lau = 80.0 * math.cos(5.0 * theta)
                
                # Linearly interpolate coordinate values for morphing
                lx = lx_yi * (1.0 - morph_factor) + lx_lau * morph_factor
                ly = ly_yi * (1.0 - morph_factor) + ly_lau * morph_factor
                lz = lz_yi * (1.0 - morph_factor) + lz_lau * morph_factor
                
                px, py = project_3d(lx, ly, lz, cam_x, cam_y, cam_z, cam_yaw, cam_pitch, zoom, WIDTH, HEIGHT)
                points_layer.append((px, py))
                
            # Draw segment lines with bamboo highlights & shadows
            w = 3 if idx == 1 else 1
            for i in range(num_points):
                p1 = points_layer[i]
                p2 = points_layer[(i + 1) % num_points]
                
                # 3D Embossed look
                p1_sh = (p1[0] + 1, p1[1] + 1)
                p2_sh = (p2[0] + 1, p2[1] + 1)
                p1_hl = (p1[0] - 1, p1[1] - 1)
                p2_hl = (p2[0] - 1, p2[1] - 1)
                
                draw.line([p1_sh, p2_sh], fill=(0, 0, 0, 100), width=w)
                draw.line([p1_hl, p2_hl], fill=(255, 255, 255, 60), width=w)
                draw.line([p1, p2], fill=color_rgb + (220,), width=w)
                
        # Write image bytes to FFmpeg stdin pipe
        pipe.stdin.write(bg.tobytes())
        if f % 300 == 0:
            print(f"[Pipeline] Processed {f}/{TOTAL_FRAMES} frames...")
            
    pipe.stdin.close()
    pipe.wait()
    print(f"[Pipeline] Video compilation complete: {output_video}")

if __name__ == "__main__":
    main()
