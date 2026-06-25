import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
BPM = 140
EIGHTH_DUR = 30.0 / BPM  # ~0.2143s per eighth note step
BAR_STEPS = 13  # 13/8 time signature
BAR_DUR = BAR_STEPS * EIGHTH_DUR  # ~2.7857 seconds
DURATION = 180.0  # 3 minutes
num_samples = int(SAMPLE_RATE * DURATION)
total_frames = int(DURATION * FPS)
dt = 1.0 / SAMPLE_RATE

CX = 360
CY = 640

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

def generate_complex_notes():
    scale = [55.00, 65.41, 73.42, 77.78, 87.31, 98.00, 110.00] # A minor / C major hybrid
    pattern = [0, 2, 3, 1, 4, 3, 2, 5, 4, 0, 6, 5, 3]
    return [scale[i] for i in pattern]

def generate_3m_audio():
    print("[DSP] Synthesizing 3-minute coupled-syrinx 13/8 Dubstep soundtrack...")
    dur_ext = DURATION + 2.0
    num_samples_ext = int(SAMPLE_RATE * dur_ext)
    
    # 1. Coupled dual-source syrinx simulator with asymmetric stiffness (Channel 1)
    speech = np.zeros(num_samples_ext)
    glottal_flow = np.zeros(num_samples_ext)
    
    # 3-Mass Ishizaka-Flanagan style Verlet physical model simulation variables
    # Mass 1 (Lower Fold)
    x1, x1_prev = 0.05, 0.05
    m1 = 0.15
    epibar1 = 1000.0
    hypobar1 = 3500.0
    c1 = 1.4
    Ps1 = 0.55
    
    # Mass 2 (Upper Fold)
    x2, x2_prev = 0.04, 0.04
    m2 = 0.12
    epibar2 = 1380.0
    hypobar2 = 4000.0
    c2 = 1.1
    Ps2 = 0.65
    
    # Mass 3 (Resonant Load Piston representing vocal tract impedance)
    x3, x3_prev = 0.0, 0.0
    m3 = 0.08
    k3 = 800.0
    c3 = 0.5
    
    # Coupling parameters
    Kc = 180.0
    A_fold = 0.2
    
    for s in range(1, num_samples_ext - 1):
        # Asymmetric stiffness based on displacement (Epibar for tension, Hypobar for compression)
        stiffness1 = epibar1 if x1 > 0.0 else hypobar1
        stiffness2 = epibar2 if x2 > 0.0 else hypobar2
        
        # Aerodynamic driving forces
        f_p1 = Ps1 * A_fold if x1 > 0.0 else 0.0
        f_p2 = Ps2 * A_fold if x2 > 0.0 else 0.0
        
        # Velocities
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        v3 = (x3 - x3_prev) / dt
        
        # Resonant load piston driven by glottal flow
        current_flow = (max(x1, 0.0) ** 2) + (max(x2, 0.0) ** 2)
        f_p3 = current_flow * 2.5
        
        # Feedback force from resonant piston onto upper glottal mass
        F_fb = -k3 * x3 - c3 * v3
        
        # Coupled acceleration calculations
        acc1 = (f_p1 - stiffness1 * x1 - c1 * v1 + Kc * (x2 - x1)) / m1
        acc2 = (f_p2 - stiffness2 * x2 - c2 * v2 + Kc * (x1 - x2) + F_fb) / m2
        acc3 = (f_p3 - k3 * x3 - c3 * v3) / m3
        
        # Verlet integration step
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        x3_next = 2.0 * x3 - x3_prev + acc3 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.2, min(1.0, x1_next))
        x2_prev, x2 = x2, max(-0.2, min(1.0, x2_next))
        x3_prev, x3 = x3, max(-0.5, min(1.0, x3_next))
        
        # Save output flow (modulated by acoustic load piston)
        glottal_flow[s] = current_flow
        
    glottal_flow -= np.mean(glottal_flow)
    if np.max(np.abs(glottal_flow)) > 0:
        glottal_flow /= np.max(np.abs(glottal_flow))
        
    # Consonant noises (high-pass & band-pass)
    noise = np.random.uniform(-1.0, 1.0, num_samples_ext)
    s_noise = noise.copy()
    for _ in range(3):
        s_noise = np.concatenate([[0.0], s_noise[1:] - 0.95 * s_noise[:-1]])
    s_noise /= np.maximum(np.max(np.abs(s_noise)), 1e-5)
    
    sh_noise = np.concatenate([[0.0], noise[1:] - 0.82 * noise[:-1]])
    sh_noise = np.convolve(sh_noise, np.ones(6)/6.0, mode='same')
    sh_noise /= np.maximum(np.max(np.abs(sh_noise)), 1e-5)
    
    for s in range(num_samples_ext):
        time_sec = s / SAMPLE_RATE
        # Vocals active: 0s-45s (ambient sweeps) and 150s-180s (outro resolution)
        if time_sec < 45.0 or time_sec >= 150.0:
            cycle_time = time_sec % 8.0
            if cycle_time < 0.8:
                speech[s] = s_noise[s] * 0.22
            elif cycle_time < 2.5:
                # EE vowel with coupled syrinx
                f1, f2 = 270.0, 2290.0
                speech[s] = glottal_flow[s] * (0.6 * math.sin(2.0 * np.pi * f1 * time_sec) + 0.4 * math.sin(2.0 * np.pi * f2 * time_sec)) * 0.35
            elif cycle_time < 3.3:
                speech[s] = sh_noise[s] * 0.2
            elif cycle_time < 5.0:
                # AH vowel
                ratio = (cycle_time - 3.3) / 1.7
                f1 = 270.0 * (1.0 - ratio) + 730.0 * ratio
                f2 = 2290.0 * (1.0 - ratio) + 1090.0 * ratio
                speech[s] = glottal_flow[s] * (0.6 * math.sin(2.0 * np.pi * f1 * time_sec) + 0.4 * math.sin(2.0 * np.pi * f2 * time_sec)) * 0.35
            elif cycle_time < 5.1:
                speech[s] = 0.0
            elif cycle_time < 5.25:
                decay = math.exp(-(cycle_time - 5.1) / 0.02)
                speech[s] = s_noise[s] * 0.3 * decay
            elif cycle_time < 7.0:
                # OO vowel
                ratio = (cycle_time - 5.25) / 1.75
                f1 = 730.0 * (1.0 - ratio) + 300.0 * ratio
                f2 = 1090.0 * (1.0 - ratio) + 870.0 * ratio
                speech[s] = glottal_flow[s] * (0.6 * math.sin(2.0 * np.pi * f1 * time_sec) + 0.4 * math.sin(2.0 * np.pi * f2 * time_sec)) * 0.35
            else:
                speech[s] = s_noise[s] * 0.2

    # 2. 13/8 Sequenced Drums (Channel 2)
    drums = np.zeros(num_samples_ext)
    step_samples = int(SAMPLE_RATE * EIGHTH_DUR)
    num_steps_ext = int(dur_ext / EIGHTH_DUR)
    
    for i in range(num_steps_ext + 2):
        onset_sample = int(i * step_samples)
        time_sec = onset_sample / SAMPLE_RATE
        step_idx = i % BAR_STEPS
        
        # Build up drums starting at 45s, climaxing 90s-150s
        if time_sec >= 45.0 and time_sec < 150.0:
            kick_trigger = (step_idx == 0 or step_idx == 6)
            snare_trigger = (step_idx == 3 or step_idx == 11)
            hat_trigger = (step_idx % 2 == 1)
            
            # Subdued drums 45s-90s, full force 90s-150s
            vol_factor = 0.5 if time_sec < 90.0 else 1.0
            
            if kick_trigger:
                k_len = min(num_samples_ext - onset_sample, int(SAMPLE_RATE * 0.3))
                if k_len > 0:
                    k_age = np.arange(k_len) / SAMPLE_RATE
                    freq = 55.0 + 150.0 * np.exp(-k_age / 0.04)
                    kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.22)
                    drums[onset_sample:onset_sample+k_len] += np.tanh(kick_val * 1.9) * 0.45 * vol_factor
                    
            if snare_trigger:
                s_len = min(num_samples_ext - onset_sample, int(SAMPLE_RATE * 0.35))
                if s_len > 0:
                    s_age = np.arange(s_len) / SAMPLE_RATE
                    noise_buf = (np.random.rand(s_len) - 0.5) * 2.0
                    snare_val = (noise_buf * np.exp(-s_age / 0.12)) + np.sin(2.0 * np.pi * 170.0 * s_age) * np.exp(-s_age / 0.06)
                    drums[onset_sample:onset_sample+s_len] += np.tanh(snare_val * 1.6) * 0.4 * vol_factor
                    
            if hat_trigger:
                h_len = min(num_samples_ext - onset_sample, int(SAMPLE_RATE * 0.06))
                if h_len > 0:
                    h_age = np.arange(h_len) / SAMPLE_RATE
                    noise_buf = (np.random.rand(h_len) - 0.5) * 2.0
                    drums[onset_sample:onset_sample+h_len] += noise_buf * np.exp(-h_age / 0.02) * 0.04 * vol_factor

    # 3. Wobble Bassline (Channel 3)
    bass = np.zeros(num_samples_ext)
    bass_notes = generate_complex_notes()
    
    for i in range(num_steps_ext):
        onset = i * step_samples
        time_sec = onset / SAMPLE_RATE
        step_idx = i % BAR_STEPS
        
        # Bass active 45s - 150s
        if 45.0 <= time_sec < 150.0:
            note_freq = bass_notes[step_idx]
            end = min(num_samples_ext, onset + step_samples)
            b_len = end - onset
            if b_len > 0:
                b_age = np.arange(b_len) / SAMPLE_RATE
                
                # Wobble rate is slow (3Hz) during 45s-90s, speeds up to 8Hz during the drop (90s-150s)
                lfo_rate = 3.0 if time_sec < 90.0 else (8.0 if step_idx >= 6 else 4.0)
                lfo = 0.5 + 0.5 * np.sin(2.0 * np.pi * lfo_rate * (b_age / EIGHTH_DUR))
                
                # Applying post-filter spectral sharpening mapping: boost formant amplitude peak
                sharpen_gain = 1.0 if time_sec < 90.0 else 1.45
                
                phase = (note_freq * b_age) % 1.0
                saw = 2.0 * phase - 1.0
                sig = saw * 0.45 * lfo * sharpen_gain
                
                # Germanium distortion modeling
                sig = np.where(sig > 0, 1.0 - np.exp(-sig * 3.8), -1.0 + np.exp(sig * 3.2))
                bass[onset:end] += np.tanh(sig * 1.5) * 0.35

    # 4. Lead Arpeggiator (Channel 4)
    lead = np.zeros(num_samples_ext)
    arp_notes = [n * 3.0 for n in bass_notes]
    for i in range(num_steps_ext):
        onset = i * step_samples
        time_sec = onset / SAMPLE_RATE
        step_idx = i % BAR_STEPS
        # Lead active during the drop (90s - 150s)
        if 90.0 <= time_sec < 150.0:
            note_freq = arp_notes[(step_idx * 3) % len(arp_notes)]
            end = min(num_samples_ext, onset + int(0.12 * SAMPLE_RATE))
            l_len = end - onset
            if l_len > 0:
                l_age = np.arange(l_len) / SAMPLE_RATE
                sig = np.sin(2.0 * np.pi * note_freq * l_age) * np.exp(-l_age / 0.05)
                lead[onset:end] += sig * 0.14

    mix = speech + drums + bass + lead
    mix = np.clip(mix, -1.0, 1.0)
    
    # Wrap loop boundaries
    loop_samples = int(SAMPLE_RATE * DURATION)
    tail = mix[loop_samples:]
    mix[:len(tail)] += tail
    mix = mix[:loop_samples]
    
    audio_path = "temp_3m_demo.wav"
    
    # Save 16-bit Mono WAV
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
        v = int(val * 32767) if val >= 0 else int(val * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(audio_path, "wb") as f:
        f.write(byte_data)
        
    return audio_path, glottal_flow

def render_3m_video(audio_path, glottal_flow, output_mp4):
    vertices, edges = generate_3d_teddy_wireframe()
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    np.random.seed(999)
    stars = [{'x': np.random.uniform(-300, 300), 'y': np.random.uniform(-400, 400), 'z': np.random.uniform(20, 600)} for _ in range(60)]
    
    for frame in range(total_frames):
        time_sec = frame / float(FPS)
        progress = time_sec / DURATION
        
        # Sequencer timing sync
        measure_time = time_sec % BAR_DUR
        step_index = int(measure_time / EIGHTH_DUR)
        step_phase = (measure_time % EIGHTH_DUR) / EIGHTH_DUR
        
        is_downbeat = (step_index == 0 or step_index == 6)
        beat_amp = math.exp(-step_phase / 0.15) if is_downbeat else 0.0
        
        bg = np.zeros((1280, 720, 3), dtype=np.uint8)
        img = Image.fromarray(bg)
        draw = ImageDraw.Draw(img)
        
        sample_idx = min(num_samples - 1, int(time_sec * SAMPLE_RATE))
        amp_flow = glottal_flow[sample_idx]
        
        # Starfield speed depends on performance section
        star_speed = 3.0
        if 45.0 <= time_sec < 90.0:
            star_speed = 6.0
        elif 90.0 <= time_sec < 150.0:
            star_speed = 14.0 + 18.0 * beat_amp
            
        for star in stars:
            z_pos = (star['z'] - frame * (star_speed / 8.0) * 10.0) % 580 + 20
            factor = 280.0 / z_pos
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            if 0 <= sx < 720 and 0 <= sy < 1280:
                color_fill = (255, 120, 255) if (90.0 <= time_sec < 150.0 and is_downbeat) else (130, 255, 255)
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=color_fill)
                
        # Draw perspective grid
        draw.line([(0, 950), (720, 950)], fill="#330066", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 1120.0 - 200.0
            draw.line([(CX, 950), (x_bottom, 1200)], fill=(80, 0, 100), width=1)
            
        # Rotate Teddy Bear
        angle_y = progress * 2.0 * math.pi * 5.0
        angle_x = 0.15 * math.cos(progress * 2.0 * math.pi * 8.0)
        qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
        qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
        q_rot = quaternion_multiply(qy, qx)
        
        projected = []
        for vx, vy, vz, joint in vertices:
            # Speak (jaw displacement) during vocal phases
            if joint == 1 and (time_sec < 45.0 or time_sec >= 150.0):
                vy = vy - 0.25 * amp_flow
                
            rx, ry, rz = rotate_vector_by_quaternion((vx, vy, vz), q_rot)
            
            # Zoom in on drops
            depth = 3.8
            if 90.0 <= time_sec < 150.0:
                depth = 3.8 - 0.4 * beat_amp
            rz += depth
            factor = 450.0 / rz
            
            proj_x = CX + int(rx * factor * 220.0)
            proj_y = CY - int(ry * factor * 220.0)
            projected.append((proj_x, proj_y))
            
        # Wireframe color switches based on drop phase
        color = "#33ccff"
        if 90.0 <= time_sec < 150.0:
            color = "#ff3399" if step_index >= 6 else "#ffff00"
        elif time_sec >= 150.0:
            color = "#00ffcc"
            
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=color, width=1)
            
        # Margin EQ bars
        if 45.0 <= time_sec < 150.0:
            num_bands = 8
            bar_w = 12
            bar_spacing = 4
            for band_idx in range(num_bands):
                val_mod = math.sin(progress * 2.0 * math.pi * (2.0 + band_idx) + (6.0 if step_index < 6 else 14.0) * time_sec)
                band_val = (0.4 * beat_amp + 0.6 * (0.5 + 0.5 * val_mod)) if time_sec >= 90.0 else 0.4 * (0.5 + 0.5 * val_mod)
                num_segs = int(band_val * 12)
                lx = 30 + band_idx * (bar_w + bar_spacing)
                rx = 690 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
                for seg in range(12):
                    y_pos = 780 - seg * 12
                    color_seg = (255, 0, 128) if seg < 6 else ((255, 128, 0) if seg < 10 else (0, 255, 255))
                    if seg < num_segs:
                        draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color_seg)
                        draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color_seg)
                        
        # Telemetry info
        draw.text((40, 60), "TSFi/2: 3-MINUTE DUAL-SYRINX DEMOSCENE SELECTION", fill="#00ffcc")
        draw.text((40, 80), f"TIMING: {'AMBIENT' if time_sec < 45.0 or time_sec >= 150.0 else '13/8 SEQUENCED'}", fill="#ff3399")
        draw.text((40, 100), f"PERFORMANCE STEP: {int(time_sec)}s / 180s", fill="#ffff00")
        
        reg_val_103 = 1000 - int(840 * beat_amp) if (45.0 <= time_sec < 150.0) else 1000
        draw.text((480, 60), f"MQ[103]: {reg_val_103:03d}", fill="#ffff00")
        draw.text((480, 80), f"SHARPENING: {'ACTIVE' if time_sec >= 90.0 else 'OFF'}", fill="#00ff00")
        draw.text((480, 100), f"PHONEME: {'S-EE-SH-AH-T-OO' if (time_sec < 45.0 or time_sec >= 150.0) else 'WOBBLE_LFO'}", fill="#ff00ff")
        
        # Scrolltext
        scroll_text = "TSFi/2 3-MINUTE PERFORMANCE SESSION --- DUAL-SOURCE SYRINX MODULATION --- ASYMMETRIC VERLET SPRING DYNAMICS --- DETERMINISTIC DETERMINISM FOREVER..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 720 - int(progress * 8.0 * (total_text_width + 720)) % (total_text_width + 720)
        draw.text((scroll_x, 1220), scroll_text, fill="#39ff14")
        
        pipe.stdin.write(img.tobytes())
        
        if frame % 600 == 0:
            print(f"  -> Rendering 3m Demo: {frame}/{total_frames} frames ({progress*100:.1f}%)...")
            
    pipe.stdin.close()
    pipe.wait()
    print("[SUCCESS] 3m presentation successfully saved!")

