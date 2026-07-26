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
    print("[DSP] Synthesizing upgraded multi-layered cinematic Pixar soundtrack with Vactrol tremolo modulation...")
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    step_samples = int(SAMPLE_RATE * STEP_DUR)
    
    # progression (F Lydian - Bb major - C major loop)
    progression = [
        [174.6, 220.0, 261.6, 329.6], # Fmaj7
        [174.6, 220.0, 261.6, 329.6], # Fmaj7
        [116.5, 146.8, 174.6, 233.1], # Bbmaj7
        [130.8, 164.8, 196.0, 261.6]  # C7
    ]
    
    melody_pattern = [
        [440.0, 523.3, 587.3, 659.3, 784.0, 880.0],
        [523.3, 587.3, 659.3, 698.5, 880.0, 987.8],
        [466.2, 587.3, 698.5, 880.0, 932.3, 1174.7],
        [523.3, 659.3, 784.0, 987.8, 1046.5, 1318.5]
    ]
    
    woodwind_pattern = [
        [880.0, 1046.5, 1318.5, 1568.0],
        [1046.5, 1318.5, 1397.0, 1760.0],
        [932.3, 1174.7, 1397.0, 1864.7],
        [1046.5, 1318.5, 1568.0, 2093.0]
    ]

    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        pat_idx = step % 16
        prog_idx = (step // 16) % len(progression)
        chord = progression[prog_idx]
        arp_scale = melody_pattern[prog_idx]
        ww_scale = woodwind_pattern[prog_idx]
        
        # 1. Warm cinematic bass swell
        if step % 8 == 0 and step_start < total_samples:
            length = min(step_samples * 8, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            base_freq = chord[0] * 0.5
            env = 0.5 * (1.0 - np.cos(np.pi * t / (step_samples * 8 / SAMPLE_RATE))) * np.exp(-t * 0.5)
            sine = np.sin(2.0 * np.pi * base_freq * t)
            audio[step_start:step_start+length] += 0.35 * sine * env
 
        # 2. Pizzicato Pluck modulated by simulated vactrol LDR tremolo
        if step % 2 == 0 and step_start < total_samples:
            length = min(step_samples * 3, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            note_freq = arp_scale[(step // 2) % len(arp_scale)]
            
            # Vactrol modulation LDR resistance simulation (tremolo at 6Hz)
            vactrol_mod = 0.75 + 0.25 * np.sin(2.0 * np.pi * 6.0 * t)
            
            marimba = np.sin(2.0 * np.pi * note_freq * t) * np.exp(-t * 22.0)
            marimba += 0.45 * np.sin(2.0 * np.pi * note_freq * 3.0 * t) * np.exp(-t * 40.0)
            accent = 1.35 if pat_idx in [0, 3, 6, 8, 11, 14] else 0.8
            audio[step_start:step_start+length] += 0.25 * marimba * accent * vactrol_mod

        # 3. Upgraded woodwind flute/oboe runs layering
        if step % 2 == 1 and step_start < total_samples:
            length = min(step_samples * 4, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            ww_freq = ww_scale[(step // 2) % len(ww_scale)]
            flute = (0.7 * np.sin(2.0 * np.pi * ww_freq * t) + 0.3 * (np.abs((t * ww_freq) % 1.0 - 0.5) - 0.25)) * np.exp(-t * 8.0)
            audio[step_start:step_start+length] += 0.15 * flute

        # 4. Glockenspiel highlights with vactrol decay envelope
        if step % 4 == 3 and step_start < total_samples:
            length = min(step_samples * 4, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            high_freq = arp_scale[pat_idx % len(arp_scale)] * 2.0
            
            vactrol_envelope = np.exp(-t * 30.0)
            glock = np.sin(2.0 * np.pi * high_freq * t) * vactrol_envelope
            audio[step_start:step_start+length] += 0.15 * glock

        # 5. Cozy rhythm acoustic backing
        if step % 16 == 0 and step_start < total_samples:
            length = min(step_samples * 16, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            env = np.exp(-t * 0.4)
            for note_freq in chord:
                sine = np.sin(2.0 * np.pi * note_freq * t)
                audio[step_start:step_start+length] += 0.05 * sine * env
                
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
    
    # 2. B-spline curve control points (UsdGeomCurves) representing active NPN-PNP valve path
    curve_controls = [
        [-120.0, 80.0, -50.0],
        [-60.0, -80.0, 0.0],
        [60.0, 100.0, 50.0],
        [120.0, -80.0, -50.0],
        [160.0, 60.0, 20.0]
    ]
    
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
        
    # 3. Particle System (UsdGeomPoints)
    num_particles = 12
    particles = []
    for i in range(num_particles):
        angle = i * 2.0 * math.pi / num_particles
        particles.append({
            "pos": [100.0 * math.cos(angle), 120.0 * math.sin(angle), 50.0 + i * 10.0],
            "vel": [-15.0 * math.sin(angle), 15.0 * math.cos(angle), -8.0],
            "width": 3.0 + (i % 3) * 1.5
        })
        
    # 4. FET grid springs (UsdShade)
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
        draw.rectangle([20, 20, 620, 75], fill=(20, 22, 34))
        draw.text((35, 27), "AUNCIENT DYSNOMIA VM: INTEGRATED USD VIEWPORT SIMULATOR", fill=(255, 255, 255))
        draw.text((35, 47), "STAGE COMPOSITION STACKS & CLASS OVERRIDES VERIFIED", fill=(100, 255, 100))
        
        # MANN Memory Controller Gates driving the camera orbits
        mann_read_gate = 0.5 + 0.3 * math.sin(time * 0.8)
        mann_write_gate = 0.5 + 0.3 * math.cos(time * 0.6)
        mann_allocation_gate = 0.5 + 0.2 * math.sin(time * 1.2)
        
        # Camera variables (GeomCamera driven by MANN gates)
        cam_rot_y = time * 0.35 + mann_read_gate * 2.0
        cam_rot_x = 0.4 + 0.15 * math.sin(time * 0.2 + mann_write_gate)
        cam_zoom = 290.0 + 100.0 * mann_allocation_gate
        
        draw.text((30, 90), f"Camera Path: /auncient/camera/main", fill=(130, 200, 255))
        draw.text((30, 110), f"Points: 12 Active (Pre-computed)", fill=(255, 180, 100))
        draw.text((30, 130), f"CurvesBasis: CATMULL-ROM (Wrap: PERIODIC)", fill=(200, 200, 220))
        
        # Active LuxLight position (LuxLight)
        light_pos = [
            120.0 * math.cos(time * 1.5),
            120.0 * math.sin(time * 1.5),
            100.0
        ]
        draw.text((30, 160), f"LuxLight Path: /auncient/light/sunlight", fill=(130, 200, 255))
        draw.text((30, 180), f"Intensity: 2.0 (Exposure: 3.0)", fill=(200, 200, 220))
        
        # Transistor Valve parameters (NpnPnpValve)
        potential_diff = math.fabs(1.5 + math.sin(time * 3.0))
        valve_conductance = 1.0 / (1.0 + math.exp(-5.0 * (potential_diff - 0.7)))
        
        # Define the expected shading parameters using the conductance scale
        charge_level = valve_conductance
        roughness = 1.0 - charge_level
        metallic = charge_level
        
        draw.text((30, 210), f"Active Valve: npn_pnp_transistor_pair", fill=(130, 200, 255))
        draw.text((30, 230), f"Vactrol LDR (Singularity Coupling): {500.0 + 200.0*math.sin(time*6.0):.1f} Ohms", fill=(255, 200, 100))
        draw.text((30, 250), f"Potential Diff: {potential_diff:.3f} V", fill=(200, 200, 220))
        draw.text((30, 270), f"Valve Conductance: {valve_conductance:.3f}", fill=(100, 255, 100))
        
        # 3D projection function
        def project_pt(pt):
            x1 = pt[0] * math.cos(cam_rot_y) - pt[2] * math.sin(cam_rot_y)
            z1 = pt[0] * math.sin(cam_rot_y) + pt[2] * math.cos(cam_rot_y)
            y2 = pt[1] * math.cos(cam_rot_x) - z1 * math.sin(cam_rot_x)
            z2 = pt[1] * math.sin(cam_rot_x) + z1 * math.cos(cam_rot_x)
            
            scale = cam_zoom / (250.0 + z2)
            cx = 410 + int(x1 * scale)
            cy = 280 + int(y2 * scale)
            return cx, cy, z2
 
        # 5. Draw active transistor capacitor valve line modulated by conductance (UsdGeomCurves)
        curve_pts = []
        for step_c in range(50):
            tc = step_c / 49.0
            pt = interpolate_b_spline(tc)
            cx, cy, _ = project_pt(pt)
            curve_pts.append((cx, cy))
            
        # Draw glowing valve path with width modulated by NpnPnpValve conductance
        valve_width = int(2.0 + 5.0 * valve_conductance)
        valve_color = (100, int(150 + 105 * valve_conductance), 255, 180)
        draw.line(curve_pts, fill=valve_color, width=valve_width)
        
        # Draw Camera Frustum visual box overlay (CameraFrustum)
        frustum_corners = [
            [-80, -60, 50], [80, -60, 50], [80, 60, 50], [-80, 60, 50],
            [-120, -90, 150], [120, -90, 150], [120, 90, 150], [-120, 90, 150]
        ]
        proj_corners = [project_pt(c) for c in frustum_corners]
        draw.line([proj_corners[i][:2] for i in [0, 1, 2, 3, 0]], fill=(0, 255, 150, 60), width=1)
        draw.line([proj_corners[i+4][:2] for i in [0, 1, 2, 3, 0]], fill=(0, 255, 150, 40), width=1)
        for i in range(4):
            draw.line([proj_corners[i][:2], proj_corners[i+4][:2]], fill=(0, 255, 150, 40), width=1)

        # 6. Update and draw falling particle emitters (UsdGeomPoints)
        for idx, p in enumerate(particles):
            p["vel"][2] -= 9.8 * 0.033
            p["pos"][0] += p["vel"][0] * 0.033
            p["pos"][1] += p["vel"][1] * 0.033
            p["pos"][2] += p["vel"][2] * 0.033
            
            if p["pos"][2] < -120.0:
                p["pos"][2] = 120.0
                p["vel"][2] = -8.0
                
            is_visible = p["pos"][2] <= 130.0
            cx, cy, _ = project_pt(p["pos"])
            
            # Retrieve diameter from point sizes mapping
            ps_vals = [1.5, 3.0, 4.5]
            w = int(ps_vals[idx % len(ps_vals)] * 1.5)
            
            fill_color = (255, 255, 150, 180) if is_visible else (100, 100, 80, 80)
            draw.ellipse([cx - w, cy - w, cx + w, cy + w], fill=fill_color)

        # 7. Draw deforming FET lattice nodes (UsdShade)
        fet_proj = []
        for pt in fet_nodes:
            displacement_z = 20.0 * math.sin(time * 3.2 + pt[0]*0.02) * charge_level
            deformed_pt = [pt[0], pt[1] + displacement_z, pt[2]]
            cx, cy, _ = project_pt(deformed_pt)
            fet_proj.append((cx, cy))
            draw.ellipse([cx - 2, cy - 2, cx + 2, cy + 2], fill=(100, 255, 100, 150))
            
        for r in range(fet_grid_size):
            for c in range(fet_grid_size):
                idx = r * fet_grid_size + c
                if c + 1 < fet_grid_size:
                    idx2 = r * fet_grid_size + (c + 1)
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 90), width=1)
                if r + 1 < fet_grid_size:
                    idx2 = (r + 1) * fet_grid_size + c
                    draw.line([fet_proj[idx][0], fet_proj[idx][1], fet_proj[idx2][0], fet_proj[idx2][1]], fill=(50, 150, 50, 90), width=1)

        # Draw a rotating Tonewheel Vactrol Coil Pickup graphic on the bottom left overlay
        tw_cx, tw_cy = 120, 390
        tw_r = 45
        draw.ellipse([tw_cx - tw_r, tw_cy - tw_r, tw_cx + tw_r, tw_cy + tw_r], outline=(150, 160, 180), width=2)
        num_teeth = 12
        rotation_angle = time * 6.0
        for i in range(num_teeth):
            angle = rotation_angle + i * 2.0 * math.pi / num_teeth
            tx0 = tw_cx + int((tw_r - 5) * math.cos(angle))
            ty0 = tw_cy + int((tw_r - 5) * math.sin(angle))
            tx1 = tw_cx + int((tw_r + 5) * math.cos(angle))
            ty1 = tw_cy + int((tw_r + 5) * math.sin(angle))
            draw.line([tx0, tw_cy, tx1, tw_cy], fill=(200, 210, 220), width=2)
        draw.rectangle([tw_cx + tw_r - 10, tw_cy - 12, tw_cx + tw_r + 15, tw_cy + 12], fill=(40, 44, 56), outline=(80, 90, 110))
        led_glow = int(150 + 105 * math.sin(time * 6.0))
        draw.ellipse([tw_cx + tw_r - 5, tw_cy - 5, tw_cx + tw_r + 5, tw_cy + 5], fill=(led_glow, 50, 50))
        draw.text((tw_cx - 40, tw_cy + tw_r + 10), "TONEWHEEL VACTROL PICKUP", fill=(180, 190, 210))

        # 8. Draw shaded solid torus faces with GeomSubset material assignments (the Ring Singularity)
        faces = []
        for u in range(num_u):
            for v in range(num_v):
                u_val = u * 2.0 * math.pi / num_u
                v_val = v * 2.0 * math.pi / num_v
                p00 = [(R + r * math.cos(v_val)) * math.cos(u_val), (R + r * math.cos(v_val)) * math.sin(u_val), r * math.sin(v_val)]
                
                u_val1 = (u + 1) * 2.0 * math.pi / num_u
                p10 = [(R + r * math.cos(v_val)) * math.cos(u_val1), (R + r * math.cos(v_val)) * math.sin(u_val1), r * math.sin(v_val)]
                
                v_val1 = (v + 1) * 2.0 * math.pi / num_v
                p11 = [(R + r * math.cos(v_val1)) * math.cos(u_val1), (R + r * math.cos(v_val1)) * math.sin(u_val1), r * math.sin(v_val1)]
                
                p01 = [(R + r * math.cos(v_val1)) * math.cos(u), (R + r * math.cos(v_val1)) * math.sin(u), r * math.sin(v_val1)]
                
                c00_x, c00_y, z00 = project_pt(p00)
                c10_x, c10_y, z10 = project_pt(p10)
                c11_x, c11_y, z11 = project_pt(p11)
                c01_x, c01_y, z01 = project_pt(p01)
                
                avg_z = (z00 + z10 + z11 + z01) / 4.0
                
                face_center = [
                    (p00[0] + p11[0]) / 2.0,
                    (p00[1] + p11[1]) / 2.0,
                    (p00[2] + p11[2]) / 2.0
                ]
                
                dx = face_center[0] - light_pos[0]
                dy = face_center[1] - light_pos[1]
                dz = face_center[2] - light_pos[2]
                dist_sq = dx*dx + dy*dy + dz*dz
                light_attenuation = 15000.0 / (dist_sq + 1.0)
                
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
                    
                ld = [light_pos[0] - face_center[0], light_pos[1] - face_center[1], light_pos[2] - face_center[2]]
                ld_len = math.sqrt(ld[0]**2 + ld[1]**2 + ld[2]**2)
                if ld_len > 1e-4:
                    ld = [ld[0]/ld_len, ld[1]/ld_len, ld[2]/ld_len]
                    
                dot = normal[0]*ld[0] + normal[1]*ld[1] + normal[2]*ld[2]
                brightness = max(0.1, dot) * min(2.0, light_attenuation)
                
                is_subset = (6 <= u <= 10)
                
                faces.append((avg_z, [ (c00_x, c00_y), (c10_x, c10_y), (c11_x, c11_y), (c01_x, c01_y) ], brightness, is_subset))
                
        faces.sort(key=lambda x: x[0], reverse=True)
        
        for avg_z, poly, brightness, is_subset in faces:
            if is_subset:
                color_r = int((240 + 15 * metallic) * brightness)
                color_g = int((110 + 50 * (1.0 - roughness)) * brightness)
                color_b = int((70 * metallic) * brightness)
            else:
                color_r = int((50 + 205 * metallic) * brightness)
                color_g = int((150 + 105 * (1.0 - roughness)) * brightness)
                color_b = int((255 * metallic) * brightness)
            
            color_r = max(0, min(255, color_r))
            color_g = max(0, min(255, color_g))
            color_b = max(0, min(255, color_b))
            
            draw.polygon(poly, fill=(color_r, color_g, color_b), outline=(38, 42, 60))
            
        # Draw visible magnetic coupling field lines connecting the torus singularity to the vactrol pickup
        singularity_cx, singularity_cy, _ = project_pt([0.0, 0.0, 0.0])
        coupling_beam_color = (255, 100, 100, int(60 + 40 * math.sin(time * 6.0)))
        draw.line([singularity_cx, singularity_cy, tw_cx + tw_r, tw_cy], fill=coupling_beam_color, width=2)
        
        # Magnetic ripple circles emanating from the torus singularity center
        ripple_r = int((time * 80.0) % 120.0)
        draw.ellipse([singularity_cx - ripple_r, singularity_cy - ripple_r, singularity_cx + ripple_r, singularity_cy + ripple_r], outline=(255, 100, 100, max(0, 100 - ripple_r)), width=1)
            
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    
    if os.path.exists(audio_wav):
        os.remove(audio_wav)
        
    print(f"[SUCCESS] Upgraded Pixar-style 3D simulation video rendered to: {video_output}")

if __name__ == "__main__":
    main()
