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
    
    # Simple Lydian waltz chord progression
    melody_notes = [
        # Ouverture (C Lydian)
        [261.6, 293.7, 329.6, 392.0],
        # Plier (Slow descent)
        [220.0, 246.9, 277.2, 329.6],
        # Relever (Rising octave)
        [329.6, 392.0, 440.0, 523.3],
        # Sauter (High pitch arpeggio)
        [523.3, 587.3, 659.3, 784.0],
        # Tourner (Spinning sequence)
        [392.0, 440.0, 493.9, 587.3],
        # Arabesque (Suspended chords)
        [440.0, 493.9, 554.4, 659.3],
        # Elancer (Leaping runs)
        [523.3, 659.3, 784.0, 1046.5],
        # Reverence (Soft resolution)
        [261.6, 329.6, 392.0, 523.3]
    ]
    
    for s in range(total_samples):
        t_sec = s / float(SAMPLE_RATE)
        movement_idx = int(t_sec // 3.0) % 8
        chord = melody_notes[movement_idx]
        
        # Waltz beat accentuation (Bass on 1, chord plucks on 2 & 3)
        beat_idx = int(s // beat_samples)
        beat_pos = (s % beat_samples) / float(beat_samples)
        
        sig = 0.0
        if beat_idx % 3 == 0:
            # Deep root note swell
            root_f = chord[0] * 0.5
            env = math.exp(-beat_pos * 4.0)
            sig += math.sin(2.0 * math.pi * root_f * t_sec) * env * 0.5
        else:
            # Plucked harp chord
            env = math.exp(-beat_pos * 8.0)
            for f in chord[1:]:
                sig += math.sin(2.0 * math.pi * f * t_sec) * env * 0.12
                
        audio[s] = sig
        
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

def get_ballet_geometry(time_sec):
    # 8 movements, each lasting exactly 3 seconds
    movement = int(time_sec // 3.0)
    m_time = time_sec % 3.0
    m_progress = m_time / 3.0
    
    # Base defaults
    x_disp = 0.0
    y_disp = 0.05
    z_disp = 0.0
    
    body_pitch = 0.0
    body_yaw = 1.0  # Angled slightly towards camera
    body_roll = 0.0
    
    # Default limb rotations
    theta_lthigh = 0.0
    theta_rthigh = 0.0
    theta_lknee = 0.0
    theta_rknee = 0.0
    
    theta_luarm = 0.2
    theta_ruarm = 0.2
    theta_lelbow = 0.3
    theta_relbow = 0.3
    
    m_name = "Ouverture"
    
    if movement == 0:
        # 1. Ouverture (Opening arms)
        m_name = "Ouverture (Opening)"
        # Gracefully raise arms from first position to fifth position overhead
        theta_luarm = -0.5 * m_progress + 0.2 * (1.0 - m_progress)
        theta_ruarm = -0.5 * m_progress + 0.2 * (1.0 - m_progress)
        theta_lelbow = 0.4 * m_progress + 0.3 * (1.0 - m_progress)
        theta_relbow = 0.4 * m_progress + 0.3 * (1.0 - m_progress)
        
    elif movement == 1:
        # 2. Plier (Bending of knees)
        m_name = "Plier (Plie Knee Bend)"
        # Descents body, bends knees outwards, extends arms to second position (sides)
        plie_amp = math.sin(m_progress * math.pi)
        y_disp -= 0.25 * plie_amp
        theta_lthigh = 0.4 * plie_amp
        theta_rthigh = 0.4 * plie_amp
        theta_lknee = 0.8 * plie_amp
        theta_rknee = 0.8 * plie_amp
        
        theta_luarm = -0.1 * plie_amp - 0.5 * (1.0 - plie_amp)
        theta_ruarm = -0.1 * plie_amp - 0.5 * (1.0 - plie_amp)
        theta_lelbow = 0.15 * plie_amp + 0.4 * (1.0 - plie_amp)
        theta_relbow = 0.15 * plie_amp + 0.4 * (1.0 - plie_amp)
        
    elif movement == 2:
        # 3. Relever & Pointe (Rising on toes)
        m_name = "Relever & Pointe (On Toes)"
        # Rise up, lift arms overhead, straight knees
        rise_amp = math.sin(m_progress * math.pi)
        y_disp += 0.2 * rise_amp
        theta_lthigh = -0.1 * rise_amp
        theta_rthigh = -0.1 * rise_amp
        
        theta_luarm = -0.7 * rise_amp
        theta_ruarm = -0.7 * rise_amp
        theta_lelbow = 0.5 * rise_amp
        theta_relbow = 0.5 * rise_amp
        
    elif movement == 3:
        # 4. Sauter & Batterie (Leaping with leg clicks)
        m_name = "Sauter & Batterie (Leap & Beats)"
        # Leap high, click legs together, raise arms
        leap_phase = m_progress * math.pi
        y_disp += 1.1 * math.sin(leap_phase)
        
        # Click legs together in mid-leap
        click_amp = 0.0
        if 0.25 < m_progress < 0.75:
            click_amp = math.sin((m_progress - 0.25) * 2.0 * math.pi * 2.0)
            
        theta_lthigh = 0.3 * click_amp
        theta_rthigh = -0.3 * click_amp
        
        theta_luarm = -0.8
        theta_ruarm = -0.8
        theta_lelbow = 0.6
        theta_relbow = 0.6
        
    elif movement == 4:
        # 5. Tourner (Spinning rotation)
        m_name = "Tourner (Pirouette Spin)"
        # Spin 360 degrees, hold left leg in Passe position (knee bent up)
        body_yaw = m_progress * 2.0 * math.pi * 2.0 + 1.0
        
        # Passe leg configuration
        theta_lthigh = 0.75
        theta_lknee = 1.3
        theta_rthigh = -0.1
        
        # Arms in curved first position holding the spin center
        theta_luarm = -0.2
        theta_ruarm = -0.2
        theta_lelbow = 0.6
        theta_relbow = 0.6
        
    elif movement == 5:
        # 6. Arabesque / Adagio (Slow Balance)
        m_name = "Arabesque (Slow Balance)"
        # Lean forward, stand on left leg, lift right leg high behind
        adagio_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.35 * adagio_amp
        
        # Stand on left, raise right leg back
        theta_lthigh = -0.1 * adagio_amp
        theta_rthigh = -0.75 * adagio_amp
        theta_rknee = 0.3 * adagio_amp
        
        # Reach left arm forward, right arm backward
        theta_luarm = -0.6 * adagio_amp
        theta_ruarm = 0.6 * adagio_amp
        
    elif movement == 6:
        # 7. Elancer (Darting forward glide)
        m_name = "Elancer (Darting Glide)"
        # Gliding leap across from left to right
        x_disp = -2.5 + 5.0 * m_progress
        y_disp += 0.45 * math.sin(m_progress * math.pi)
        
        # Extend legs in split position in air
        theta_lthigh = -0.5 * math.sin(m_progress * math.pi)
        theta_rthigh = 0.5 * math.sin(m_progress * math.pi)
        
        theta_luarm = -0.5
        theta_ruarm = -0.2
        
    elif movement == 7:
        # 8. Reverence (Bowing curtsy)
        m_name = "Reverence (Bow)"
        # Bow forward, sweep arms down, slide one leg back
        bow_amp = math.sin(m_progress * math.pi)
        body_pitch = 0.5 * bow_amp
        y_disp -= 0.15 * bow_amp
        
        # Left leg back
        theta_lthigh = -0.4 * bow_amp
        theta_rthigh = 0.1 * bow_amp
        
        # Arms sweep down in gesture
        theta_luarm = 0.4 * bow_amp + 0.2 * (1.0 - bow_amp)
        theta_ruarm = 0.4 * bow_amp + 0.2 * (1.0 - bow_amp)
        theta_lelbow = 0.1 * bow_amp + 0.3 * (1.0 - bow_amp)
        theta_relbow = 0.1 * bow_amp + 0.3 * (1.0 - bow_amp)
        
    parts = {}
    
    # 1. Main body translation
    parts["Body"] = {
        "pos": (x_disp, 0.55 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, body_roll),
        "shape": "ellipsoid",
        "size": (0.7, 0.85, 0.6),
        "color": (120, 80, 54) # Brown
    }
    
    # 2. Head (local translation relative to body)
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
    
    # 4. Left Arm (Upper Arm & Forearm)
    larm_pitch = theta_luarm + body_pitch
    al_x, al_y, al_z = rotate_y(0.0, -0.25, 0.0, body_yaw)
    al_x, al_y, al_z = rotate_z(al_x, al_y, al_z, larm_pitch)
    parts["LeftUpperArm"] = {
        "pos": (x_disp - 0.65 + al_x, 0.9 + y_disp + al_y, al_z),
        "rot": (larm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (120, 80, 54)
    }
    
    f_larm_pitch = larm_pitch + theta_lelbow
    fl_x, fl_y, fl_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    fl_x, fl_y, fl_z = rotate_z(fl_x, fl_y, fl_z, f_larm_pitch)
    parts["LeftForearm"] = {
        "pos": (x_disp - 0.65 + fl_x, 0.9 + y_disp + fl_y, fl_z),
        "rot": (f_larm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.15, 0.25, 0.15),
        "color": (100, 65, 40)
    }
    
    # 5. Right Arm (Upper Arm & Forearm)
    rarm_pitch = theta_ruarm + body_pitch
    ar_x, ar_y, ar_z = rotate_y(0.0, -0.25, 0.0, body_yaw)
    ar_x, ar_y, ar_z = rotate_z(ar_x, ar_y, ar_z, rarm_pitch)
    parts["RightUpperArm"] = {
        "pos": (x_disp + 0.65 + ar_x, 0.9 + y_disp + ar_y, ar_z),
        "rot": (rarm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (120, 80, 54)
    }
    
    f_rarm_pitch = rarm_pitch + theta_relbow
    fr_x, fr_y, fr_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    fr_x, fr_y, fr_z = rotate_z(fr_x, fr_y, fr_z, f_rarm_pitch)
    parts["RightForearm"] = {
        "pos": (x_disp + 0.65 + fr_x, 0.9 + y_disp + fr_y, fr_z),
        "rot": (f_rarm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.15, 0.25, 0.15),
        "color": (100, 65, 40)
    }
    
    # 6. Left Leg (Thigh & Calf)
    lthigh_pitch = theta_lthigh + body_pitch
    llt_x, llt_y, llt_z = rotate_y(0.0, -0.25, 0.0, body_yaw)
    llt_x, llt_y, llt_z = rotate_z(llt_x, llt_y, llt_z, lthigh_pitch)
    parts["LeftThigh"] = {
        "pos": (x_disp - 0.35 + llt_x, 0.3 + y_disp + llt_y, llt_z),
        "rot": (lthigh_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.22, 0.35, 0.22),
        "color": (120, 80, 54)
    }
    
    lknee_pitch = lthigh_pitch - theta_lknee
    llc_x, llc_y, llc_z = rotate_y(0.0, -0.55, 0.0, body_yaw)
    llc_x, llc_y, llc_z = rotate_z(llc_x, llc_y, llc_z, lknee_pitch)
    parts["LeftCalf"] = {
        "pos": (x_disp - 0.35 + llc_x, 0.3 + y_disp + llc_y, llc_z),
        "rot": (lknee_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (100, 65, 40)
    }
    
    # 7. Right Leg (Thigh & Calf)
    rthigh_pitch = theta_rthigh + body_pitch
    rrt_x, rrt_y, rrt_z = rotate_y(0.0, -0.25, 0.0, body_yaw)
    rrt_x, rrt_y, rrt_z = rotate_z(rrt_x, rrt_y, rrt_z, rthigh_pitch)
    parts["RightThigh"] = {
        "pos": (x_disp + 0.35 + rrt_x, 0.3 + y_disp + rrt_y, rrt_z),
        "rot": (rthigh_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.22, 0.35, 0.22),
        "color": (120, 80, 54)
    }
    
    rknee_pitch = rthigh_pitch - theta_rknee
    rrc_x, rrc_y, rrc_z = rotate_y(0.0, -0.55, 0.0, body_yaw)
    rrc_x, rrc_y, rrc_z = rotate_z(rrc_x, rrc_y, rrc_z, rknee_pitch)
    parts["RightCalf"] = {
        "pos": (x_disp + 0.35 + rrc_x, 0.3 + y_disp + rrc_y, rrc_z),
        "rot": (rknee_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (100, 65, 40)
    }
    
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
        
        # Stage spotlights lines (diagonal glowing rays)
        for spotlight_x in [100, 320, 540]:
            draw.line([(spotlight_x, 0), (width / 2.0, 320)], fill=(80, 50, 100), width=3)
            
        # Draw perspective floor
        draw.rectangle([0, 320, width, height], fill=(15, 10, 25))
        for x_floor in range(-200, width + 200, 50):
            draw.line([(320, 320), (x_floor, height)], fill=(90, 40, 110), width=2)
            
        # Dynamic depth-sorting based on center projected Z coordinate
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
