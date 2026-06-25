import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
BPM = 120  # Standard House tempo
BEAT_DUR = 60.0 / BPM  # 0.5s per beat
DURATION = 100.0  # 4 birds * 25 seconds = 100s
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

def run_coupled_syrinx(s_len, mode, start_time):
    # Modes: "crow", "thrush", "pigeon", "arpeggiator_chaos"
    x1, x1_prev = 0.05, 0.05
    x2, x2_prev = 0.04, 0.04
    x3, x3_prev = 0.0, 0.0
    out = np.zeros(s_len)
    
    # Configure parameters based on bird model (Epibar for tension, Hypobar for compression)
    if mode == "crow":
        m1, m2 = 0.15, 0.12
        epibar1, hypobar1 = 800.0, 3800.0
        epibar2, hypobar2 = 1000.0, 4400.0
        c1, c2 = 1.5, 1.2
        Ps1, Ps2 = 0.7, 0.8
        Kc = 220.0
    elif mode == "thrush":
        m1, m2 = 0.08, 0.06
        epibar1, hypobar1 = 1800.0, 4000.0
        epibar2, hypobar2 = 2200.0, 4800.0
        c1, c2 = 0.8, 0.6
        Ps1, Ps2 = 0.4, 0.4
        Kc = 15.0  # Decoupled bilateral voicing
    elif mode == "pigeon":
        m1, m2 = 0.25, 0.22
        epibar1, hypobar1 = 450.0, 600.0  # Symmetric/soft stiffness
        epibar2, hypobar2 = 500.0, 650.0
        c1, c2 = 2.2, 2.0  # Highly dampened
        Ps1, Ps2 = 0.3, 0.3
        Kc = 80.0
    else:  # "arpeggiator_chaos"
        m1, m2 = 0.1, 0.08
        epibar1, hypobar1 = 1200.0, 3200.0
        epibar2, hypobar2 = 1500.0, 3800.0
        c1, c2 = 1.0, 0.8
        Ps1, Ps2 = 0.6, 0.7
        Kc = 140.0

    # Resonant Load Piston (Mass 3 representing vocal tract impedance)
    m3 = 0.06
    k3 = 850.0
    c3 = 0.45
    A_fold = 0.2
    
    # Dual-Substrate Hertzian contact parameters
    d_soft = 0.005  # outer soft mucosal shell boundary
    d_hard = 0.015  # inner hard cartilage core boundary
    k_soft = 1200.0 # stiffness of mucosal layer
    k_hard = 8500.0 # stiffness of rigid core
    alpha = 1.5     # Hertzian exponent (standard 3/2 contact rule)
    eta = 8.5       # depth-dependent contact damping factor
    
    for s in range(1, s_len - 1):
        t_sec = start_time + s * dt
        
        # Dynamic frequency modulations for active bird calls
        stiff_mod = 1.0
        if mode == "thrush":
            stiff_mod = 1.0 + 0.5 * math.sin(2.0 * math.pi * 9.0 * t_sec)
        elif mode == "crow":
            stiff_mod = 1.0 + 0.15 * math.sin(2.0 * math.pi * 3.0 * t_sec)
        elif mode == "pigeon":
            stiff_mod = 1.0 + 0.08 * math.sin(2.0 * math.pi * 5.0 * t_sec)
        elif mode == "arpeggiator_chaos":
            stiff_mod = 1.0 + 0.8 * math.sin(2.0 * math.pi * (2.0 + (int(t_sec * 4) % 7)) * t_sec)
            
        stiffness1 = (epibar1 if x1 > 0.0 else hypobar1) * stiff_mod
        stiffness2 = (epibar2 if x2 > 0.0 else hypobar2) * stiff_mod
        
        # Velocities
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        v3 = (x3 - x3_prev) / dt
        
        # Dual-Substrate Hertzian contact force calculations (Left Labia)
        f_col1 = 0.0
        penetration1 = 0.0
        if x1 < -d_soft:
            penetration1 = -x1 - d_soft
            # Soft mucosal layer compression
            f_col1 = -k_soft * (penetration1 ** alpha)
            if x1 < -d_hard:
                # Hard rigid core cartilage compression
                f_col1 += -k_hard * (( -x1 - d_hard ) ** alpha)
            # Power-law damping proportional to penetration depth and velocity
            f_col1 -= eta * penetration1 * v1
            
        # Dual-Substrate Hertzian contact force calculations (Right Labia)
        f_col2 = 0.0
        penetration2 = 0.0
        if x2 < -d_soft:
            penetration2 = -x2 - d_soft
            # Soft mucosal layer compression
            f_col2 = -k_soft * (penetration2 ** alpha)
            if x2 < -d_hard:
                # Hard rigid core cartilage compression
                f_col2 += -k_hard * (( -x2 - d_hard ) ** alpha)
            # Power-law damping proportional to penetration depth and velocity
            f_col2 -= eta * penetration2 * v2
        
        # Aerodynamic driving forces (Bernoulli pressure modulation)
        # Flow velocity depends on dynamic area aperture (relative to outer soft boundary)
        aperture1 = max(x1 + d_soft, 0.0)
        aperture2 = max(x2 + d_soft, 0.0)
        
        # Bernoulli pressure drops across the glottal folds
        f_p1 = Ps1 * A_fold * (1.0 - (aperture1 / (aperture1 + 0.05))) if aperture1 > 0.0 else 0.0
        f_p2 = Ps2 * A_fold * (1.0 - (aperture2 / (aperture2 + 0.05))) if aperture2 > 0.0 else 0.0
        
        # Resonant load piston driven by glottal flow area sum
        current_flow = (aperture1 ** 2) + (aperture2 ** 2)
        f_p3 = current_flow * 2.5
        
        # Feedback force from resonant piston onto upper glottal mass
        F_fb = -k3 * x3 - c3 * v3
        
        # Coupled acceleration calculations (including Hertzian dualist contact forces)
        acc1 = (f_p1 + f_col1 - stiffness1 * x1 - c1 * v1 + Kc * (x2 - x1)) / m1
        acc2 = (f_p2 + f_col2 - stiffness2 * x2 - c2 * v2 + Kc * (x1 - x2) + F_fb) / m2
        acc3 = (f_p3 - k3 * x3 - c3 * v3) / m3
        
        # Verlet integration step
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        x3_next = 2.0 * x3 - x3_prev + acc3 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.4, min(1.0, x1_next))
        x2_prev, x2 = x2, max(-0.4, min(1.0, x2_next))
        x3_prev, x3 = x3, max(-0.5, min(1.0, x3_next))
        
        # Save output flow (modulated by acoustic load piston)
        out[s] = current_flow
        
    out -= np.mean(out)
    if np.max(np.abs(out)) > 0:
        out /= np.max(np.abs(out))
    return out

