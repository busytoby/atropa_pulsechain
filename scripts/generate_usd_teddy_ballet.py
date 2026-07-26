import os
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 24.0
total_frames = int(DURATION * FPS)

def generate_walking_soundtrack(wav_path):
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    # Rhythmic heavy footsteps (90 BPM -> 1.5 steps per second)
    step_dur = 60.0 / 90.0
    step_samples = int(SAMPLE_RATE * step_dur)
    
    for s in range(total_samples):
        t_sec = s / float(SAMPLE_RATE)
        step_pos = (s % step_samples) / float(step_samples)
        
        # Heavy thumbing bass drum footstep sound
        env = math.exp(-step_pos * 12.0)
        sig = math.sin(2.0 * math.pi * 55.0 * t_sec) * env * 0.7
        # Soft structural cotton squeak/rustle
        rustle_env = math.exp(-step_pos * 4.0)
        sig += math.sin(2.0 * math.pi * 880.0 * t_sec) * rustle_env * 0.05
        
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

def get_walking_geometry(time_sec):
    # Walk cycle frequency
    freq = 2.0 * math.pi * 1.5 # 1.5 Hz walk cycle
    
    # Forward translation along X-axis, loop across screen
    x_disp = -2.5 + (time_sec * 0.6) % 5.0
    # Gentle up-down bobbing of the body twice per cycle
    y_disp = 0.05 + 0.06 * abs(math.sin(freq * time_sec))
    z_disp = 0.0
    
    # Body sway (roll) and twist (yaw)
    body_pitch = 0.08 * math.sin(freq * time_sec)
    body_yaw = 1.0 + 0.05 * math.cos(freq * time_sec)
    body_roll = 0.04 * math.sin(freq * time_sec)
    
    # Alternating leg swing
    swing_l = 0.35 * math.sin(freq * time_sec)
    swing_r = -0.35 * math.sin(freq * time_sec)
    
    # Knee bends on backswing
    t_lknee = 0.4 * (1.0 + math.cos(freq * time_sec)) if swing_l < 0 else 0.1
    t_rknee = 0.4 * (1.0 - math.cos(freq * time_sec)) if swing_r < 0 else 0.1
    
    # Alternating arm swing (opposite to legs)
    t_luarm = -0.3 * math.sin(freq * time_sec) + 0.1
    t_ruarm = 0.3 * math.sin(freq * time_sec) + 0.1
    t_lelbow = 0.3
    t_relbow = 0.3
    
    m_name = "Walking Cycle"
    
    parts = {}
    
    parts["Body"] = {
        "pos": (x_disp, 0.55 + y_disp, z_disp),
        "rot": (body_pitch, body_yaw, body_roll),
        "local_pos": (x_disp, 0.55 + y_disp, z_disp),
        "local_rot": (body_pitch, body_yaw, body_roll),
        "shape": "ellipsoid",
        "size": (0.7, 0.85, 0.6),
        "color": (120, 80, 54)
    }
    
    head_local_pos = (0.0, 0.9, 0.0)
    head_local_rot = (0.04 * math.sin(freq * time_sec * 2.0), 0.0, 0.0)
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
    
    def add_arm(side, uarm_pitch, elbow_pitch, color_u, color_f):
        side_sign = -1.0 if side == "Left" else 1.0
        uarm_local_pos = (side_sign * 0.65, 0.35, 0.0)
        uarm_local_rot = (uarm_pitch, 0.0, 0.0)
        
        ax, ay, az = rotate_x(uarm_local_pos[0], uarm_local_pos[1], uarm_local_pos[2], body_pitch)
        ax, ay, az = rotate_y(ax, ay, az, body_yaw)
        ax, ay, az = rotate_z(ax, ay, az, body_roll)
        
        parts[f"{side}UpperArm"] = {
            "pos": (x_disp + ax, 0.55 + y_disp + ay, z_disp + az),
            "rot": (body_pitch + uarm_pitch, body_yaw, body_roll),
            "local_pos": uarm_local_pos,
            "local_rot": uarm_local_rot,
            "shape": "ellipsoid",
            "size": (0.18, 0.3, 0.18),
            "color": color_u
        }
        
        forearm_local_pos = (0.0, -0.3, 0.0)
        forearm_local_rot = (elbow_pitch, 0.0, 0.0)
        
        fx, fy, fz = rotate_x(forearm_local_pos[0], forearm_local_pos[1], forearm_local_pos[2], body_pitch + uarm_pitch)
        fx, fy, fz = rotate_y(fx, fy, fz, body_yaw)
        fx, fy, fz = rotate_z(fx, fy, fz, body_roll)
        
        parts[f"{side}Forearm"] = {
            "pos": (parts[f"{side}UpperArm"]["pos"][0] + fx, parts[f"{side}UpperArm"]["pos"][1] + fy, parts[f"{side}UpperArm"]["pos"][2] + fz),
            "rot": (body_pitch + uarm_pitch + elbow_pitch, body_yaw, body_roll),
            "local_pos": forearm_local_pos,
            "local_rot": forearm_local_rot,
            "shape": "ellipsoid",
            "size": (0.15, 0.25, 0.15),
            "color": color_f
        }
        
    def add_leg(side, thigh_pitch, knee_pitch, color_t, color_c):
        side_sign = -1.0 if side == "Left" else 1.0
        thigh_local_pos = (side_sign * 0.35, -0.25, 0.0)
        thigh_local_rot = (thigh_pitch, 0.0, 0.0)
        
        lx, ly, lz = rotate_x(thigh_local_pos[0], thigh_local_pos[1], thigh_local_pos[2], body_pitch)
        lx, ly, lz = rotate_y(lx, ly, lz, body_yaw)
        lx, ly, lz = rotate_z(lx, ly, lz, body_roll)
        
        parts[f"{side}Thigh"] = {
            "pos": (x_disp + lx, 0.55 + y_disp + ly, z_disp + lz),
            "rot": (body_pitch + thigh_pitch, body_yaw, body_roll),
            "local_pos": thigh_local_pos,
            "local_rot": thigh_local_rot,
            "shape": "ellipsoid",
            "size": (0.22, 0.35, 0.22),
            "color": color_t
        }
        
        calf_local_pos = (0.0, -0.3, 0.0)
        calf_local_rot = (-knee_pitch, 0.0, 0.0)
        
        cx, cy, cz = rotate_x(calf_local_pos[0], calf_local_pos[1], calf_local_pos[2], body_pitch + thigh_pitch)
        cx, cy, cz = rotate_y(cx, cy, cz, body_yaw)
        cx, cy, cz = rotate_z(cx, cy, cz, body_roll)
        
        parts[f"{side}Calf"] = {
            "pos": (parts[f"{side}Thigh"]["pos"][0] + cx, parts[f"{side}Thigh"]["pos"][1] + cy, parts[f"{side}Thigh"]["pos"][2] + cz),
            "rot": (body_pitch + thigh_pitch - knee_pitch, body_yaw, body_roll),
            "local_pos": calf_local_pos,
            "local_rot": calf_local_rot,
            "shape": "ellipsoid",
            "size": (0.18, 0.3, 0.18),
            "color": color_c
        }

    add_arm("Left", t_luarm, t_lelbow, (120, 80, 54), (100, 65, 40))
    add_arm("Right", t_ruarm, t_relbow, (120, 80, 54), (100, 65, 40))
    add_leg("Left", swing_l, t_lknee, (120, 80, 54), (100, 65, 40))
    add_leg("Right", swing_r, t_rknee, (120, 80, 54), (100, 65, 40))
    
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

