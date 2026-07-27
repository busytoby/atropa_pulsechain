import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 64.0 # 64 second demo loop
BPM = 174
BEAT_DUR = 60.0 / BPM  # ~0.3448 seconds per beat

BLOCK_FONT = {
    'T': [
        [1, 1, 1, 1, 1],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0]
    ],
    'S': [
        [0, 1, 1, 1, 1],
        [1, 0, 0, 0, 0],
        [0, 1, 1, 1, 0],
        [0, 0, 0, 0, 1],
        [1, 1, 1, 1, 0]
    ],
    'F': [
        [1, 1, 1, 1, 1],
        [1, 0, 0, 0, 0],
        [1, 1, 1, 1, 0],
        [1, 0, 0, 0, 0],
        [1, 0, 0, 0, 0]
    ],
    'i': [
        [0, 0, 1, 0, 0],
        [0, 0, 0, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0]
    ],
    '/': [
        [0, 0, 0, 0, 1],
        [0, 0, 0, 1, 0],
        [0, 0, 1, 0, 0],
        [0, 1, 0, 0, 0],
        [1, 0, 0, 0, 0]
    ],
    '2': [
        [0, 1, 1, 1, 0],
        [0, 0, 0, 0, 1],
        [0, 1, 1, 1, 0],
        [1, 0, 0, 0, 0],
        [1, 1, 1, 1, 1]
    ]
}

