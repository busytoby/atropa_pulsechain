import os
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 24.0  # 24 seconds ballet sequence (8 movements * 3s)
total_frames = int(DURATION * FPS)

def generate_ballet_soundtrack(wav_path):
    print("[DSP] Synthesizing beautiful classical ballet synth-orchestra soundtrack...")
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    
    # 3/4 waltz time (90 BPM)
    beat_dur = 60.0 / 90.0
    beat_samples = int(SAMPLE_RATE * beat_dur)
    
    # Lydian waltz chord progression
    melody_notes = [
        [261.6, 293.7, 329.6, 392.0], # C Lydian
        [220.0, 246.9, 277.2, 329.6], # A Lydian
        [329.6, 392.0, 440.0, 523.3], # E minor/Lydian
        [523.3, 587.3, 659.3, 784.0], # High C
        [392.0, 440.0, 493.9, 587.3], # G Lydian
        [440.0, 493.9, 554.4, 659.3], # F# minor
        [523.3, 659.3, 784.0, 1046.5], # C major run
        [261.6, 329.6, 392.0, 523.3]  # Resolution
    ]
    
    for s in range(total_samples):
        t_sec = s / float(SAMPLE_RATE)
        movement_idx = int(t_sec // 3.0) % 8
        chord = melody_notes[movement_idx]
        
        beat_idx = int(s // beat_samples)
        beat_pos = (s % beat_samples) / float(beat_samples)
        
        sig = 0.0
        if beat_idx % 3 == 0:
            root_f = chord[0] * 0.5
            env = math.exp(-beat_pos * 4.0)
            sig += math.sin(2.0 * math.pi * root_f * t_sec) * env * 0.5
        else:
            env = math.exp(-beat_pos * 8.0)
            for f in chord[1:]:
                sig += math.sin(2.0 * math.pi * f * t_sec) * env * 0.12
                
        audio[s] = sig
        
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

def get_ballet_geometry(time_sec):
    movement = int(time_sec // 3.0)
    m_time = time_sec % 3.0
    m_progress = m_time / 3.0
    
    x_disp = 0.0
    y_disp = 0.05
    z_disp = 0.0
    
    body_pitch = 0.0
    body_yaw = 1.0
    body_roll = 0.0
    
    # Standard Leg parameters
    t_lthigh, t_rthigh = 0.0, 0.0
    t_lknee, t_rknee = 0.0, 0.0
    
    # Standard Arm parameters
    t_luarm, t_ruarm = 0.2, 0.2
    t_lelbow, t_relbow = 0.3, 0.3
    
    m_name = "Ouverture"
    
    if movement == 0:
        m_name = "Ouverture (Opening)"
        t_luarm = t_ruarm = -0.5 * m_progress + 0.2 * (1.0 - m_progress)
        t_lelbow = t_relbow = 0.4 * m_progress + 0.3 * (1.0 - m_progress)
        
    elif movement == 1:
        m_name = "Plier (Plie Knee Bend)"
        plie_amp = math.sin(m_progress * math.pi)
        y_disp -= 0.25 * plie_amp
        t_lthigh = t_rthigh = 0.4 * plie_amp
        t_lknee = t_rknee = 0.8 * plie_amp
        t_luarm = t_ruarm = -0.1 * plie_amp - 0.5 * (1.0 - plie_amp)
        t_lelbow = t_relbow = 0.15 * plie_amp + 0.4 * (1.0 - plie_amp)
        
    elif movement == 2:
        m_name = "Relever & Pointe (On Toes)"
        rise_amp = math.sin(m_progress * math.pi)
        y_disp += 0.2 * rise_amp
        t_lthigh = t_rthigh = -0.1 * rise_amp
        t_luarm = t_ruarm = -0.7 * rise_amp
        t_lelbow = t_relbow = 0.5 * rise_amp
        
    elif movement == 3:
        m_name = "Sauter & Batterie (Leap & Beats)"
        leap_phase = m_progress * math.pi
        y_disp += 1.1 * math.sin(leap_phase)
        
        click_amp = 0.0
        if 0.25 < m_progress < 0.75:
            click_amp = math.sin((m_progress - 0.25) * 2.0 * math.pi * 2.0)
            
        t_lthigh = 0.3 * click_amp
        t_rthigh = -0.3 * click_amp
        t_luarm = t_ruarm = -0.8
        t_lelbow = t_relbow = 0.6
        
    elif movement == 4:
        m_name = "Tourner (Pirouette Spin)"
        body_yaw = m_progress * 2.0 * math.pi * 2.0 + 1.0
        t_lthigh = 0.75
        t_lknee = 1.3
        t_rthigh = -0.1
        t_luarm = t_ruarm = -0.2
        t_lelbow = t_relbow = 0.6
        
    elif movement == 5:
        m_name = "Arabesque (Slow Balance)"
        adagio_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.35 * adagio_amp
        t_lthigh = -0.1 * adagio_amp
        t_rthigh = -0.75 * adagio_amp
        t_rknee = 0.3 * adagio_amp
        t_luarm = -0.6 * adagio_amp
        t_ruarm = 0.6 * adagio_amp
        
    elif movement == 6:
        m_name = "Elancer (Darting Glide)"
        x_disp = -2.5 + 5.0 * m_progress
        y_disp += 0.45 * math.sin(m_progress * math.pi)
        t_lthigh = -0.5 * math.sin(m_progress * math.pi)
        t_rthigh = 0.5 * math.sin(m_progress * math.pi)
        t_luarm = -0.5
        t_ruarm = -0.2
        
    elif movement == 7:
        m_name = "Reverence (Bow)"
        bow_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.5 * bow_amp
        y_disp -= 0.15 * bow_amp
        t_lthigh = -0.4 * bow_amp
        t_rthigh = 0.1 * bow_amp
        t_luarm = t_ruarm = 0.4 * bow_amp
        t_lelbow = t_relbow = 0.1 * bow_amp
        
    parts = {}
    
    # 1. Main body translation
    parts["Body"] = {
        "pos": (x_disp, 0.55 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, body_roll),
        "shape": "ellipsoid",
        "size": (0.7, 0.85, 0.6),
        "color": (120, 80, 54)
    }
    
    # 2. Head
    parts["Head"] = {
        "pos": (x_disp, 1.45 + y_disp, z_disp),
        "rot": (body_pitch + 0.05 * math.sin(time_sec * 4.0), body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.55, 0.55, 0.5),
        "color": (120, 80, 54)
    }
    
    # 3. Ears
    parts["LeftEar"] = {
        "pos": (x_disp - 0.4, 1.85 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    parts["RightEar"] = {
        "pos": (x_disp + 0.4, 1.85 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    
    # Helper to register upper arm and forearm segments
    def add_arm(side, uarm_pitch, elbow_pitch, color_u, color_f):
        side_sign = -1.0 if side == "Left" else 1.0
        # Upper Arm
        uarm_rot_pitch = uarm_pitch + body_pitch
        ax, ay, az = rotate_y(0.0, -0.25, 0.0, body_yaw)
        ax, ay, az = rotate_z(ax, ay, az, uarm_rot_pitch)
        
        px = x_disp + side_sign * 0.65 + ax
        py = 0.9 + y_disp + ay
        pz = z_disp + az
        
        parts[f"{side}UpperArm"] = {
            "pos": (px, py, pz),
            "rot": (uarm_rot_pitch, body_yaw, 0.0),
            "shape": "ellipsoid",
            "size": (0.18, 0.3, 0.18),
            "color": color_u
        }
        
        # Forearm
        f_pitch = uarm_rot_pitch + elbow_pitch
        fx, fy, fz = rotate_y(0.0, -0.45, 0.0, body_yaw)
        fx, fy, fz = rotate_z(fx, fy, fz, f_pitch)
        parts[f"{side}Forearm"] = {
            "pos": (x_disp + side_sign * 0.65 + fx, 0.9 + y_disp + fy, z_disp + fz),
            "rot": (f_pitch, body_yaw, 0.0),
            "shape": "ellipsoid",
            "size": (0.15, 0.25, 0.15),
            "color": color_f
        }
        
    # Helper to register thigh and calf segments
    def add_leg(side, thigh_pitch, knee_pitch, color_t, color_c):
        side_sign = -1.0 if side == "Left" else 1.0
        # Thigh
        t_rot_pitch = thigh_pitch + body_pitch
        lx, ly, lz = rotate_y(0.0, -0.25, 0.0, body_yaw)
        lx, ly, lz = rotate_z(lx, ly, lz, t_rot_pitch)
        
        px = x_disp + side_sign * 0.35 + lx
        py = 0.3 + y_disp + ly
        pz = z_disp + lz
        
        parts[f"{side}Thigh"] = {
            "pos": (px, py, pz),
            "rot": (t_rot_pitch, body_yaw, 0.0),
            "shape": "ellipsoid",
            "size": (0.22, 0.35, 0.22),
            "color": color_t
        }
        
        # Calf
        k_pitch = t_rot_pitch - knee_pitch
        cx, cy, cz = rotate_y(0.0, -0.55, 0.0, body_yaw)
        cx, cy, cz = rotate_z(cx, cy, cz, k_pitch)
        parts[f"{side}Calf"] = {
            "pos": (x_disp + side_sign * 0.35 + cx, 0.3 + y_disp + cy, z_disp + cz),
            "rot": (k_pitch, body_yaw, 0.0),
            "shape": "ellipsoid",
            "size": (0.18, 0.3, 0.18),
            "color": color_c
        }

    # Add 2 Arms
    add_arm("Left", t_luarm, t_lelbow, (120, 80, 54), (100, 65, 40))
    add_arm("Right", t_ruarm, t_relbow, (120, 80, 54), (100, 65, 40))
    
    # Add 2 Legs
    add_leg("Left", t_lthigh, t_lknee, (120, 80, 54), (100, 65, 40))
    add_leg("Right", t_rthigh, t_rknee, (120, 80, 54), (100, 65, 40))
    
    return parts, m_name

def generate_ellipsoid_mesh(size, num_segments=10):
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
    wav_output = "temp_teddy_ballet_track.wav"
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_ballet_demo.mp4"
    generate_ballet_soundtrack(wav_output)
    
    width, height = 640, 480
    
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
    usd_samples = {}
    
    for f in range(total_frames):
        time_sec = f / float(FPS)
        parts, current_movement_name = get_ballet_geometry(time_sec)
        
        img = Image.new("RGB", (width, height), (10, 5, 20))
        draw = ImageDraw.Draw(img)
        
        # Spotlights
        for spotlight_x in [100, 320, 540]:
            draw.line([(spotlight_x, 0), (width / 2.0, 320)], fill=(80, 50, 100), width=3)
            
        # Floor
        draw.rectangle([0, 320, width, height], fill=(15, 10, 25))
        for x_floor in range(-200, width + 200, 50):
            draw.line([(320, 320), (x_floor, height)], fill=(90, 40, 110), width=2)
            
        # Depth sorting
        render_queue = []
        for part_name, part in parts.items():
            px, py, pz = part["pos"]
            rotated_z = pz + 4.5
            render_queue.append((rotated_z, part_name))
            
        render_queue.sort(key=lambda item: item[0], reverse=True)
        
        for rz_val, part_name in render_queue:
            part = parts[part_name]
            px, py, pz = part["pos"]
            rx_ang, ry_ang, rz_ang = part["rot"]
            
            if part_name not in usd_samples:
                usd_samples[part_name] = []
            usd_samples[part_name].append((f, px, py, pz, rx_ang, ry_ang, rz_ang))
            
            verts = generate_ellipsoid_mesh(part["size"])
            projected_pts = []
            
            for vx, vy, vz in verts:
                vx, vy, vz = rotate_x(vx, vy, vz, rx_ang)
                vx, vy, vz = rotate_y(vx, vy, vz, ry_ang)
                vx, vy, vz = rotate_z(vx, vy, vz, rz_ang)
                
                gx = vx + px
                gy = vy + py
                gz = vz + pz + 4.5
                
                fov = 420.0
                screen_x = int(width / 2.0 + (gx * fov) / gz)
                screen_y = int(height / 2.0 - (gy * fov) / gz)
                projected_pts.append((screen_x, screen_y))
                
            outline_color = (255, 255, 255)
            fill_color = part["color"]
            
            num_seg = 10
            for i in range(num_seg - 1):
                for j in range(num_seg):
                    p1_idx = i * num_seg + j
                    p2_idx = i * num_seg + ((j + 1) % num_seg)
                    p3_idx = (i + 1) * num_seg + ((j + 1) % num_seg)
                    p4_idx = (i + 1) * num_seg + j
                    
                    poly = [projected_pts[p1_idx], projected_pts[p2_idx], projected_pts[p3_idx], projected_pts[p4_idx]]
                    draw.polygon(poly, fill=fill_color, outline=outline_color)
                    
        # Onscreen HUD
        draw.text((20, 20), "TSFi2 AUNCIENT BALLET PERFORMANCE", fill=(255, 215, 0))
        draw.text((20, 35), f"MOVEMENT: {current_movement_name}", fill=(0, 255, 255))
        draw.text((20, 50), f"TIME CODE: {time_sec:.2f}s / {DURATION:.2f}s", fill=(0, 255, 0))
        
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Teddy ballet video rendered: {video_output}")
    
    usda_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_ballet_scene.usda"
    with open(usda_output, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    startTimeCode = 0\n")
        f.write(f"    endTimeCode = {total_frames - 1}\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        f.write("def Xform \"TeddyBalletScene\"\n")
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
    
    try:
        os.remove(wav_output)
    except Exception:
        pass

if __name__ == "__main__":
    main()
