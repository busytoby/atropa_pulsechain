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
    
    # 4 legs (Front pair indices 1, Back pair indices 2)
    t_lthigh1, t_rthigh1 = 0.0, 0.0
    t_lknee1, t_rknee1 = 0.0, 0.0
    t_lthigh2, t_rthigh2 = 0.0, 0.0
    t_lknee2, t_rknee2 = 0.0, 0.0
    
    # 4 arms (Front pair indices 1, Back pair indices 2)
    t_luarm1, t_ruarm1 = 0.2, 0.2
    t_lelbow1, t_relbow1 = 0.3, 0.3
    t_luarm2, t_ruarm2 = 0.3, 0.3
    t_lelbow2, t_relbow2 = 0.4, 0.4
    
    m_name = "Ouverture"
    
    if movement == 0:
        m_name = "Ouverture (Opening)"
        t_luarm1 = t_ruarm1 = -0.5 * m_progress + 0.2 * (1.0 - m_progress)
        t_luarm2 = t_ruarm2 = -0.3 * m_progress + 0.3 * (1.0 - m_progress)
        t_lelbow1 = t_relbow1 = 0.4 * m_progress + 0.3 * (1.0 - m_progress)
        t_lelbow2 = t_relbow2 = 0.5 * m_progress + 0.4 * (1.0 - m_progress)
        
    elif movement == 1:
        m_name = "Plier (Plie 4 Legs)"
        plie_amp = math.sin(m_progress * math.pi)
        y_disp -= 0.25 * plie_amp
        t_lthigh1 = t_rthigh1 = t_lthigh2 = t_rthigh2 = 0.4 * plie_amp
        t_lknee1 = t_rknee1 = t_lknee2 = t_rknee2 = 0.8 * plie_amp
        t_luarm1 = t_ruarm1 = -0.1 * plie_amp - 0.5 * (1.0 - plie_amp)
        t_luarm2 = t_ruarm2 = 0.1 * plie_amp - 0.3 * (1.0 - plie_amp)
        
    elif movement == 2:
        m_name = "Relever & Pointe (4 Legs Pointe)"
        rise_amp = math.sin(m_progress * math.pi)
        y_disp += 0.25 * rise_amp
        t_luarm1 = t_ruarm1 = -0.7 * rise_amp
        t_luarm2 = t_ruarm2 = -0.5 * rise_amp
        
    elif movement == 3:
        m_name = "Sauter & Batterie (Leap with 4 Leg Beats)"
        leap_phase = m_progress * math.pi
        y_disp += 1.1 * math.sin(leap_phase)
        
        click_amp = 0.0
        if 0.25 < m_progress < 0.75:
            click_amp = math.sin((m_progress - 0.25) * 2.0 * math.pi * 2.5)
            
        t_lthigh1 = t_lthigh2 = 0.35 * click_amp
        t_rthigh1 = t_rthigh2 = -0.35 * click_amp
        
    elif movement == 4:
        m_name = "Tourner (Pirouette Spin)"
        body_yaw = m_progress * 2.0 * math.pi * 2.0 + 1.0
        t_lthigh1 = t_lthigh2 = 0.75
        t_lknee1 = t_lknee2 = 1.3
        
    elif movement == 5:
        m_name = "Arabesque (Balance on 2 Legs)"
        adagio_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.35 * adagio_amp
        t_lthigh1 = t_lthigh2 = -0.1 * adagio_amp
        t_rthigh1 = t_rthigh2 = -0.75 * adagio_amp
        t_rknee1 = t_rknee2 = 0.3 * adagio_amp
        t_luarm1 = -0.6 * adagio_amp
        t_ruarm1 = 0.6 * adagio_amp
        t_luarm2 = 0.6 * adagio_amp
        t_ruarm2 = -0.6 * adagio_amp
        
    elif movement == 6:
        m_name = "Elancer (Darting Split Leap)"
        x_disp = -2.5 + 5.0 * m_progress
        y_disp += 0.5 * math.sin(m_progress * math.pi)
        t_lthigh1 = t_lthigh2 = -0.5 * math.sin(m_progress * math.pi)
        t_rthigh1 = t_rthigh2 = 0.5 * math.sin(m_progress * math.pi)
        
    elif movement == 7:
        m_name = "Reverence (4-Armed Bow)"
        bow_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.5 * bow_amp
        y_disp -= 0.15 * bow_amp
        t_lthigh2 = t_rthigh2 = -0.4 * bow_amp
        t_luarm1 = t_ruarm1 = t_luarm2 = t_ruarm2 = 0.4 * bow_amp
        
    parts = {}
    
    parts["Body"] = {
        "pos": (x_disp, 0.55 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, body_roll),
        "local_pos": (x_disp, 0.55 + y_disp, z_disp),
        "local_rot": (body_pitch, body_yaw, body_roll),
        "shape": "ellipsoid",
        "size": (0.7, 0.9, 0.75),
        "color": (120, 80, 54)
    }
    
    head_local_pos = (0.0, 0.95, 0.0)
    head_local_rot = (0.05 * math.sin(time_sec * 4.0), 0.0, 0.0)
    hx, hy, hz = rotate_x(head_local_pos[0], head_local_pos[1], head_local_pos[2], body_pitch)
    hx, hy, hz = rotate_y(hx, hy, hz, body_yaw)
    hx, hy, hz = rotate_z(hx, hy, hz, body_roll)
    parts["Head"] = {
        "pos": (x_disp + hx, 0.55 + y_disp + hy, z_disp + hz),
        "rot": (body_pitch + head_local_rot[0], body_yaw + head_local_rot[1], body_roll + head_local_rot[2]),
        "local_pos": head_local_pos,
        "local_rot": head_local_rot,
        "shape": "ellipsoid",
        "size": (0.55, 0.55, 0.5),
        "color": (120, 80, 54)
    }
    
    parts["LeftEar"] = {
        "pos": (parts["Head"]["pos"][0] - 0.4, parts["Head"]["pos"][1] + 0.4, parts["Head"]["pos"][2]),
        "rot": parts["Head"]["rot"],
        "local_pos": (-0.4, 0.4, 0.0),
        "local_rot": (0.0, 0.0, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    parts["RightEar"] = {
        "pos": (parts["Head"]["pos"][0] + 0.4, parts["Head"]["pos"][1] + 0.4, parts["Head"]["pos"][2]),
        "rot": parts["Head"]["rot"],
        "local_pos": (0.4, 0.4, 0.0),
        "local_rot": (0.0, 0.0, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    
    def add_arm(side, index, uarm_pitch, elbow_pitch, color_u, color_f):
        side_sign = -1.0 if side == "Left" else 1.0
        z_offset = -0.22 if index == 2 else 0.22 # Front is index 1 (+0.22), Back is index 2 (-0.22)
        
        uarm_local_pos = (side_sign * 0.65, 0.35, z_offset)
        uarm_local_rot = (uarm_pitch, 0.0, 0.0)
        
        ax, ay, az = rotate_x(uarm_local_pos[0], uarm_local_pos[1], uarm_local_pos[2], body_pitch)
        ax, ay, az = rotate_y(ax, ay, az, body_yaw)
        ax, ay, az = rotate_z(ax, ay, az, body_roll)
        
        parts[f"{side}UpperArm{index}"] = {
            "pos": (x_disp + ax, 0.55 + y_disp + ay, z_disp + az),
            "rot": (body_pitch + uarm_pitch, body_yaw, body_roll),
            "local_pos": uarm_local_pos,
            "local_rot": uarm_local_rot,
            "shape": "ellipsoid",
            "size": (0.16, 0.26, 0.16),
            "color": color_u
        }
        
        forearm_local_pos = (0.0, -0.3, 0.0)
        forearm_local_rot = (elbow_pitch, 0.0, 0.0)
        
        fx, fy, fz = rotate_x(forearm_local_pos[0], forearm_local_pos[1], forearm_local_pos[2], body_pitch + uarm_pitch)
        fx, fy, fz = rotate_y(fx, fy, fz, body_yaw)
        fx, fy, fz = rotate_z(fx, fy, fz, body_roll)
        
        parts[f"{side}Forearm{index}"] = {
            "pos": (parts[f"{side}UpperArm{index}"]["pos"][0] + fx, parts[f"{side}UpperArm{index}"]["pos"][1] + fy, parts[f"{side}UpperArm{index}"]["pos"][2] + fz),
            "rot": (body_pitch + uarm_pitch + elbow_pitch, body_yaw, body_roll),
            "local_pos": forearm_local_pos,
            "local_rot": forearm_local_rot,
            "shape": "ellipsoid",
            "size": (0.13, 0.22, 0.13),
            "color": color_f
        }
        
    def add_leg(side, index, thigh_pitch, knee_pitch, color_t, color_c):
        side_sign = -1.0 if side == "Left" else 1.0
        z_offset = -0.22 if index == 2 else 0.22 # Front is index 1 (+0.22), Back is index 2 (-0.22)
        
        thigh_local_pos = (side_sign * 0.35, -0.25, z_offset)
        thigh_local_rot = (thigh_pitch, 0.0, 0.0)
        
        lx, ly, lz = rotate_x(thigh_local_pos[0], thigh_local_pos[1], thigh_local_pos[2], body_pitch)
        lx, ly, lz = rotate_y(lx, ly, lz, body_yaw)
        lx, ly, lz = rotate_z(lx, ly, lz, body_roll)
        
        parts[f"{side}Thigh{index}"] = {
            "pos": (x_disp + lx, 0.55 + y_disp + ly, z_disp + lz),
            "rot": (body_pitch + thigh_pitch, body_yaw, body_roll),
            "local_pos": thigh_local_pos,
            "local_rot": thigh_local_rot,
            "shape": "ellipsoid",
            "size": (0.2, 0.3, 0.2),
            "color": color_t
        }
        
        calf_local_pos = (0.0, -0.3, 0.0)
        calf_local_rot = (-knee_pitch, 0.0, 0.0)
        
        cx, cy, cz = rotate_x(calf_local_pos[0], calf_local_pos[1], calf_local_pos[2], body_pitch + thigh_pitch)
        cx, cy, cz = rotate_y(cx, cy, cz, body_yaw)
        cx, cy, cz = rotate_z(cx, cy, cz, body_roll)
        
        parts[f"{side}Calf{index}"] = {
            "pos": (parts[f"{side}Thigh{index}"]["pos"][0] + cx, parts[f"{side}Thigh{index}"]["pos"][1] + cy, parts[f"{side}Thigh{index}"]["pos"][2] + cz),
            "rot": (body_pitch + thigh_pitch - knee_pitch, body_yaw, body_roll),
            "local_pos": calf_local_pos,
            "local_rot": calf_local_rot,
            "shape": "ellipsoid",
            "size": (0.16, 0.25, 0.16),
            "color": color_c
        }

    # Add 4 Arms (Front set = 1, Back set = 2)
    add_arm("Left", 1, t_luarm1, t_lelbow1, (120, 80, 54), (100, 65, 40))
    add_arm("Right", 1, t_ruarm1, t_relbow1, (120, 80, 54), (100, 65, 40))
    add_arm("Left", 2, t_luarm2, t_lelbow2, (110, 72, 48), (90, 58, 35))
    add_arm("Right", 2, t_ruarm2, t_relbow2, (110, 72, 48), (90, 58, 35))
    
    # Add 4 Legs (Front set = 1, Back set = 2)
    add_leg("Left", 1, t_lthigh1, t_lknee1, (120, 80, 54), (100, 65, 40))
    add_leg("Right", 1, t_rthigh1, t_rknee1, (120, 80, 54), (100, 65, 40))
    add_leg("Left", 2, t_lthigh2, t_lknee2, (110, 72, 48), (90, 58, 35))
    add_leg("Right", 2, t_rthigh2, t_rknee2, (110, 72, 48), (90, 58, 35))
    
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
            
            lpx, lpy, lpz = part["local_pos"]
            lrx, lry, lrz = part["local_rot"]
            
            if part_name not in usd_samples:
                usd_samples[part_name] = []
            usd_samples[part_name].append((f, lpx, lpy, lpz, lrx, lry, lrz))
            
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
        draw.text((20, 20), "TSFi2 AUNCIENT 4-ARMED 4-LEGGED BALLET PERFORMANCE", fill=(255, 215, 0))
        draw.text((20, 35), f"MOVEMENT: {current_movement_name}", fill=(0, 255, 255))
        draw.text((20, 50), f"TIME CODE: {time_sec:.2f}s / {DURATION:.2f}s", fill=(0, 255, 0))
        
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Teddy ballet video rendered: {video_output}")
    
    # Write fully nested and connected skeleton Xform hierarchy to USDA file
    usda_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_ballet_scene.usda"
    
    def write_usda_joint(f, name, samples, indent_level):
        ind = "    " * indent_level
        f.write(f"{ind}def Xform \"{name}\"\n")
        f.write(f"{ind}{{\n")
        f.write(f"{ind}    double3 xformOp:translate.timeSamples = {{\n")
        for f_idx, px, py, pz, rx, ry, rz in samples:
            f.write(f"{ind}        {f_idx}: ({px:.4f}, {py:.4f}, {pz:.4f}),\n")
        f.write(f"{ind}    }}\n")
        f.write(f"{ind}    double3 xformOp:rotateXYZ.timeSamples = {{\n")
        for f_idx, px, py, pz, rx, ry, rz in samples:
            f.write(f"{ind}        {f_idx}: ({rx * 57.2958:.4f}, {ry * 57.2958:.4f}, {rz * 57.2958:.4f}),\n")
        f.write(f"{ind}    }}\n")
        f.write(f"{ind}    uniform token[] xformOpOrder = [\"xformOp:translate\", \"xformOp:rotateXYZ\"]\n")
        
    with open(usda_output, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    startTimeCode = 0\n")
        f.write(f"    endTimeCode = {total_frames - 1}\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        f.write("def Xform \"TeddyWalkerScene\"\n")
        f.write("{\n")
        
        # 1. Body (Parent Root)
        write_usda_joint(f, "Body", usd_samples["Body"], 1)
        
        # 2. Head (nested under Body)
        write_usda_joint(f, "Head", usd_samples["Head"], 2)
        
        # 3. Ears (nested under Head)
        write_usda_joint(f, "LeftEar", usd_samples["LeftEar"], 3)
        f.write("        }\n")
        write_usda_joint(f, "RightEar", usd_samples["RightEar"], 3)
        f.write("        }\n")
        f.write("    }\n") # close Head
        
        # 4. Front Left Arm (nested under Body)
        write_usda_joint(f, "LeftUpperArm1", usd_samples["LeftUpperArm1"], 2)
        write_usda_joint(f, "LeftForearm1", usd_samples["LeftForearm1"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 5. Front Right Arm (nested under Body)
        write_usda_joint(f, "RightUpperArm1", usd_samples["RightUpperArm1"], 2)
        write_usda_joint(f, "RightForearm1", usd_samples["RightForearm1"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 6. Back Left Arm (nested under Body)
        write_usda_joint(f, "LeftUpperArm2", usd_samples["LeftUpperArm2"], 2)
        write_usda_joint(f, "LeftForearm2", usd_samples["LeftForearm2"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 7. Back Right Arm (nested under Body)
        write_usda_joint(f, "RightUpperArm2", usd_samples["RightUpperArm2"], 2)
        write_usda_joint(f, "RightForearm2", usd_samples["RightForearm2"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 8. Front Left Leg (nested under Body)
        write_usda_joint(f, "LeftThigh1", usd_samples["LeftThigh1"], 2)
        write_usda_joint(f, "LeftCalf1", usd_samples["LeftCalf1"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 9. Front Right Leg (nested under Body)
        write_usda_joint(f, "RightThigh1", usd_samples["RightThigh1"], 2)
        write_usda_joint(f, "RightCalf1", usd_samples["RightCalf1"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 10. Back Left Leg (nested under Body)
        write_usda_joint(f, "LeftThigh2", usd_samples["LeftThigh2"], 2)
        write_usda_joint(f, "LeftCalf2", usd_samples["LeftCalf2"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        # 11. Back Right Leg (nested under Body)
        write_usda_joint(f, "RightThigh2", usd_samples["RightThigh2"], 2)
        write_usda_joint(f, "RightCalf2", usd_samples["RightCalf2"], 3)
        f.write("        }\n")
        f.write("    }\n")
        
        f.write("}\n") # close Body
        f.write("}\n") # close scene
        
    print(f"[SUCCESS] Pixar USDA scene description exported: {usda_output}")
    
    try:
        os.remove(wav_output)
    except Exception:
        pass

if __name__ == "__main__":
    main()
