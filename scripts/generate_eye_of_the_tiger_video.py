import os
import json
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

# --- AUNCIENT TRIGONOMETRIC LUT SETUP ---
LUT_SIZE = 22528
LUT_MASK = LUT_SIZE - 1
LUT_FACTOR = LUT_SIZE / (2 * np.pi)
sin_lut = np.sin(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))
cos_lut = np.cos(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))

def lut_sin(angle_val, shift=0):
    val = (angle_val % (2.0 * np.pi)) * LUT_FACTOR
    idx = (int(math.floor(val)) + shift) & LUT_MASK
    return sin_lut[idx]

def lut_cos(angle_val, shift=0):
    val = (angle_val % (2.0 * np.pi)) * LUT_FACTOR
    idx = (int(math.floor(val)) + shift) & LUT_MASK
    return cos_lut[idx]

# Hopf Fibration coordinates modulated by active YI contract registers (7D Lissajous Convolution Matrix)
def get_lissajous_shape(state, t_secs, steps, sig_segment, r_scale, samplings, lut_shift):
    f_w = (state["Manifold"] % 4) + 1
    f_x = (state["Monopole"] % 5) + 1
    f_y = (state["Rod_Dynamo"] % 4) + 1
    f_z = (state["Cone_Dynamo"] % 5) + 1
    f_r = (state["Ring"] % 3) + 1.0
    f_i = (state["Identity"] % 3) + 1
    
    # 8th variable (Element) scales delay offset, modulated by stereo width (8D)
    f_e = ((state["Element"] % 3) + 1.0) * (1.0 + 0.3 * samplings["stereo_width"])
    # 9th variable (Chin) scales vertical bottom warping, modulated by transient flux (9D)
    f_c = ((state["Chin"] % 100) / 400.0) * (1.0 + 0.5 * samplings["flux"])
    
    # 7th variable (Base) modulates the shear rotation, scaled by RMS volume
    shear_factor = ((state["Base"] % 100) / 1000.0) * (1.0 + 0.4 * samplings["rms"])
    
    # Calibrate base phase shift from user's Identity & Prime values
    phi_base = (state["Identity"] / state["Prime"]) * 2.0 * np.pi
    
    # Phase spin velocity scales dynamically with pitch and transient acceleration
    spin_velocity = (samplings["pitch"] / 220.0) * t_secs * (1.0 + 0.2 * samplings["flux"])
    phase_w = spin_velocity * 0.05 + phi_base
    phase_x = spin_velocity * 0.06 + phi_base
    phase_y = spin_velocity * 0.07 + phi_base
    
    points = []
    # Dynamic modulus delay scaled by Element parameter
    delay = int(8 * f_e)
    
    # Modulate X-frequency by brightness timbre
    f_x_mod = f_x * (1.0 + 0.3 * samplings["brightness"])
    # Modulate Z-depth scale by wave crest factor (harshness)
    f_z_mod = f_z * (1.0 + 0.4 * samplings["crest"])
    # Modulate Hopf fibration octave multiplier by Zero-Crossing Rate
    f_i_mod = f_i * (1.0 + 0.2 * samplings["zcr"])
    
    # 14th Dimension: Hypotrochoid parameters derived from Delegate state alignments (collision-free prime modulos)
    R_hyp = (state["Identity"] % 97) + 20.0
    r_hyp = (state["Element"] % 53) + 10.0
    d_hyp = (state["Chin"] % 41) + 5.0
    
    for i in range(steps):
        # Sound reaction vector: current amplitude and phase-delayed amplitude (1D & 2D)
        sig_now = sig_segment[i] * 1.8 if i < len(sig_segment) else 0.0
        sig_delayed = sig_segment[i - delay] * 1.8 if (i >= delay and i - delay < len(sig_segment)) else 0.0
        
        # Convolve sound amplitudes directly into the Hopf Fibration angles (smooth curve generation)
        theta = (i * 2.0 * np.pi) / steps
        eta = ((theta * f_w + phase_w + sig_now * 0.5) * 0.5) % (np.pi / 2.0)
        xi1 = ((theta * f_x_mod + phase_x + sig_delayed * 0.5) * f_i_mod) % (2.0 * np.pi)
        xi2 = ((theta * f_y + phase_y) * 3) % (2.0 * np.pi)
        
        # Resolve YI LUT convolution coefficients
        qx_g = lut_cos(eta, lut_shift) * lut_sin(xi1, lut_shift)
        qy_g = lut_sin(eta, lut_shift) * lut_cos(xi2, lut_shift)
        
        # Project sound vector through the Lissajous Convolution Matrix:
        # [ X ] = [  qx   qy ] * [ sig_now     ]
        # [ Y ] = [ -qy   qx ]   [ sig_delayed ]
        x = (qx_g * sig_now + qy_g * sig_delayed) * r_scale
        y = (-qy_g * sig_now + qx_g * sig_delayed) * r_scale * f_z_mod
        
        # Apply 14D Hypotrochoid vector offset tracing (Delegate token signature)
        h_x = (R_hyp - r_hyp) * math.cos(theta) + d_hyp * math.cos(((R_hyp - r_hyp) / r_hyp) * theta)
        h_y = (R_hyp - r_hyp) * math.sin(theta) - d_hyp * math.sin(((R_hyp - r_hyp) / r_hyp) * theta)
        
        # Apply shear rotation utilizing Base parameter and blend in the hypotrochoid signature
        x_final = x + h_x * (1.0 + 0.3 * sig_now)
        y_final = y + x * shear_factor + h_y * (1.0 + 0.3 * sig_delayed)
        
        # Apply 9D Chin hemisphere vertical asymmetry warping (asymmetric clamping)
        if y_final < 0:
            y_final *= (1.0 + f_c)
            
        # Physical CRT Electrostatic deflection limit (Soft tanh compression limit)
        max_tube_radius = r_scale * 0.9
        radial_distance = math.sqrt(x_final**2 + y_final**2)
        if radial_distance > 0.0:
            compressed_radius = math.tanh(radial_distance / max_tube_radius) * max_tube_radius
            scaling_clamp = compressed_radius / radial_distance
            x_final *= scaling_clamp
            y_final *= scaling_clamp
            
        points.append((x_final, y_final))
    return points

