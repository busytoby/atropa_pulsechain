import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
BPM = 120
BEAT_DUR = 60.0 / BPM  # 0.5s per beat
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

def run_emotional_syrinx(s_len, start_time):
    out = np.zeros(s_len)
    t = np.arange(s_len) / SAMPLE_RATE
    
    x1, x1_prev = 0.05, 0.05
    x2, x2_prev = 0.04, 0.04
    m1, m2 = 0.12, 0.09
    k_open = 1100.0
    k_closed = 3200.0
    Kc = 120.0
    A_fold = 0.2
    
    E_ag = 0.1
    H_pl = 0.9
    decay = 0.96
    
    for s in range(1, s_len - 1):
        t_sec = start_time + t[s]
        
        # Vaesen emotional state sweep
        if t_sec < 45.0:
            dyn_in = 0.06
            error_in = 0.03
        elif t_sec < 90.0:
            dyn_in = 0.35 + 0.15 * math.sin(t_sec)
            error_in = 0.25
        elif t_sec < 150.0:
            dyn_in = 0.85
            error_in = 0.88
        else:
            dyn_in = 0.08
            error_in = 0.05
            
        E_ag = decay * E_ag + (1.0 - decay) * dyn_in
        H_pl = decay * H_pl + (1.0 - decay) * (1.0 - error_in)
        
        k_closed_mod = k_closed * (1.0 + 1.8 * E_ag)
        Kc_mod = Kc * (1.0 - 0.7 * H_pl)
        stiff_mod = 1.0 + 0.45 * math.sin(2.0 * math.pi * (6.0 + 5.0 * E_ag) * t_sec)
        
        stiffness1 = (k_open if x1 > 0.0 else k_closed_mod) * stiff_mod
        stiffness2 = (k_open * 1.35 if x2 > 0.0 else k_closed_mod * 1.35) * stiff_mod
        
        f_p1 = 0.55 * A_fold if x1 > 0.0 else 0.0
        f_p2 = 0.65 * A_fold if x2 > 0.0 else 0.0
        
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        
        acc1 = (f_p1 - stiffness1 * x1 - 1.4 * v1 + Kc_mod * (x2 - x1)) / m1
        acc2 = (f_p2 - stiffness2 * x2 - 1.1 * v2 + Kc_mod * (x1 - x2)) / m2
        
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.2, min(1.0, x1_next))
        x2_prev, x2 = x2, max(-0.2, min(1.0, x2_next))
        
        out[s] = (max(x1, 0.0) ** 2) + (max(x2, 0.0) ** 2)
        
    out -= np.mean(out)
    if np.max(np.abs(out)) > 0:
        out /= np.max(np.abs(out))
    return out

def generate_wing_flaps(length, start_time):
    out = np.zeros(length)
    noise = np.random.uniform(-1.0, 1.0, length)
    t = np.arange(length) / SAMPLE_RATE
    
    for s in range(length):
        t_sec = start_time + t[s]
        rate = 5.0 if t_sec < 45.0 else (8.5 if t_sec < 150.0 else 4.0)
        
        # Periodic burst modulation to make flutters sound natural and spaced out
        # Every 6.0 seconds, do a 1.5-second burst of flaps
        burst_period = 6.0
        burst_duration = 1.5
        cycle = t_sec % burst_period
        if cycle < burst_duration:
            fade_env = 0.5 - 0.5 * math.cos(2.0 * math.pi * (cycle / burst_duration))
            flap_env = (0.5 + 0.5 * math.sin(2.0 * math.pi * rate * t_sec)) * fade_env
        else:
            flap_env = 0.0
        
        cutoff = 80.0 + 300.0 * flap_env
        alpha = 2.0 * math.pi * cutoff * dt
        y = 0.0 if s == 0 else out[s-1]
        y = alpha * noise[s] + (1.0 - alpha) * y
        out[s] = y * flap_env * 0.4
    return out