def generate_3d_teddy_wireframe():
    vertices = []
    edges = []
    
    head_center = (0.0, 1.4, 0.0)
    head_rad = 0.5
    head_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat, sin_lat = math.cos(lat_angle), math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = head_center[0] + head_rad * cos_lat * math.cos(lon_angle)
            vy = head_center[1] + head_rad * sin_lat
            vz = head_center[2] + head_rad * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    for lat in range(9):
        for lon in range(8):
            curr = head_v_start + lat * 8 + lon
            edges.append((curr, head_v_start + lat * 8 + ((lon + 1) % 8)))
            if lat < 8:
                edges.append((curr, head_v_start + (lat + 1) * 8 + lon))
                
    jaw_v_start = len(vertices)
    jaw_center = (0.0, 1.1, 0.25)
    jaw_rad = 0.2
    for lon in range(6):
        lon_angle = (lon / 6.0) * math.pi + math.pi
        vx = jaw_center[0] + jaw_rad * math.cos(lon_angle)
        vy = jaw_center[1] + jaw_rad * math.sin(lon_angle)
        vz = jaw_center[2]
        vertices.append((vx, vy, vz, 1))
        
    for lon in range(5):
        edges.append((jaw_v_start + lon, jaw_v_start + lon + 1))
        
    body_center = (0.0, 0.3, 0.0)
    body_rad_x, body_rad_y, body_rad_z = 0.7, 0.8, 0.5
    body_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat, sin_lat = math.cos(lat_angle), math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = body_center[0] + body_rad_x * cos_lat * math.cos(lon_angle)
            vy = body_center[1] + body_rad_y * sin_lat
            vz = body_center[2] + body_rad_z * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    for lat in range(9):
        for lon in range(8):
            curr = body_v_start + lat * 8 + lon
            edges.append((curr, body_v_start + lat * 8 + ((lon + 1) % 8)))
            if lat < 8:
                edges.append((curr, body_v_start + (lat + 1) * 8 + lon))
                
    for sign in [-1, 1]:
        ear_center = (sign * 0.45, 1.8, 0.0)
        ear_rad = 0.2
        ear_v_start = len(vertices)
        for lat in range(-2, 3):
            lat_angle = (lat / 3.0) * (math.pi / 2.0)
            cos_lat, sin_lat = math.cos(lat_angle), math.sin(lat_angle)
            for lon in range(6):
                lon_angle = (lon / 6.0) * 2.0 * math.pi
                vx = ear_center[0] + ear_rad * cos_lat * math.cos(lon_angle)
                vy = ear_center[1] + ear_rad * sin_lat
                vz = ear_center[2] + ear_rad * cos_lat * math.sin(lon_angle)
                vertices.append((vx, vy, vz, 0))
                
        for lat in range(5):
            for lon in range(6):
                curr = ear_v_start + lat * 6 + lon
                edges.append((curr, ear_v_start + lat * 6 + ((lon + 1) % 6)))
                if lat < 4:
                    edges.append((curr, ear_v_start + (lat + 1) * 6 + lon))
                    
    return vertices, edges

if __name__ == "__main__":
    audio_path, glottal_flow = generate_3m_audio()
    
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "teddy_3m_performance.mp4")
    
    render_3m_video(audio_path, glottal_flow, output_mp4)
    
    try:
        os.remove(audio_path)
    except Exception:
        pass
