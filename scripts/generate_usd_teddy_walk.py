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
    # Walk parameters: frequency = 1.6 Hz
    walk_freq = 1.6
    phase = time_sec * 2.0 * math.pi * walk_freq
    
    # Forward translation: walks from left to right across viewport
    progress = time_sec / DURATION
    x_disp = -4.0 + 8.0 * progress
    
    # Realistic double-step bobbing (lowest at transition/support, highest at mid-stance)
    y_bob = 0.06 * math.cos(2.0 * phase) + 0.1
    
    # Body tilt/lean forward
    body_pitch = 0.08 + 0.04 * math.sin(2.0 * phase)
    body_yaw = 0.1 * math.sin(phase * 0.1) + 1.2
    
    # Leg/Knee angles
    # Hip swing back and forth
    theta_lthigh = 0.5 * math.sin(phase)
    theta_rthigh = 0.5 * math.sin(phase + math.pi)
    
    # Knee bends during forward swing phase to clear the floor
    # Left leg swings forward when cosine of phase > 0
    theta_lknee = 0.0
    if math.cos(phase) > -0.2:
        theta_lknee = 0.55 * (1.0 - math.sin(phase))
        
    # Right leg swings forward when cosine of (phase + pi) > -0.2
    theta_rknee = 0.0
    if math.cos(phase + math.pi) > -0.2:
        theta_rknee = 0.55 * (1.0 - math.sin(phase + math.pi))
        
    # Arm/Elbow angles
    theta_luarm = -0.4 * math.sin(phase)
    theta_ruarm = -0.4 * math.sin(phase + math.pi)
    
    theta_lelbow = 0.35 * (1.0 + math.cos(phase))
    theta_relbow = 0.35 * (1.0 + math.cos(phase + math.pi))
    
    parts = {}
    
    # 1. Main body translation
    parts["Body"] = {
        "pos": (x_disp, 0.55 + y_bob, 0.0),
        "rot": (body_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.7, 0.85, 0.6),
        "color": (120, 80, 54) # Brown
    }
    
    # 2. Head (local translation relative to body)
    parts["Head"] = {
        "pos": (x_disp, 1.45 + y_bob, 0.0),
        "rot": (body_pitch + 0.05 * math.sin(phase), body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.55, 0.55, 0.5),
        "color": (120, 80, 54)
    }
    
    # 3. Ears
    parts["LeftEar"] = {
        "pos": (x_disp - 0.4, 1.85 + y_bob, 0.0),
        "rot": (body_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.2, 0.2, 0.15),
        "color": (100, 65, 40)
    }
    parts["RightEar"] = {
        "pos": (x_disp + 0.4, 1.85 + y_bob, 0.0),
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
        "pos": (x_disp - 0.65 + al_x, 0.9 + y_bob + al_y, al_z),
        "rot": (larm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (120, 80, 54)
    }
    
    # Forearm pivot at the end of Upper Arm (length ~0.25)
    f_larm_pitch = larm_pitch + theta_lelbow
    fl_x, fl_y, fl_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    fl_x, fl_y, fl_z = rotate_z(fl_x, fl_y, fl_z, f_larm_pitch)
    parts["LeftForearm"] = {
        "pos": (x_disp - 0.65 + fl_x, 0.9 + y_bob + fl_y, fl_z),
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
        "pos": (x_disp + 0.65 + ar_x, 0.9 + y_bob + ar_y, ar_z),
        "rot": (rarm_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (120, 80, 54)
    }
    
    f_rarm_pitch = rarm_pitch + theta_relbow
    fr_x, fr_y, fr_z = rotate_y(0.0, -0.45, 0.0, body_yaw)
    fr_x, fr_y, fr_z = rotate_z(fr_x, fr_y, fr_z, f_rarm_pitch)
    parts["RightForearm"] = {
        "pos": (x_disp + 0.65 + fr_x, 0.9 + y_bob + fr_y, fr_z),
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
        "pos": (x_disp - 0.35 + llt_x, 0.3 + y_bob + llt_y, llt_z),
        "rot": (lthigh_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.22, 0.35, 0.22),
        "color": (120, 80, 54)
    }
    
    # Knee bend: Thigh pitch + Knee flexion
    lknee_pitch = lthigh_pitch - theta_lknee
    llc_x, llc_y, llc_z = rotate_y(0.0, -0.55, 0.0, body_yaw)
    llc_x, llc_y, llc_z = rotate_z(llc_x, llc_y, llc_z, lknee_pitch)
    parts["LeftCalf"] = {
        "pos": (x_disp - 0.35 + llc_x, 0.3 + y_bob + llc_y, llc_z),
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
        "pos": (x_disp + 0.35 + rrt_x, 0.3 + y_bob + rrt_y, rrt_z),
        "rot": (rthigh_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.22, 0.35, 0.22),
        "color": (120, 80, 54)
    }
    
    rknee_pitch = rthigh_pitch - theta_rknee
    rrc_x, rrc_y, rrc_z = rotate_y(0.0, -0.55, 0.0, body_yaw)
    rrc_x, rrc_y, rrc_z = rotate_z(rrc_x, rrc_y, rrc_z, rknee_pitch)
    parts["RightCalf"] = {
        "pos": (x_disp + 0.35 + rrc_x, 0.3 + y_bob + rrc_y, rrc_z),
        "rot": (rknee_pitch, body_yaw, 0.0),
        "shape": "ellipsoid",
        "size": (0.18, 0.3, 0.18),
        "color": (100, 65, 40)
    }
    
    return parts

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
    wav_output = "temp_teddy_walk_track.wav"
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_walk_demo.mp4"
    generate_soundtrack(wav_output)
    
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
        parts = get_teddy_geometry(time_sec)
        
        img = Image.new("RGB", (width, height), (15, 10, 25))
        draw = ImageDraw.Draw(img)
        
        # Grid backdrop
        grid_pos = int(time_sec * 60.0) % 80
        for x_g in range(-80, width + 80, 80):
            draw.line([(x_g - grid_pos, 0), (x_g - grid_pos, height)], fill=(30, 20, 50), width=1)
        for y_g in range(0, height, 60):
            draw.line([(0, y_g), (width, y_g)], fill=(30, 20, 50), width=1)
            
        # Perspective floor
        draw.rectangle([0, 320, width, height], fill=(10, 5, 20))
        for x_floor in range(-200, width + 200, 50):
            draw.line([(320, 320), (x_floor, height)], fill=(50, 30, 80), width=2)
            
        # Dynamic depth-sorting based on center projected Z coordinate
        render_queue = []
        for part_name, part in parts.items():
            px, py, pz = part["pos"]
            # Rotated Z coordinate for simple back-to-front sorting
            rotated_z = pz + 4.5
            render_queue.append((rotated_z, part_name))
            
        # Sort desc (larger Z = further away = render first)
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
        draw.text((20, 20), "USD TEDDY BEAR WALKER DEMO (REALISTIC)", fill=(255, 215, 0))
        draw.text((20, 35), f"FRAME: {f:03d} / {total_frames:03d}", fill=(0, 255, 255))
        draw.text((20, 50), f"X-POS: {parts['Body']['pos'][0]:.3f}", fill=(0, 255, 0))
        
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Teddy walking video rendered: {video_output}")
    
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
    
    try:
        os.remove(wav_output)
    except Exception:
        pass

if __name__ == "__main__":
    main()
