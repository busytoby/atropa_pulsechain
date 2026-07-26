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
    R, r = 70.0, 24.0
    
    # 2. B-spline curve control points (UsdGeomCurves)
    curve_controls = [
        [-120.0, 80.0, -50.0],
        [-60.0, -80.0, 0.0],
        [60.0, 100.0, 50.0],
        [120.0, -80.0, -50.0],
        [160.0, 60.0, 20.0]
    ]
    
    # Cubic B-spline interpolation helper
    def interpolate_b_spline(t):
        num_segments = len(curve_controls) - 3
        scaled_t = t * num_segments
        seg_idx = int(math.floor(scaled_t))
        if seg_idx >= num_segments:
            seg_idx = num_segments - 1
        local_t = scaled_t - seg_idx
        t2 = local_t * local_t
        t3 = t2 * local_t
        
        p0 = curve_controls[seg_idx]
        p1 = curve_controls[seg_idx + 1]
        p2 = curve_controls[seg_idx + 2]
        p3 = curve_controls[seg_idx + 3]
        
        b0 = (-t3 + 3.0*t2 - 3.0*local_t + 1.0) / 6.0
        b1 = (3.0*t3 - 6.0*t2 + 4.0) / 6.0
        b2 = (-3.0*t3 + 3.0*t2 + 3.0*local_t + 1.0) / 6.0
        b3 = t3 / 6.0
        
        return [
            b0*p0[0] + b1*p1[0] + b2*p2[0] + b3*p3[0],
            b0*p0[1] + b1*p1[1] + b2*p2[1] + b3*p3[1],
            b0*p0[2] + b1*p1[2] + b2*p2[2] + b3*p3[2]
        ]
        
    # 3. Particle System coordinates and velocities (UsdGeomPoints)
    num_particles = 12
    particles = []
    for i in range(num_particles):
        angle = i * 2.0 * math.pi / num_particles
        particles.append({
            "pos": [100.0 * math.cos(angle), 120.0 * math.sin(angle), 50.0 + i * 10.0],
            "vel": [-15.0 * math.sin(angle), 15.0 * math.cos(angle), -8.0],
            "width": 3.0 + (i % 3) * 1.5
        })
        
    # 4. Simulated FET lattice nodes (UsdShade)
    fet_grid_size = 5
    fet_nodes = []
    for row in range(fet_grid_size):
        for col in range(fet_grid_size):
            fet_nodes.append([
                (col - 2.0) * 24.0,
                -70.0,
                (row - 2.0) * 24.0
            ])
            
    print("[PixarDemo] Rendering 3D viewport cinematic simulation video...")
    for frame in range(total_frames):
        time = frame / float(FPS)
        
        # Solid dark-mode canvas background
        img = Image.new("RGB", (width, height), (12, 14, 20))
        draw = ImageDraw.Draw(img)
        
        # Grid blueprint overlay
        for x in range(0, width, 40):
            draw.line([x, 0, x, height], fill=(18, 20, 28), width=1)
        for y in range(0, height, 40):
            draw.line([0, y, width, y], fill=(18, 20, 28), width=1)
            
        # Outer viewport border
        draw.rectangle([10, 10, width - 10, height - 10], outline=(38, 42, 60), width=2)
        
        # HUD Panel headers
        draw.rectangle([20, 20, 620, 70], fill=(20, 22, 34))
        draw.text((35, 33), "AUNCIENT DYSNOMIA VM: INTEGRATED USD VIEWPORT SIMULATOR", fill=(255, 255, 255))
        
        # Camera variables (GeomCamera)
        cam_rot_y = time * 0.55
        cam_rot_x = 0.4 + 0.1 * math.sin(time * 0.4)
        cam_zoom = 290.0 + 70.0 * math.sin(time * 0.3)
        
        draw.text((30, 90), f"Camera Path: /auncient/camera/main", fill=(130, 200, 255))
        draw.text((30, 115), f"Focal Length: {int(48 + 22*math.sin(time))}mm", fill=(200, 200, 220))
        
        # Active LuxLight position (LuxLight)
        light_pos = [
            120.0 * math.cos(time * 1.5),
            120.0 * math.sin(time * 1.5),
            100.0
        ]
        draw.text((30, 150), f"LuxLight Path: /auncient/light/sunlight", fill=(130, 200, 255))
        draw.text((30, 175), f"Intensity: 2.0 (Exposure: 3.0)", fill=(200, 200, 220))
        
        # Material parameters (UsdShade)
        charge_level = 0.5 + 0.5 * math.sin(time * 2.0 * math.pi * (BPM / 120.0))
        roughness = 1.0 - charge_level
        metallic = charge_level
        
        draw.text((30, 210), f"Material: gold_foil_interface", fill=(130, 200, 255))
        draw.text((30, 235), f"Roughness: {roughness:.3f}", fill=(200, 200, 220))
        draw.text((30, 260), f"Metallic: {metallic:.3f}", fill=(200, 200, 220))
        
        # 3D projection function
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

        # 5. Draw background B-spline paths (UsdGeomCurves)
        curve_pts = []
        for step_c in range(50):
            tc = step_c / 49.0
            pt = interpolate_b_spline(tc)
            cx, cy, _ = project_pt(pt)
            curve_pts.append((cx, cy))
        draw.line(curve_pts, fill=(100, 100, 255, 120), width=3)
        
        # 6. Update and draw falling particle emitters (UsdGeomPoints)
        for p in particles:
            # Integrate physics (gravitational acceleration along Z axis)
            p["vel"][2] -= 9.8 * 0.033 # delta_time = 1/30
            p["pos"][0] += p["vel"][0] * 0.033
            p["pos"][1] += p["vel"][1] * 0.033
            p["pos"][2] += p["vel"][2] * 0.033
            
            # Wrap bounds
            if p["pos"][2] < -120.0:
                p["pos"][2] = 120.0
                p["vel"][2] = -8.0
                
            cx, cy, _ = project_pt(p["pos"])
            w = int(p["width"])
            draw.ellipse([cx - w, cy - w, cx + w, cy + w], fill=(255, 255, 150, 180))

        # 7. Draw deforming FET lattice nodes (UsdShade)
        fet_proj = []
        for pt in fet_nodes:
            displacement_z = 20.0 * math.sin(time * 3.2 + pt[0]*0.02) * charge_level
            deformed_pt = [pt[0], pt[1] + displacement_z, pt[2]]
            cx, cy, _ = project_pt(deformed_pt)
            fet_proj.append((cx, cy))
            draw.ellipse([cx - 2, cy - 2, cx + 2, cy + 2], fill=(100, 255, 100, 150))
            
        # Draw lattice springs
        for r in range(fet_grid_size):
            for c in range(fet_grid_size):
                idx = r * fet_grid_size + c
                if c + 1 < fet_grid_size:
                    idx2 = r * fet_grid_size + (c + 1)
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 90), width=1)
                if r + 1 < fet_grid_size:
                    idx2 = (r + 1) * fet_grid_size + c
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 90), width=1)

        # 8. Draw shaded solid torus faces (UsdGeomMesh) using dynamic light source attenuation
        faces = []
        for u in range(num_u):
            for v in range(num_v):
                # Corners of a quad face
                u_val = u * 2.0 * math.pi / num_u
                v_val = v * 2.0 * math.pi / num_v
                p00 = [(R + r * math.cos(v_val)) * math.cos(u_val), (R + r * math.cos(v_val)) * math.sin(u_val), r * math.sin(v_val)]
                
                u_val1 = (u + 1) * 2.0 * math.pi / num_u
                p10 = [(R + r * math.cos(v_val)) * math.cos(u_val1), (R + r * math.cos(v_val)) * math.sin(u_val1), r * math.sin(v_val)]
                
                v_val1 = (v + 1) * 2.0 * math.pi / num_v
                p11 = [(R + r * math.cos(v_val1)) * math.cos(u_val1), (R + r * math.cos(v_val1)) * math.sin(u_val1), r * math.sin(v_val1)]
                
                p01 = [(R + r * math.cos(v_val1)) * math.cos(u_val), (R + r * math.cos(v_val1)) * math.sin(u_val), r * math.sin(v_val1)]
                
                c00_x, c00_y, z00 = project_pt(p00)
                c10_x, c10_y, z10 = project_pt(p10)
                c11_x, c11_y, z11 = project_pt(p11)
                c01_x, c01_y, z01 = project_pt(p01)
                
                avg_z = (z00 + z10 + z11 + z01) / 4.0
                
                # Dynamic light vector to the center of the face
                face_center = [
                    (p00[0] + p11[0]) / 2.0,
                    (p00[1] + p11[1]) / 2.0,
                    (p00[2] + p11[2]) / 2.0
                ]
                
                # Inverse-square law attenuation
                dx = face_center[0] - light_pos[0]
                dy = face_center[1] - light_pos[1]
                dz = face_center[2] - light_pos[2]
                dist_sq = dx*dx + dy*dy + dz*dz
                light_attenuation = 15000.0 / (dist_sq + 1.0)
                
                # Lambertian dot product
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
                    
                # Light direction
                ld = [light_pos[0] - face_center[0], light_pos[1] - face_center[1], light_pos[2] - face_center[2]]
                ld_len = math.sqrt(ld[0]**2 + ld[1]**2 + ld[2]**2)
                if ld_len > 1e-4:
                    ld = [ld[0]/ld_len, ld[1]/ld_len, ld[2]/ld_len]
                    
                dot = normal[0]*ld[0] + normal[1]*ld[1] + normal[2]*ld[2]
                brightness = max(0.1, dot) * min(2.0, light_attenuation)
                
                faces.append((avg_z, [ (c00_x, c00_y), (c10_x, c10_y), (c11_x, c11_y), (c01_x, c01_y) ], brightness))
                
        faces.sort(key=lambda x: x[0], reverse=True)
        
        for avg_z, poly, brightness in faces:
            color_r = int((50 + 205 * metallic) * brightness)
            color_g = int((150 + 105 * (1.0 - roughness)) * brightness)
            color_b = int((255 * metallic) * brightness)
            
            # Clip bounds
            color_r = max(0, min(255, color_r))
            color_g = max(0, min(255, color_g))
            color_b = max(0, min(255, color_b))
            
            draw.polygon(poly, fill=(color_r, color_g, color_b), outline=(38, 42, 60))
            
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    
    if os.path.exists(audio_wav):
        os.remove(audio_wav)
        
    print(f"[SUCCESS] Pixar-style 3D simulation video rendered to: {video_output}")

if __name__ == "__main__":
    main()