def generate_feather_flutter(length, start_time, turbulence_sig):
    out = np.zeros(length)
    noise = np.random.uniform(-1.0, 1.0, length)
    t = np.arange(length) / SAMPLE_RATE
    
    center_freqs = [2200.0, 3800.0, 5500.0]
    q_factor = 25.0
    w_states = [[0.0, 0.0] for _ in center_freqs]
    
    for s in range(length):
        t_sec = start_time + t[s]
        velocity_excite = abs(turbulence_sig[s]) * 2.5
        excitation_source = noise[s] * velocity_excite
        
        sig_mix = 0.0
        for i, f0 in enumerate(center_freqs):
            f_dyn = f0 * (1.0 + 0.08 * math.sin(2.0 * math.pi * 32.0 * t_sec))
            
            omega = 2.0 * math.pi * f_dyn * dt
            alpha = math.sin(omega) / (2.0 * q_factor)
            b0 = alpha
            b2 = -alpha
            a0 = 1.0 + alpha
            a1 = -2.0 * math.cos(omega)
            a2 = 1.0 - alpha
            
            w0 = excitation_source - (a1 * w_states[i][0] + a2 * w_states[i][1]) / a0
            y0 = (b0 * w0 + b2 * w_states[i][1]) / a0
            w_states[i][1], w_states[i][0] = w_states[i][0], w0
            sig_mix += y0
            
        out[s] = sig_mix * 0.35
    return out

def generate_percussion_claps(length, beat_samples, start_time):
    out = np.zeros(length)
    noise = np.random.uniform(-1.0, 1.0, length)
    total_beats = int(length / beat_samples)
    
    for i in range(total_beats):
        onset_1 = int(i * beat_samples)
        time_sec = start_time + (onset_1 / SAMPLE_RATE)
        
        if i % 2 == 1:
            onset_2 = onset_1 + int(SAMPLE_RATE * 0.024)
            if 45.0 <= time_sec < 150.0:
                len_1 = min(length - onset_1, int(SAMPLE_RATE * 0.045))
                if len_1 > 0:
                    age = np.arange(len_1) / SAMPLE_RATE
                    decay = np.exp(-age / 0.007)
                    bp = np.sin(2.0 * np.pi * 850.0 * age) * noise[onset_1:onset_1+len_1] * decay
                    out[onset_1:onset_1+len_1] += bp * 0.38
                    
                len_2 = min(length - onset_2, int(SAMPLE_RATE * 0.045))
                if len_2 > 0:
                    age = np.arange(len_2) / SAMPLE_RATE
                    decay = np.exp(-age / 0.007)
                    bp = np.sin(2.0 * np.pi * 850.0 * age) * noise[onset_2:onset_2+len_2] * decay
                    out[onset_2:onset_2+len_2] += bp * 0.32
    return out