def generate_osiris_audio():
    print("[DSP] Synthesizing Osiris House beat (120 BPM) with 4-phase microtonal model sweeps...")
    mix = np.zeros(num_samples)
    
    # 1. House Beat Generator (Active throughout)
    drums = np.zeros(num_samples)
    beat_samples = int(SAMPLE_RATE * BEAT_DUR)
    total_beats = int(DURATION / BEAT_DUR)
    
    for i in range(total_beats):
        onset = int(i * beat_samples)
        
        # Kick on every beat (Four-to-the-floor)
        k_len = min(num_samples - onset, int(SAMPLE_RATE * 0.25))
        if k_len > 0:
            k_age = np.arange(k_len) / SAMPLE_RATE
            freq = 50.0 + 120.0 * np.exp(-k_age / 0.035)
            kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.2)
            drums[onset:onset+k_len] += np.tanh(kick_val * 1.8) * 0.45
            
        # Clap/Snare on beats 2 and 4 (indices 1, 3, 5, 7...)
        if i % 2 == 1:
            c_len = min(num_samples - onset, int(SAMPLE_RATE * 0.28))
            if c_len > 0:
                c_age = np.arange(c_len) / SAMPLE_RATE
                noise = (np.random.rand(c_len) - 0.5) * 2.0
                clap_val = noise * np.exp(-c_age / 0.08)
                drums[onset:onset+c_len] += np.tanh(clap_val * 1.5) * 0.35
                
        # Off-beat Hi-hats (half-way between beats)
        h_onset = onset + int(beat_samples / 2)
        h_len = min(num_samples - h_onset, int(SAMPLE_RATE * 0.05))
        if h_len > 0:
            h_age = np.arange(h_len) / SAMPLE_RATE
            noise = (np.random.rand(h_len) - 0.5) * 2.0
            drums[h_onset:h_onset+h_len] += noise * np.exp(-h_age / 0.015) * 0.03

    # 2. Classic House Bassline (C minor pentatonic arpeggio)
    bass = np.zeros(num_samples)
    bass_scale = [65.41, 77.78, 87.31, 98.00, 116.54] # C, Eb, F, G, Bb
    step_samples = int(beat_samples / 2) # eighth notes
    num_bass_steps = int(num_samples / step_samples)
    for i in range(num_bass_steps):
        onset = i * step_samples
        note_freq = bass_scale[(i % 8) % len(bass_scale)] * 0.5
        end = min(num_samples, onset + step_samples)
        b_len = end - onset
        if b_len > 0:
            b_age = np.arange(b_len) / SAMPLE_RATE
            phase = (note_freq * b_age) % 1.0
            saw = 2.0 * phase - 1.0
            bass[onset:end] += saw * np.exp(-b_age / 0.12) * 0.25

    # 3. 4-Phase Microtonal Synthesizer Sections (25 seconds each)
    segment_samples = int(SAMPLE_RATE * 25.0)
    modes = ["crow", "thrush", "pigeon", "arpeggiator_chaos"]
    
    birds_audio = np.zeros(num_samples)
    for phase_idx, mode in enumerate(modes):
        onset = phase_idx * segment_samples
        print(f"  -> Synthesizing phase {phase_idx+1}/4: {mode.upper()} ({onset/SAMPLE_RATE}s - {(onset+segment_samples)/SAMPLE_RATE}s)...")
        # Run the coupled syrinx engine for the 25-second block
        bird_block = run_coupled_syrinx(segment_samples, mode, onset/SAMPLE_RATE)
        birds_audio[onset:onset+segment_samples] = bird_block * 0.35

    mix = drums + bass + birds_audio
    mix = np.clip(mix, -1.0, 1.0)
    
    # Save mono 16-bit PCM WAV
    audio_path = "temp_osiris_demo.wav"
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
        
    return audio_path, birds_audio