YI_BASE_STATE = {
    "Prime": 953467954114363,
    "Name": "YI-Alpha (Motzkin)",
    "Manifold": 1631052951685751,
    "Monopole": 563316215773011,
    "Ring": 432408158703388,
    "Rod_Dynamo": 231949031122517,
    "Cone_Dynamo": 1417158305948761,
    "Identity": 1700684580,
    "Base": 5208
}

# --- CONFIGURATION ---
SAMPLE_RATE = 44100
FPS = 30
WIDTH = 1280
HEIGHT = 720
DURATION_SECS = 20  # Fast demo duration for preview/rendering speed
TOTAL_FRAMES = DURATION_SECS * FPS
TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS

# Load the Bionika score
with open("assets/bionika/eye_of_the_tiger.bio", "r") as f:
    score = json.load(f)

# Note frequency dictionary
NOTE_FREQS = {
    "C2": 65.41, "C#2": 69.30, "D2": 73.42, "Eb2": 77.78, "E2": 82.41, "F2": 87.31,
    "F#2": 92.50, "G2": 98.00, "Ab2": 103.83, "A2": 110.00, "Bb2": 116.54, "B2": 123.47,
    "C3": 130.81, "C#3": 138.59, "D3": 146.83, "Eb3": 155.56, "E3": 164.81, "F3": 174.61,
    "F#3": 185.00, "G3": 196.00, "Ab3": 207.65, "A3": 220.00, "Bb3": 233.08, "B3": 246.94,
    "C4": 261.63, "C#4": 277.18, "D4": 293.66, "Eb4": 311.13, "E4": 329.63, "F4": 349.23,
    "F#4": 369.99, "G4": 392.00, "Ab4": 415.30, "A4": 440.00, "Bb4": 466.16, "B4": 493.88,
    "Ab0": 25.96, "Bb0": 29.14, "C1": 32.70
}