def generate_acid_house_bass(length, beat_samples, start_time):
    # Generates a TB-303 squelch baseline using a Chamberlin SVF sweep
    out = np.zeros(length)
    step_samples = int(beat_samples / 2) # eighth notes
    total_steps = int(length / step_samples)
    
    # C minor pentatonic acid pattern
    scale = [65.41, 77.78, 87.31, 98.00, 116.54, 130.81]
    pattern = [0, 4, 3, 0, 5, 2, 4, 1, 3, 0, 5, 4, 2, 0, 3, 1]
    
    filter_state = [0.0, 0.0]
    vaesen_rep = 0.0
    last_note = -1
    
    for i in range(total_steps):
        onset = i * step_samples
        time_sec = start_time + (onset / SAMPLE_RATE)
        
        # Bass active 45s - 150s
        if 45.0 <= time_sec < 150.0:
            note_idx = pattern[i % len(pattern)]
            note_freq = scale[note_idx] * 0.5
            
            # Accumulator increments on note repetition or hitting root note (scale[0])
            if note_idx == last_note:
                vaesen_rep = 0.92 * vaesen_rep + 0.08 * 1.0
            elif note_idx == 0:
                vaesen_rep = 0.92 * vaesen_rep + 0.08 * 0.7
            else:
                vaesen_rep = 0.92 * vaesen_rep
                
            last_note = note_idx
            
            # Slide and Accent modulation
            is_slide = (i % 4 == 3)
            is_accent = (i % 8 == 0 or i % 8 == 4)
            
            end = min(length, onset + step_samples)
            b_len = end - onset
            if b_len > 0:
                b_age = np.arange(b_len) / SAMPLE_RATE
                
                # Dynamic filter cutoff envelope
                decay_rate = 0.08 if is_slide else 0.16
                filter_env = np.exp(-b_age / decay_rate)
                
                # Acid Squelch frequency modulation
                f0 = 200.0 + 800.0 * filter_env
                if is_accent:
                    f0 += 400.0 * filter_env
                
                # Chamberlin State Variable Filter simulation
                for s in range(b_len):
                    # Sawtooth oscillator
                    phase = (note_freq * b_age[s]) % 1.0
                    saw = 2.0 * phase - 1.0
                    
                    # Run Chamberlin SVF
                    fc = f0[s]
                    omega = 2.0 * math.sin(math.pi * fc * dt)
                    
                    # Base resonance
                    q = 0.12 if is_accent else 0.18
                    # Repetitive note/dwell accumulator lowers q (boosting squelch resonance)
                    q_mod = q - 0.07 * vaesen_rep
                    if q_mod < 0.05:
                        q_mod = 0.05
                    
                    low = filter_state[0] + omega * filter_state[1]
                    high = saw - low - q_mod * filter_state[1]
                    band = omega * high + filter_state[1]
                    
                    filter_state[0] = low
                    filter_state[1] = band
                    
                    # Apply clipping/waveshaping
                    sig = np.tanh(low * (1.6 if is_accent else 1.2))
                    out[onset + s] = sig * 0.28
                    
    return out

def generate_grand_acid_audio():
    print("[DSP] Synthesizing 3-minute Grand Osiris Acid Bird House soundtrack...")
    mix = np.zeros(num_samples)
    
    # 1. House beat
    drums = np.zeros(num_samples)
    beat_samples = int(SAMPLE_RATE * BEAT_DUR)
    total_beats = int(DURATION / BEAT_DUR)
    
    for i in range(total_beats):
        onset = int(i * beat_samples)
        time_sec = onset / SAMPLE_RATE
        
        if time_sec >= 45.0:
            k_len = min(num_samples - onset, int(SAMPLE_RATE * 0.25))
            if k_len > 0:
                k_age = np.arange(k_len) / SAMPLE_RATE
                freq = 46.0 + 130.0 * np.exp(-k_age / 0.035)
                kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.18)
                drums[onset:onset+k_len] += np.tanh(kick_val * 1.9) * 0.45
                
        if 45.0 <= time_sec < 150.0:
            h_onset = onset + int(beat_samples / 2)
            h_len = min(num_samples - h_onset, int(SAMPLE_RATE * 0.05))
            if h_len > 0:
                h_age = np.arange(h_len) / SAMPLE_RATE
                noise = (np.random.rand(h_len) - 0.5) * 2.0
                drums[h_onset:h_onset+h_len] += noise * np.exp(-h_age / 0.015) * 0.035

    # 2. Acid Bassline
    acid_bass = generate_acid_house_bass(num_samples, beat_samples, 0.0)

    # 3. Flaps, Flutters, Claps, and Syrinx vocals
    syrinx_vocals = run_emotional_syrinx(num_samples, 0.0)
    turbulence = generate_wing_flaps(num_samples, 0.0)
    flutter = generate_feather_flutter(num_samples, 0.0, turbulence)
    percussion_claps = generate_percussion_claps(num_samples, beat_samples, 0.0)
    
    mix = drums + acid_bass + syrinx_vocals * 0.32 + turbulence * 0.9 + flutter * 1.1 + percussion_claps * 0.98
    mix = np.clip(mix, -1.0, 1.0)
    
    audio_path = "temp_grand_osiris_acid.wav"
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
        
    return audio_path, syrinx_vocals, turbulence, flutter, percussion_claps, acid_bass

