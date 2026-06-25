import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
BPM = 120
BEAT_DUR = 60.0 / BPM  # 0.5s per beat
DURATION = 60.0  # 60s demo
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

def generate_wing_flaps_and_flutters(length):
    # Generates wing flaps/wooshes using low-pass swept white noise
    out = np.zeros(length)
    noise = np.random.uniform(-1.0, 1.0, length)
    
    # 6Hz LFO representing flap rate
    t = np.arange(length) / SAMPLE_RATE
    flap_envelope = 0.5 + 0.5 * np.sin(2.0 * np.pi * 6.0 * t)
    
    # Dynamic low-pass filter simulation
    # y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
    # alpha is swept dynamically by the flap envelope (sweeping cutoff from 100Hz to 600Hz)
    y = 0.0
    for s in range(length):
        cutoff = 100.0 + 500.0 * flap_envelope[s]
        alpha = 2.0 * math.pi * cutoff * dt
        alpha = max(0.0, min(1.0, alpha))
        y = alpha * noise[s] + (1.0 - alpha) * y
        out[s] = y * flap_envelope[s] * 0.4
        
    return out

def generate_bird_double_claps(length, beat_samples):
    # Triggers bird double claps (wing snaps) on beats 2 and 4 (beats 1, 3, 5...)
    out = np.zeros(length)
    noise = np.random.uniform(-1.0, 1.0, length)
    
    total_beats = int(length / beat_samples)
    for i in range(total_beats):
        if i % 2 == 1:  # Beats 2 and 4
            onset_1 = int(i * beat_samples)
            # Double clap: second impact 22ms later
            onset_2 = onset_1 + int(SAMPLE_RATE * 0.022)
            
            # Impact 1
            len_1 = min(length - onset_1, int(SAMPLE_RATE * 0.04))
            if len_1 > 0:
                age = np.arange(len_1) / SAMPLE_RATE
                decay = np.exp(-age / 0.008)
                # Bandpass filter around 800Hz
                bp = np.sin(2.0 * np.pi * 800.0 * age) * noise[onset_1:onset_1+len_1] * decay
                out[onset_1:onset_1+len_1] += bp * 0.35
                
            # Impact 2
            len_2 = min(length - onset_2, int(SAMPLE_RATE * 0.04))
            if len_2 > 0:
                age = np.arange(len_2) / SAMPLE_RATE
                decay = np.exp(-age / 0.008)
                bp = np.sin(2.0 * np.pi * 800.0 * age) * noise[onset_2:onset_2+len_2] * decay
                out[onset_2:onset_2+len_2] += bp * 0.3

    return out

def generate_coupled_syrinx_vocals(length):
    out = np.zeros(length)
    
    # 3-Mass Ishizaka-Flanagan style Verlet physical model simulation variables
    # Mass 1 (Lower Fold)
    x1, x1_prev = 0.05, 0.05
    m1 = 0.1
    epibar1 = 1400.0
    hypobar1 = 3600.0
    c1 = 1.0
    Ps1 = 0.5
    
    # Mass 2 (Upper Fold)
    x2, x2_prev = 0.04, 0.04
    m2 = 0.08
    epibar2 = 1600.0
    hypobar2 = 4200.0
    c2 = 0.8
    Ps2 = 0.6
    
    # Mass 3 (Resonant Load Piston representing vocal tract impedance)
    x3, x3_prev = 0.0, 0.0
    m3 = 0.06
    k3 = 900.0
    c3 = 0.4
    
    # Coupling parameters
    Kc = 140.0
    A_fold = 0.2
    
    for s in range(1, length - 1):
        t_sec = s * dt
        
        # Periodic birdsong whistle modulation
        stiff_mod = 1.0 + 0.6 * math.sin(2.0 * math.pi * 8.0 * t_sec)
        
        # Asymmetric stiffness (Epibar for tension, Hypobar for compression)
        stiffness1 = epibar1 if x1 > 0.0 else hypobar1
        stiffness2 = epibar2 if x2 > 0.0 else hypobar2
        
        stiffness1 *= stiff_mod
        stiffness2 *= stiff_mod
        
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
        out[s] = current_flow
        
    out -= np.mean(out)
    if np.max(np.abs(out)) > 0:
        out /= np.max(np.abs(out))
    return out