# --- AUDIO SYNTHESIS & ANALOG PIPELINE ---
def synthesize_audio():
    print("[AUDIO] Synthesizing score...")
    tempo = score["tempo"]
    step_duration = (60.0 / tempo) / 4.0
    step_samples = int(SAMPLE_RATE * step_duration)
    
    master_left = np.zeros(TOTAL_SAMPLES)
    master_right = np.zeros(TOTAL_SAMPLES)
    
    # Store channel-specific data for rendering
    channels_audio = {
        "bass": np.zeros(TOTAL_SAMPLES),
        "lead": np.zeros(TOTAL_SAMPLES),
        "arp": np.zeros(TOTAL_SAMPLES),
        "growl": np.zeros(TOTAL_SAMPLES),
        "kick": np.zeros(TOTAL_SAMPLES),
        "snare": np.zeros(TOTAL_SAMPLES)
    }

    arrangement = score["arrangement"]
    patterns = score["patterns"]
    
    current_sample = 0
    pattern_idx = 0
    
    # Pre-generate noise for drums
    snare_noise = np.random.normal(0, 0.15, step_samples * 2)
    hat_noise = np.random.normal(0, 0.08, step_samples)
    
    while current_sample < TOTAL_SAMPLES and pattern_idx < len(arrangement):
        pat_name = arrangement[pattern_idx]
        pat = patterns[pat_name]
        steps = 32 # Patterns have 32 steps
        
        for step in range(steps):
            if current_sample >= TOTAL_SAMPLES:
                break
                
            # --- Arpeggiator Filter Control (CV Modulator) ---
            arp_cutoff = 800.0 # Default filter cutoff frequency in Hz
            if "arpeggiator_filter" in pat and step < len(pat["arpeggiator_filter"]["sequence"]):
                arp_note = pat["arpeggiator_filter"]["sequence"][step]
                if arp_note != "REST" and arp_note in NOTE_FREQS:
                    arp_cutoff = NOTE_FREQS[arp_note]
            
            # Write arpeggiator filter CV modulation to visual telemetry buffer
            t_len = min(step_samples, TOTAL_SAMPLES - current_sample)
            if t_len > 0:
                channels_audio["arp"][current_sample:current_sample+t_len] = (arp_cutoff - 800.0) / 400.0

            # --- Bass Channel ---
            if "bass" in pat and step < len(pat["bass"]["sequence"]):
                note = pat["bass"]["sequence"][step]
                accents_list = pat["bass"].get("accents", [0] * steps)
                slides_list = pat["bass"].get("slides", [0] * steps)
                accent = accents_list[step] if step < len(accents_list) else 0
                slide = slides_list[step] if step < len(slides_list) else 0
                if note != "REST" and note in NOTE_FREQS:
                    freq = NOTE_FREQS[note]
                    dur = step_samples * (2 if slide else 1)
                    t = np.arange(min(dur, TOTAL_SAMPLES - current_sample)) / SAMPLE_RATE
                    sig = 0.3 * (2 * (t * freq - np.floor(t * freq + 0.5))) # Triangle wave
                    
                    # Apply arpeggiator filter modulation to the bass sound (subtle warm wobble)
                    bass_cutoff_ratio = min(0.35, max(0.08, arp_cutoff / 2800.0))
                    filtered_bass = np.zeros_like(sig)
                    last_bass_val = 0.0
                    for k in range(len(sig)):
                        last_bass_val = last_bass_val + bass_cutoff_ratio * (sig[k] - last_bass_val)
                        filtered_bass[k] = last_bass_val

                    env = np.exp(-4 * t) if not slide else np.ones_like(t)
                    gain = 1.4 if accent else 0.8
                    sig_out = filtered_bass * env * gain
                    master_left[current_sample:current_sample+len(t)] += sig_out * 0.7
                    master_right[current_sample:current_sample+len(t)] += sig_out * 0.7
                    channels_audio["bass"][current_sample:current_sample+len(t)] += sig_out

            # --- Lead Channel ---
            if "lead" in pat and step < len(pat["lead"]["sequence"]):
                note = pat["lead"]["sequence"][step]
                accents_list = pat["lead"].get("accents", [0] * steps)
                accent = accents_list[step] if step < len(accents_list) else 0
                if note != "REST" and note in NOTE_FREQS:
                    freq = NOTE_FREQS[note]
                    dur = step_samples
                    t = np.arange(min(dur, TOTAL_SAMPLES - current_sample)) / SAMPLE_RATE
                    # Saw + Square blend for pop synthesizer tone
                    sig = 0.15 * (2 * (t * freq - np.floor(t * freq))) + 0.08 * np.sign(np.sin(2 * np.pi * freq * t))
                    
                    # Apply the Arpeggiator Filter sweep (Tighter, subtler sweep range)
                    cutoff_ratio = min(0.65, max(0.12, arp_cutoff / 1800.0))
                    filtered_sig = np.zeros_like(sig)
                    last_val = 0.0
                    for k in range(len(sig)):
                        last_val = last_val + cutoff_ratio * (sig[k] - last_val)
                        filtered_sig[k] = last_val
                        
                    env = np.exp(-6 * t)
                    gain = 1.3 if accent else 0.8
                    sig_out = filtered_sig * env * gain
                    master_left[current_sample:current_sample+len(t)] += sig_out * 0.5
                    master_right[current_sample:current_sample+len(t)] += sig_out * 0.8
                    channels_audio["lead"][current_sample:current_sample+len(t)] += sig_out

            # --- Sub-Growl Channel ---
            if "sub_growl" in pat and step < len(pat["sub_growl"]["sequence"]):
                note = pat["sub_growl"]["sequence"][step]
                gain_val = pat["sub_growl"]["gain"][step]
                mod_val = pat["sub_growl"]["modulation_rate"][step]
                if note != "REST" and note in NOTE_FREQS and gain_val > 0.0:
                    freq = NOTE_FREQS[note]
                    dur = step_samples * 2
                    t = np.arange(min(dur, TOTAL_SAMPLES - current_sample)) / SAMPLE_RATE
                    # Gated pure sine carrier to produce a perfect visual circle that collapses to a central dot
                    sig = 0.6 * np.sin(2 * np.pi * freq * t)
                    
                    # Attack-decay envelope: fast rising edge, decaying to form the central dot
                    env = np.exp(-4.5 * t) * (1.0 - np.exp(-40 * t))
                    
                    # Raise gain slightly to ensure prominent visibility
                    sig_out = sig * env * (gain_val * 1.5)
                    master_left[current_sample:current_sample+len(t)] += sig_out * 0.95
                    master_right[current_sample:current_sample+len(t)] += sig_out * 0.95
                    channels_audio["growl"][current_sample:current_sample+len(t)] += sig_out

            # --- Drums ---
            if "drums" in pat:
                # Kick
                if step < len(pat["drums"]["kick"]) and pat["drums"]["kick"][step] > 0:
                    t = np.arange(min(step_samples * 2, TOTAL_SAMPLES - current_sample)) / SAMPLE_RATE
                    # Pitch sweep kick
                    freq = 120 * np.exp(-35 * t) + 40
                    sig = 0.6 * np.sin(2 * np.pi * freq * t) * np.exp(-8 * t)
                    master_left[current_sample:current_sample+len(t)] += sig * 0.8
                    master_right[current_sample:current_sample+len(t)] += sig * 0.8
                    channels_audio["kick"][current_sample:current_sample+len(t)] += sig
                
                # Snare
                if step < len(pat["drums"]["snare"]) and pat["drums"]["snare"][step] > 0:
                    t = np.arange(min(step_samples * 2, TOTAL_SAMPLES - current_sample)) / SAMPLE_RATE
                    sig_noise = snare_noise[:len(t)] * np.exp(-12 * t)
                    sig_body = 0.35 * np.sin(2 * np.pi * 180 * t) * np.exp(-18 * t)
                    sig = (sig_noise + sig_body) * 0.45
                    master_left[current_sample:current_sample+len(t)] += sig * 0.7
                    master_right[current_sample:current_sample+len(t)] += sig * 0.7
                    channels_audio["snare"][current_sample:current_sample+len(t)] += sig

            current_sample += step_samples
        pattern_idx += 1

    # --- Run emulate_analog_pipeline C-translation with DC blocking filters ---
    print("[AUDIO] Running AC-coupled substrate amplifier emulation...")
    V_c_in = 0.0
    V_c_e = 0.0
    env_state = 0.0
    s1_x, s1_y = 0.0, 0.0
    s2_x, s2_y = 0.0, 0.0
    bias_trans = 0.68
    bias_pp = 0.18
    loadRL = 8.0
    V_max = 1.97 * loadRL
    softLimit = V_max * 0.95

    # Simple 17-point Germanium lookup table
    lut = [0.0, 0.04, 0.08, 0.115, 0.14, 0.16, 0.175, 0.185, 0.195, 0.202, 0.208, 0.213, 0.218, 0.222, 0.226, 0.230, 0.235]

    for i in range(TOTAL_SAMPLES):
        inp = master_left[i] * 0.8
        V_b = (inp + 0.25) - V_c_in
        V_e = V_c_e
        Vbe_input = V_b - V_e
        
        Vbe = 0.0
        if Vbe_input > 0.0:
            s_idx = Vbe_input / 0.1
            idx = int(s_idx)
            frac = s_idx - idx
            if idx >= 15:
                Vbe = lut[16]
            else:
                Vbe = lut[idx] + (lut[idx+1] - lut[idx]) * frac
        
        Ib = (Vbe_input - Vbe) / 100000.0 if Vbe_input > Vbe else 0.0
        Ic = Ib * 100.0
        stage1_out = 9.0 - Ic * 4700.0
        
        V_c_in = V_c_in + Ib * 0.01 - V_c_in * 0.001
        V_c_e = V_c_e + Ib * 101.0 * 0.01 - V_c_e * 0.005

        s1_raw = (5.5 - stage1_out) * 0.25
        stage1_ac = s1_raw - s1_x + 0.995 * s1_y
        s1_x, s1_y = s1_raw, stage1_ac

        modulated_offset = max(0.1, min(1.0, 0.6 - stage1_ac * 0.05))
        Vbe_input_trans = bias_trans - modulated_offset
        Ib_trans = Vbe_input_trans / 10000.0 if Vbe_input_trans > 0.0 else 0.0
        Ic_trans = Ib_trans * 150.0
        stage2_out = max(0.0, min(9.0, 9.0 - Ic_trans * 2200.0))

        s2_raw = (8.0 - stage2_out) * 1.0
        stage2_ac = s2_raw - s2_x + 0.995 * s2_y
        s2_x, s2_y = s2_raw, stage2_ac

        eff_thresh = max(0.0, 0.2 - bias_pp)
        pp_out = 0.0
        if stage2_ac > eff_thresh:
            pp_out = stage2_ac - eff_thresh
        elif stage2_ac < -eff_thresh:
            pp_out = stage2_ac + eff_thresh

        abs_inp = abs(pp_out)
        env_state = (env_state + abs_inp) / 2.0 if abs_inp > env_state else env_state * 0.99
        gain = V_max / env_state if env_state > V_max else 1.0
        final_out = max(-softLimit, min(softLimit, pp_out * gain))
        master_left[i] = final_out / softLimit

    # Normalize audio levels
    peak = np.max(np.abs(master_left))
    if peak > 0.01:
        master_left *= (0.95 / peak)
        master_right *= (0.95 / peak)

    # Save audio wave file
    print("[AUDIO] Writing output soundtrack...")
    with wave.open("assets/bionika/eye_of_the_tiger.wav", "wb") as wav_file:
        wav_file.setnchannels(2)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        for i in range(TOTAL_SAMPLES):
            l_val = int(max(-32768, min(32767, master_left[i] * 32767.0)))
            r_val = int(max(-32768, min(32767, master_right[i] * 32767.0)))
            wav_file.writeframes(struct.pack("<hh", l_val, r_val))

    return master_left, channels_audio

