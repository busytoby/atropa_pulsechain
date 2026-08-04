#!/usr/bin/env python3
import os
import re
import math
import numpy as np
from PIL import Image, ImageDraw

def generate_3d_teddy_wireframe():
    vertices = []
    edges = []
    
    # 1. Head (Sphere)
    head_center = (0.0, 1.4, 0.0)
    head_rad = 0.5
    head_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat = math.cos(lat_angle)
        sin_lat = math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = head_center[0] + head_rad * cos_lat * math.cos(lon_angle)
            vy = head_center[1] + head_rad * sin_lat
            vz = head_center[2] + head_rad * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    for lat in range(9):
        for lon in range(8):
            curr = head_v_start + lat * 8 + lon
            nxt_lon = head_v_start + lat * 8 + ((lon + 1) % 8)
            edges.append((curr, nxt_lon))
            if lat < 8:
                nxt_lat = head_v_start + (lat + 1) * 8 + lon
                edges.append((curr, nxt_lat))
                
    # 2. Lower Jaw / Mouth
    jaw_v_start = len(vertices)
    jaw_center = (0.0, 1.1, 0.25)
    jaw_rad = 0.2
    for lon in range(6):
        lon_angle = (lon / 6.0) * math.pi + math.pi
        vx = jaw_center[0] + jaw_rad * math.cos(lon_angle)
        vy = jaw_center[1] + jaw_rad * math.sin(lon_angle)
        vz = jaw_center[2]
        vertices.append((vx, vy, vz, 1)) # joint type 1: moving jaw
        
    for lon in range(5):
        edges.append((jaw_v_start + lon, jaw_v_start + lon + 1))
        
    # 3. Body (Ellipsoid)
    body_center = (0.0, 0.3, 0.0)
    body_rad_x, body_rad_y, body_rad_z = 0.7, 0.8, 0.5
    body_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat = math.cos(lat_angle)
        sin_lat = math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = body_center[0] + body_rad_x * cos_lat * math.cos(lon_angle)
            vy = body_center[1] + body_rad_y * sin_lat
            vz = body_center[2] + body_rad_z * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    for lat in range(9):
        for lon in range(8):
            curr = body_v_start + lat * 8 + lon
            nxt_lon = body_v_start + lat * 8 + ((lon + 1) % 8)
            edges.append((curr, nxt_lon))
            if lat < 8:
                nxt_lat = body_v_start + (lat + 1) * 8 + lon
                edges.append((curr, nxt_lat))
                
    # 4. Ears
    for sign in [-1, 1]:
        ear_center = (sign * 0.45, 1.8, 0.0)
        ear_rad = 0.2
        ear_v_start = len(vertices)
        for lat in range(-2, 3):
            lat_angle = (lat / 3.0) * (math.pi / 2.0)
            cos_lat = math.cos(lat_angle)
            sin_lat = math.sin(lat_angle)
            for lon in range(6):
                lon_angle = (lon / 6.0) * 2.0 * math.pi
                vx = ear_center[0] + ear_rad * cos_lat * math.cos(lon_angle)
                vy = ear_center[1] + ear_rad * sin_lat
                vz = ear_center[2] + ear_rad * cos_lat * math.sin(lon_angle)
                vertices.append((vx, vy, vz, 0))
                
        for lat in range(5):
            for lon in range(6):
                curr = ear_v_start + lat * 6 + lon
                nxt_lon = ear_v_start + lat * 6 + ((lon + 1) % 6)
                edges.append((curr, nxt_lon))
                if lat < 4:
                    nxt_lat = ear_v_start + (lat + 1) * 6 + lon
                    edges.append((curr, nxt_lat))
                    
    return vertices, edges

def quaternion_multiply(q1, q2):
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    return (
        w1*w2 - x1*x2 - y1*y2 - z1*z2,
        w1*x2 + x1*w2 + y1*z2 - z1*y2,
        w1*y2 - x1*z2 + y1*w2 + z1*x2,
        w1*z2 + x1*y2 - y1*x2 + z1*w2
    )