def generate_flaps_audio():
    print("[DSP] Synthesizing Osiris House (Flaps & Flutters Edition)...")
    mix = np.zeros(num_samples)
    
    # 1. Four-to-the-floor House kick and hats
    drums = np.zeros(num_samples)
    beat_samples = int(SAMPLE_RATE * BEAT_DUR)
    total_beats = int(DURATION / BEAT_DUR)
    
    for i in range(total_beats):
        onset = int(i * beat_samples)
        
        # Kick
        k_len = min(num_samples - onset, int(SAMPLE_RATE * 0.25))
        if k_len > 0:
            k_age = np.arange(k_len) / SAMPLE_RATE
            freq = 48.0 + 130.0 * np.exp(-k_age / 0.035)
            kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.2)
            drums[onset:onset+k_len] += np.tanh(kick_val * 1.8) * 0.45
            
        # Off-beat hats
        h_onset = onset + int(beat_samples / 2)
        h_len = min(num_samples - h_onset, int(SAMPLE_RATE * 0.05))
        if h_len > 0:
            h_age = np.arange(h_len) / SAMPLE_RATE
            noise = (np.random.rand(h_len) - 0.5) * 2.0
            drums[h_onset:h_onset+h_len] += noise * np.exp(-h_age / 0.015) * 0.035

    # 2. House Bassline
    bass = np.zeros(num_samples)
    bass_scale = [65.41, 77.78, 87.31, 98.00, 116.54] # C, Eb, F, G, Bb
    step_samples = int(beat_samples / 2)
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

    # 3. Flaps, Flutters, and Double Claps
    wing_flaps = generate_wing_flaps_and_flutters(num_samples)
    double_claps = generate_bird_double_claps(num_samples, beat_samples)
    
    # 4. Coupled Syrinx bird song sweeps
    syrinx_vocals = generate_coupled_syrinx_vocals(num_samples)
    
    mix = drums + bass + wing_flaps * 0.9 + double_claps * 1.0 + syrinx_vocals * 0.25
    mix = np.clip(mix, -1.0, 1.0)
    
    audio_path = "temp_flaps_demo.wav"
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
        
    return audio_path, wing_flaps, double_claps