# --- VIDEO GENERATION ---
def generate_video(audio_wave, channels):
    print("[VIDEO] Starting 3D vector Menorah rendering...")
    
    # Setup ffmpeg pipeline
    cmd = [
        "ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", str(FPS),
        "-i", "-", "-i", "assets/bionika/eye_of_the_tiger.wav",
        "-c:v", "libx264", "-pix_fmt", "yuv420p", "-shortest",
        "assets/bionika/eye_of_the_tiger.mp4"
    ]
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE)

    # Monitor circle layouts for the menorah (7 screens)
    # (x, y, radius, label)
    screens = [
        (640, 480, 160, "MASTER ATTRACTOR"), # Center
        (220, 240, 70, "SUB GROWL"),         # Left 1
        (360, 260, 70, "BASS SHIFT"),        # Left 2
        (500, 280, 70, "ARPEGGIATOR"),       # Left 3
        (780, 280, 70, "KICK DRUM"),         # Right 3
        (920, 260, 70, "SNARE DRUM"),        # Right 2
        (1060, 240, 70, "LEAD SYNTH")        # Right 1
    ]

    last_rms_values = [0.0] * 7

    for frame_idx in range(TOTAL_FRAMES):
        if frame_idx % 30 == 0:
            print(f"[VIDEO] Rendered frame {frame_idx}/{TOTAL_FRAMES}...")
            
        time_secs = frame_idx / FPS
        sample_idx = int(time_secs * SAMPLE_RATE)
        
        # Base image
        im = Image.new("RGB", (WIDTH, HEIGHT), (11, 15, 25))
        draw = ImageDraw.Draw(im)

        # Draw Menorah structure/branches connecting screens
        draw.line([(640, 680), (640, 480)], fill=(31, 41, 55), width=8)
        draw.arc([140, 240, 1140, 680], 0, 180, fill=(31, 41, 55), width=6)

        # Render each screen
        for i, (cx, cy, r, label) in enumerate(screens):
            # Screen boundary circle
            draw.ellipse([cx-r, cy-r, cx+r, cy+r], outline=(55, 65, 81), width=4, fill=(6, 9, 19))
            
            # Fetch corresponding signal data segment
            sig_segment = None
            if i == 0:
                sig_segment = audio_wave[sample_idx:sample_idx+512]
            elif i == 1:
                growl_raw = channels["growl"][sample_idx:sample_idx+512]
                # Boost visual amplitude by 8.0x so it is clearly visible on the screen
                sig_segment = growl_raw * 8.0
                # If signal is silent or extremely quiet, add a minimal active 60Hz humming ripple
                rms = np.sqrt(np.mean(sig_segment**2)) if len(sig_segment) > 0 else 0.0
                if rms < 0.04:
                    t_rip = np.linspace(0, 4 * np.pi, len(sig_segment) if len(sig_segment) > 0 else 256)
                    sig_segment = sig_segment + 0.05 * np.sin(t_rip)
            elif i == 2:
                sig_segment = channels["bass"][sample_idx:sample_idx+512]
            elif i == 3:
                sig_segment = channels["arp"][sample_idx:sample_idx+512]
            elif i == 4:
                sig_segment = channels["kick"][sample_idx:sample_idx+512]
            elif i == 5:
                sig_segment = channels["snare"][sample_idx:sample_idx+512]
            elif i == 6:
                sig_segment = channels["lead"][sample_idx:sample_idx+512]

            if sig_segment is None or len(sig_segment) < 256:
                sig_segment = np.zeros(256)

            # Calculate current performance step index
            bpm = score.get("tempo", 120)
            step_duration = 60.0 / (bpm * 4.0)
            step = int(time_secs / step_duration)
            
            # Map screen index to track pattern sequence
            chan_map = {
                1: "sub_growl",
                2: "bass",
                3: "arpeggiator_filter",
                6: "lead"
            }
            active_freq = 220.0 # Default reference frequency
            
            # Determine pattern from arrangement
            arr_idx = int(time_secs / (step_duration * 32))
            if arr_idx < len(score["arrangement"]):
                pat_name = score["arrangement"][arr_idx]
                pat = score["patterns"].get(pat_name, {})
                track_name = chan_map.get(i)
                if track_name and track_name in pat:
                    seq = pat[track_name].get("sequence", [])
                    step_in_pat = step % 32
                    if step_in_pat < len(seq):
                        note = seq[step_in_pat]
                        if note != "REST" and note in NOTE_FREQS:
                            active_freq = NOTE_FREQS[note]

            # Dynamically modulate color based on pitch for melodic tracks
            if i in [1, 2, 3, 6]:
                # 55Hz (low growl/bass) -> Red (hue=0)
                # 880Hz (high lead/arp) -> Violet (hue=280)
                hue = min(280, max(0, int((active_freq - 55.0) / 800.0 * 280.0)))
                import colorsys
                r_rgb, g_rgb, b_rgb = colorsys.hsv_to_rgb(hue / 360.0, 0.95, 0.95)
                line_color = (int(r_rgb * 255), int(g_rgb * 255), int(b_rgb * 255))
            else:
                color_map = {
                    0: (139, 92, 246),  # Master: Purple
                    4: (16, 185, 129),  # Kick: Emerald
                    5: (239, 68, 68),   # Snare: Red
                }
                line_color = color_map.get(i, (6, 182, 212))

            # Derive unique YI contract state registers for this menorah screen index (7D WinchesterMQ registers)
            # Mathematically computed based on glossary rules
            prime = 953467954114363 # MotzkinPrime constant
            base_val = [3, 2, 5, 7, 11, 13, 17][i]
            sig_val = [5, 11, 13, 17, 19, 23, 29][i]
            secret_val = [7, 17, 19, 23, 29, 31, 37][i]
            ident_val = [11, 23, 29, 31, 37, 41, 43][i]
            element_val = base_val + sig_val
            
            state = {
                "Prime": prime,
                "Base": base_val,
                "Identity": ident_val,
                "Ring": (base_val + secret_val) % prime, # Ring parameter
                "Monopole": pow(base_val, sig_val, prime), # Channel
                "Rod_Dynamo": pow(base_val, secret_val, prime), # Pole
                "Cone_Dynamo": pow(base_val, ident_val, prime), # Foundation
                "Manifold": pow(base_val, sig_val, element_val), # Dynamo
                "Element": element_val, # Element parameter (8D)
                "Chin": (base_val + secret_val) % prime # Chin parameter (9D)
            }

            # AC-couple signal segment to remove DC offsets causing straight radial lines
            sig_ac = sig_segment - np.mean(sig_segment)
            
            # Compute 9 types of sampling features for YI register modulation
            rms = np.sqrt(np.mean(sig_ac**2)) if len(sig_ac) > 0 else 0.0
            peak = np.max(np.abs(sig_ac)) if len(sig_ac) > 0 else 0.0
            crest_factor = peak / (rms + 1e-6)
            zero_crossings = np.sum(np.diff(np.sign(sig_ac)) != 0) if len(sig_ac) > 0 else 0
            
            # Tracking spectral flux over video frames
            flux_val = abs(rms - last_rms_values[i])
            last_rms_values[i] = rms
            
            # Normalize samplings to [0, 1] bands
            samplings = {
                "pitch": active_freq,
                "rms": min(1.0, rms * 2.5),
                "crest": min(1.0, crest_factor / 12.0),
                "zcr": min(1.0, zero_crossings / 120.0),
                "brightness": min(1.0, zero_crossings / 90.0),
                "stereo_width": 0.5 * (1.0 + np.sin(time_secs * 2.0 * np.pi * 0.05 + i)),
                "flux": min(1.0, flux_val * 15.0)
            }
            
            # Interpolate signal segment to 2048 steps to guarantee high resolution curves (zero boxiness!)
            steps = 2048
            sig_interp = np.interp(np.linspace(0, len(sig_ac) - 1, steps), np.arange(len(sig_ac)), sig_ac)
            
            # Generate coordinates using active YI-based LUT tables convolved with sound vector
            lut_shift = int((active_freq - 110.0) * 10.0)
            pts = get_lissajous_shape(state, time_secs, steps, sig_interp, r * 0.7, samplings, lut_shift)
            
            # Apply a 3-point moving average filter to coordinates to guarantee rounded arcs
            smoothed_pts = []
            for j in range(len(pts)):
                p_prev = pts[j - 1] if j > 0 else pts[j]
                p_curr = pts[j]
                p_next = pts[j + 1] if j < len(pts) - 1 else pts[j]
                smoothed_x = (p_prev[0] + p_curr[0] + p_next[0]) / 3.0
                smoothed_y = (p_prev[1] + p_curr[1] + p_next[1]) / 3.0
                smoothed_pts.append((cx + smoothed_x, cy + smoothed_y))
            points = smoothed_pts

            # Draw vector points
            if len(points) > 1:
                draw.line(points, fill=line_color, width=2)

            # Screen Text label
            draw.text((cx - 30, cy + r - 20), label, fill=(156, 163, 175))

        # Apply minor bloom filter to create phosphor glow
        im_blur = im.filter(ImageFilter.GaussianBlur(1.5))
        im = Image.blend(im, im_blur, 0.4)

        # Write frame to pipe
        im.save(process.stdin, "PNG")

    process.stdin.close()
    process.wait()
    print("[VIDEO] Successfully rendered MP4 to assets/bionika/eye_of_the_tiger.mp4")

if __name__ == "__main__":
    os.makedirs("assets/bionika", exist_ok=True)
    audio_wave, channels = synthesize_audio()
    generate_video(audio_wave, channels)