def rotate_vector_by_quaternion(v, q):
    q_vec = (0, v[0], v[1], v[2])
    q_conj = (q[0], -q[1], -q[2], -q[3])
    temp = quaternion_multiply(q, q_vec)
    result = quaternion_multiply(temp, q_conj)
    return (result[1], result[2], result[3])

def render_frames():
    log_path = "chorus_simulation.log"
    output_dir = "rendered_frames"
    os.makedirs(output_dir, exist_ok=True)
    
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
        "Trusty":  {"x": 120, "y": 240, "color": "#0099ff"},
        "Aggro":   {"x": 520, "y": 240, "color": "#ff3333"},
        "Skeptic": {"x": 220, "y": 120, "color": "#e6e600"},
        "Eerie":   {"x": 420, "y": 120, "color": "#9933ff"},
        "Coop":    {"x": 320, "y": 360, "color": "#33cc66"}
    }
    
    WIDTH, HEIGHT = 640, 480
    vertices, edges = generate_3d_teddy_wireframe()
    
    # 3D Starfield background
    np.random.seed(420)
    stars = [{'x': np.random.uniform(-150, 150), 'y': np.random.uniform(-200, 200), 'z': np.random.uniform(20, 300)} for _ in range(30)]
    
    for idx, step in enumerate(log_data):
        time_sec = idx / 10.0
        progress = idx / len(log_data)
        
        bg = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        img = Image.fromarray(bg)
        draw = ImageDraw.Draw(img)
        
        # 1. Draw Starfield
        for star in stars:
            z_pos = (star['z'] - idx * 2.0) % 280 + 20
            factor = 140.0 / z_pos
            sx = int(WIDTH / 2) + int(star['x'] * factor)
            sy = int(HEIGHT / 2) + int(star['y'] * factor)
            if 0 <= sx < WIDTH and 0 <= sy < HEIGHT:
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=(130, 130, 255))
                
        # 2. Draw 3D Perspective Grid
        draw.line([(0, 380), (WIDTH, 380)], fill="#003366", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 840.0 - 100.0
            draw.line([(WIDTH / 2, 380), (x_bottom, 480)], fill=(0, 30, 90), width=1)
            
        # 3. Rotate and Render 5 Teddy Bears in 3D wireframe layout
        for name, bear in bears.items():
            cx, cy = bear["x"], bear["y"]
            
            # Rotation angles based on conversation step
            angle_y = progress * 2.0 * math.pi + (hash(name) % 100) / 10.0
            angle_x = 0.1 * math.sin(time_sec * 2.0 * math.pi)
            
            qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
            qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
            q_rot = quaternion_multiply(qy, qx)
            
            # Active speaker mouth moves
            amp_flow = 0.0
            if name == step["speaker"]:
                amp_flow = 0.8 + 0.2 * math.sin(time_sec * 30.0)
                
            projected = []
            for vx, vy, vz, joint in vertices:
                if joint == 1:
                    vy = vy - 0.15 * amp_flow
                    
                rx, ry, rz = rotate_vector_by_quaternion((vx, vy, vz), q_rot)
                rz += 3.8
                factor = 120.0 / rz
                
                proj_x = cx + int(rx * factor * 50.0)
                proj_y = cy - int(ry * factor * 50.0)
                projected.append((proj_x, proj_y))
                
            # Draw bear wireframe edges
            for edge in edges:
                p1 = projected[edge[0]]
                p2 = projected[edge[1]]
                draw.line([p1, p2], fill=bear["color"], width=1)
                
        # Draw status overlay
        draw.text((20, 20), "TSFi/2: 5-VOICE BEAR CONVERSATION WIREFRAME", fill="#ff007f")
        draw.text((20, 35), f"Step: {idx + 1}/313 | Speaker: {step['speaker']} -> Listener: {step['listener']}", fill="#00f2fe")
        draw.text((20, 50), f"Active Parameter: /auncient/voltage = {step['voltage']:.3f} V | pitch = {step['pitch']:.1f} Hz", fill="#ffd700")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