def render_osiris_video(audio_path, birds_audio, output_mp4):
    vertices, edges = generate_3d_teddy_wireframe()
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    np.random.seed(333)
    stars = [{'x': np.random.uniform(-300, 300), 'y': np.random.uniform(-400, 400), 'z': np.random.uniform(20, 600)} for _ in range(70)]
    
    for frame in range(total_frames):
        time_sec = frame / float(FPS)
        progress = time_sec / DURATION
        
        # House Beat pulse (120 BPM = 2 beats per second)
        beat_phase = (time_sec / BEAT_DUR) * math.pi * 2.0
        beat_amp = max(0.0, math.sin(beat_phase)) * math.exp(-((time_sec % BEAT_DUR) / 0.12))
        
        bg = np.zeros((1280, 720, 3), dtype=np.uint8)
        img = Image.fromarray(bg)
        draw = ImageDraw.Draw(img)
        
        sample_idx = min(num_samples - 1, int(time_sec * SAMPLE_RATE))
        amp_flow = abs(birds_audio[sample_idx])
        
        # Starfield
        star_speed = 4.0 + 12.0 * beat_amp
        for star in stars:
            z_pos = (star['z'] - frame * (star_speed / 8.0) * 10.0) % 580 + 20
            factor = 280.0 / z_pos
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            if 0 <= sx < 720 and 0 <= sy < 1280:
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=(130, 255, 255) if beat_amp > 0.4 else (130, 130, 255))
                
        # Perspective Grid
        draw.line([(0, 950), (720, 950)], fill="#330066", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 1120.0 - 200.0
            draw.line([(CX, 950), (x_bottom, 1200)], fill=(80, 0, 100), width=1)
            
        # Rotate Teddy Bear
        angle_y = progress * 2.0 * math.pi * 4.0
        angle_x = 0.2 * math.sin(progress * 2.0 * math.pi * 6.0)
        qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
        qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
        q_rot = quaternion_multiply(qy, qx)
        
        # Determine bird phase
        phase_idx = int(time_sec / 25.0)
        modes = ["CROW", "SONG THRUSH", "PIGEON", "CHAOTIC ARPEGGIATOR"]
        active_mode = modes[min(3, phase_idx)]
        
        projected = []
        for vx, vy, vz, joint in vertices:
            # Sync jaw to syrinx amplitude flow
            if joint == 1:
                vy = vy - 0.25 * amp_flow
                
            rx, ry, rz = rotate_vector_by_quaternion((vx, vy, vz), q_rot)
            
            # Pulse zoom on kick beat
            depth = 3.8 - 0.45 * beat_amp
            rz += depth
            factor = 450.0 / rz
            
            proj_x = CX + int(rx * factor * 220.0)
            proj_y = CY - int(ry * factor * 220.0)
            projected.append((proj_x, proj_y))
            
        # Wireframe color cycles per bird phase
        colors = ["#ffd700", "#ff00ff", "#33ccff", "#39ff14"]
        color = colors[min(3, phase_idx)]
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=color, width=1)
            
        # Equalizer margins
        num_bands = 8
        bar_w = 12
        bar_spacing = 4
        for band_idx in range(num_bands):
            val_mod = math.sin(progress * 2.0 * math.pi * (2.0 + band_idx) + 12.0 * time_sec)
            band_val = 0.4 * beat_amp + 0.6 * (0.5 + 0.5 * val_mod)
            num_segs = int(band_val * 12)
            lx = 30 + band_idx * (bar_w + bar_spacing)
            rx = 690 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
            for seg in range(12):
                y_pos = 780 - seg * 12
                color_seg = (255, 0, 128) if seg < 6 else ((255, 128, 0) if seg < 10 else (0, 255, 255))
                if seg < num_segs:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color_seg)
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color_seg)
                    
        # Telemetry overlay
        draw.text((40, 60), "TSFi/2: 'OSIRIS' 4-PHASE MICROTONAL BIRD HOUSE DEMO", fill="#00e5ff")
        draw.text((40, 80), f"ACTIVE MODEL: {active_mode}", fill="#ff007f")
        draw.text((40, 100), f"ELAPSED TIME: {int(time_sec)}s / 100s", fill="#ffff00")
        
        reg_val_103 = 1000 - int(840 * beat_amp)
        draw.text((480, 60), f"MQ[103]: {reg_val_103:03d}", fill="#ffff00")
        draw.text((480, 80), "METER: 4/4 HOUSE BEAT", fill="#00ff00")
        draw.text((480, 100), "STATUS: DETERMINISTIC_PLAY", fill="#ff00ff")
        
        # Scrolltext
        scroll_text = f"TSFi/2 'OSIRIS' BIRD-SYNTH DEMO --- PHASE 1: CROW CAW --- PHASE 2: SONG THRUSH CHIRP --- PHASE 3: PIGEON COO --- PHASE 4: MICROTONAL CHAOTIC ARPEGGIATION --- DRIVEN BY COUPLED DUAL-SYRINX SOLVERS..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 720 - int(progress * 6.0 * (total_text_width + 720)) % (total_text_width + 720)
        draw.text((scroll_x, 1220), scroll_text, fill="#39ff14")
        
        pipe.stdin.write(img.tobytes())
        
        if frame % 300 == 0:
            print(f"  -> Rendering Osiris: {frame}/{total_frames} frames ({progress*100:.1f}%)...")
            
    pipe.stdin.close()
    pipe.wait()
    print("[SUCCESS] Osiris presentation successfully saved!")

def generate_3d_teddy_wireframe():
    # Keep teddy model shape identical
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
    audio_path, birds_audio = generate_osiris_audio()
    
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "osiris_birds_demo.mp4")
    
    render_osiris_video(audio_path, birds_audio, output_mp4)
    
    try:
        os.remove(audio_path)
    except Exception:
        pass
