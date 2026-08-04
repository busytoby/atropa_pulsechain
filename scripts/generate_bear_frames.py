#!/usr/bin/env python3
import os
import re
import math
import numpy as np
from PIL import Image, ImageDraw

def render_frames():
    log_path = "chorus_simulation.log"
    output_dir = "rendered_frames"
    os.makedirs(output_dir, exist_ok=True)
    
    # Parse logs for positions and voltages
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
        "Trusty":  {"x": 120, "y": 240, "color": (50, 150, 255)},
        "Aggro":   {"x": 520, "y": 240, "color": (255, 50, 50)},
        "Skeptic": {"x": 220, "y": 120, "color": (200, 200, 50)},
        "Eerie":   {"x": 420, "y": 120, "color": (150, 50, 200)},
        "Coop":    {"x": 320, "y": 360, "color": (50, 200, 100)}
    }
    
    WIDTH, HEIGHT = 640, 480
    
    # Render exactly 313 frames corresponding to conversation steps
    for idx, step in enumerate(log_data):
        t = idx / 10.0
        scroll_x = int(t * 30.0) % WIDTH
        img = Image.new("RGB", (WIDTH, HEIGHT), color=(12, 14, 20)) # Dark mode blue-gray canvas
        draw = ImageDraw.Draw(img)
        
        # Grid blueprint scanlines
        for y_line in range(0, HEIGHT, 40):
            draw.line([(0, y_line), (WIDTH, y_line)], fill=(18, 20, 28), width=1)
        for x_line in range(-scroll_x, WIDTH + 40, 40):
            draw.line([(x_line, 0), (x_line, HEIGHT)], fill=(18, 20, 28), width=1)
            
        # Draw Verlet Spline connection rope between speaker and listener
        speaker_pos = bears.get(step["speaker"], bears["Trusty"])
        listener_pos = bears.get(step["listener"], bears["Coop"])
        
        rope_points = []
        num_segments = 15
        for s in range(num_segments + 1):
            ratio = s / float(num_segments)
            # Add dynamic gravity sag and wind sway (Rule 14)
            sag = 40.0 * math.sin(ratio * math.pi) * (1.0 + 0.1 * math.sin(t * 2.0))
            wind = 15.0 * math.sin(t * 3.0) * math.sin(ratio * math.pi)
            rx = speaker_pos["x"] + (listener_pos["x"] - speaker_pos["x"]) * ratio + wind
            ry = speaker_pos["y"] + (listener_pos["y"] - speaker_pos["y"]) * ratio + sag
            rope_points.append((rx, ry))
            
        draw.line(rope_points, fill=(255, 255, 255, 180), width=3)
        
        # Draw each bear head sphere
        for name, bear in bears.items():
            base_radius = 40
            if name == step["listener"]:
                base_radius += int(min(step["voltage"] * 3, 60))
            
            x, y = bear["x"], bear["y"]
            
            # Menorah-style vacuum tube backing glow
            for tube_idx in range(-3, 4):
                tx = x + (tube_idx * 12)
                ty = y + 50
                glow_val = int(min(100 + step["voltage"] * 15, 255))
                draw.line([tx, ty, tx, ty - 30], fill=(glow_val, int(glow_val * 0.5), 0), width=3)
            
            # Woven cloth fabric shading mask
            mask = Image.new("L", (WIDTH, HEIGHT), 0)
            mask_draw = ImageDraw.Draw(mask)
            mask_draw.ellipse([x - base_radius, y - base_radius, x + base_radius, y + base_radius], fill=255)
            
            img_arr = np.array(img)
            mask_arr = np.array(mask)
            y_coords, x_coords = np.where(mask_arr > 0)
            
            warp = np.sin(x_coords * 1.2) * 12.0
            weft = np.cos(y_coords * 1.2) * 12.0
            cloth_shading = warp + weft
            
            r = np.clip(bear["color"][0] + cloth_shading, 0, 255).astype(np.uint8)
            g = np.clip(bear["color"][1] + cloth_shading, 0, 255).astype(np.uint8)
            b = np.clip(bear["color"][2] + cloth_shading, 0, 255).astype(np.uint8)
            
            img_arr[y_coords, x_coords, 0] = r
            img_arr[y_coords, x_coords, 1] = g
            img_arr[y_coords, x_coords, 2] = b
            
            new_img = Image.fromarray(img_arr)
            img.paste(new_img, (0, 0))
            
            draw.ellipse([x - base_radius, y - base_radius, x + base_radius, y + base_radius], outline=(255, 255, 255), width=2)
            
            # Draw 3D Shaded Torus around active speaker (Pixar / RenderMan Ring Singularity)
            if name == step["speaker"]:
                torus_faces = []
                R, r_torus = base_radius * 1.4, base_radius * 0.3
                num_u, num_v = 12, 12
                
                # Orbit camera angle variables
                cam_rot_y = t * 0.5
                cam_rot_x = 0.5
                
                def project_torus(pt):
                    x1 = pt[0] * math.cos(cam_rot_y) - pt[2] * math.sin(cam_rot_y)
                    z1 = pt[0] * math.sin(cam_rot_y) + pt[2] * math.cos(cam_rot_y)
                    y2 = pt[1] * math.cos(cam_rot_x) - z1 * math.sin(cam_rot_x)
                    z2 = pt[1] * math.sin(cam_rot_x) + z1 * math.cos(cam_rot_x)
                    
                    scale = 200.0 / (180.0 + z2)
                    cx = x + int(x1 * scale)
                    cy = y + int(y2 * scale)
                    return cx, cy, z2
                
                for u in range(num_u):
                    for v_idx in range(num_v):
                        u_val = u * 2.0 * math.pi / num_u
                        v_val = v_idx * 2.0 * math.pi / num_v
                        
                        p00 = [(R + r_torus * math.cos(v_val)) * math.cos(u_val), (R + r_torus * math.cos(v_val)) * math.sin(u_val), r_torus * math.sin(v_val)]
                        
                        u_val1 = (u + 1) * 2.0 * math.pi / num_u
                        p10 = [(R + r_torus * math.cos(v_val)) * math.cos(u_val1), (R + r_torus * math.cos(v_val)) * math.sin(u_val1), r_torus * math.sin(v_val)]
                        
                        v_val1 = (v_idx + 1) * 2.0 * math.pi / num_v
                        p11 = [(R + r_torus * math.cos(v_val1)) * math.cos(u_val1), (R + r_torus * math.cos(v_val1)) * math.sin(u_val1), r_torus * math.sin(v_val1)]
                        
                        p01 = [(R + r_torus * math.cos(v_val1)) * math.cos(u_val), (R + r_torus * math.cos(v_val1)) * math.sin(u_val), r_torus * math.sin(v_val1)]
                        
                        c00_x, c00_y, z00 = project_torus(p00)
                        c10_x, c10_y, z10 = project_torus(p10)
                        c11_x, c11_y, z11 = project_torus(p11)
                        c01_x, c01_y, z01 = project_torus(p01)
                        
                        avg_z = (z00 + z10 + z11 + z01) / 4.0
                        
                        # Flat shading dot product
                        normal = [math.cos(u_val), math.sin(u_val), math.sin(v_val)]
                        dot = 0.5 + 0.5 * (normal[0] * -0.577 + normal[1] * -0.577 + normal[2] * 0.577)
                        
                        torus_faces.append((avg_z, [(c00_x, c00_y), (c10_x, c10_y), (c11_x, c11_y), (c01_x, c01_y)], dot))
                        
                torus_faces.sort(key=lambda f: f[0], reverse=True)
                for _, poly, sh in torus_faces:
                    tr = int(255 * sh)
                    tg = int(215 * sh)
                    tb = int(0 * sh) # Glowing gold Ring Singularity
                    draw.polygon(poly, fill=(tr, tg, tb), outline=(38, 42, 60))
            
            # Draw Lissajous projection orbit loops around the head (Tripartite VM manifestation)
            orbit_points = []
            steps_count = 30
            for pt in range(steps_count):
                angle = (pt / steps_count) * 2.0 * math.pi
                fx = 3.0
                fy = (step["pitch"] / 110.0)
                ox = x + int(base_radius * 1.3 * math.sin(fx * angle))
                oy = y + int(base_radius * 1.3 * math.cos(fy * angle + step["voltage"]))
                orbit_points.append((ox, oy))
            
            for pt in range(steps_count):
                p1 = orbit_points[pt]
                p2 = orbit_points[(pt + 1) % steps_count]
                draw.line([p1[0], p1[1], p2[0], p2[1]], fill=(0, 255, 200), width=1)
                
            draw.text((x - 20, y - 5), name, fill="black")
            
        # Draw status info
        draw.text((20, 20), "AUNCIENT DYSNOMIA VM: INTEGRATED USD VIEWPORT SIMULATOR", fill="white")
        draw.text((20, 35), f"Step: {idx + 1}/313 | Speaker: {step['speaker']} -> Listener: {step['listener']}", fill="white")
        draw.text((20, 50), f"Active Parameter: /auncient/voltage = {step['voltage']:.3f} V | pitch = {step['pitch']:.1f} Hz", fill="white")
        
        frame_path = os.path.join(output_dir, f"frame_{idx:04d}.png")
        img.save(frame_path)
        
    print(f"Rendered {len(log_data)} bear frames to '{output_dir}/'")

if __name__ == "__main__":
    render_frames()
