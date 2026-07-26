import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont

SAMPLE_RATE = 44100
FPS = 30
DURATION = 45.0  # 45 seconds long demo
BPM = 100
STEP_DUR = 60.0 / (BPM * 4)  # 16th note step (~0.15s)

def generate_audio():
    print("[DSP] Synthesizing whimsical cinematic Pixar-style soundtrack...")
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    step_samples = int(SAMPLE_RATE * STEP_DUR)
    
    # Whimsical progression (F Lydian - Bb major - C major loop)
    progression = [
        [174.6, 220.0, 261.6, 329.6], # Fmaj7
        [174.6, 220.0, 261.6, 329.6], # Fmaj7
        [116.5, 146.8, 174.6, 233.1], # Bbmaj7
        [130.8, 164.8, 196.0, 261.6]  # C7
    ]
    
    # Randy Newman style pizzicato mallet/string melodies
    melody_pattern = [
        [440.0, 523.3, 587.3, 659.3, 784.0, 880.0], # Whimsical runs
        [523.3, 587.3, 659.3, 698.5, 880.0, 987.8],
        [466.2, 587.3, 698.5, 880.0, 932.3, 1174.7],
        [523.3, 659.3, 784.0, 987.8, 1046.5, 1318.5]
    ]

    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        pat_idx = step % 16
        prog_idx = (step // 16) % len(progression)
        chord = progression[prog_idx]
        arp_scale = melody_pattern[prog_idx]
        
        # 1. Warm cinematic bass swell
        if step % 8 == 0 and step_start < total_samples:
            length = min(step_samples * 8, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            base_freq = chord[0] * 0.5
            env = 0.5 * (1.0 - np.cos(np.pi * t / (step_samples * 8 / SAMPLE_RATE))) * np.exp(-t * 0.5)
            sine = np.sin(2.0 * np.pi * base_freq * t)
            audio[step_start:step_start+length] += 0.3 * sine * env

        # 2. Whimsical Pizzicato String Pluck (Mallet)
        if step % 2 == 0 and step_start < total_samples:
            length = min(step_samples * 3, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            note_freq = arp_scale[(step // 2) % len(arp_scale)]
            
            # Sine wave with fast-decaying odd harmonics (woodblock / marimba timbre)
            marimba = np.sin(2.0 * np.pi * note_freq * t) * np.exp(-t * 22.0)
            marimba += 0.45 * np.sin(2.0 * np.pi * note_freq * 3.0 * t) * np.exp(-t * 40.0)
            marimba += 0.20 * np.sin(2.0 * np.pi * note_freq * 5.0 * t) * np.exp(-t * 60.0)
            
            # Syncopated rhythm accentuation
            accent = 1.35 if pat_idx in [0, 3, 6, 8, 11, 14] else 0.8
            audio[step_start:step_start+length] += 0.25 * marimba * accent

        # 3. Soft Glockenspiel highlights
        if step % 4 == 3 and step_start < total_samples:
            length = min(step_samples * 4, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            high_freq = arp_scale[pat_idx % len(arp_scale)] * 2.0
            glock = np.sin(2.0 * np.pi * high_freq * t) * np.exp(-t * 35.0)
            audio[step_start:step_start+length] += 0.12 * glock

        # 4. Cozy rhythm acoustic backing
        if step % 16 == 0 and step_start < total_samples:
            length = min(step_samples * 16, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            env = np.exp(-t * 0.4)
            for note_freq in chord:
                sine = np.sin(2.0 * np.pi * note_freq * t)
                audio[step_start:step_start+length] += 0.04 * sine * env
                
    # Normalize audio
    max_val = np.max(np.abs(audio))
    if max_val > 1.0:
        audio /= max_val
        
    audio_path = "/tmp/pixar_demo_tune.wav"
    import wave
    import struct
    with wave.open(audio_path, "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        for val in audio:
            int_val = int(max(-32768, min(32767, val * 32767)))
            wav_file.writeframesraw(struct.pack("<h", int_val))
    return audio_path

def main():
    audio_wav = generate_audio()
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/pixar_demo.mp4"
    width, height = 640, 480
    total_frames = int(FPS * DURATION)
    
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_wav,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    # 1. Torus Mesh geometry (UsdGeomMesh)
    num_u, num_v = 16, 16
    R, r = 75.0, 28.0
    
    # 2. Simulated FET lattice nodes (UsdShade)
    fet_grid_size = 6
    fet_nodes = []
    for row in range(fet_grid_size):
        for col in range(fet_grid_size):
            fet_nodes.append([
                (col - 2.5) * 20.0,
                -75.0, # Placed below the torus
                (row - 2.5) * 20.0
            ])
            
    print("[PixarDemo] Rendering 3D viewport cinematic simulation video...")
    for frame in range(total_frames):
        time = frame / float(FPS)
        
        # Draw all elements directly onto a clean solid dark-mode canvas to save compile time
        img = Image.new("RGB", (width, height), (12, 14, 20))
        draw = ImageDraw.Draw(img)
        
        # Subtle coordinate grid lines
        for x in range(0, width, 40):
            draw.line([x, 0, x, height], fill=(18, 20, 28), width=1)
        for y in range(0, height, 40):
            draw.line([0, y, width, y], fill=(18, 20, 28), width=1)
            
        # Outer border
        draw.rectangle([10, 10, width - 10, height - 10], outline=(38, 42, 60), width=2)
        
        # HUD Panel headers
        draw.rectangle([20, 20, 620, 70], fill=(20, 22, 34))
        draw.text((35, 33), "AUNCIENT DYSNOMIA VM: USD CINEMATIC RENDER DEMO", fill=(255, 255, 255))
        
        # Camera variables (GeomCamera)
        cam_rot_y = time * 0.65
        cam_rot_x = 0.35 + 0.12 * math.sin(time * 0.5)
        cam_zoom = 280.0 + 90.0 * math.sin(time * 0.35)
        
        draw.text((30, 90), f"Camera Path: /auncient/camera/main", fill=(130, 200, 255))
        draw.text((30, 115), f"Focal Length: {int(45 + 25*math.sin(time))}mm", fill=(200, 200, 220))
        draw.text((30, 140), f"Clipping: [0.1, 1000.0]", fill=(200, 200, 220))
        
        # Material parameters (UsdShade)
        charge_level = 0.5 + 0.5 * math.sin(time * 2.0 * math.pi * (BPM / 120.0))
        roughness = 1.0 - charge_level
        metallic = charge_level
        
        draw.text((30, 180), f"Material: gold_foil_interface", fill=(130, 200, 255))
        draw.text((30, 205), f"Roughness: {roughness:.3f}", fill=(200, 200, 220))
        draw.text((30, 230), f"Metallic: {metallic:.3f}", fill=(200, 200, 220))
        
        # Directional light source orbiting in 3D
        light_dir = [
            math.cos(time * 1.5),
            math.sin(time * 1.5),
            0.5
        ]
        # Normalize light vector
        l_len = math.sqrt(light_dir[0]**2 + light_dir[1]**2 + light_dir[2]**2)
        light_dir = [light_dir[0]/l_len, light_dir[1]/l_len, light_dir[2]/l_len]
        
        # Project 3D point function
        def project_pt(pt):
            # Rotate Y
            x1 = pt[0] * math.cos(cam_rot_y) - pt[2] * math.sin(cam_rot_y)
            z1 = pt[0] * math.sin(cam_rot_y) + pt[2] * math.cos(cam_rot_y)
            # Rotate X
            y2 = pt[1] * math.cos(cam_rot_x) - z1 * math.sin(cam_rot_x)
            z2 = pt[1] * math.sin(cam_rot_x) + z1 * math.cos(cam_rot_x)
            
            scale = cam_zoom / (250.0 + z2)
            cx = 410 + int(x1 * scale)
            cy = 280 + int(y2 * scale)
            return cx, cy, z2

        # 3. Draw deforming FET lattice nodes (UsdShade)
        fet_proj = []
        for pt in fet_nodes:
            displacement_z = 22.0 * math.sin(time * 3.5 + pt[0]*0.02) * charge_level
            deformed_pt = [pt[0], pt[1] + displacement_z, pt[2]]
            cx, cy, _ = project_pt(deformed_pt)
            fet_proj.append((cx, cy))
            draw.ellipse([cx - 2, cy - 2, cx + 2, cy + 2], fill=(100, 255, 100, 160))
            
        # Draw lattice springs
        for r in range(fet_grid_size):
            for c in range(fet_grid_size):
                idx = r * fet_grid_size + c
                if c + 1 < fet_grid_size:
                    idx2 = r * fet_grid_size + (c + 1)
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 110), width=1)
                if r + 1 < fet_grid_size:
                    idx2 = (r + 1) * fet_grid_size + c
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 110), width=1)

        # 4. Draw shaded solid torus faces (UsdGeomMesh) using Painter's Algorithm for depth sorting
        faces = []
        for u in range(num_u):
            for v in range(num_v):
                # Fetch indices of the 4 corners of a quadrilateral face
                idx00 = u * num_v + v
                idx10 = ((u + 1) % num_u) * num_v + v
                idx11 = ((u + 1) % num_u) * num_v + ((v + 1) % num_v)
                idx01 = u * num_v + ((v + 1) % num_v)
                
                # Fetch original 3D coordinates
                # Corner 00
                u_val = u * 2.0 * math.pi / num_u
                v_val = v * 2.0 * math.pi / num_v
                p00 = [(R + r * math.cos(v_val)) * math.cos(u_val), (R + r * math.cos(v_val)) * math.sin(u_val), r * math.sin(v_val)]
                
                # Corner 10
                u_val1 = (u + 1) * 2.0 * math.pi / num_u
                p10 = [(R + r * math.cos(v_val)) * math.cos(u_val1), (R + r * math.cos(v_val)) * math.sin(u_val1), r * math.sin(v_val)]
                
                # Corner 11
                v_val1 = (v + 1) * 2.0 * math.pi / num_v
                p11 = [(R + r * math.cos(v_val1)) * math.cos(u_val1), (R + r * math.cos(v_val1)) * math.sin(u_val1), r * math.sin(v_val1)]
                
                # Corner 01
                p01 = [(R + r * math.cos(v_val1)) * math.cos(u_val), (R + r * math.cos(v_val1)) * math.sin(u_val), r * math.sin(v_val1)]
                
                # Project coordinates
                c00_x, c00_y, z00 = project_pt(p00)
                c10_x, c10_y, z10 = project_pt(p10)
                c11_x, c11_y, z11 = project_pt(p11)
                c01_x, c01_y, z01 = project_pt(p01)
                
                avg_z = (z00 + z10 + z11 + z01) / 4.0
                
                # Calculate normal vector of the face using cross product
                v1 = [p10[0] - p00[0], p10[1] - p00[1], p10[2] - p00[2]]
                v2 = [p01[0] - p00[0], p01[1] - p00[1], p01[2] - p00[2]]
                normal = [
                    v1[1]*v2[2] - v1[2]*v2[1],
                    v1[2]*v2[0] - v1[0]*v2[2],
                    v1[0]*v2[1] - v1[1]*v2[0]
                ]
                n_len = math.sqrt(normal[0]**2 + normal[1]**2 + normal[2]**2)
                if n_len > 1e-4:
                    normal = [normal[0]/n_len, normal[1]/n_len, normal[2]/n_len]
                else:
                    normal = [0.0, 0.0, 1.0]
                    
                # Lambertian diffuse lighting dot product
                dot = normal[0]*light_dir[0] + normal[1]*light_dir[1] + normal[2]*light_dir[2]
                brightness = max(0.1, dot)
                
                faces.append((avg_z, [ (c00_x, c00_y), (c10_x, c10_y), (c11_x, c11_y), (c01_x, c01_y) ], brightness))
                
        # Sort faces by depth (back to front) for Painter's algorithm
        faces.sort(key=lambda x: x[0], reverse=True)
        
        # Draw shaded polygons
        for avg_z, poly, brightness in faces:
            # Dynamic albedo colors modulated by charge and specular highlights
            color_r = int((50 + 205 * metallic) * brightness)
            color_g = int((150 + 105 * (1.0 - roughness)) * brightness)
            color_b = int((255 * metallic) * brightness)
            
            draw.polygon(poly, fill=(color_r, color_g, color_b), outline=(38, 42, 60))
            
        # Draw some vertex points on top with bloom
        for avg_z, poly, _ in faces[::4]:
            cx, cy = poly[0]
            draw.ellipse([cx - 3, cy - 3, cx + 3, cy + 3], fill=(255, 255, 200))
            
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    
    if os.path.exists(audio_wav):
        os.remove(audio_wav)
        
    print(f"[SUCCESS] Pixar-style 3D simulation video rendered to: {video_output}")

if __name__ == "__main__":
    main()