def render_flaps_video(audio_path, wing_flaps, double_claps, output_mp4):
    vertices, edges = generate_3d_teddy_wireframe()
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    np.random.seed(111)
    stars = [{'x': np.random.uniform(-300, 300), 'y': np.random.uniform(-400, 400), 'z': np.random.uniform(20, 600)} for _ in range(60)]
    
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
        amp_flap = abs(wing_flaps[sample_idx])
        amp_clap = abs(double_claps[sample_idx])
        
        # Starfield speed reacts to wing flaps (wooshes)
        star_speed = 3.0 + 35.0 * amp_flap
        for star in stars:
            z_pos = (star['z'] - frame * (star_speed / 8.0) * 10.0) % 580 + 20
            factor = 280.0 / z_pos
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            if 0 <= sx < 720 and 0 <= sy < 1280:
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=(130, 255, 130) if amp_flap > 0.1 else (130, 130, 255))
                
        # Perspective Grid
        draw.line([(0, 950), (720, 950)], fill="#1a3300", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 1120.0 - 200.0
            draw.line([(CX, 950), (x_bottom, 1200)], fill=(0, 60, 20), width=1)
            
        # Rotate Teddy Bear
        angle_y = progress * 2.0 * math.pi * 3.0
        angle_x = 0.25 * math.cos(progress * 2.0 * math.pi * 4.0)
        qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
        qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
        q_rot = quaternion_multiply(qy, qx)
        
        # Physics: Verlet Squash and Stretch mapping
        # Teddy squashes when a clap occurs, or on the downbeat
        scale_x = 1.0 + 0.25 * amp_clap
        scale_y = 1.0 - 0.25 * amp_clap
        
        projected = []
        for vx, vy, vz, joint in vertices:
            # Scale coordinates with squash and stretch
            vx_s = vx * scale_x
            vy_s = vy * scale_y
            
            # Animate arms to flap when wings flap
            if joint == 2:  # Left Arm
                vx_s -= 0.3 * amp_flap
            elif joint == 3:  # Right Arm
                vx_s += 0.3 * amp_flap
                
            rx, ry, rz = rotate_vector_by_quaternion((vx_s, vy_s, vz), q_rot)
            
            # Zoom in on clap impact
            depth = 3.8 - 0.5 * amp_clap
            rz += depth
            factor = 450.0 / rz
            
            proj_x = CX + int(rx * factor * 220.0)
            proj_y = CY - int(ry * factor * 220.0)
            projected.append((proj_x, proj_y))
            
        # Draw Teddy bear wireframe (flashes green/cyan on wing clap)
        color = "#39ff14" if amp_clap > 0.05 else "#00e5ff"
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=color, width=1)
            
        # Margin EQ bars
        num_bands = 8
        bar_w = 12
        bar_spacing = 4
        for band_idx in range(num_bands):
            val_mod = math.sin(progress * 2.0 * math.pi * (2.0 + band_idx) + 15.0 * time_sec)
            band_val = 0.5 * amp_flap + 0.5 * (0.5 + 0.5 * val_mod)
            num_segs = int(band_val * 12)
            lx = 30 + band_idx * (bar_w + bar_spacing)
            rx = 690 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
            for seg in range(12):
                y_pos = 780 - seg * 12
                color_seg = (0, 255, 128) if seg < 6 else ((255, 255, 0) if seg < 10 else (255, 0, 128))
                if seg < num_segs:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color_seg)
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color_seg)
                    
        # Telemetry
        draw.text((40, 60), "TSFi/2: 'WING FLUTTERS & DOUBLE CLAPS' PERFORMER", fill="#39ff14")
        draw.text((40, 80), f"WING FLUTTER POWER: {amp_flap:.4f}", fill="#00e5ff")
        draw.text((40, 100), f"DOUBLE CLAP IMPACT: {'IMPACT' if amp_clap > 0.05 else 'OFF'}", fill="#ff00ff")
        
        reg_val_103 = 1000 - int(840 * amp_clap)
        draw.text((480, 60), f"MQ[103]: {reg_val_103:03d}", fill="#ffff00")
        draw.text((480, 80), f"SQUASH: {scale_y:.3f}x", fill="#00ff00")
        draw.text((480, 100), "STATUS: FLUTTERS_ACTIVE", fill="#ff00ff")
        
        # Scrolltext
        scroll_text = "TSFi/2 WING FLAPS AND FLUTTERS ENVELOPE --- DOUBLE CLAP WING SNAPS ON BEATS 2 & 4 --- VERLET SQUASH AND STRETCH DEFORMATION DYNAMICS --- PEGGING THE BLOCKCHAIN..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 720 - int(progress * 4.0 * (total_text_width + 720)) % (total_text_width + 720)
        draw.text((scroll_x, 1220), scroll_text, fill="#39ff14")
        
        pipe.stdin.write(img.tobytes())
        
        if frame % 300 == 0:
            print(f"  -> Rendering Flaps & Flutters: {frame}/{total_frames} frames ({progress*100:.1f}%)...")
            
    pipe.stdin.close()
    pipe.wait()
    print("[SUCCESS] Flaps & Flutters presentation successfully saved!")

def generate_3d_teddy_wireframe():
    # Build teddy bear model with joints designated for wings/arms
    # joint_type: 0 = static body, 1 = lower jaw, 2 = left arm/wing, 3 = right arm/wing
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
                
    # Flapping Wings / Arms
    # Left Arm (joint_type 2)
    left_arm_start = len(vertices)
    for i in range(4):
        vx = -0.7 - i * 0.25
        vy = 0.6 + i * 0.1
        vz = 0.0
        vertices.append((vx, vy, vz, 2))
    for i in range(3):
        edges.append((left_arm_start + i, left_arm_start + i + 1))
        
    # Right Arm (joint_type 3)
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
    audio_path, wing_flaps, double_claps = generate_flaps_audio()
    
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "teddy_flaps_flutters.mp4")
    
    render_flaps_video(audio_path, wing_flaps, double_claps, output_mp4)
    
    try:
        os.remove(audio_path)
    except Exception:
        pass
