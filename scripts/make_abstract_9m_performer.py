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
        
        # Macro structural break and drop timing (each measure is 16 steps)
        measure = step // 16
        is_break = (measure % 16 == 15)  # 15th measure of a 16-measure block is a break (comfort solo)
        is_drop = (measure % 16 == 0 and step % 16 == 0 and step > 0)
        
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
        # 1. When Bird 1 (Bass) triggers a Kick, it comforts Bird 2 (Center):
        is_kick_trigger = (kick_pat[step % len(kick_pat)] == 1) and not is_break
        if is_kick_trigger:
            pl2 = min(1.0, pl2 + 0.16)
            ag2 = max(0.01, ag2 - 0.12)
            
        # 2. When Bird 2 (Center) triggers a Fascinator sweep, it delights Bird 3 (Right):
        is_fasc_trigger = (pl2 > 0.58 and step % 4 == 0) and not is_break
        if is_fasc_trigger:
            pl3 = min(1.0, pl3 + 0.18)
            ag3 = max(0.01, ag3 - 0.14)
            
        # 3. When Bird 3 (Right) is pleased, it triggers a Resonance Chirp to comfort Bird 1 (Left):
        is_growl_trigger = (pl3 > 0.60 and step % 8 == 0) and not is_break
        if is_growl_trigger:
            pl1 = min(1.0, pl1 + 0.15)
            ag1 = max(0.01, ag1 - 0.10)
            
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
            
        # Render step drums modulated by macro breaks (Bird 1 triggers Kicks)
        if is_kick_trigger:
            l = min(num_samples - onset, kick_len)
            if l > 0:
                mix[onset:onset+l] += kick_sound[:l]
                kick_hits[onset:onset+l] = 1.0
                
        if snare_pat[step % len(snare_pat)] == 1 and not is_break:
            l = min(num_samples - onset, snare_len)
            if l > 0:
                mix[onset:onset+l] += snare_sound[:l] * (1.0 + 0.6 * ag1)
                
        if hat_pat[step % len(hat_pat)] == 1 and not is_break:
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
        if rep3 > 0.40 and step % 2 == 0 and not is_break:
            c_len = int(SAMPLE_RATE * 0.015)
            c_l = min(num_samples - onset, c_len)
            if c_l > 0:
                c_age = np.arange(c_l) / SAMPLE_RATE
                clicks = np.sin(2.0 * np.pi * 3800.0 * c_age) * np.exp(-c_age / 0.0035) * 0.065
                mix[onset:onset+c_l] += clicks
                
        # FM bells
        arp_rate = 6 if pl2 > 0.75 else 4
        if step % arp_rate == 0 and not is_break:
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
                
        # Resonance Chirps driven by Bird 3 Pleasure
        if is_growl_trigger:
            chirp_len = int(SAMPLE_RATE * 0.28)
            c_age = np.arange(min(num_samples - onset, chirp_len)) / SAMPLE_RATE
            if len(c_age) > 0:
                f_start = 800.0 + 400.0 * pl3
                f_end = 2200.0 + 800.0 * pl3
                sweep_freq = f_start + (f_end - f_start) * (c_age / 0.28)
                chirp_sig = np.sin(2.0 * np.pi * sweep_freq * c_age) * np.exp(-c_age / 0.08) * 0.12
                mix[onset:onset+len(c_age)] += chirp_sig * 0.45
                growl_hits[onset:onset+len(c_age)] = 1.0
                
        # 4. Warm, loving comfort trill (vibrato whistle solo) during macro breaks
        if is_break and step % 4 == 0:
            trill_len = int(SAMPLE_RATE * 0.45)
            t_age = np.arange(min(num_samples - onset, trill_len)) / SAMPLE_RATE
            if len(t_age) > 0:
                vib = 15.0 * np.sin(2.0 * np.pi * 6.0 * t_age)
                f_base = 900.0 + 300.0 * math.sin(step * 0.5)
                trill_sig = np.sin(2.0 * np.pi * (f_base + vib) * t_age) * np.exp(-t_age / 0.15) * 0.08
                trill_sig *= np.minimum(1.0, t_age / 0.03)  # smooth fade-in
                mix[onset:onset+len(t_age)] += trill_sig
                
        # 5. Euphoric harmonized chord chime on the beat drop
        if is_drop:
            chord = [261.63, 329.63, 392.00, 523.25]  # C Major triad + octave confort chord
            for f_root in chord:
                chime_len = int(SAMPLE_RATE * 0.60)
                chime_onset = onset
                c_l = min(num_samples - chime_onset, chime_len)
                if c_l > 0:
                    c_age = np.arange(c_l) / SAMPLE_RATE
                    chime = np.sin(2.0 * np.pi * f_root * c_age) * np.exp(-c_age / 0.18) * 0.035
                    mix[chime_onset:chime_onset+c_l] += chime

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
    print("[VIDEO] Starting 9-minute frame rendering with Verlet physical environmental simulator...")
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
    
    # ---------------- 1. Physical Environment States ----------------
    # Position, Velocity, and Tilt angles of the bird agents
    bird_x = [width * 0.25, width * 0.5, width * 0.75]
    bird_y = [height * 0.5, height * 0.5, height * 0.5]
    bird_vx = [0.0, 0.0, 0.0]
    bird_vy = [0.0, 0.0, 0.0]
    bird_angle = [0.0, 0.0, 0.0]
    
    # Bird impact registers tracking when particles hit them directly
    bird_impact = [0.0, 0.0, 0.0]
    
    # Bird DNA configuration specifying physical characteristics and behaviors
    bird_dna = [
        # Bird 0: Bass Bird (heavy, sluggish, large wings)
        {
            "mass": 1.5,
            "drag": 0.93,
            "lift_coeff": 0.0055,
            "base_wing_len": 26,
            "wing_color": (120, 80, 200),
            "body_color": (255, 150, 100),
            "ripple_color": ACCENT_COLOR,
            "excitability": 1.2,
            "spring_k": 0.03
        },
        # Bird 1: Fascinator Bird (lightweight, highly responsive/agile, small wings)
        {
            "mass": 0.7,
            "drag": 0.86,
            "lift_coeff": 0.0034,
            "base_wing_len": 28,
            "wing_color": WARNING_COLOR,
            "body_color": (100, 242, 254),
            "ripple_color": PEACH_COLOR,
            "excitability": 0.8,
            "spring_k": 0.05
        },
        # Bird 2: Agitation Bird (standard mass, aggressive, medium wings)
        {
            "mass": 1.0,
            "drag": 0.90,
            "lift_coeff": 0.0044,
            "base_wing_len": 25,
            "wing_color": (150, 0, 50),
            "body_color": (255, 0, 127),
            "ripple_color": HUD_COLOR,
            "excitability": 1.0,
            "spring_k": 0.04
        }
    ]
    
    k_env = 0.0
    f_env = 0.0
    g_env = 0.0
    
    particles = []
    
    for f in range(TOTAL_FRAMES):
        sample_idx = int(f * (SAMPLE_RATE / FPS))
        if sample_idx >= len(ag1_arr):
            break
            
        ag1, pl1, rep1 = ag1_arr[sample_idx], pl1_arr[sample_idx], rep1_arr[sample_idx]
        ag2, pl2, rep2 = ag2_arr[sample_idx], pl2_arr[sample_idx], rep2_arr[sample_idx]
        ag3, pl3, rep3 = ag3_arr[sample_idx], pl3_arr[sample_idx], rep3_arr[sample_idx]
        
        k_env = k_env * 0.78 + kick_hits[sample_idx] * 0.22
        f_env = f_env * 0.78 + fascinator_hits[sample_idx] * 0.22
        g_env = g_env * 0.78 + growl_hits[sample_idx] * 0.22
        
        t_sec = f / FPS
        
        # ---------------- 2. Echolocation Sound Sparks ----------------
        if kick_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[0], "y": bird_y[0],
                    "target_bird": 1,
                    "progress": 0.0, "speed": 0.08 + np.random.rand() * 0.04,
                    "color": PEACH_COLOR
                })
        if fascinator_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[1], "y": bird_y[1],
                    "target_bird": 2,
                    "progress": 0.0, "speed": 0.08 + np.random.rand() * 0.04,
                    "color": HUD_COLOR
                })
        if growl_hits[sample_idx] > 0.5 and np.random.rand() < 0.3:
            for _ in range(3):
                particles.append({
                    "x": bird_x[2], "y": bird_y[2],
                    "target_bird": 0,
                    "progress": 0.0, "speed": 0.05 + np.random.rand() * 0.03,
                    "color": ACCENT_COLOR
                })
        
        step_idx = int(sample_idx / (SAMPLE_RATE * 60.0 / 124.0 / 4))
        sig_name = sig_names[min(step_idx, len(sig_names)-1)]
        
        img = Image.new("RGB", (width, height), BG_COLOR)
        draw = ImageDraw.Draw(img)
        
        # ---------------- 3. Oscilloscope Wave Background ----------------
        w_size = 320
        start_w = max(0, sample_idx - w_size // 2)
        end_w = min(len(mix_audio), sample_idx + w_size // 2)
        wave_slice = mix_audio[start_w:end_w]
        
        if len(wave_slice) > 1:
            points = []
            dx = width / len(wave_slice)
            for j, val in enumerate(wave_slice):
                wx = j * dx
                wy = height * 0.5 + val * 120.0 * (1.0 + 0.5 * ag1)
                points.append((wx, wy))
            draw.line(points, fill=(10, int(35 + 50 * pl2), int(70 + 100 * pl2)), width=2)
            
        # Draw physical boundary walls (Ceiling and Floor lines)
        # React and glow when triggers occur (echolocation feedback)
        boundary_glow = int(40 + 200 * max(k_env, max(f_env, g_env)))
        draw.line([(0, 45), (width, 45)], fill=(boundary_glow, 30, 80), width=3)
        draw.line([(0, 315), (width, 315)], fill=(boundary_glow, 30, 80), width=3)
        
        # Draw static background grid
        grid_spacing = int(50 + 15 * pl2)
        for x in range(0, width, grid_spacing):
            draw.line([(x, 45), (x, 315)], fill=(5, 15, 30))
        for y in range(45, 315, grid_spacing):
            draw.line([(0, y), (width, y)], fill=(5, 15, 30))
            
        # ---------------- 4. Update Particle Physics & Echolocation Impacts ----------------
        # Decay impact registers
        for i in range(num_birds):
            bird_impact[i] *= 0.85
            
        next_particles = []
        for p in particles:
            p["progress"] += p["speed"]
            tb = p["target_bird"]
            # Dynamically target bird's current coordinates
            p_tx, p_ty = bird_x[tb], bird_y[tb]
            if p["progress"] < 1.0:
                px = p["x"] + (p_tx - p["x"]) * p["progress"]
                py = p["y"] + (p_ty - p["y"]) * p["progress"]
                arc = math.sin(p["progress"] * math.pi) * 30.0
                draw.ellipse([px - 2, py - 2 - arc, px + 2, py + 2 - arc], fill=p["color"])
                next_particles.append(p)
            else:
                # Particle arrived: trigger impact on target bird
                bird_impact[tb] = min(1.0, bird_impact[tb] + 0.45)
        particles = next_particles
        
        # ---------------- 5. Verlet Gravity, Lift & Wind Environment Simulator ----------------
        # Environmental Wind gust model
        wind_x = math.sin(t_sec * 2.8) * 0.22 + (np.random.rand() - 0.5) * 0.3
        
        for i in range(num_birds):
            dna = bird_dna[i]
            # Scale wing flap speed & body shape size on direct echolocation impact
            impact_scale = 1.0 + 0.4 * bird_impact[i]
            
            if i == 0:
                wing_freq = 3.0 + 8.0 * rep1 + 10.0 * bird_impact[0] * dna["excitability"]
                wing_amp = 16.0 + 35.0 * k_env + 15.0 * bird_impact[0]
            elif i == 1:
                wing_freq = 5.0 + 20.0 * pl2 + 8.0 * bird_impact[1] * dna["excitability"]
                wing_amp = 18.0 + 10.0 * f_env + 12.0 * bird_impact[1]
            else:
                wing_freq = 4.0 + 22.0 * ag3 + 12.0 * bird_impact[2] * dna["excitability"]
                wing_amp = 20.0 + 15.0 * g_env + 16.0 * bird_impact[2]
            
            # Periodic wing flapping phase & kinematics
            wing_phase = 2.0 * math.pi * wing_freq * t_sec
            wing_offset = math.sin(wing_phase) * wing_amp
            # Wing velocity (rate of wing position change)
            wing_velocity = math.cos(wing_phase) * wing_freq * wing_amp
            
            # Physics Model: Gravity pulls DOWN
            gravity_force = 0.38
            
            # Downstroke generates lift and thrust; upstroke has drag and minimal lift
            if wing_velocity < 0:
                # Downstroke: wings move downwards producing upward lift and forward thrust
                lift_force = -wing_velocity * dna["lift_coeff"] * (1.2 + (k_env if i==0 else (f_env if i==1 else g_env)))
                thrust_force = -wing_velocity * 0.003 * (1.2 + (k_env if i==0 else (f_env if i==1 else g_env)))
            else:
                # Upstroke: wings fold/return upwards producing minor lift and slight drag
                lift_force = wing_velocity * 0.0008
                thrust_force = -wing_velocity * 0.0005
            
            # Apply forces with species mass DNA (Acceleration = Force / Mass)
            bird_vy[i] += (gravity_force - lift_force) / dna["mass"]
            
            # Birds attempt to remain within their designated sector centers (steering force spring)
            target_x = width * 0.165 + i * 160
            steering_force = (target_x - bird_x[i]) * dna["spring_k"]
            
            bird_vx[i] += (thrust_force + wind_x * 0.15 + steering_force) / dna["mass"]
            
            # Drag resistance damping from species DNA
            bird_vy[i] *= dna["drag"]
            bird_vx[i] *= dna["drag"]
            
            # Update coordinate positions
            bird_x[i] += bird_vx[i]
            bird_y[i] += bird_vy[i]
            
            # Boundary collisions (rebound on ceiling/floor)
            if bird_y[i] < 65:
                bird_y[i] = 65
                bird_vy[i] = -bird_vy[i] * 0.4
            elif bird_y[i] > 295:
                bird_y[i] = 295
                bird_vy[i] = -bird_vy[i] * 0.4
                
            # Horizontal boundary limits
            min_x = width * 0.08 + i * 160
            max_x = width * 0.25 + i * 160
            bird_x[i] = max(min_x, min(max_x, bird_x[i]))
            
            # Pitch Angle Leaning (lean into movement, with physical pitch body vibration from wing beats)
            target_angle = bird_vx[i] * 6.5 + math.cos(wing_phase) * 4.0
            bird_angle[i] = bird_angle[i] * 0.82 + target_angle * 0.18
            
            # Render bird with rotation angle (bird_angle[i]) using trig projection
            bx = bird_x[i]
            by = bird_y[i]
            
            cos_a = math.cos(math.radians(bird_angle[i]))
            sin_a = math.sin(math.radians(bird_angle[i]))
            
            wing_offset = math.sin(2.0 * math.pi * wing_freq * t_sec) * wing_amp
            
            # Helper to rotate local offsets
            def get_rot_point(dx, dy):
                rx = bx + dx * cos_a - dy * sin_a
                ry = by + dx * sin_a + dy * cos_a
                return (rx, ry)
                
            # Sonar ripple reflecting sound on impact
            if bird_impact[i] > 0.05:
                rip_col = dna["ripple_color"]
                ripple_r = int(12 + 65 * (1.0 - bird_impact[i]))
                dim_color = (
                    int(rip_col[0] * bird_impact[i]),
                    int(rip_col[1] * bird_impact[i]),
                    int(rip_col[2] * bird_impact[i])
                )
                draw.ellipse([bx - ripple_r, by - ripple_r, bx + ripple_r, by + ripple_r], outline=dim_color, width=2)
                
            # Individual shapes
            if i == 0:
                body_h = (6 + 12 * k_env) * impact_scale
                body_w = (12 - 4 * k_env) * impact_scale
                b_color = (dna["body_color"][0], int(dna["body_color"][1] + 100 * k_env), dna["body_color"][2])
                w_color = (int(100 + 155 * k_env), dna["wing_color"][1], dna["wing_color"][2])
                
                # Draw rotated body
                draw.polygon([get_rot_point(-body_w, -body_h), get_rot_point(body_w, -body_h),
                              get_rot_point(body_w, body_h), get_rot_point(-body_w, body_h)], fill=b_color)
                # Draw wings
                draw.line([get_rot_point(0, 0), get_rot_point(int(-dna["base_wing_len"] * impact_scale), -8 + wing_offset)], fill=w_color, width=4)
                draw.line([get_rot_point(0, 0), get_rot_point(int(dna["base_wing_len"] * impact_scale), -8 + wing_offset)], fill=w_color, width=4)
                # Tail
                draw.polygon([get_rot_point(int(-8 * impact_scale), 0), get_rot_point(int(-18 * impact_scale), -4), get_rot_point(int(-18 * impact_scale), 4)], fill=WARNING_COLOR)
                
            elif i == 1:
                b_color = (int(dna["body_color"][0] - 50 * f_env), dna["body_color"][1], dna["body_color"][2])
                w_color = dna["wing_color"]
                
                # concentric scan lines pointing to Bird 3
                if f_env > 0.05:
                    draw.line([get_rot_point(0, 0), (bird_x[2], bird_y[2])], fill=(0, 242, 254, 80), width=2)
                    r_rad = int(35 * f_env)
                    draw.ellipse([bx - r_rad, by - r_rad, bx + r_rad, by + r_rad], outline=(0, 242, 254, 100), width=2)
                    
                draw.ellipse([bx - int(10 * impact_scale), by - int(8 * impact_scale), bx + int(10 * impact_scale), by + int(8 * impact_scale)], fill=b_color)
                draw.line([get_rot_point(0, 0), get_rot_point(int(-dna["base_wing_len"] * impact_scale), -12 + wing_offset)], fill=w_color, width=3)
                draw.line([get_rot_point(0, 0), get_rot_point(int(dna["base_wing_len"] * impact_scale), -12 + wing_offset)], fill=w_color, width=3)
                draw.polygon([get_rot_point(int(-10 * impact_scale), 0), get_rot_point(int(-18 * impact_scale), -5), get_rot_point(int(-18 * impact_scale), 5)], fill=ACCENT_COLOR)
                
            else:
                b_color = (dna["body_color"][0], dna["body_color"][1], int(dna["body_color"][2] + 128 * g_env))
                w_color = (int(150 + 105 * g_env), dna["wing_color"][1], dna["wing_color"][2])
                
                if g_env > 0.05:
                    draw.line([get_rot_point(0, 0), (bird_x[0], bird_y[0])], fill=(255, 0, 127, 80), width=2)
                    
                draw.polygon([get_rot_point(int(-11 * impact_scale), -6), get_rot_point(int(11 * impact_scale), -6), get_rot_point(int(11 * impact_scale), 6), get_rot_point(int(-11 * impact_scale), 6)], fill=b_color)
                draw.line([get_rot_point(0, 0), get_rot_point(int(-dna["base_wing_len"] * impact_scale), -10 + wing_offset)], fill=w_color, width=3)
                draw.line([get_rot_point(0, 0), get_rot_point(int(dna["base_wing_len"] * impact_scale), -10 + wing_offset)], fill=w_color, width=3)
                draw.polygon([get_rot_point(int(-11 * impact_scale), 0), get_rot_point(int(-19 * impact_scale), -4), get_rot_point(int(-19 * impact_scale), 4)], fill=WARNING_COLOR)
                
            # Rotated Eye (Dilation flash effect on impact)
            ep = get_rot_point(5, -2)
            eye_rad = 1.5 + 2.0 * bird_impact[i]
            draw.ellipse([ep[0]-eye_rad, ep[1]-eye_rad, ep[0]+eye_rad, ep[1]+eye_rad], fill=(255, 255, 255))
            
        # HUD overlays
        draw.text((20, 20), "TSFI/2: AUNCIENT FLOCK AGENT COMMUNICATION NETWORK", fill=HUD_COLOR)
        draw.text((20, 36), f"ACTIVE SIGNATURE: {sig_name.upper()}", fill=WARNING_COLOR)
        
        # Display 3 columns of accumulators
        draw.text((20, 240), "BIRD 1 (LEFT/BASS)", fill=PEACH_COLOR)
        draw.text((20, 255), f"AG1: {int(ag1*100)}% PL1: {int(pl1*100)}%", fill=(180, 180, 180))
        draw.rectangle([20, 270, 180, 276], outline=PEACH_COLOR)
        draw.rectangle([20, 270, int(20 + 160 * ag1), 276], fill=ACCENT_COLOR)
        
        draw.text((220, 240), "BIRD 2 (CENTER/FASC)", fill=HUD_COLOR)
        draw.text((220, 255), f"AG2: {int(ag2*100)}% PL2: {int(pl2*100)}%", fill=(180, 180, 180))
        draw.rectangle([220, 270, 380, 276], outline=HUD_COLOR)
        draw.rectangle([220, 270, int(220 + 160 * pl2), 276], fill=HUD_COLOR)
        
        draw.text((420, 240), "BIRD 3 (RIGHT/RESONANCE)", fill=ACCENT_COLOR)
        draw.text((420, 255), f"AG3: {int(ag3*100)}% PL3: {int(pl3*100)}%", fill=(180, 180, 180))
        draw.rectangle([420, 270, 580, 276], outline=ACCENT_COLOR)
        draw.rectangle([420, 270, int(420 + 160 * ag3), 276], fill=ACCENT_COLOR)
        
        # HUD triggers
        if g_env > 0.05:
            draw.text((420, 290), "🌸 RESONANCE CHIRP B1", fill=ACCENT_COLOR)
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