def generate_fur_strands(center, size, num_strands=16):
    strands = []
    np.random.seed(42)
    for _ in range(num_strands):
        theta = np.random.uniform(0, 2.0 * np.pi)
        phi = np.random.uniform(0, np.pi)
        dx = size[0] * math.sin(phi) * math.cos(theta)
        dy = size[1] * math.sin(phi) * math.sin(theta)
        dz = size[2] * math.cos(phi)
        
        strand = []
        for step in range(4):
            factor = 1.0 + step * 0.12
            x = center[0] + dx * factor
            y = center[1] + dy * factor
            z = center[2] + dz * factor
            x += np.random.uniform(-0.02, 0.02)
            y += np.random.uniform(-0.02, 0.02)
            z += np.random.uniform(-0.02, 0.02)
            strand.append((x, y, z))
        strands.append(strand)
    return strands

def write_static_usda_asset(filepath, name, size):
    mesh_verts = generate_ellipsoid_mesh(size, num_segments=8)
    
    with open(filepath, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        f.write(f"def Mesh \"{name}\"\n")
        f.write("{\n")
        f.write("    point3f[] points = [\n")
        for vx, vy, vz in mesh_verts:
            f.write(f"        ({vx:.4f}, {vy:.4f}, {vz:.4f}),\n")
        f.write("    ]\n")
        
        face_indices = []
        face_counts = []
        num_seg = 8
        for i in range(num_seg - 1):
            for j in range(num_seg):
                p1 = i * num_seg + j
                p2 = i * num_seg + ((j + 1) % num_seg)
                p3 = (i + 1) * num_seg + ((j + 1) % num_seg)
                p4 = (i + 1) * num_seg + j
                face_indices.extend([p1, p2, p3, p4])
                face_counts.append(4)
                
        f.write(f"    int[] faceVertexIndices = {face_indices}\n")
        f.write(f"    int[] faceVertexCounts = {face_counts}\n")
        
        f.write("    def BasisCurves \"fur\"\n")
        f.write("    {\n")
        f.write("        uniform token basis = \"bezier\"\n")
        f.write("        uniform token type = \"cubic\"\n")
        
        fur_strands = generate_fur_strands((0.0, 0.0, 0.0), size, num_strands=8)
        f.write("        point3f[] points = [\n")
        for strand in fur_strands:
            for sx, sy, sz in strand:
                f.write(f"            ({sx:.4f}, {sy:.4f}, {sz:.4f}),\n")
        f.write("        ]\n")
        
        curve_counts = [4] * len(fur_strands)
        f.write(f"            int[] curveVertexCounts = {curve_counts}\n")
        f.write("    }\n")
        f.write("}\n")

def main():
    wav_output = "temp_teddy_walk_track.wav"
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_ballet_demo.mp4"
    generate_walking_soundtrack(wav_output)
    
    width, height = 640, 480
    ffmpeg_cmd = [
        "ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", str(FPS),
        "-i", "-", "-i", wav_output, "-c:v", "libx264", "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480", "-c:a", "aac", "-shortest", video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    usd_samples = {}
    
    for f in range(total_frames):
        time_sec = f / float(FPS)
        parts, current_movement_name = get_walking_geometry(time_sec)
        
        img = Image.new("RGB", (width, height), (10, 5, 20))
        draw = ImageDraw.Draw(img)
        
        # Spotlights / stage backdrop
        for spotlight_x in [100, 320, 540]:
            draw.line([(spotlight_x, 0), (width / 2.0, 320)], fill=(80, 50, 100), width=3)
            
        draw.rectangle([0, 320, width, height], fill=(15, 10, 25))
        for x_floor in range(-200, width + 200, 50):
            draw.line([(320, 320), (x_floor, height)], fill=(90, 40, 110), width=2)
            
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
                
            fill_color = part["color"]
            num_seg = 10
            for i in range(num_seg - 1):
                for j in range(num_seg):
                    p1_idx = i * num_seg + j
                    p2_idx = i * num_seg + ((j + 1) % num_seg)
                    p3_idx = (i + 1) * num_seg + ((j + 1) % num_seg)
                    p4_idx = (i + 1) * num_seg + j
                    poly = [projected_pts[p1_idx], projected_pts[p2_idx], projected_pts[p3_idx], projected_pts[p4_idx]]
                    draw.polygon(poly, fill=fill_color, outline=(255, 255, 255))
            
            strands = generate_fur_strands((px, py, pz), part["size"], num_strands=12)
            for strand in strands:
                screen_strand = []
                for sx, sy, sz in strand:
                    sgz = sz + 4.5
                    ssx = int(width / 2.0 + (sx * fov) / sgz)
                    ssy = int(height / 2.0 - (sy * fov) / sgz)
                    screen_strand.append((ssx, ssy))
                for pt_idx in range(len(screen_strand) - 1):
                    draw.line([screen_strand[pt_idx], screen_strand[pt_idx+1]], fill=(140, 95, 65), width=2)
                    
        draw.text((20, 20), "TSFi2 AUNCIENT FURRY SKELETAL WALK CYCLE", fill=(255, 215, 0))
        draw.text((20, 35), f"MOVEMENT: {current_movement_name}", fill=(0, 255, 255))
        draw.text((20, 50), f"TIME CODE: {time_sec:.2f}s / {DURATION:.2f}s", fill=(0, 255, 0))
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Teddy walk video rendered: {video_output}")
    
    assets_dir = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/assets"
    os.makedirs(assets_dir, exist_ok=True)
    
    parts_zero, _ = get_walking_geometry(0.0)
    for part_name, part in parts_zero.items():
        asset_file = os.path.join(assets_dir, f"teddy_{part_name.lower()}.usda")
        write_static_usda_asset(asset_file, f"Mesh_{part_name}", part["size"])
        
    usda_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_ballet_scene.usda"
    with open(usda_output, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    startTimeCode = 0\n")
        f.write(f"    endTimeCode = {total_frames - 1}\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        
        f.write("def SkelRoot \"TeddyBearSkelCharacter\"\n")
        f.write("{\n")
        
        f.write("    def Skeleton \"skele\"\n")
        f.write("    {\n")
        f.write("        uniform token[] joints = [\n")
        f.write("            \"/\", \"/Head\", \"/Head/LeftEar\", \"/Head/RightEar\",\n")
        f.write("            \"/LeftUpperArm\", \"/LeftUpperArm/LeftForearm\",\n")
        f.write("            \"/RightUpperArm\", \"/RightUpperArm/RightForearm\",\n")
        f.write("            \"/LeftThigh\", \"/LeftThigh/LeftCalf\",\n")
        f.write("            \"/RightThigh\", \"/RightThigh/RightCalf\"\n")
        f.write("        ]\n")
        f.write("    }\n\n")
        
        f.write("    def SkelAnimation \"anim\"\n")
        f.write("    {\n")
        f.write("        uniform token[] joints = [\n")
        f.write("            \"/\", \"/Head\", \"/Head/LeftEar\", \"/Head/RightEar\",\n")
        f.write("            \"/LeftUpperArm\", \"/LeftUpperArm/LeftForearm\",\n")
        f.write("            \"/RightUpperArm\", \"/RightUpperArm/RightForearm\",\n")
        f.write("            \"/LeftThigh\", \"/LeftThigh/LeftCalf\",\n")
        f.write("            \"/RightThigh\", \"/RightThigh/RightCalf\"\n")
        f.write("        ]\n")
        f.write("    }\n\n")
        
        for part_name, samples in usd_samples.items():
            f.write(f"    def \"{part_name}\" (\n")
            f.write(f"        references = @./tsfi2-deepseek/assets/teddy_{part_name.lower()}.usda@\n")
            f.write("    )\n")
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
            f.write("    }\n\n")
            
        f.write("}\n")
        
    print(f"[SUCCESS] Composed walk scene file exported: {usda_output}")
    try:
        os.remove(wav_output)
    except Exception:
        pass

if __name__ == "__main__":
    main()
