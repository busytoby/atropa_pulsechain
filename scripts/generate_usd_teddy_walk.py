import os
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 15.0  # 15 seconds walking animation
total_frames = int(DURATION * FPS)

def generate_soundtrack(wav_path):
    print("[DSP] Synthesizing cute teddy walk walk-beat soundtrack...")
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    
    # 120 BPM walk rhythm
    beat_dur = 60.0 / 120.0
    beat_samples = int(SAMPLE_RATE * beat_dur)
    
    for s in range(total_samples):
        t_sec = s / float(SAMPLE_RATE)
        
        # Thumpin synthesizer bass growl on beats
        beat_idx = int(s // beat_samples)
        phase_in_beat = (s % beat_samples) / float(beat_samples)
        
        # Bass frequency (F note = 87.3 Hz)
        bass_freq = 87.3
        bass_env = math.exp(-phase_in_beat * 5.0)
        # Sine wave with harmonics for growl
        bass_val = math.sin(2.0 * math.pi * bass_freq * t_sec) + 0.3 * math.sin(2.0 * math.pi * bass_freq * 2.0 * t_sec)
        
        # Melodic glockenspiel pluck on alternate beats
        pluck_val = 0.0
        if beat_idx % 2 == 1:
            pluck_freq = 880.0 if (beat_idx % 4 == 1) else 1046.5
            pluck_env = math.exp(-phase_in_beat * 15.0)
            pluck_val = math.sin(2.0 * math.pi * pluck_freq * t_sec) * pluck_env * 0.4
            
        audio[s] = (bass_val * bass_env * 0.45) + pluck_val
        
    # Write WAV file
    with wave.open(wav_path, "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        for sample in audio:
            sample_val = int(max(-32768, min(32767, sample * 32767)))
            wav_file.writeframes(struct.pack('<h', sample_val))

def rotate_x(x, y, z, angle):
    c = math.cos(angle)
    s = math.sin(angle)
    return x, y * c - z * s, y * s + z * c

def rotate_y(x, y, z, angle):
    c = math.cos(angle)
    s = math.sin(angle)
    return x * c + z * s, y, -x * s + z * c

def rotate_z(x, y, z, angle):
    c = math.cos(angle)
    s = math.sin(angle)
    return x * c - y * s, x * s + y * c, z

def get_teddy_geometry(time_sec):
    # Walk parameters: frequency = 1.5 Hz
    walk_freq = 1.5
    phase = time_sec * 2.0 * math.pi * walk_freq
    
    # Forward translation: walks from left to right across viewport
    # Range: -5.0 to 5.0
    progress = time_sec / DURATION
    x_disp = -4.0 + 8.0 * progress
    
    # Bobbing up and down
    y_bob = 0.1 * abs(math.sin(phase))
    
    # Left/Right leg swings (out of phase)
    theta_lleg = 0.45 * math.sin(phase)
    theta_rleg = -0.45 * math.sin(phase)
    
    # Left/Right arm swings (opposite legs)
    theta_larm = -0.35 * math.sin(phase)
    theta_rarm = 0.35 * math.sin(phase)
    
    # Body orientation (angled slightly towards direction of walk)
    body_yaw = 0.15 * math.sin(phase * 0.1) + 1.2
    
    parts = {}
    
    # 1. Main body translation
    parts["Body"] = {
        "pos": (x_disp, 0.5 + y_bob, 0.0),
        "rot": (0.0, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.7, 0.85, 0.6),
        "color": (120, 80, 54) # Brown
    }
    
    # 2. Head (local translation relative to body)
    parts["Head"] = {
        "pos": (x_disp, 1.45 + y_bob, 0.0),
        "rot": (0.05 * math.sin(phase), body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.55, 0.55, 0.5),
        "color": (120, 80, 54)
    }
    
    # 3. Ears
    parts["LeftEar"] = {
        "pos": (x_disp - 0.4, 1.85 + y_bob, 0.0),
        "rot": (0.0, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    parts["RightEar"] = {
        "pos": (x_disp + 0.4, 1.85 + y_bob, 0.0),
        "rot": (0.0, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    
    # 4. Left Arm (swings back and forth)
    # Hinge point: (x_disp - 0.6, 0.8 + y_bob, 0.0)
    al_x, al_y, al_z = rotate_y(0.0, -0.4, 0.0, body_yaw)
    al_x, al_y, al_z = rotate_z(al_x, al_y, al_z, theta_larm)
    parts["LeftArm"] = {
        "pos": (x_disp - 0.65 + al_x, 0.8 + y_bob + al_y, al_z),
        "rot": (theta_larm, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.45, 0.2),
        "color": (120, 80, 54)
    }
    
    # 5. Right Arm
    ar_x, ar_y, ar_z = rotate_y(0.0, -0.4, 0.0, body_yaw)
    ar_x, ar_y, ar_z = rotate_z(ar_x, ar_y, ar_z, theta_rarm)
    parts["RightArm"] = {
        "pos": (x_disp + 0.65 + ar_x, 0.8 + y_bob + ar_y, ar_z),
        "rot": (theta_rarm, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.45, 0.2),
        "color": (120, 80, 54)
    }
    
    # 6. Left Leg (swings opposite)
    ll_x, ll_y, ll_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    ll_x, ll_y, ll_z = rotate_z(ll_x, ll_y, ll_z, theta_lleg)
    parts["LeftLeg"] = {
        "pos": (x_disp - 0.35 + ll_x, 0.1 + y_bob + ll_y, ll_z),
        "rot": (theta_lleg, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.25, 0.5, 0.25),
        "color": (100, 65, 40)
    }
    
    # 7. Right Leg
    lr_x, lr_y, lr_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    lr_x, lr_y, lr_z = rotate_z(lr_x, lr_y, lr_z, theta_rleg)
    parts["RightLeg"] = {
        "pos": (x_disp + 0.35 + lr_x, 0.1 + y_bob + lr_y, lr_z),
        "rot": (theta_rleg, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.25, 0.5, 0.25),
        "color": (100, 65, 40)
    }
    
    return parts

def generate_ellipsoid_mesh(size, num_segments=12):
    # Generates 3D points on an ellipsoid
    sx, sy, sz = size
    vertices = []
    for i in range(num_segments):
        lat = (i / float(num_segments - 1)) * math.pi - math.pi / 2.0
        cos_lat = math.cos(lat)
        sin_lat = math.sin(lat)
        for j in range(num_segments):
            lon = (j / float(num_segments)) * 2.0 * math.pi
            vx = sx * cos_lat * math.cos(lon)
            vy = sy * sin_lat
            vz = sz * cos_lat * math.sin(lon)
            vertices.append((vx, vy, vz))
    return vertices

def main():
    wav_output = "temp_teddy_walk_track.wav"
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_walk_demo.mp4"
    generate_soundtrack(wav_output)
    
    width, height = 640, 480
    
    # Initialize FFmpeg pipeline
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", wav_output,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480",
        "-c:a", "aac",
        "-shortest",
        video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    # Track joint translation time samples for USD export
    usd_samples = {}
    
    for f in range(total_frames):
        time_sec = f / float(FPS)
        parts = get_teddy_geometry(time_sec)
        
        # Render frame
        img = Image.new("RGB", (width, height), (15, 10, 25))
        draw = ImageDraw.Draw(img)
        
        # Draw dynamic background grid
        grid_pos = int(time_sec * 60.0) % 80
        for x_g in range(-80, width + 80, 80):
            draw.line([(x_g - grid_pos, 0), (x_g - grid_pos, height)], fill=(30, 20, 50), width=1)
        for y_g in range(0, height, 60):
            draw.line([(0, y_g), (width, y_g)], fill=(30, 20, 50), width=1)
            
        # Draw perspective floor
        draw.rectangle([0, 320, width, height], fill=(10, 5, 20))
        for x_floor in range(-200, width + 200, 50):
            draw.line([(320, 320), (x_floor, height)], fill=(50, 30, 80), width=2)
            
        # Collect and project 3D parts
        # For simple sorting/rendering, draw back parts first
        draw_order = ["LeftLeg", "RightLeg", "LeftArm", "RightArm", "Body", "Head", "LeftEar", "RightEar"]
        
        for part_name in draw_order:
            part = parts[part_name]
            px, py, pz = part["pos"]
            rx_ang, ry_ang, rz_ang = part["rot"]
            
            # Save USDA time samples
            if part_name not in usd_samples:
                usd_samples[part_name] = []
            usd_samples[part_name].append((f, px, py, pz, rx_ang, ry_ang, rz_ang))
            
            # Generate ellipsoid vertices
            verts = generate_ellipsoid_mesh(part["size"])
            projected_pts = []
            
            for vx, vy, vz in verts:
                # Rotate
                vx, vy, vz = rotate_x(vx, vy, vz, rx_ang)
                vx, vy, vz = rotate_y(vx, vy, vz, ry_ang)
                vx, vy, vz = rotate_z(vx, vy, vz, rz_ang)
                
                # Global translation
                gx = vx + px
                gy = vy + py
                gz = vz + pz + 4.5 # Push back in Z
                
                # Perspective Projection
                fov = 420.0
                screen_x = int(width / 2.0 + (gx * fov) / gz)
                screen_y = int(height / 2.0 - (gy * fov) / gz)
                projected_pts.append((screen_x, screen_y))
                
            # Draw mesh wireframe
            outline_color = (255, 255, 255) # High visibility white border outline
            fill_color = part["color"]
            
            # Draw polygon mesh segments
            num_seg = 12
            for i in range(num_seg - 1):
                for j in range(num_seg):
                    p1_idx = i * num_seg + j
                    p2_idx = i * num_seg + ((j + 1) % num_seg)
                    p3_idx = (i + 1) * num_seg + ((j + 1) % num_seg)
                    p4_idx = (i + 1) * num_seg + j
                    
                    poly = [projected_pts[p1_idx], projected_pts[p2_idx], projected_pts[p3_idx], projected_pts[p4_idx]]
                    # Draw flat shading with light outline
                    draw.polygon(poly, fill=fill_color, outline=outline_color)
                    
        # Onscreen HUD
        draw.text((20, 20), "USD TEDDY BEAR WALKER DEMO", fill=(255, 215, 0))
        draw.text((20, 35), f"FRAME: {f:03d} / {total_frames:03d}", fill=(0, 255, 255))
        draw.text((20, 50), f"X-POS: {parts['Body']['pos'][0]:.3f}", fill=(0, 255, 0))
        
        # Pipe frame
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Teddy walking video rendered: {video_output}")
    
    # Export Pixar USD ASCII (USDA) file mapping the walking joints hierarchy
    usda_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_walk_scene.usda"
    with open(usda_output, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    startTimeCode = 0\n")
        f.write(f"    endTimeCode = {total_frames - 1}\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        f.write("def Xform \"TeddyWalkerScene\"\n")
        f.write("{\n")
        
        for part_name, samples in usd_samples.items():
            f.write(f"    def Xform \"{part_name}\"\n")
            f.write("    {\n")
            f.write("        double3 xformOp:translate.timeSamples = {\n")
            for f_idx, px, py, pz, rx, ry, rz in samples:
                f.write(f"            {f_idx}: ({px:.4f}, {py:.4f}, {pz:.4f}),\n")
            f.write("        }\n")
            f.write("        double3 xformOp:rotateXYZ.timeSamples = {\n")
            for f_idx, px, py, pz, rx, ry, rz in samples:
                f.write(f"            {f_idx}: ({rx * 57.2958:.4f}, {ry * 57.2958:.4f}, {rz * 57.2958:.4f}),\n")
            f.write("        }\n")
            f.write("        uniform token[] xformOpOrder = [\"xformOp:translate\", \"xformOp:rotateXYZ\"]\n")
            f.write("    }\n")
            
        f.write("}\n")
    print(f"[SUCCESS] Pixar USDA scene description exported: {usda_output}")
    
    # Clean up temp WAV
    try:
        os.remove(wav_output)
    except Exception:
        pass

if __name__ == "__main__":
    main()
