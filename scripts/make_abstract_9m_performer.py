import os
import math
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# DSP parameters
SAMPLE_RATE = 22050  # Lower sample rate to render audio extremely fast
FPS = 20            # 20 FPS to render 9 minutes of video (10,800 frames) fast
DURATION_SEC = 540  # 9 minutes
TOTAL_FRAMES = DURATION_SEC * FPS
dt = 1.0 / SAMPLE_RATE

# Colors
BG_COLOR = (5, 5, 10)
HUD_COLOR = (0, 242, 254)
ACCENT_COLOR = (255, 0, 127)
WARNING_COLOR = (255, 204, 0)
PEACH_COLOR = (255, 150, 100)

def generate_9m_performance():
    print("[DSP] Starting 9-minute abstract flock-communication performance synthesis...")
    num_samples = int(DURATION_SEC * SAMPLE_RATE)
    mix = np.zeros(num_samples)
    
    # 3 Agent Birds individual accumulator states
    # Bird 1: Bass Bird (Left)
    ag1, pl1, rep1 = 0.1, 0.9, 0.0
    # Bird 2: Fascinator Bird (Center)
    ag2, pl2, rep2 = 0.15, 0.8, 0.0
    # Bird 3: Agitation Bird (Right)
    ag3, pl3, rep3 = 0.2, 0.7, 0.0
    
    decay = 0.992
    
    # Beat parameters
    bpm = 124.0
    beat_samples = int((60.0 / bpm) * SAMPLE_RATE)
    step_samples = int(beat_samples / 4)
    total_steps = int(num_samples / step_samples)
    
    # Base patterns
    base_kick = [1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    base_snare = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    base_hat = [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1]
    
    # Pre-render drum sounds
    kick_len = int(SAMPLE_RATE * 0.18)
    k_age = np.arange(kick_len) / SAMPLE_RATE
    k_freq = 50.0 + 120.0 * np.exp(-k_age / 0.03)
    kick_sound = np.sin(2.0 * np.pi * k_freq * k_age) * np.exp(-k_age / 0.12)
    kick_sound = np.tanh(kick_sound * 1.5) * 0.4
    
    snare_len = int(SAMPLE_RATE * 0.15)
    s_age = np.arange(snare_len) / SAMPLE_RATE
    snare_noise = np.random.uniform(-1.0, 1.0, snare_len)
    snare_sound = snare_noise * np.exp(-s_age / 0.045) * 0.25
    
    hat_len = int(SAMPLE_RATE * 0.04)
    h_age = np.arange(hat_len) / SAMPLE_RATE
    hat_noise = np.random.uniform(-1.0, 1.0, hat_len)
    hat_sound = np.sin(2.0 * np.pi * 9000.0 * h_age) * hat_noise * np.exp(-h_age / 0.012) * 0.12
    
    # Time signature configurations
    signatures = [
        {"name": "4/4 Standard", "steps_per_measure": 16},
        {"name": "5/4 Odd Phase", "steps_per_measure": 20},
        {"name": "7/8 Micro-Phase", "steps_per_measure": 14},
        {"name": "13/8 Complex", "steps_per_measure": 26}
    ]
    current_sig_idx = 0
    consecutive_repeats = 0
    last_pattern_hash = 0
    
    # Active pattern states
    kick_pat = base_kick.copy()
    snare_pat = base_snare.copy()
    hat_pat = base_hat.copy()
    
    # Store history arrays for the renderer
    ag1_arr = np.zeros(num_samples)
    pl1_arr = np.zeros(num_samples)
    rep1_arr = np.zeros(num_samples)
    
    ag2_arr = np.zeros(num_samples)
    pl2_arr = np.zeros(num_samples)
    rep2_arr = np.zeros(num_samples)
    
    ag3_arr = np.zeros(num_samples)
    pl3_arr = np.zeros(num_samples)
    rep3_arr = np.zeros(num_samples)
    
    kick_hits = np.zeros(num_samples)
    fascinator_hits = np.zeros(num_samples)
    growl_hits = np.zeros(num_samples)
    
    sig_names = []
    
    for step in range(total_steps):
        onset = step * step_samples
        if onset >= num_samples:
            break
            
        time_sec = onset / SAMPLE_RATE
        sig = signatures[current_sig_idx]
        
        # 4-Second Phase Trigger: mutate time signature based on overall agitation
        phase_idx = int(time_sec / 4.0)
        is_phase_trigger = False
        if step > 0:
            prev_time = (step - 1) * step_samples / SAMPLE_RATE
            if int(prev_time / 4.0) != phase_idx:
                is_phase_trigger = True
                
        if is_phase_trigger:
            current_sig_idx = (current_sig_idx + 1) % len(signatures)
            sig = signatures[current_sig_idx]
            consecutive_repeats = 0
            
            # Phase trigger startles Bird 1 & 3, causing pattern shifts
            ag1 = min(1.0, ag1 + 0.3)
            ag3 = min(1.0, ag3 + 0.25)
            
            # Scramble patterns dynamically
            kick_pat = [1 if (x or np.random.rand() < 0.28) else 0 for x in base_kick]
            snare_pat = [1 if (x or np.random.rand() < 0.20) else 0 for x in base_snare]
            hat_pat = [1 if (x or np.random.rand() < 0.35) else 0 for x in base_hat]
            
        # Calculate pattern repetition
        step_hash = (kick_pat[step % len(kick_pat)] << 2) | (snare_pat[step % len(snare_pat)] << 1) | hat_pat[step % len(hat_pat)]
        if step_hash == last_pattern_hash:
            consecutive_repeats += 1
        else:
            consecutive_repeats = max(0, consecutive_repeats - 1)
        last_pattern_hash = step_hash
        
        # Update Bird 1 (Bass) repetition index
        rep1_input = 1.0 if consecutive_repeats > 20 else 0.0
        rep1 = decay * rep1 + (1.0 - decay) * rep1_input
        
        # If Bird 1 (Bass) gets too repetitive, it agitates itself and resets patterns
        if rep1 > 0.65:
            current_sig_idx = (current_sig_idx + 1) % len(signatures)
            sig = signatures[current_sig_idx]
            consecutive_repeats = 0
            ag1 = min(1.0, ag1 + 0.35)
            pl1 = max(0.1, pl1 - 0.3)
            kick_pat = [1 if np.random.rand() < 0.3 else 0 for _ in base_kick]
            snare_pat = [1 if np.random.rand() < 0.25 else 0 for _ in base_snare]
            hat_pat = [1 if np.random.rand() < 0.4 else 0 for _ in base_hat]
            
        # Leak/decay individual states back to baseline
        ag1 = decay * ag1 + (1.0 - decay) * 0.08
        pl1 = decay * pl1 + (1.0 - decay) * 0.75
        
        ag2 = decay * ag2 + (1.0 - decay) * 0.05
        pl2 = decay * pl2 + (1.0 - decay) * 0.8
        
        ag3 = decay * ag3 + (1.0 - decay) * 0.12
        pl3 = decay * pl3 + (1.0 - decay) * 0.65
        
        # ---------------- Flock Cross-Talk Communication Loops ----------------
        # 1. When Bird 1 (Bass) triggers a Kick, it startles Bird 2 (Center):
        is_kick_trigger = (kick_pat[step % len(kick_pat)] == 1)
        if is_kick_trigger:
            ag2 = min(1.0, ag2 + 0.18)
            pl2 = max(0.1, pl2 - 0.08)
            
        # 2. When Bird 2 (Center) triggers a Fascinator sweep, it calms Bird 3 (Right):
        is_fasc_trigger = (pl2 > 0.58 and step % 4 == 0)
        if is_fasc_trigger:
            pl3 = min(1.0, pl3 + 0.14)
            ag3 = max(0.01, ag3 - 0.08)
            
        # 3. When Bird 3 (Right) triggers a Growl/Mimic, it startles Bird 1 (Left):
        is_growl_trigger = (ag3 > 0.35 and step % 8 == 0)
        if is_growl_trigger:
            ag1 = min(1.0, ag1 + 0.22)
            pl1 = max(0.05, pl1 - 0.12)
            
        # Also track Resonance Dwell triggers for Bird 2 & 3
        if consecutive_repeats < 4:
            rep2 = decay * rep2 + (1.0 - decay) * 0.8
            rep3 = decay * rep3 + (1.0 - decay) * 0.2
        else:
            rep2 = decay * rep2 + (1.0 - decay) * 0.1
            rep3 = decay * rep3 + (1.0 - decay) * 0.7
            
        # Store histories for video frames
        sig_len = min(num_samples - onset, step_samples)
        if sig_len > 0:
            ag1_arr[onset:onset+sig_len] = ag1
            pl1_arr[onset:onset+sig_len] = pl1
            rep1_arr[onset:onset+sig_len] = rep1
            
            ag2_arr[onset:onset+sig_len] = ag2
            pl2_arr[onset:onset+sig_len] = pl2
            rep2_arr[onset:onset+sig_len] = rep2
            
            ag3_arr[onset:onset+sig_len] = ag3
            pl3_arr[onset:onset+sig_len] = pl3
            rep3_arr[onset:onset+sig_len] = rep3
            
        if len(sig_names) <= step:
            sig_names.append(sig["name"])
            
        # Render step drums (Bird 1 triggers Kicks)
        if is_kick_trigger:
            l = min(num_samples - onset, kick_len)
            if l > 0:
                mix[onset:onset+l] += kick_sound[:l]
                kick_hits[onset:onset+l] = 1.0
                
        if snare_pat[step % len(snare_pat)] == 1:
            l = min(num_samples - onset, snare_len)
            if l > 0:
                mix[onset:onset+l] += snare_sound[:l] * (1.0 + 0.6 * ag1)
                
        if hat_pat[step % len(hat_pat)] == 1:
            l = min(num_samples - onset, hat_len)
            if l > 0:
                mix[onset:onset+l] += hat_sound[:l]
                
        # 1. Fascinators (High Pleasure trigger from Bird 2)
        if is_fasc_trigger:
            f_start = 2500.0 + 1500.0 * math.sin(step * 0.4)
            f_end = f_start + 2000.0 * (1.0 - np.random.rand() * 0.4)
            f_len = int(SAMPLE_RATE * 0.08)
            l = min(num_samples - onset, f_len)
            if l > 0:
                age_f = np.arange(l) / SAMPLE_RATE
                sweep = f_start + (f_end - f_start) * (age_f / 0.08)
                fascinator_sig = np.sin(2.0 * np.pi * sweep * age_f) * np.exp(-age_f / 0.018) * 0.04
                mix[onset:onset+l] += fascinator_sig
                fascinator_hits[onset:onset+l] = 1.0
                
        # 2. Mimics (High Agitation trigger from Bird 3)
        if is_growl_trigger and step > 4:
            prev_onset = onset - step_samples * 2
            m_l = min(num_samples - onset, step_samples)
            if m_l > 0 and prev_onset >= 0:
                mimic_sig = -mix[prev_onset:prev_onset+m_l] * 0.45
                quantized = np.round(mimic_sig * 6.0) / 6.0
                mix[onset:onset+m_l] += quantized
                
        # 3. Hybrid click trains (Dwell/Repetition trigger from Bird 2/3)
        if rep3 > 0.40 and step % 2 == 0:
            c_len = int(SAMPLE_RATE * 0.015)
            c_l = min(num_samples - onset, c_len)
            if c_l > 0:
                c_age = np.arange(c_l) / SAMPLE_RATE
                clicks = np.sin(2.0 * np.pi * 3800.0 * c_age) * np.exp(-c_age / 0.0035) * 0.065
                mix[onset:onset+c_l] += clicks
                
        # FM bells
        arp_rate = 6 if pl2 > 0.75 else 4
        if step % arp_rate == 0:
            notes = [130.81, 146.83, 164.81, 196.00, 220.00, 261.63, 293.66, 329.63]
            note_idx = (step // arp_rate) % len(notes)
            f_carrier = notes[note_idx] * (1.5 if rep2 > 0.5 else 1.0)
            f_mod = f_carrier * 2.01
            
            bell_len = int(SAMPLE_RATE * 0.25)
            b_onset = onset
            b_l = min(num_samples - b_onset, bell_len)
            if b_l > 0:
                b_age = np.arange(b_l) / SAMPLE_RATE
                mod_env = np.exp(-b_age / 0.05) * 4.0
                modulator = np.sin(2.0 * np.pi * f_mod * b_age) * mod_env
                bell = np.sin(2.0 * np.pi * f_carrier * b_age + modulator) * np.exp(-b_age / 0.12)
                mix[b_onset:b_onset+b_l] += bell * 0.045
                
        # Growl tests driven by Bird 3 Agitation
        if is_growl_trigger:
            growl_len = int(SAMPLE_RATE * 0.35)
            g_age = np.arange(min(num_samples - onset, growl_len)) / SAMPLE_RATE
            if len(g_age) > 0:
                f_growl = 60.0 + 120.0 * np.sin(2.0 * np.pi * 3.0 * g_age)
                growl_osc = np.sign(np.sin(2.0 * np.pi * f_growl * g_age))
                f_f1 = 280.0 + 300.0 * ag3
                f_f2 = 800.0 + 1200.0 * (1.0 - pl3)
                y_f1 = np.sin(2.0 * np.pi * f_f1 * g_age) * growl_osc
                y_f2 = np.sin(2.0 * np.pi * f_f2 * g_age) * growl_osc
                growl_sig = (y_f1 + y_f2) * np.exp(-g_age / 0.15) * 0.18
                growl_sig = np.tanh(growl_sig * (2.0 + 4.0 * ag3))
                mix[onset:onset+len(g_age)] += growl_sig * 0.22
                growl_hits[onset:onset+len(g_age)] = 1.0

    mix = np.clip(mix, -1.0, 1.0)
    
    audio_path = "temp_9m_performance.wav"
    print(f"[DSP] Writing WAV file to {audio_path}...")
    
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
        
    return (audio_path, mix,
            ag1_arr, pl1_arr, rep1_arr,
            ag2_arr, pl2_arr, rep2_arr,
            ag3_arr, pl3_arr, rep3_arr,
            sig_names, kick_hits, fascinator_hits, growl_hits)

def render_9m_video(audio_path, mix_audio,
                    ag1_arr, pl1_arr, rep1_arr,
                    ag2_arr, pl2_arr, rep2_arr,
                    ag3_arr, pl3_arr, rep3_arr,
                    sig_names, kick_hits, fascinator_hits, growl_hits, output_mp4):
    print("[VIDEO] Starting 9-minute frame rendering with cross-bird visual feedback and waveform grid...")
    width, height = 640, 360
    
    import subprocess
    cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-shortest",
        output_mp4
    ]
    
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    
    num_birds = 3
    bird_x = [width * 0.25, width * 0.5, width * 0.75]
    bird_y = [height * 0.45, height * 0.5, height * 0.55]
    bird_vy = [0.0, 0.0, 0.0]
    
    k_env = 0.0
    f_env = 0.0
    g_env = 0.0
    
    # Track particles for Echolocutions
    particles = []
    
    for f in range(TOTAL_FRAMES):
        sample_idx = int(f * (SAMPLE_RATE / FPS))
        if sample_idx >= len(ag1_arr):
            break
            
        # Retrieve individual bird state values
        ag1, pl1, rep1 = ag1_arr[sample_idx], pl1_arr[sample_idx], rep1_arr[sample_idx]
        ag2, pl2, rep2 = ag2_arr[sample_idx], pl2_arr[sample_idx], rep2_arr[sample_idx]
        ag3, pl3, rep3 = ag3_arr[sample_idx], pl3_arr[sample_idx], rep3_arr[sample_idx]
        
        # Smooth trigger envelopes
        k_env = k_env * 0.78 + kick_hits[sample_idx] * 0.22
        f_env = f_env * 0.78 + fascinator_hits[sample_idx] * 0.22
        g_env = g_env * 0.78 + growl_hits[sample_idx] * 0.22
        
        # 1. Spawn Echolocation particles on triggers
        # Kick trigger spawns sparks from Bird 1 (Bass) to Bird 2 (Center)
        if kick_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[0],
                    "y": bird_y[0],
                    "tx": bird_x[1],
                    "ty": bird_y[1],
                    "progress": 0.0,
                    "speed": 0.08 + np.random.rand() * 0.04,
                    "color": PEACH_COLOR
                })
        # Fascinator trigger spawns sparks from Bird 2 (Center) to Bird 3 (Right)
        if fascinator_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[1],
                    "y": bird_y[1],
                    "tx": bird_x[2],
                    "ty": bird_y[2],
                    "progress": 0.0,
                    "speed": 0.08 + np.random.rand() * 0.04,
                    "color": HUD_COLOR
                })
        # Growl trigger spawns sparks from Bird 3 (Right) back to Bird 1 (Left)
        if growl_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[2],
                    "y": bird_y[2],
                    "tx": bird_x[0],
                    "ty": bird_y[0],
                    "progress": 0.0,
                    "speed": 0.05 + np.random.rand() * 0.03,
                    "color": ACCENT_COLOR
                })
        
        step_idx = int(sample_idx / (SAMPLE_RATE * 60.0 / 124.0 / 4))
        sig_name = sig_names[min(step_idx, len(sig_names)-1)]
        
        img = Image.new("RGB", (width, height), BG_COLOR)
        draw = ImageDraw.Draw(img)
        
        # ---------------- 2. Live Oscilloscope Waveform Background ----------------
        # Render the actual current audio buffer as a vibrating line across the screen background
        w_size = 320
        start_w = max(0, sample_idx - w_size // 2)
        end_w = min(len(mix_audio), sample_idx + w_size // 2)
        wave_slice = mix_audio[start_w:end_w]
        
        if len(wave_slice) > 1:
            points = []
            dx = width / len(wave_slice)
            for j, val in enumerate(wave_slice):
                wx = j * dx
                wy = height * 0.5 + val * 120.0 * (1.0 + 0.5 * ag1) # ag1 adds warp height
                points.append((wx, wy))
            draw.line(points, fill=(10, int(35 + 50 * pl2), int(70 + 100 * pl2)), width=2)
            
        # Render static background grid with lower opacity
        grid_spacing = int(50 + 15 * pl2)
        for x in range(0, width, grid_spacing):
            draw.line([(x, 0), (x, height)], fill=(5, 15, 30))
        for y in range(0, height, grid_spacing):
            draw.line([(0, y), (width, y)], fill=(5, 15, 30))
            
        t_sec = f / FPS
        
        # ---------------- 3. Draw Echolocation particles ----------------
        next_particles = []
        for p in particles:
            p["progress"] += p["speed"]
            if p["progress"] < 1.0:
                # Interpolate coordinate
                px = p["x"] + (p["tx"] - p["x"]) * p["progress"]
                py = p["y"] + (p["ty"] - p["y"]) * p["progress"]
                # Add sinusoidal arc
                arc = math.sin(p["progress"] * math.pi) * 30.0
                draw.ellipse([px - 2, py - 2 - arc, px + 2, py + 2 - arc], fill=p["color"])
                next_particles.append(p)
        particles = next_particles
        
        # ---------------- 4. Draw Birds (Reacting Individually) ----------------
        for i in range(num_birds):
            bx = bird_x[i]
            by = bird_y[i]
            
            if i == 0:
                # BIRD 1: Bass / Kick Bird (Left)
                wing_freq = 3.0 + 8.0 * rep1
                wing_amp = 16.0 + 35.0 * k_env
                
                # Verlet hover bobbing
                bird_vy[0] = 0.85 * bird_vy[0] - 12.0 * kick_hits[sample_idx] * 0.15 + (math.sin(t_sec * 6.0) * 0.5)
                bird_y[0] = max(50, min(height - 50, bird_y[0] + bird_vy[0]))
                by = bird_y[0]
                
                # Squash and stretch
                body_h = 6 + 12 * k_env
                body_w = 12 - 4 * k_env
                
                wing_offset = math.sin(2.0 * math.pi * wing_freq * t_sec) * wing_amp
                b_color = (255, int(150 + 100 * k_env), 100)
                w_color = (int(100 + 155 * k_env), 80, 200)
                
                draw.ellipse([bx - body_w, by - body_h, bx + body_w, by + body_h], fill=b_color)
                draw.line([(bx, by), (bx - 26, by - 8 + wing_offset)], fill=w_color, width=4)
                draw.line([(bx, by), (bx + 26, by - 8 + wing_offset)], fill=w_color, width=4)
                draw.polygon([(bx - 8, by), (bx - 18, by - 4), (bx - 18, by + 4)], fill=WARNING_COLOR)
                
            elif i == 1:
                # BIRD 2: Fascinator Bird (Center)
                wing_freq = 5.0 + 20.0 * pl2
                wing_amp = 18.0 + 10.0 * f_env
                
                bird_vy[1] = 0.9 * bird_vy[1] + 0.1 * math.sin(t_sec * 12.0) * (2.0 + 5.0 * pl2)
                bird_y[1] = max(50, min(height - 50, bird_y[1] + bird_vy[1]))
                by = bird_y[1]
                
                wing_offset = math.sin(2.0 * math.pi * wing_freq * t_sec) * wing_amp
                
                # Fascinator concentric scan rings
                if f_env > 0.05:
                    r_rad = int(35 * f_env)
                    draw.line([(bx, by), (bird_x[2], bird_y[2])], fill=(0, 242, 254, 80), width=2)
                    draw.ellipse([bx - r_rad, by - r_rad, bx + r_rad, by + r_rad], outline=(0, 242, 254, 100), width=2)
                
                b_color = (int(100 - 50 * f_env), 242, 254)
                w_color = WARNING_COLOR
                
                draw.ellipse([bx - 10, by - 8, bx + 10, by + 8], fill=b_color)
                draw.line([(bx, by), (bx - 28, by - 12 + wing_offset)], fill=w_color, width=3)
                draw.line([(bx, by), (bx + 28, by - 12 + wing_offset)], fill=w_color, width=3)
                draw.polygon([(bx - 10, by), (bx - 18, by - 5), (bx - 18, by + 5)], fill=ACCENT_COLOR)
                
            else:
                # BIRD 3: Agitation Bird (Right)
                wing_freq = 4.0 + 22.0 * ag3
                wing_amp = 20.0 + 15.0 * g_env
                
                # Jitter displacement
                jx = (np.random.rand() - 0.5) * 18.0 * g_env
                jy = (np.random.rand() - 0.5) * 18.0 * g_env
                bx += jx
                by += jy
                
                wing_offset = math.sin(2.0 * math.pi * wing_freq * t_sec) * wing_amp
                
                if g_env > 0.05:
                    draw.line([(bx, by), (bird_x[0], bird_y[0])], fill=(255, 0, 127, 80), width=2)
                
                b_color = (255, 0, int(127 + 128 * g_env))
                w_color = (int(150 + 105 * g_env), 0, 50)
                
                draw.ellipse([bx - 11, by - 6, bx + 11, by + 6], fill=b_color)
                draw.line([(bx, by), (bx - 25, by - 10 + wing_offset)], fill=w_color, width=3)
                draw.line([(bx, by), (bx + 25, by - 10 + wing_offset)], fill=w_color, width=3)
                draw.polygon([(bx - 11, by), (bx - 19, by - 4), (bx - 19, by + 4)], fill=WARNING_COLOR)
                
            draw.ellipse([bx + 4, by - 3, bx + 7, by], fill=(255, 255, 255))
            
        # HUD overlays
        draw.text((20, 20), "TSFI/2: AUNCIENT FLOCK AGENT COMMUNICATION NETWORK", fill=HUD_COLOR)
        draw.text((20, 36), f"ACTIVE SIGNATURE: {sig_name.upper()}", fill=WARNING_COLOR)
        
        # Display 3 columns of accumulators
        # Bird 1 Column
        draw.text((20, 240), "BIRD 1 (LEFT/BASS)", fill=PEACH_COLOR)
        draw.text((20, 255), f"AG1: {int(ag1*100)}% PL1: {int(pl1*100)}%", fill=(180, 180, 180))
        draw.rectangle([20, 270, 180, 276], outline=PEACH_COLOR)
        draw.rectangle([20, 270, int(20 + 160 * ag1), 276], fill=ACCENT_COLOR)
        
        # Bird 2 Column
        draw.text((220, 240), "BIRD 2 (CENTER/FASC)", fill=HUD_COLOR)
        draw.text((220, 255), f"AG2: {int(ag2*100)}% PL2: {int(pl2*100)}%", fill=(180, 180, 180))
        draw.rectangle([220, 270, 380, 276], outline=HUD_COLOR)
        draw.rectangle([220, 270, int(220 + 160 * pl2), 276], fill=HUD_COLOR)
        
        # Bird 3 Column
        draw.text((420, 240), "BIRD 3 (RIGHT/GROWL)", fill=ACCENT_COLOR)
        draw.text((420, 255), f"AG3: {int(ag3*100)}% PL3: {int(pl3*100)}%", fill=(180, 180, 180))
        draw.rectangle([420, 270, 580, 276], outline=ACCENT_COLOR)
        draw.rectangle([420, 270, int(420 + 160 * ag3), 276], fill=ACCENT_COLOR)
        
        # HUD triggers
        if g_env > 0.05:
            draw.text((420, 290), "⚡ GROWL TRIGGER B1", fill=ACCENT_COLOR)
        if f_env > 0.05:
            draw.text((220, 290), "⭐ FASCINATE TRIGGER B3", fill=HUD_COLOR)
        if kick_hits[sample_idx] > 0.5:
            draw.text((20, 290), "⚡ KICK TRIGGER B2", fill=PEACH_COLOR)
            
        draw.text((540, 20), f"TIME: {int(t_sec // 60):02d}:{int(t_sec % 60):02d}", fill=HUD_COLOR)
        
        img.save(process.stdin, "PNG")
        
        if f % (FPS * 60) == 0:
            progress = f / TOTAL_FRAMES
            print(f"  -> Rendering 9m Performer: {int(t_sec//60)}m ({progress*100:.1f}%)...")
            
    process.stdin.close()
    process.wait()
    print("[SUCCESS] 9-minute abstract beat performance successfully rendered!")

if __name__ == "__main__":
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "abstract_9m_performance.mp4")
    
    (audio_path, mix_audio,
     ag1_arr, pl1_arr, rep1_arr,
     ag2_arr, pl2_arr, rep2_arr,
     ag3_arr, pl3_arr, rep3_arr,
     sig_names, kick_hits, fascinator_hits, growl_hits) = generate_9m_performance()
     
    render_9m_video(audio_path, mix_audio,
                    ag1_arr, pl1_arr, rep1_arr,
                    ag2_arr, pl2_arr, rep2_arr,
                    ag3_arr, pl3_arr, rep3_arr,
                    sig_names, kick_hits, fascinator_hits, growl_hits, output_mp4)
    
    if os.path.exists(audio_path):
        os.remove(audio_path)
