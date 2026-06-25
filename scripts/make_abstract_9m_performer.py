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
    print("[DSP] Starting 9-minute abstract technical performance synthesis...")
    num_samples = int(DURATION_SEC * SAMPLE_RATE)
    mix = np.zeros(num_samples)
    
    # Track states
    vaesen_ag = 0.1
    vaesen_pl = 0.9
    vaesen_rep = 0.0
    decay = 0.995
    
    # Beat parameters
    bpm = 124.0
    beat_samples = int((60.0 / bpm) * SAMPLE_RATE)
    
    # We will slice time into steps of 16th notes
    step_samples = int(beat_samples / 4)
    total_steps = int(num_samples / step_samples)
    
    # Base pattern
    base_kick = [1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    base_snare = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]
    base_hat = [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1]
    
    # Pre-render a kick drum sound
    kick_len = int(SAMPLE_RATE * 0.18)
    k_age = np.arange(kick_len) / SAMPLE_RATE
    k_freq = 50.0 + 120.0 * np.exp(-k_age / 0.03)
    kick_sound = np.sin(2.0 * np.pi * k_freq * k_age) * np.exp(-k_age / 0.12)
    kick_sound = np.tanh(kick_sound * 1.5) * 0.4
    
    # Pre-render a snare drum sound
    snare_len = int(SAMPLE_RATE * 0.15)
    s_age = np.arange(snare_len) / SAMPLE_RATE
    snare_noise = np.random.uniform(-1.0, 1.0, snare_len)
    snare_sound = snare_noise * np.exp(-s_age / 0.045) * 0.25
    
    # Pre-render a hat sound
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
    
    print("[DSP] Generating abstract beats and dynamic loops...")
    
    # Keep track of active patterns
    kick_pat = base_kick.copy()
    snare_pat = base_snare.copy()
    hat_pat = base_hat.copy()
    
    ag_state = np.zeros(num_samples)
    pl_state = np.zeros(num_samples)
    rep_state = np.zeros(num_samples)
    sig_names = []
    
    for step in range(total_steps):
        onset = step * step_samples
        if onset >= num_samples:
            break
            
        time_sec = onset / SAMPLE_RATE
        sig = signatures[current_sig_idx]
        
        # 4-Second Phase Trigger: force extreme interest, shifts, and mutations
        phase_idx = int(time_sec / 4.0)
        is_phase_trigger = False
        if step > 0:
            prev_time = (step - 1) * step_samples / SAMPLE_RATE
            if int(prev_time / 4.0) != phase_idx:
                is_phase_trigger = True
                
        if is_phase_trigger:
            # Shift time signature phase
            current_sig_idx = (current_sig_idx + 1) % len(signatures)
            sig = signatures[current_sig_idx]
            consecutive_repeats = 0
            
            # Spike agitation to trigger mimics, lower pleasure
            vaesen_ag = min(1.0, vaesen_ag + 0.38)
            vaesen_pl = max(0.1, vaesen_pl - 0.3)
            
            # Scramble patterns radically to introduce fresh syncopations
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
        
        # Repetition accumulator updates
        rep_input = 1.0 if consecutive_repeats > 20 else 0.0
        vaesen_rep = decay * vaesen_rep + (1.0 - decay) * rep_input
        
        # If repetition stays too high, trigger pattern resets and agitation spikes
        if vaesen_rep > 0.65:
            current_sig_idx = (current_sig_idx + 1) % len(signatures)
            sig = signatures[current_sig_idx]
            consecutive_repeats = 0
            vaesen_ag = min(1.0, vaesen_ag + 0.4)
            vaesen_pl = max(0.05, vaesen_pl - 0.35)
            
            # Regenerate random beat maps
            kick_pat = [1 if np.random.rand() < 0.3 else 0 for _ in base_kick]
            snare_pat = [1 if np.random.rand() < 0.25 else 0 for _ in base_snare]
            hat_pat = [1 if np.random.rand() < 0.4 else 0 for _ in base_hat]
        else:
            # Slow leak back to resting emotional state
            vaesen_ag = decay * vaesen_ag + (1.0 - decay) * 0.08
            vaesen_pl = decay * vaesen_pl + (1.0 - decay) * 0.75
            
        sig_len = min(num_samples - onset, step_samples)
        if sig_len > 0:
            ag_state[onset:onset+sig_len] = vaesen_ag
            pl_state[onset:onset+sig_len] = vaesen_pl
            rep_state[onset:onset+sig_len] = vaesen_rep
            
        if len(sig_names) <= step:
            sig_names.append(sig["name"])
            
        # Render step drums
        if kick_pat[step % len(kick_pat)] == 1:
            l = min(num_samples - onset, kick_len)
            if l > 0:
                mix[onset:onset+l] += kick_sound[:l]
                
        if snare_pat[step % len(snare_pat)] == 1:
            l = min(num_samples - onset, snare_len)
            if l > 0:
                mix[onset:onset+l] += snare_sound[:l] * (1.0 + 0.6 * vaesen_ag)
                
        if hat_pat[step % len(hat_pat)] == 1:
            l = min(num_samples - onset, hat_len)
            if l > 0:
                mix[onset:onset+l] += hat_sound[:l]
                
        # 1. Fascinators (High Pleasure trigger)
        # Fast, microtonal whistling trills/sweeps designed to captivate avian focus
        if vaesen_pl > 0.58 and step % 4 == 0:
            f_start = 2500.0 + 1500.0 * math.sin(step * 0.4)
            f_end = f_start + 2000.0 * (1.0 - np.random.rand() * 0.4)
            f_len = int(SAMPLE_RATE * 0.08)
            l = min(num_samples - onset, f_len)
            if l > 0:
                age_f = np.arange(l) / SAMPLE_RATE
                sweep = f_start + (f_end - f_start) * (age_f / 0.08)
                fascinator_sig = np.sin(2.0 * np.pi * sweep * age_f) * np.exp(-age_f / 0.018) * 0.04
                mix[onset:onset+l] += fascinator_sig
                
        # 2. Mimics (High Agitation trigger)
        # Copies previous step's mix buffer with a delay, bitcrushing, and phase inversion
        if vaesen_ag > 0.35 and step > 4:
            prev_onset = onset - step_samples * 2
            m_l = min(num_samples - onset, step_samples)
            if m_l > 0 and prev_onset >= 0:
                mimic_sig = -mix[prev_onset:prev_onset+m_l] * 0.45
                # Decimate/bitcrush
                quantized = np.round(mimic_sig * 6.0) / 6.0
                mix[onset:onset+m_l] += quantized
                
        # 3. Hybrid click trains (Dwell/Repetition trigger)
        if vaesen_rep > 0.40 and step % 2 == 0:
            c_len = int(SAMPLE_RATE * 0.015)
            c_l = min(num_samples - onset, c_len)
            if c_l > 0:
                c_age = np.arange(c_l) / SAMPLE_RATE
                clicks = np.sin(2.0 * np.pi * 3800.0 * c_age) * np.exp(-c_age / 0.0035) * 0.065
                mix[onset:onset+c_l] += clicks
                
        # Subtle musicalities: FM bells
        arp_rate = 6 if vaesen_pl > 0.75 else 4
        if step % arp_rate == 0:
            notes = [130.81, 146.83, 164.81, 196.00, 220.00, 261.63, 293.66, 329.63]
            note_idx = (step // arp_rate) % len(notes)
            f_carrier = notes[note_idx] * (1.5 if vaesen_rep > 0.5 else 1.0)
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
                
        # Growl tests driven by Agitation
        if vaesen_ag > 0.4 and step % 8 == 0:
            growl_len = int(SAMPLE_RATE * 0.35)
            g_age = np.arange(min(num_samples - onset, growl_len)) / SAMPLE_RATE
            if len(g_age) > 0:
                f_growl = 60.0 + 120.0 * np.sin(2.0 * np.pi * 3.0 * g_age)
                growl_osc = np.sign(np.sin(2.0 * np.pi * f_growl * g_age))
                f_f1 = 280.0 + 300.0 * vaesen_ag
                f_f2 = 800.0 + 1200.0 * (1.0 - vaesen_pl)
                y_f1 = np.sin(2.0 * np.pi * f_f1 * g_age) * growl_osc
                y_f2 = np.sin(2.0 * np.pi * f_f2 * g_age) * growl_osc
                growl_sig = (y_f1 + y_f2) * np.exp(-g_age / 0.15) * 0.18
                growl_sig = np.tanh(growl_sig * (2.0 + 4.0 * vaesen_ag))
                mix[onset:onset+len(g_age)] += growl_sig * 0.22

    mix = np.clip(mix, -1.0, 1.0)
    
    # Save audio wave file
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
        
    return audio_path, ag_state, pl_state, rep_state, sig_names

def render_9m_video(audio_path, ag_state, pl_state, rep_state, sig_names, output_mp4):
    print("[VIDEO] Starting 9-minute frame rendering...")
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
    bird_y = [height * 0.4, height * 0.5, height * 0.6]
    bird_vy = [0.0, 0.0, 0.0]
    
    for f in range(TOTAL_FRAMES):
        sample_idx = int(f * (SAMPLE_RATE / FPS))
        if sample_idx >= len(ag_state):
            break
            
        ag = ag_state[sample_idx]
        pl = pl_state[sample_idx]
        rep = rep_state[sample_idx]
        
        step_idx = int(sample_idx / (SAMPLE_RATE * 60.0 / 124.0 / 4))
        sig_name = sig_names[min(step_idx, len(sig_names)-1)]
        
        img = Image.new("RGB", (width, height), BG_COLOR)
        draw = ImageDraw.Draw(img)
        
        # Grid lines reacting to Pleasure
        grid_spacing = int(40 + 20 * pl)
        for x in range(0, width, grid_spacing):
            draw.line([(x, 0), (x, height)], fill=(10, int(20 + 40 * pl), int(40 + 80 * pl)))
        for y in range(0, height, grid_spacing):
            draw.line([(0, y), (width, y)], fill=(10, int(20 + 40 * pl), int(40 + 80 * pl)))
            
        # Draw Birds
        for i in range(num_birds):
            wing_freq = 4.0 + 15.0 * ag
            wing_amp = 18.0 + 12.0 * rep
            
            bird_vy[i] = 0.9 * bird_vy[i] + 0.1 * math.sin(f * 0.15 + i) * (2.0 + 8.0 * ag)
            bird_y[i] = max(50, min(height - 50, bird_y[i] + bird_vy[i]))
            
            bx = bird_x[i]
            by = bird_y[i]
            
            t_sec = f / FPS
            wing_offset = math.sin(2.0 * math.pi * wing_freq * t_sec) * wing_amp
            
            draw.ellipse([bx - 12, by - 6, bx + 12, by + 6], fill=PEACH_COLOR)
            draw.line([(bx, by), (bx - 28, by - 10 + wing_offset)], fill=ACCENT_COLOR, width=3)
            draw.line([(bx, by), (bx + 28, by - 10 + wing_offset)], fill=ACCENT_COLOR, width=3)
            draw.polygon([(bx - 12, by), (bx - 20, by - 6), (bx - 20, by + 6)], fill=WARNING_COLOR)
            draw.ellipse([bx + 6, by - 3, bx + 9, by], fill=(255, 255, 255))
            
        # HUD overlays
        draw.text((20, 20), "TSFI/2: AUNCIENT FLOCK & SPECTRAL ACCUMULATORS", fill=HUD_COLOR)
        draw.text((20, 36), f"ACTIVE SIGNATURE: {sig_name.upper()}", fill=WARNING_COLOR)
        
        # Draw indicator trigger states for mimics/fascinators
        if ag > 0.35:
            draw.text((400, 20), "⚡ MIMIC ENGAGED", fill=ACCENT_COLOR)
        if pl > 0.58:
            draw.text((400, 36), "⭐ FASCINATOR ACTIVE", fill=HUD_COLOR)
        if rep > 0.40:
            draw.text((400, 52), "⚠️ DWELL LOCK ACTIVE", fill=WARNING_COLOR)
            
        draw.text((20, 270), f"AGITATION (E_ag): {int(ag*100)}%", fill=ACCENT_COLOR)
        draw.rectangle([20, 282, 220, 290], outline=ACCENT_COLOR)
        draw.rectangle([20, 282, int(20 + 200 * ag), 290], fill=ACCENT_COLOR)
        
        draw.text((20, 298), f"PLEASURE (H_pl):  {int(pl*100)}%", fill=HUD_COLOR)
        draw.rectangle([20, 310, 220, 318], outline=HUD_COLOR)
        draw.rectangle([20, 310, int(20 + 200 * pl), 318], fill=HUD_COLOR)
        
        draw.text((20, 326), f"REPETITION ACC:   {int(rep*100)}%", fill=WARNING_COLOR)
        draw.rectangle([20, 338, 220, 346], outline=WARNING_COLOR)
        draw.rectangle([20, 338, int(20 + 200 * rep), 346], fill=WARNING_COLOR)
        
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
    
    audio_path, ag_state, pl_state, rep_state, sig_names = generate_9m_performance()
    render_9m_video(audio_path, ag_state, pl_state, rep_state, sig_names, output_mp4)
    
    if os.path.exists(audio_path):
        os.remove(audio_path)