def generate_chaos_audio():
    print("[DSP] Synthesizing Chaos Of Sidrath 64-second soundtrack (174 BPM)...")
    num_samples = int(SAMPLE_RATE * DURATION)
    t = np.linspace(0, DURATION, num_samples, endpoint=False)
    
    # 1. Channel 3: Drum & Bass Beat
    drums = np.zeros_like(t)
    beat_samples = int(SAMPLE_RATE * BEAT_DUR)
    num_beats = int(DURATION / BEAT_DUR)
    
    for i in range(num_beats + 2):
        onset_sample = int(i * beat_samples)
        time_sec = onset_sample / SAMPLE_RATE
        
        # Kick drum trigger
        if time_sec < 16.0:
            # Intro/Verse 1
            kick_trigger = (i % 4 == 0) or (i % 8 == 2)
        elif time_sec < 32.0:
            # Drop/Verse 2 (Halftime drop kick rhythm)
            kick_trigger = (i % 8 == 0) or (i % 8 == 3) or (i % 8 == 6)
        elif time_sec < 48.0:
            # Build-up (Rising kick roll)
            if time_sec < 40.0:
                kick_trigger = (i % 2 == 0)
            else:
                kick_trigger = True
        else:
            # Climax (Amen-style breakbeat kick rhythm)
            kick_trigger = (i % 4 == 0) or (i % 8 == 3) or (i % 8 == 5) or (i % 8 == 7)
            
        if kick_trigger:
            age = (np.arange(num_samples) - onset_sample) / SAMPLE_RATE
            mask = (age >= 0) & (age < 0.25)
            if np.any(mask):
                k_age = age[mask]
                freq = 45.0 + 130.0 * np.exp(-k_age / 0.03)
                kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.18)
                drums[mask] += np.tanh(kick_val * 1.8)
                
        # Snare drum trigger
        if time_sec < 16.0:
            snare_trigger = (i % 4 == 2)
        elif time_sec < 32.0:
            snare_trigger = (i % 8 == 4)
        elif time_sec < 48.0:
            # No snare during the build-up roll
            snare_trigger = False
        else:
            # High-intensity breakbeat double snare syncopations
            snare_trigger = (i % 4 == 2) or (i % 16 == 15) or (i % 32 == 31)
            
        if snare_trigger:
            age = (np.arange(num_samples) - onset_sample) / SAMPLE_RATE
            mask = (age >= 0) & (age < 0.3)
            if np.any(mask):
                s_age = age[mask]
                noise = (np.random.rand(len(s_age)) - 0.5) * 2.0
                snare_val = noise * np.exp(-s_age / 0.08)
                snare_val += np.sin(2.0 * np.pi * 180.0 * s_age) * np.exp(-s_age / 0.05)
                drums[mask] += np.tanh(snare_val * 1.5)
                
        # Hats
        for half in [0, 0.5]:
            onset_hat = int((i + half) * beat_samples)
            age = (np.arange(num_samples) - onset_hat) / SAMPLE_RATE
            mask = (age >= 0) & (age < 0.05)
            if np.any(mask):
                h_age = age[mask]
                noise = (np.random.rand(len(h_age)) - 0.5) * 2.0
                drums[mask] += noise * np.exp(-h_age / 0.015) * 0.08

    # 2. Channel 1: Sidrath Sub-Attractor Bassline
    bass = np.zeros_like(t)
    bass_notes = [65.41, 65.41, 77.78, 58.27, 87.31, 87.31, 73.42, 48.99]
    step_samples = int(beat_samples * 2)
    num_bass_steps = int(num_samples / step_samples)
    
    for i in range(num_bass_steps + 1):
        onset = i * step_samples
        time_sec = onset / SAMPLE_RATE
        note_freq = bass_notes[i % len(bass_notes)]
        
        if time_sec < 16.0:
            wobble_freq = 5.8
            bass_level = 0.45
        elif time_sec < 32.0:
            note_freq = note_freq * 0.5
            wobble_freq = 6.0 + 6.0 * np.sin(2.0 * np.pi * 0.25 * (np.arange(num_samples) / SAMPLE_RATE))
            bass_level = 0.55
        elif time_sec < 48.0:
            note_freq = 32.7
            wobble_freq = 0.0
            bass_level = 0.35
        else:
            note_freq = note_freq * 0.5
            wobble_freq = 12.0
            bass_level = 0.6
            
        age = (np.arange(num_samples) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < (step_samples / SAMPLE_RATE))
        if np.any(mask):
            b_age = age[mask]
            phase = (note_freq * b_age) % 1.0
            saw = 2.0 * phase - 1.0
            tri = 1.0 - 4.0 * np.abs(phase - 0.5)
            mix = (saw * 0.4) + (tri * 0.6)
            
            has_wobble = isinstance(wobble_freq, np.ndarray) or (wobble_freq != 0.0)
            if has_wobble:
                if isinstance(wobble_freq, np.ndarray):
                    w_freq_mask = wobble_freq[mask]
                else:
                    w_freq_mask = wobble_freq
                wobble = 0.5 + 0.5 * np.sin(2.0 * np.pi * w_freq_mask * b_age)
            else:
                wobble = 1.0
            env = np.exp(-b_age / 0.5) * wobble
            bass[mask] += np.tanh(mix * (1.5 + env * 4.0)) * bass_level

    # 3. Channel 2: Trigonometric Lead
    lead = np.zeros_like(t)
    arp_notes = [130.81, 155.56, 196.00, 233.08, 293.66, 196.00, 155.56, 130.81]
    arp_samples = int(beat_samples / 4)
    num_arp_steps = int(num_samples / arp_samples)
    
    for i in range(num_arp_steps + 1):
        onset = i * arp_samples
        time_sec = onset / SAMPLE_RATE
        note_freq = arp_notes[i % len(arp_notes)] * 2.0
        
        if time_sec < 16.0:
            gate_dur = 0.12
            decay_const = 0.04
            dual_voice = False
        elif time_sec < 32.0:
            note_freq = note_freq * 2.0
            gate_dur = 0.24
            decay_const = 0.08
            dual_voice = False
        elif time_sec < 48.0:
            slide_factor = 1.0 + 2.0 * ((time_sec - 32.0) / 16.0)
            note_freq = note_freq * slide_factor
            gate_dur = 0.10
            decay_const = 0.03
            dual_voice = False
        else:
            note_freq = note_freq * 2.0
            gate_dur = 0.16
            decay_const = 0.06
            dual_voice = True
            
        age = (np.arange(num_samples) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < gate_dur)
        if np.any(mask):
            l_age = age[mask]
            lfo = 1.0 + 0.03 * np.sin(2.0 * np.pi * 6.0 * l_age)
            phase1 = (note_freq * lfo * l_age) % 1.0
            saw1 = 2.0 * phase1 - 1.0
            
            if dual_voice:
                phase2 = (note_freq * 1.015 * lfo * l_age) % 1.0
                saw2 = 2.0 * phase2 - 1.0
                voice_mix = 0.5 * saw1 + 0.5 * saw2
            else:
                voice_mix = saw1
                
            lead[mask] += voice_mix * np.exp(-l_age / decay_const) * 0.15

    mix = (drums * 0.4) + (bass * 0.35) + (lead * 0.2)
    mix = np.clip(mix, -1.0, 1.0)
    
    backing_path = "temp_chaos_track.wav"
    byte_data = bytearray(44 + len(mix) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(mix) * 2).to_bytes(4, "little")
    byte_data[8:12] = b"WAVE"
    byte_data[12:16] = b"fmt "
    byte_data[16:20] = int(16).to_bytes(4, "little")
    byte_data[20:22] = int(1).to_bytes(2, "little")
    byte_data[22:24] = int(1).to_bytes(2, "little")
    byte_data[24:28] = int(SAMPLE_RATE).to_bytes(4, "little")
    byte_data[28:32] = int(SAMPLE_RATE * 2).to_bytes(4, "little")
    byte_data[32:34] = int(2).to_bytes(2, "little")
    byte_data[34:36] = int(16).to_bytes(2, "little")
    byte_data[36:40] = b"data"
    byte_data[40:44] = int(len(mix) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in mix:
        s = max(-1.0, min(1.0, val))
        v = int(s * 32767) if s >= 0 else int(s * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(backing_path, "wb") as f:
        f.write(byte_data)
    return backing_path

def rotate4D(x, y, z, w, theta, phi):
    cos_t, sin_t = math.cos(theta), math.sin(theta)
    rx = x * cos_t - w * sin_t
    rw = x * sin_t + w * cos_t
    
    cos_p, sin_p = math.cos(phi), math.sin(phi)
    ry = y * cos_p - z * sin_p
    rz = y * sin_p + z * cos_p
    
    return rx, ry, rz, rw

def generate_video_direct(audio_path, output_mp4):
    print("[TSFI] Initiating direct FFmpeg pipe rendering...")
    
    # FFmpeg setup for direct pipe ingestion
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 1280x720 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    total_frames = int(DURATION * FPS)
    
    # Initialize 3D Starfield particles
    np.random.seed(953)
    stars = []
    for _ in range(100):
        stars.append({
            'x': np.random.uniform(-400, 400),
            'y': np.random.uniform(-300, 300),
            'z': np.random.uniform(50, 600)
        })
        
    # Define Tesseract 16 vertices in 4D space
    vertices_4d = []
    for x in [-1, 1]:
        for y in [-1, 1]:
            for z in [-1, 1]:
                for w in [-1, 1]:
                    vertices_4d.append((x, y, z, w))
                    
    edges = []
    for i in range(16):
        for j in range(i + 1, 16):
            diff = 0
            for k in range(4):
                if vertices_4d[i][k] != vertices_4d[j][k]:
                    diff += 1
            if diff == 1:
                edges.append((i, j))

    # Define 3D Torus vertices and edges orbiting the tesseract
    torus_vertices = []
    R_torus = 160.0  # Orbit radius
    r_torus = 20.0   # Tube radius
    num_theta = 8    # Step around tube
    num_phi = 16     # Step around ring
    for p in range(num_phi):
        phi_val = (p / num_phi) * 2.0 * math.pi
        for t in range(num_theta):
            theta_val = (t / num_theta) * 2.0 * math.pi
            tx = (R_torus + r_torus * math.cos(theta_val)) * math.cos(phi_val)
            ty = r_torus * math.sin(theta_val)
            tz = (R_torus + r_torus * math.cos(theta_val)) * math.sin(phi_val)
            torus_vertices.append((tx, ty, tz))
            
    torus_edges = []
    for p in range(num_phi):
        for t in range(num_theta):
            curr = p * num_theta + t
            next_t = p * num_theta + ((t + 1) % num_theta)
            torus_edges.append((curr, next_t))
            next_p = ((p + 1) % num_phi) * num_theta + t
            torus_edges.append((curr, next_p))

    # Precompute Lorenz chaotic attractor trajectory
    lorenz_pts = []
    lx, ly, lz = 0.1, 0.0, 0.0
    dt_lorenz = 0.008
    for _ in range(6000):
        dx = 10.0 * (ly - lx) * dt_lorenz
        dy = (lx * (28.0 - lz) - ly) * dt_lorenz
        dz = (lx * ly - (8.0 / 3.0) * lz) * dt_lorenz
        lx += dx
        ly += dy
        lz += dz
        # Map and scale points so the butterfly shape stands upright
        lorenz_pts.append((lx * 4.5, (lz - 25.0) * 4.5, ly * 4.5))

    # Initialize Verlet soft-body FET discharge grid (Rule 10: Verlet constraints apply to FET discharge)
    fet_grid = []
    for r in range(4):
        for c in range(4):
            fet_grid.append({
                'x': (c - 1.5) * 20.0,
                'y': (r - 1.5) * 20.0,
                'z': 0.0,
                'px': (c - 1.5) * 20.0,
                'py': (r - 1.5) * 20.0,
                'pz': 0.0
            })

    for frame in range(total_frames):
        time = frame / float(FPS)
        
        # Retro trigonometric plasma background (computed at 160x90 and upscaled)
        px, py = np.meshgrid(np.linspace(0, 8.0, 160), np.linspace(0, 4.5, 90))
        plasma_val = (
            np.sin(px + time * 1.2) +
            np.sin(1.5 * (py - time * 0.9)) +
            np.sin(px + py + time * 1.8) +
            np.cos(np.sqrt(px**2 + py**2) - time * 1.5)
        )
        # Normalize plasma to [0, 1] range using fixed bounds to prevent luminance flashing/pulsing
        plasma_val = np.clip((plasma_val + 4.0) / 8.0, 0.0, 1.0)
        
        # Color mapping (very dark cosmic purple/blue palette for minimal background distraction)
        pr = (plasma_val * 35.0).astype(np.uint8)
        pg = (plasma_val * 15.0).astype(np.uint8)
        pb = (40.0 + plasma_val * 70.0).astype(np.uint8)
        
        plasma_arr = np.stack([pr, pg, pb], axis=-1)
        img = Image.fromarray(plasma_arr).resize((1280, 720), Image.Resampling.BILINEAR)
        draw = ImageDraw.Draw(img)
        
        beat_phase = (time / BEAT_DUR) * math.pi * 2.0
        beat_amplitude = max(0.0, math.sin(beat_phase)) * math.exp(-((time % BEAT_DUR) / 0.15))
        
        # 1. Draw Amiga dual interference copper/raster bars
        for bar_idx in range(6):
            bar_y1 = 360.0 + 220.0 * math.sin(time * 1.5 + bar_idx * 0.8)
            bar_y2 = 360.0 + 180.0 * math.cos(time * 2.2 - bar_idx * 0.5)
            
            # Combine waves to create visual interference
            bar_y = (bar_y1 + bar_y2) / 2.0
            bar_height = 40
            for dy in range(-bar_height // 2, bar_height // 2):
                y_pos = int(bar_y + dy)
                if 0 <= y_pos < 720:
                    intensity = 1.0 - abs(dy) / (bar_height / 2.0)
                    r = int(10 * intensity)
                    g = int(24 * intensity + 20 * intensity * math.sin(time * 4.0))
                    b = int(120 * intensity + 60 * intensity * math.cos(time * 2.0 + bar_idx))
                    draw.line([(0, y_pos), (1280, y_pos)], fill=(r, g, b), width=1)
                    
        # 2. Draw 3D Parallax Starfield (Surges speed and size on beats)
        star_speed = 8.0 + 18.0 * beat_amplitude
        for star in stars:
            star['z'] -= star_speed # Move towards camera
            if star['z'] <= 10:
                star['z'] = np.random.uniform(500, 600)
                star['x'] = np.random.uniform(-400, 400)
                star['y'] = np.random.uniform(-300, 300)
                
            # Perspective projection of star
            factor = 350.0 / star['z']
            sx = 640 + int(star['x'] * factor)
            sy = 330 + int(star['y'] * factor)
            s_size = max(1, int((3.0 + 5.0 * beat_amplitude) * factor))
            
            if 0 <= sx < 1280 and 0 <= sy < 720:
                glow_val = min(255, int((200 + 55 * beat_amplitude) * factor))
                draw.ellipse([sx, sy, sx + s_size, sy + s_size], fill=(glow_val, glow_val, 255))

        # 3. 3D Perspective Grid Floor (horizon Y=440 to boundary Y=600)
        draw.line([(0, 440), (1280, 440)], fill="#002244", width=2)
        grid_speed = 2.5
        num_horizontal = 12
        for i in range(num_horizontal):
            ratio = ((i - time * grid_speed) % num_horizontal) / num_horizontal
            y_grid = 440.0 + 160.0 * (ratio ** 2)
            intensity = int(20 + 180 * ratio)
            draw.line([(0, y_grid), (1280, y_grid)], fill=(0, intensity // 3, intensity), width=1)
            
        num_vertical = 20
        for i in range(num_vertical + 1):
            x_bottom = (i / num_vertical) * 1680.0 - 200.0
            draw.line([(640, 440), (x_bottom, 600)], fill=(0, 40, 110), width=1)

        # 4. Equalizer Spectrum Bars (8 bands on left and right margins)
        num_bands = 8
        bar_w = 12
        bar_spacing = 4
        for band_idx in range(num_bands):
            band_val = beat_amplitude * 0.4 + 0.6 * (0.5 + 0.5 * math.sin(time * (4.0 + band_idx * 1.8) + band_idx))
            num_segs = int(band_val * 12)
            
            lx = 60 + band_idx * (bar_w + bar_spacing)
            rx = 1140 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
            
            for seg in range(12):
                y_pos = 380 - seg * 12
                if seg < 6:
                    color = (0, 220, 80)
                elif seg < 10:
                    color = (220, 220, 0)
                else:
                    color = (255, 40, 40)
                    
                if seg < num_segs:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color)
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color)
                else:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], outline=(30, 30, 45))
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], outline=(30, 30, 45))

        # 5. Draw dynamic FFT visualizer line at the bottom
        wave_pts = []
        amp = 20.0 + 90.0 * beat_amplitude
        for x in range(0, 1280, 10):
            y = 620.0 + amp * math.sin(x * 0.02 + time * 18.0) * math.cos(x * 0.005)
            wave_pts.append((x, y))
        draw.line(wave_pts, fill="#ff007f", width=2)
        
        # 4. 4D Tesseract projection (Beat-Reactive Color & Thickness)
        theta = time * 0.8
        phi = time * 0.6
        pulse = 140.0 + 50.0 * beat_amplitude
        
        projected = []
        for pt in vertices_4d:
            rx, ry, rz, rw = rotate4D(pt[0], pt[1], pt[2], pt[3], theta, phi)
            d = 2.5
            factor = 1.0 / (d - rw)
            x3d = rx * factor * pulse
            y3d = ry * factor * pulse
            z3d = rz * factor * pulse
            
            proj_x = 640 + x3d
            proj_y = 330 + y3d
            projected.append((proj_x, proj_y))
            
        # Beat-reactive color shift for wireframe
        hue_tess = int((time * 60 + beat_amplitude * 120) % 360)
        line_color = f"hsl({hue_tess}, 100%, 65%)"
        line_width = 2 + int(3 * beat_amplitude)
        
        # Draw 4D edges
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=line_color, width=line_width)
            
        # Draw vertex nodes
        for pt in projected:
            draw.ellipse([pt[0]-4, pt[1]-4, pt[0]+4, pt[1]+4], fill="#ff007f", outline="#ffffff", width=1)
            
        # 4b. Draw Orbiting 3D Torus (independent rotation and neon cyan wireframe)
        t_rot_x = time * 0.9
        t_rot_y = time * 1.3
        cos_tx, sin_tx = math.cos(t_rot_x), math.sin(t_rot_x)
        cos_ty, sin_ty = math.cos(t_rot_y), math.sin(t_rot_y)
        
        torus_projected = []
        for vx, vy, vz in torus_vertices:
            # Rotate around X
            ry = vy * cos_tx - vz * sin_tx
            rz = vy * sin_tx + vz * cos_tx
            # Rotate around Y
            rx = vx * cos_ty - rz * sin_ty
            rz = vx * sin_ty + rz * cos_ty
            
            # Project to screen
            d_cam = 450.0
            factor = 320.0 / (d_cam - rz)
            proj_x = 640 + rx * factor
            proj_y = 330 + ry * factor
            torus_projected.append((proj_x, proj_y))
            
        for edge in torus_edges:
            p1 = torus_projected[edge[0]]
            p2 = torus_projected[edge[1]]
            # Neon cyan wireframe line
            draw.line([p1, p2], fill="#00f2fe", width=1)
            
        # 4c. Draw Lorenz chaotic attractor trail (retro phosphor green)
        trail_len = 150
        trail_step = 4
        trail_end = (frame * trail_step) % len(lorenz_pts)
        trail_start = max(0, trail_end - trail_len)
        trail = lorenz_pts[trail_start:trail_end]
        
        ang_la = time * 0.5
        cos_la, sin_la = math.cos(ang_la), math.sin(ang_la)
        la_projected = []
        for lx, ly, lz in trail:
            rx = lx * cos_la - lz * sin_la
            rz = lx * sin_la + lz * cos_la
            ry = ly
            
            d_cam = 450.0
            factor = 320.0 / (d_cam - rz)
            proj_x = 640 + int(rx * factor)
            proj_y = 330 + int(ry * factor)
            la_projected.append((proj_x, proj_y))
            
        for j in range(1, len(la_projected)):
            p1 = la_projected[j - 1]
            p2 = la_projected[j]
            fade = j / len(la_projected)
            color = (0, int(150 * fade + 105 * beat_amplitude), int(50 * fade))
            draw.line([p1, p2], fill=color, width=2)

        # 4d. Update and Draw Verlet soft-body FET grid (Rule 10: soft body physics applies only to FET discharge cycles)
        grid_rot_y = time * 2.2
        cos_gr, sin_gr = math.cos(grid_rot_y), math.sin(grid_rot_y)
        
        # Apply Verlet step to node coordinates reacting to beat_amplitude
        for node in fet_grid:
            temp_x, temp_y, temp_z = node['x'], node['y'], node['z']
            
            # Simple spring constraint pull towards origin plus beat oscillation force
            accel_z = -0.1 * node['z'] + beat_amplitude * 60.0 * math.sin(time * 16.0 + node['x'] * 0.1)
            accel_x = -0.05 * node['x']
            accel_y = -0.05 * node['y']
            
            # Verlet integration: x_new = 2*x - x_prev + accel * dt^2 (dt = 0.033)
            node['x'] = 2.0 * node['x'] - node['px'] + accel_x * 0.001
            node['y'] = 2.0 * node['y'] - node['py'] + accel_y * 0.001
            node['z'] = 2.0 * node['z'] - node['pz'] + accel_z * 0.001
            
            node['px'], node['py'], node['pz'] = temp_x, temp_y, temp_z

        # Project and draw grid connections
        gr_projected = []
        for node in fet_grid:
            # Scale coordinates and rotate around Y
            sx, sy, sz = node['x'] * 1.5, node['y'] * 1.5, node['z'] * 1.5
            rx = sx * cos_gr - sz * sin_gr
            rz = sx * sin_gr + sz * cos_gr
            ry = sy
            
            d_cam = 450.0
            factor = 320.0 / (d_cam - rz)
            proj_x = 240 + int(rx * factor) # Drawn on the left margin
            proj_y = 520 + int(ry * factor)
            gr_projected.append((proj_x, proj_y))

        # Draw grid wireframe connections
        for r in range(4):
            for c in range(4):
                curr_idx = r * 4 + c
                p_curr = gr_projected[curr_idx]
                if c < 3:
                    p_right = gr_projected[curr_idx + 1]
                    draw.line([p_curr, p_right], fill="#c5a059", width=1)
                if r < 3:
                    p_down = gr_projected[curr_idx + 4]
                    draw.line([p_curr, p_down], fill="#c5a059", width=1)

        # 4e. Draw Federal Worker active phase progress indicator on left margin
        phases_names = ["Imposition", "Qualified", "Audited", "Confirmed", "Selected"]
        current_phase_idx = int((time / DURATION) * 5) % 5
        for p_idx, p_name in enumerate(phases_names):
            py_pos = 140 + p_idx * 30
            is_active = (p_idx == current_phase_idx)
            is_past = (p_idx < current_phase_idx)
            
            circle_color = "#c5a059" if (is_active or is_past) else "#332211"
            draw.ellipse([60, py_pos, 70, py_pos + 10], fill=circle_color)
            text_color = "#e6dfd3" if is_active else "#665544"
            draw.text((80, py_pos - 2), p_name, fill=text_color)
            
        # 5. Draw Demoscene Bouncing Block Text "TSFi/2" in 3D Space
        text_to_draw = "TSFi/2"
        block_size = 6
        spacing = 3
        
        orbit_angle = time * 0.8
        cos_o, sin_o = math.cos(orbit_angle), math.sin(orbit_angle)
        
        for char_idx, char in enumerate(text_to_draw):
            if char in BLOCK_FONT:
                bounce_offset = math.sin(time * 8.0 - char_idx * 0.9) * 12.0
                grid = BLOCK_FONT[char]
                
                cx = (char_idx - 2.5) * 55.0
                cy = -140.0
                
                for r in range(5):
                    for c in range(5):
                        if grid[r][c] == 1:
                            lx = cx + (c - 2) * (block_size + spacing)
                            ly = cy + (r - 2) * (block_size + spacing) + bounce_offset
                            lz = 0.0
                            
                            rx = lx * cos_o - lz * sin_o
                            rz = lx * sin_o + lz * cos_o
                            ry = ly
                            
                            d_cam = 400.0
                            factor = 320.0 / (d_cam - rz)
                            
                            bx = 640 + rx * factor
                            by = 330 + ry * factor
                            size_proj = max(1, int(block_size * factor))
                            
                            hue = (time * 90 + char_idx * 40 + r * 15) % 360
                            color_str = f"hsl({hue}, 100%, 65%)"
                            draw.rectangle([bx, by, bx + size_proj, by + size_proj], fill=color_str)
            
        # Telemetry Display in Vaesen Gothic styling colors (#c5a059 gold, #e6dfd3 cream)
        draw.text((40, 40), "TSFi/2: CHAOS OF SIDRATH (AUNCIENT DEMO)", fill="#c5a059")
        draw.text((40, 60), "SYSTEM: DYSNOMIA COMPILER MAINCHAIN GATE / CADE_IMF_NATO", fill="#e6dfd3")
        draw.text((40, 80), f"TEMPO: {BPM} BPM  |  BEAT INDEX: {int(time / BEAT_DUR)}", fill="#e6dfd3")
        draw.text((40, 100), f"4D PHASES: THETA={theta:.3f} | PHI={phi:.3f}", fill="#c5a059")
        
        # WinchesterMQ registers & Federal Worker Simulation telemetry
        reg_val_103 = 1000 - int(840 * beat_amplitude)
        draw.text((900, 40), f"AuncientAnalyzer: Permissible (Opcodes: A,S) | DEFCON: 5", fill="#e6dfd3")
        draw.text((900, 60), f"Federal Worker: PHASE_SELECTED_WORKER (Saat: 1M)", fill="#c5a059")
        draw.text((900, 80), f"SSA Identity: Teddy Bear (555) | ssn: 006-12-3456", fill="#c5a059")
        draw.text((900, 100), f"DisplacementShader: Vertex Sync scale aligned", fill="#ff007f")
        
        # Amiga Scrolltext at the bottom
        scroll_text = "TSFi/2 AUNCIENT WINCHESTERMQ HARDWARE DECK EMULATION --- CHAOS OF SIDRATH --- NEUROFUNK 174 BPM DRUM AND BASS --- FEDERAL WORKER SIMULATION ACTIVE --- CICS TRANSACTION COMPLIANCE VERIFIED..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 1280 - int(time * 190.0) % (total_text_width + 1280)
        draw.text((scroll_x, 680), scroll_text, fill="#c5a059")

        # Directly pipe RGB raw bytes to FFmpeg
        pipe.stdin.write(img.tobytes())
        
        if frame % 30 == 0:
            print(f"  -> Direct Pipe: Rendered {frame}/{total_frames} frames into video stream...")
            
    pipe.stdin.close()
    pipe.wait()
    print(f"[SUCCESS] Video successfully saved to {output_mp4}!")
 
def cleanup():
    print("[CLEANUP] Purging temporary audio track...")
    try:
        os.remove("temp_chaos_track.wav")
    except Exception:
        pass
 
if __name__ == "__main__":
    backing = generate_chaos_audio()
    
    # Destination in active conversation artifacts directory
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/1b5b5d7d-d753-41a3-9301-e621c99e1f05"
    output_mp4 = os.path.join(output_dir, "chaos_of_sidrath_demo.mp4")
    
    generate_video_direct(backing, output_mp4)
    cleanup()