def render_grand_acid_video(audio_path, syrinx_vocals, turbulence, flutter, percussion_claps, acid_bass, output_mp4):
    vertices, edges = generate_3d_teddy_wireframe()
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    np.random.seed(666)
    stars = [{'x': np.random.uniform(-300, 300), 'y': np.random.uniform(-400, 400), 'z': np.random.uniform(20, 600)} for _ in range(65)]
    
    E_ag = 0.1
    H_pl = 0.9
    decay = 0.96
    
    for frame in range(total_frames):
        time_sec = frame / float(FPS)
        progress = time_sec / DURATION
        
        # House Beat pulse (120 BPM)
        beat_phase = (time_sec / BEAT_DUR) * math.pi * 2.0
        beat_amp = max(0.0, math.sin(beat_phase)) * math.exp(-((time_sec % BEAT_DUR) / 0.12))
        
        bg = np.zeros((1280, 720, 3), dtype=np.uint8)
        img = Image.fromarray(bg)
        draw = ImageDraw.Draw(img)
        
        sample_idx = min(num_samples - 1, int(time_sec * SAMPLE_RATE))
        amp_vocals = abs(syrinx_vocals[sample_idx])
        amp_turb = abs(turbulence[sample_idx])
        amp_flut = abs(flutter[sample_idx])
        amp_clap = abs(percussion_claps[sample_idx])
        amp_bass = abs(acid_bass[sample_idx])
        
        # Emotional state sweeps
        if time_sec < 45.0:
            dyn_in = 0.06
            error_in = 0.03
        elif time_sec < 90.0:
            dyn_in = 0.35 + 0.15 * math.sin(time_sec)
            error_in = 0.25
        elif time_sec < 150.0:
            dyn_in = 0.85
            error_in = 0.88
        else:
            dyn_in = 0.08
            error_in = 0.05
            
        E_ag = decay * E_ag + (1.0 - decay) * dyn_in
        H_pl = decay * H_pl + (1.0 - decay) * (1.0 - error_in)
        
        # Starfield speed reacts to wing flaps and acid bass sweeps
        star_speed = 3.0 + 15.0 * amp_turb + 15.0 * amp_flut + 20.0 * amp_bass
        for star in stars:
            z_pos = (star['z'] - frame * (star_speed / 8.0) * 10.0) % 580 + 20
            factor = 280.0 / z_pos
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            if 0 <= sx < 720 and 0 <= sy < 1280:
                color_fill = (255, 255, 100) if amp_clap > 0.05 else ((130, 255, 255) if amp_bass > 0.15 else (130, 130, 255))
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=color_fill)
                
        # Perspective Grid
        draw.line([(0, 950), (720, 950)], fill="#330033", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 1120.0 - 200.0
            draw.line([(CX, 950), (x_bottom, 1200)], fill=(80, 0, 80), width=1)
            
        # Rotate Teddy Bear
        rot_speed = 2.0 + 6.0 * E_ag
        angle_y = progress * 2.0 * math.pi * rot_speed
        angle_x = 0.25 * math.cos(progress * 2.0 * math.pi * (rot_speed + 2.0))
        qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
        qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
        q_rot = quaternion_multiply(qy, qx)
        
        # Verlet Squash and Stretch mapping based on double claps & acid bass amplitude
        scale_x = 1.0 + 0.22 * amp_clap + 0.18 * amp_bass
        scale_y = 1.0 - 0.22 * amp_clap - 0.18 * amp_bass
        
        projected = []
        for vx, vy, vz, joint in vertices:
            vx_s = vx * scale_x
            vy_s = vy * scale_y
            
            if joint == 1:
                vy_s = vy_s - 0.22 * amp_vocals
                
            if joint == 2:  # Left Wing
                vx_s -= 0.25 * amp_turb
                vy_s += 0.08 * math.sin(2.0 * math.pi * 30.0 * time_sec) * amp_flut * 6.0
            elif joint == 3:  # Right Wing
                vx_s += 0.25 * amp_turb
                vy_s += 0.08 * math.sin(2.0 * math.pi * 30.0 * time_sec) * amp_flut * 6.0
                
            rx, ry, rz = rotate_vector_by_quaternion((vx_s, vy_s, vz), q_rot)
            
            depth = 3.8 - 0.5 * beat_amp
            rz += depth
            factor = 450.0 / rz
            
            proj_x = CX + int(rx * factor * 220.0)
            proj_y = CY - int(ry * factor * 220.0)
            projected.append((proj_x, proj_y))
            
        # Wireframe color cycles to reflect internal Vaesen bird mood + acid flash
        if amp_bass > 0.2:
            color = "#39ff14"  # acid green flash
        elif E_ag > 0.7:
            color = "#ff3399" if (frame % 4 < 2) else "#ffff00"
        elif E_ag > 0.3:
            color = "#ff9900"
        else:
            color = "#00ffcc"
            
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=color, width=1)
            
        # Margin EQ bars
        num_bands = 8
        bar_w = 12
        bar_spacing = 4
        for band_idx in range(num_bands):
            val_mod = math.sin(progress * 2.0 * math.pi * (2.0 + band_idx) + 16.0 * time_sec)
            band_val = 0.3 * amp_turb + 0.3 * amp_flut + 0.2 * amp_bass + 0.2 * (0.5 + 0.5 * val_mod)
            num_segs = int(band_val * 12)
            lx = 30 + band_idx * (bar_w + bar_spacing)
            rx = 690 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
            for seg in range(12):
                y_pos = 780 - seg * 12
                color_seg = (255, 0, 128) if seg < 6 else ((255, 255, 0) if seg < 10 else (57, 255, 20))
                if seg < num_segs:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color_seg)
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color_seg)
                    
        # Telemetry
        draw.text((40, 60), "TSFi/2: 'GRAND OSIRIS' ACID BIRD HOUSE DEMO", fill="#39ff14")
        draw.text((40, 80), f"AGITATION (E_ag): {E_ag:.4f}", fill="#ff3399")
        draw.text((40, 100), f"BASS COUPLING: {amp_bass:.4f}", fill="#00e5ff")
        
        reg_val_103 = 1000 - int(840 * beat_amp)
        draw.text((480, 60), f"MQ[103]: {reg_val_103:03d}", fill="#ffff00")
        draw.text((480, 80), "SYNTH: TB-303 RESONANT SVF", fill="#00ff00")
        draw.text((480, 100), "STATUS: ACID_PLAYING", fill="#ff00ff")
        
        # Scrolltext
        scroll_text = "TSFi/2 GRAND OSIRIS ACID BIRD HOUSE RELEASE --- TB-303 EMULATION WITH SQUELCH CHAMBERLIN SVF --- EMOTIONAL VAESEN BIRD COUPLINGS --- PEGGING THE BLOCKCHAIN..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 720 - int(progress * 4.0 * (total_text_width + 720)) % (total_text_width + 720)
        draw.text((scroll_x, 1220), scroll_text, fill="#39ff14")
        
        pipe.stdin.write(img.tobytes())
        
        if frame % 600 == 0:
            print(f"  -> Rendering Grand Osiris Acid: {frame}/{total_frames} frames ({progress*100:.1f}%)...")
            
    pipe.stdin.close()
    pipe.wait()
    print("[SUCCESS] Grand Osiris Acid performance successfully saved!")

def generate_3d_teddy_wireframe():
    # Build wireframe teddy bear model
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
                
    left_arm_start = len(vertices)
    for i in range(4):
        vx = -0.7 - i * 0.25
        vy = 0.6 + i * 0.1
        vz = 0.0
        vertices.append((vx, vy, vz, 2))
    for i in range(3):
        edges.append((left_arm_start + i, left_arm_start + i + 1))
        
    right_arm_start = len(vertices)
    for i in range(4):
        vx = 0.7 + i * 0.25
        vy = 0.6 + i * 0.1
        vz = 0.0
        vertices.append((vx, vy, vz, 3))
    for i in range(3):
        edges.append((right_arm_start + i, right_arm_start + i + 1))
        
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
    audio_path, syrinx_vocals, turbulence, flutter, percussion_claps, acid_bass = generate_grand_acid_audio()
    
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "grand_osiris_acid.mp4")
    
    render_grand_acid_video(audio_path, syrinx_vocals, turbulence, flutter, percussion_claps, acid_bass, output_mp4)
    
    try:
        os.remove(audio_path)
    except Exception:
        pass
