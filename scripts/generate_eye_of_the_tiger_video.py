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

def liang_barsky_clip(x1, y1, x2, y2, xmin, ymin, xmax, ymax):
    dx = x2 - x1
    dy = y2 - y1
    p = [-dx, dx, -dy, dy]
    q = [x1 - xmin, xmax - x1, y1 - ymin, ymax - y1]
    
    t0, t1 = 0.0, 1.0
    for k in range(4):
        if p[k] == 0.0:
            if q[k] < 0.0:
                return None
        else:
            r = q[k] / p[k]
            if p[k] < 0.0:
                if r > t1:
                    return None
                elif r > t0:
                    t0 = r
            else:
                if r < t0:
                    return None
                elif r < t1:
                    t1 = r
    if t0 > t1:
        return None
    return (x1 + t0 * dx, y1 + t0 * dy, x1 + t1 * dx, y1 + t1 * dy)

class TubeVactrolWarmthTransducer:
    """
    Implements a 3D Tube Saturation Transducer where virtual triode tube parameters
    are dynamically driven by the 3D spatial resonance volume of the projected Lissajous geometry.
    """
    def __init__(self, sample_rate=44100):
        self.sample_rate = sample_rate
        self.v_plate_sag = 0.0
        
    def process_triode_stage(self, audio_in, pts):
        if len(pts) == 0:
            return audio_in, audio_in
            
        # Extract spatial dimensions from 3D projected coordinate vectors
        rx_avg = np.mean([p[0] for p in pts])
        ry_avg = np.mean([p[1] for p in pts])
        r_3d = np.mean(np.sqrt(np.sum(np.square(pts), axis=1)))
        
        # 1. Depth Warmth Modulation (optocoupler compression and plate sag)
        self.v_plate_sag = 0.95 * self.v_plate_sag + 0.05 * (r_3d * 0.0008)
        effective_plate_voltage = max(50.0, 250.0 - self.v_plate_sag * 180.0)
        
        # Asymmetric triode saturation driven by depth
        drive = 1.0 + r_3d * 0.003
        scaled_in = audio_in * drive
        asymmetry_offset = 0.001 * ry_avg
        warm_out = np.tanh(scaled_in + asymmetry_offset) * (effective_plate_voltage / 250.0)
        
        # 2. Elevation Timbral Shelving Filter (Formant-keying shift)
        # High elevation (negative ry_avg) boosts highs, low elevation (positive ry_avg) boosts lows
        treble_gain = 1.0 - ry_avg * 0.002
        bass_gain = 1.0 + ry_avg * 0.002
        
        # Simple two-band shelving filter crossover
        w_low = np.zeros_like(warm_out)
        w_high = np.zeros_like(warm_out)
        last_val = 0.0
        for k in range(len(warm_out)):
            last_val = last_val + 0.25 * (warm_out[k] - last_val) # Low pass crossover
            w_low[k] = last_val
            w_high[k] = warm_out[k] - last_val
            
        filtered_out = w_low * bass_gain + w_high * treble_gain
        
        # 3. Horizontal Stereo Panning (Rotational Space)
        pan_factor = min(0.5, max(-0.5, rx_avg * 0.003))
        left_channel = filtered_out * (1.0 - pan_factor)
        right_channel = filtered_out * (1.0 + pan_factor)
        
        gain_correction = 1.0 / (1.0 + r_3d * 0.002)
        return left_channel * gain_correction, right_channel * gain_correction

class MasterStageAccumulatorClassifier:
    """
    Accumulates master output stage electrical signal content (instrument vectors, notes)
    and estimates the virtual output stage FET parameters (gate, resistance, temperature)
    using a non-preferential accumulator model, avoiding Child-Langmuir space-charge preferences.
    """
    def __init__(self, sample_rate=44100):
        # Known weights for instrument vectors (growl, bass, arp, lead, drums)
        self.weights = {
            "growl": {"voltage_weight": 0.45, "thermal_weight": 0.65},
            "bass":  {"voltage_weight": 0.35, "thermal_weight": 0.50},
            "arp":   {"voltage_weight": 0.20, "thermal_weight": 0.30},
            "lead":  {"voltage_weight": 0.25, "thermal_weight": 0.35},
            "drums": {"voltage_weight": 0.50, "thermal_weight": 0.80}
        }
        # Non-preferential accumulator state registers
        self.acc_charge = 0.0
        self.acc_thermal = 0.0
        self.temp_ambient = 293.15
        
    def process_block(self, block_samples, active_mix_features):
        # Non-preferential integration of input electrical energy (rms)
        rms = np.sqrt(np.mean(block_samples**2)) if len(block_samples) > 0 else 0.0
        
        # Accumulate gate voltage and thermal dissipation based on instrument weights
        v_gate = 0.0
        thermal_dissipation = 0.0
        
        for inst, w in self.weights.items():
            activity = active_mix_features.get(inst, 0.0)
            v_gate += activity * w["voltage_weight"] * rms * 3.3
            thermal_dissipation += activity * w["thermal_weight"] * (rms ** 2) * 50.0
            
        # Update accumulator charge (FET Gate model)
        self.acc_charge = 0.95 * self.acc_charge + 0.05 * v_gate
        
        # Update thermal accumulator (FET Lattice temperature)
        # Bypasses Child-Langmuir law by maintaining standard mathematical continuity
        self.acc_thermal = 0.98 * self.acc_thermal + 0.02 * thermal_dissipation
        
        # Derive output stage FET parameters
        r_ds = 1.0 + 9.0 * (1.0 / (1.0 + self.acc_charge))
        temp = self.temp_ambient + self.acc_thermal * 12.0
        
        return self.acc_charge, r_ds, temp

# Hopf Fibration coordinates modulated by active YI contract registers (7D Lissajous Convolution Matrix)
def get_lissajous_shape(state, t_secs, steps, sig_segment, r_scale, samplings, lut_shift, chan_idx=0, is_visual=False):
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
    
    # Dynamic visual gain control (blow-up constraint) on heavy shock (RMS and flux) - isolated from visuals
    if is_visual:
        shock = 0.0
    else:
        shock = samplings.get("rms", 0.0) * 0.7 + samplings.get("flux", 0.0) * 0.8
    r_scale_constrained = r_scale / (1.0 + shock * 0.5) if not is_visual else r_scale

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
        
        # Calculate convolved 3D coordinates (with Z depth layer)
        x_3d = x + h_x * (1.0 + 0.3 * sig_now)
        y_3d = y + x * shear_factor + h_y * (1.0 + 0.3 * sig_delayed)
        z_3d = math.sin(theta * f_z_mod + phase_y) * r_scale * 0.35 * (1.0 + 0.3 * sig_now)
        
        # Apply 9D Chin hemisphere vertical asymmetry warping (asymmetric clamping in 3D space)
        if y_3d < 0:
            y_3d *= (1.0 + f_c)
            
        # Apply 3D single camera isometric projection matrix (decoupled acoustic cameras per tube)
        yaw_speeds = {0: 0.4, 1: 0.08, 2: 0.22, 3: 0.85, 4: 0.25, 5: 0.30, 6: 0.55}
        pitch_freqs = {0: 0.2, 1: 0.04, 2: 0.11, 3: 0.42, 4: 0.13, 5: 0.16, 6: 0.33}
        
        y_speed = yaw_speeds.get(chan_idx, 0.4)
        p_freq = pitch_freqs.get(chan_idx, 0.2)
        
        cam_yaw = t_secs * y_speed + samplings.get("flux", 0.0) * 0.15
        cam_pitch = 0.35 + 0.08 * math.sin(t_secs * 2.0 * np.pi * p_freq) + samplings.get("rms", 0.0) * 0.12
        
        # Rotate coordinates around Z-axis (Yaw)
        rx = x_3d * math.cos(cam_yaw) - y_3d * math.sin(cam_yaw)
        ry = x_3d * math.sin(cam_yaw) + y_3d * math.cos(cam_yaw)
        rz = z_3d
        
        # Rotate coordinates around X-axis (Pitch)
        ry2 = ry * math.cos(cam_pitch) - rz * math.sin(cam_pitch)
        rz2 = ry * math.sin(cam_pitch) + rz * math.cos(cam_pitch)
        
        # Apply perspective scaling factors (increase perspective distance 'dist' to reduce warping on shock)
        dist = 500.0 * (1.0 + shock * 1.5)
        scale_factor = dist / (dist + ry2)
        x_final = rx * scale_factor
        y_final = -rz2 * scale_factor
        
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

# Load the Bionika score
with open("assets/bionika/eye_of_the_tiger.bio", "r") as f:
    score = json.load(f)

# --- CONFIGURATION ---
SAMPLE_RATE = 44100
FPS = 30
WIDTH = 1280
HEIGHT = 720
# Calculate duration dynamically based on full score structure (16 steps per bar, 8 beats per pattern)
DURATION_SECS = int(len(score["arrangement"]) * (480.0 / score["tempo"]))
TOTAL_FRAMES = DURATION_SECS * FPS
TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS

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
                    # FM synthesis for growling texture: carrier modulated by low-frequency wobble rate (mod_val)
                    wobble = np.sin(2 * np.pi * mod_val * t)
                    sig = 0.6 * np.sin(2 * np.pi * freq * t + 3.5 * wobble)
                    
                    # Wobbly envelope: sustains longer and pulsates at the wobble rate
                    env = np.exp(-1.2 * t) * (1.0 - np.exp(-35 * t)) * (1.0 + 0.4 * wobble)
                    
                    sig_out = sig * env * (gain_val * 2.2)
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
        # Fill master_right buffer
        master_right[i] = master_left[i]

    # Apply 3D Auto-Warmth Effect convolving visual resonance volume
    print("[AUDIO] Running 3D tube saturation transduction (optical warmth)...")
    transducer = TubeVactrolWarmthTransducer(SAMPLE_RATE)
    
    # Setup mock state matching screen i=0 (MASTER ATTRACTOR)
    prime_warm = 953467954114363
    state_warm = {
        "Prime": prime_warm, "Base": 3, "Identity": 11, "Ring": (3 + 7) % prime_warm,
        "Monopole": pow(3, 5, prime_warm), "Rod_Dynamo": pow(3, 7, prime_warm),
        "Cone_Dynamo": pow(3, 11, prime_warm), "Manifold": pow(3, 5, 8),
        "Element": 8, "Chin": (3 + 7) % prime_warm
    }
    
    for block_idx in range(0, TOTAL_SAMPLES, 512):
        block_left = master_left[block_idx:block_idx+512]
        block_right = master_right[block_idx:block_idx+512]
        if len(block_left) == 0:
            continue
            
        time_secs = block_idx / SAMPLE_RATE
        sig_ac = block_left - np.mean(block_left)
        rms = np.sqrt(np.mean(sig_ac**2)) if len(sig_ac) > 0 else 0.0
        
        samplings = {
            "pitch": 220.0, "rms": min(1.0, rms * 2.5),
            "crest": 1.0, "zcr": 0.1, "brightness": 0.1,
            "stereo_width": 0.5 * (1.0 + np.sin(time_secs * 2.0 * np.pi * 0.05)),
            "flux": 0.1
        }
        
        steps = 512
        sig_interp = np.interp(np.linspace(0, len(sig_ac) - 1, steps), np.arange(len(sig_ac)), sig_ac)
        pts = get_lissajous_shape(state_warm, time_secs, steps, sig_interp, 160.0 * 0.7, samplings, 0, 0)
        
        # Process block through spatial triode stage
        warmed_left, warmed_right = transducer.process_triode_stage(block_left, pts)
        
        master_left[block_idx:block_idx+512] = warmed_left
        master_right[block_idx:block_idx+512] = warmed_right

    # Normalize audio levels
    peak = np.max(np.abs(master_left))
    if peak > 0.01:
        scale = 0.95 / peak
        master_left *= scale
        master_right *= scale
        for name in channels_audio:
            channels_audio[name] *= scale

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

from concurrent.futures import ProcessPoolExecutor
import io
import colorsys

# --- GLOBAL WORKER STATE FOR MULTIPROCESSING ---
g_audio_wave = None
g_channels = None
g_v_gate_array = None
g_r_ds_array = None
g_temp_array = None
g_disp_x_array = None
g_disp_y_array = None
g_vel_x_array = None
g_vel_y_array = None

def init_worker(audio_wave_val, channels_val, v_gate_val, r_ds_val, temp_val, disp_x_val, disp_y_val, vel_x_val, vel_y_val):
    global g_audio_wave, g_channels, g_v_gate_array, g_r_ds_array, g_temp_array
    global g_disp_x_array, g_disp_y_array, g_vel_x_array, g_vel_y_array
    g_audio_wave = audio_wave_val
    g_channels = channels_val
    g_v_gate_array = v_gate_val
    g_r_ds_array = r_ds_val
    g_temp_array = temp_val
    g_disp_x_array = disp_x_val
    g_disp_y_array = disp_y_val
    g_vel_x_array = vel_x_val
    g_vel_y_array = vel_y_val

def render_single_frame(frame_idx):
    global g_audio_wave, g_channels, g_v_gate_array, g_r_ds_array, g_temp_array
    global g_disp_x_array, g_disp_y_array, g_vel_x_array, g_vel_y_array
    
    time_secs = frame_idx / FPS
    sample_idx = int(time_secs * SAMPLE_RATE)
    
    screens = [
        (640, 480, 160, "MASTER ATTRACTOR"), # Center
        (220, 240, 70, "SUB GROWL"),         # Left 1
        (360, 260, 70, "BASS SHIFT"),        # Left 2
        (500, 280, 70, "ARPEGGIATOR"),       # Left 3
        (780, 280, 70, "KICK DRUM"),         # Right 3
        (920, 260, 70, "SNARE DRUM"),        # Right 2
        (1060, 240, 70, "LEAD SYNTH")        # Right 1
    ]

    im = Image.new("RGBA", (WIDTH, HEIGHT), (11, 15, 25, 255))
    draw = ImageDraw.Draw(im, "RGBA")

    # Draw Menorah structure/branches connecting screens
    draw.line([(640, 680), (640, 480)], fill=(31, 41, 55, 255), width=8)
    draw.arc([140, 240, 1140, 680], 0, 180, fill=(31, 41, 55, 255), width=6)

    # Render each screen
    for i, (cx, cy, r, label) in enumerate(screens):
        # Draw screen base with inner glow outline
        draw.ellipse([cx-r, cy-r, cx+r, cy+r], outline=(55, 65, 81, 255), width=4, fill=(6, 9, 19, 255))
        # Draw glass reflection panels (semi-transparent chords)
        draw.chord([cx-r, cy-r, cx+r, cy+r], start=210, end=330, fill=(255, 255, 255, 8))
        draw.ellipse([cx-r+2, cy-r+2, cx+r-2, cy+r-2], outline=(255, 255, 255, 12), width=1)
        
        sig_segment = None
        prev_sig_segment = None
        
        # Current window signal
        if i == 0:
            sig_segment = g_audio_wave[sample_idx:sample_idx+512]
        elif i == 1:
            sig_segment = g_channels["growl"][sample_idx:sample_idx+512] * 8.0
            rms = np.sqrt(np.mean(sig_segment**2)) if len(sig_segment) > 0 else 0.0
            if rms < 0.04:
                t_rip = np.linspace(0, 4 * np.pi, len(sig_segment) if len(sig_segment) > 0 else 256)
                sig_segment = sig_segment + 0.05 * np.sin(t_rip)
        elif i == 2:
            sig_segment = g_channels["bass"][sample_idx:sample_idx+512]
        elif i == 3:
            sig_segment = g_channels["arp"][sample_idx:sample_idx+512]
        elif i == 4:
            sig_segment = g_channels["kick"][sample_idx:sample_idx+512]
        elif i == 5:
            sig_segment = g_channels["snare"][sample_idx:sample_idx+512]
        elif i == 6:
            sig_segment = g_channels["lead"][sample_idx:sample_idx+512]

        if sig_segment is None or len(sig_segment) < 256:
            sig_segment = np.zeros(256)

        # Previous window signal for stateless flux computation
        prev_idx = max(0, sample_idx - 512)
        if i == 0:
            prev_sig_segment = g_audio_wave[prev_idx:prev_idx+512]
        elif i == 1:
            prev_sig_segment = g_channels["growl"][prev_idx:prev_idx+512] * 8.0
        elif i == 2:
            prev_sig_segment = g_channels["bass"][prev_idx:prev_idx+512]
        elif i == 3:
            prev_sig_segment = g_channels["arp"][prev_idx:prev_idx+512]
        elif i == 4:
            prev_sig_segment = g_channels["kick"][prev_idx:prev_idx+512]
        elif i == 5:
            prev_sig_segment = g_channels["snare"][prev_idx:prev_idx+512]
        elif i == 6:
            prev_sig_segment = g_channels["lead"][prev_idx:prev_idx+512]

        if prev_sig_segment is None or len(prev_sig_segment) < 256:
            prev_sig_segment = np.zeros(256)

        # Calculate current performance step index
        bpm = score.get("tempo", 120)
        step_duration = 60.0 / (bpm * 4.0)
        step = int(time_secs / step_duration)
        
        chan_map = {
            1: "sub_growl",
            2: "bass",
            3: "arpeggiator_filter",
            6: "lead"
        }
        active_freq = 220.0
        
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

        if i in [1, 2, 3, 6]:
            hue = min(280, max(0, int((active_freq - 55.0) / 800.0 * 280.0)))
            r_rgb, g_rgb, b_rgb = colorsys.hsv_to_rgb(hue / 360.0, 0.95, 0.95)
            line_color = (int(r_rgb * 255), int(g_rgb * 255), int(b_rgb * 255))
        else:
            color_map = {
                0: (139, 92, 246),  # Master
                4: (16, 185, 129),  # Kick
                5: (239, 68, 68),   # Snare
            }
            line_color = color_map.get(i, (6, 182, 212))

        prime = 953467954114363
        base_val = [3, 2, 5, 7, 11, 13, 17][i]
        sig_val = [5, 11, 13, 17, 19, 23, 29][i]
        secret_val = [7, 17, 19, 23, 29, 31, 37][i]
        ident_val = [11, 23, 29, 31, 37, 41, 43][i]
        element_val = base_val + sig_val
        
        state = {
            "Prime": prime,
            "Base": base_val,
            "Identity": ident_val,
            "Ring": (base_val + secret_val) % prime,
            "Monopole": pow(base_val, sig_val, prime),
            "Rod_Dynamo": pow(base_val, secret_val, prime),
            "Cone_Dynamo": pow(base_val, ident_val, prime),
            "Manifold": pow(base_val, sig_val, element_val),
            "Element": element_val,
            "Chin": (base_val + secret_val) % prime
        }

        sig_ac = sig_segment - np.mean(sig_segment)
        prev_sig_ac = prev_sig_segment - np.mean(prev_sig_segment)
        
        rms = np.sqrt(np.mean(sig_ac**2)) if len(sig_ac) > 0 else 0.0
        prev_rms = np.sqrt(np.mean(prev_sig_ac**2)) if len(prev_sig_ac) > 0 else 0.0
        peak = np.max(np.abs(sig_ac)) if len(sig_ac) > 0 else 0.0
        crest_factor = peak / (rms + 1e-6)
        zero_crossings = np.sum(np.diff(np.sign(sig_ac)) != 0) if len(sig_ac) > 0 else 0
        
        flux_val = abs(rms - prev_rms)
        
        samplings = {
            "pitch": active_freq,
            "rms": min(1.0, rms * 2.5),
            "crest": min(1.0, crest_factor / 12.0),
            "zcr": min(1.0, zero_crossings / 120.0),
            "brightness": min(1.0, zero_crossings / 90.0),
            "stereo_width": 0.5 * (1.0 + np.sin(time_secs * 2.0 * np.pi * 0.05 + i)),
            "flux": min(1.0, flux_val * 15.0)
        }
        
        steps = 2048
        sig_interp = np.interp(np.linspace(0, len(sig_ac) - 1, steps), np.arange(len(sig_ac)), sig_ac)
        
        lut_shift = int((active_freq - 110.0) * 10.0)
        pts = get_lissajous_shape(state, time_secs, steps, sig_interp, r * 0.7, samplings, lut_shift, i, is_visual=True)
        
        cx_phys = cx
        cy_phys = cy
        
        glow_width_factor = 1.0
        glow_alpha_factor = 1.0

        smoothed_pts = []
        for j in range(len(pts)):
            p_prev = pts[j - 1] if j > 0 else pts[j]
            p_curr = pts[j]
            p_next = pts[j + 1] if j < len(pts) - 1 else pts[j]
            smoothed_x = (p_prev[0] + p_curr[0] + p_next[0]) / 3.0
            smoothed_y = (p_prev[1] + p_curr[1] + p_next[1]) / 3.0
            smoothed_pts.append((cx_phys + smoothed_x, cy_phys + smoothed_y))
        points = smoothed_pts

        if len(points) > 1:
            # Liang-Barsky visual containment bounding box clipping
            xmin, xmax = cx - r, cx + r
            ymin, ymax = cy - r, cy + r
            for seg_idx in range(len(points) - 1):
                p1 = points[seg_idx]
                p2 = points[seg_idx + 1]
                clipped = liang_barsky_clip(p1[0], p1[1], p2[0], p2[1], xmin, ymin, xmax, ymax)
                if clipped:
                    c1 = (clipped[0], clipped[1])
                    c2 = (clipped[2], clipped[3])
                    # Draw Neon Glow segment: wide faint line, medium line, white-hot core line
                    draw.line([c1, c2], fill=(line_color[0], line_color[1], line_color[2], 50), width=6)
                    draw.line([c1, c2], fill=(line_color[0], line_color[1], line_color[2], 120), width=4)
                    draw.line([c1, c2], fill=(255, 255, 255, 255), width=2)

        draw.text((cx - 30, cy + r - 20), label, fill=(156, 163, 175, 200))
        
        # Draw visual spectrum sub-meter bars below the screen label
        num_bars = 5
        bar_width = 2
        max_height = 14
        for b_idx in range(num_bars):
            bx = cx - ((num_bars - 1) * 8) // 2 + b_idx * 8
            # Modulate height by channel RMS and simulated frequency band
            band_rms = rms * (0.3 + 0.7 * math.sin(time_secs * 25.0 + b_idx * 1.5))
            bar_h = int(max(2, min(max_height, band_rms * max_height * 2.5)))
            by_start = cy + r + 2
            by_end = by_start + bar_h
            # Draw glowing bar
            draw.line([(bx, by_start), (bx, by_end)], fill=(line_color[0], line_color[1], line_color[2], 60), width=bar_width+2)
            draw.line([(bx, by_start), (bx, by_end)], fill=(255, 255, 255, 220) if i == 0 else (line_color[0], line_color[1], line_color[2], 255), width=bar_width)

        # Overlay output stage FET accumulator diagnostics on center screen (MASTER ATTRACTOR)
        if i == 0:
            vg = g_v_gate_array[frame_idx]
            rd = g_r_ds_array[frame_idx]
            tp = g_temp_array[frame_idx]
            draw.text((cx - 75, cy + r + 18), f"FET V={vg:.2f}V R={rd:.1f}O T={tp:.1f}K", fill=(212, 175, 55, 255))
            
            # Formant-Keyed Transducer Bloom on Menorah Arc node (640, 680)
            if vg > 0.02:
                # Modulate intensity using zero-crossing rate (formant keying indicator)
                zcr_mod = samplings.get("zcr", 0.0)
                intensity = min(1.0, vg * (1.0 + 1.2 * zcr_mod))
                bloom_r = int(intensity * 35.0)
                
                # Dynamic mix detection to determine visual shape classification
                growl_r = np.sqrt(np.mean(g_channels["growl"][sample_idx:sample_idx+512]**2)) if sample_idx < len(g_channels["growl"]) else 0.0
                lead_r  = np.sqrt(np.mean(g_channels["lead"][sample_idx:sample_idx+512]**2)) if sample_idx < len(g_channels["lead"]) else 0.0
                kick_r  = np.sqrt(np.mean(g_channels["kick"][sample_idx:sample_idx+512]**2)) if sample_idx < len(g_channels["kick"]) else 0.0
                snare_r = np.sqrt(np.mean(g_channels["snare"][sample_idx:sample_idx+512]**2)) if sample_idx < len(g_channels["snare"]) else 0.0
                
                dominant = "silent"
                max_r = 0.01
                if growl_r > max_r:
                    dominant = "growl"
                    max_r = growl_r
                if lead_r > max_r:
                    dominant = "lead"
                    max_r = lead_r
                if kick_r > max_r or snare_r > max_r:
                    dominant = "drums"
                
                for ring_w in range(1, 4):
                    alpha = int(140 * intensity / ring_w)
                    r_sz = bloom_r * ring_w
                    
                    if dominant == "growl":
                        # Wobbly horizontal ellipse representing sub-bass grounding
                        draw.ellipse([640 - r_sz, 680 - r_sz // 3, 640 + r_sz, 680 + r_sz // 3], 
                                     outline=(212, 175, 55, alpha), width=1)
                    elif dominant == "lead":
                        # 5-point star starburst representing vocal formant resonance
                        pts_star = []
                        for star_k in range(10):
                            star_rad = r_sz if star_k % 2 == 0 else r_sz // 2
                            angle_star = star_k * (2.0 * np.pi / 10.0) + time_secs * 4.0
                            pts_star.append((640 + int(star_rad * math.cos(angle_star)), 
                                             680 + int(star_rad * 0.5 * math.sin(angle_star))))
                        draw.polygon(pts_star, outline=(212, 175, 55, alpha))
                    elif dominant == "drums":
                        # Concentric sharp shockwave circle representing transient beats
                        draw.ellipse([640 - r_sz // 2, 680 - r_sz // 2, 640 + r_sz // 2, 680 + r_sz // 2], 
                                     outline=(255, 255, 255, alpha), width=1)
                    else:
                        # Standard default concentric oval
                        draw.ellipse([640 - r_sz // 2, 680 - r_sz // 4, 640 + r_sz // 2, 680 + r_sz // 4], 
                                     outline=(212, 175, 55, alpha), width=1)

    # Convert back to RGB for output compatibility
    im = im.convert("RGB")
    im_blur = im.filter(ImageFilter.GaussianBlur(1.5))
    im = Image.blend(im, im_blur, 0.4)

    buf = io.BytesIO()
    im.save(buf, format="PNG")
    return buf.getvalue()

# --- VIDEO GENERATION ---
def generate_video(audio_wave, channels):
    print("[VIDEO] Starting optimized parallel 3D Menorah rendering...")
    
    cmd = [
        "ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", str(FPS),
        "-i", "-", "-i", "assets/bionika/eye_of_the_tiger.wav",
        "-c:v", "libx264", "-pix_fmt", "yuv420p", "-shortest",
        "assets/bionika/eye_of_the_tiger.mp4"
    ]
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE)

    # Precompute master FET arrays sequentially on main process thread
    classifier = MasterStageAccumulatorClassifier(SAMPLE_RATE)
    v_gate_array = np.zeros(TOTAL_FRAMES)
    r_ds_array = np.zeros(TOTAL_FRAMES)
    temp_array = np.zeros(TOTAL_FRAMES)
    
    print("[VIDEO] Precomputing master stage FET accumulator telemetry...")
    for idx in range(TOTAL_FRAMES):
        time_secs = idx / FPS
        sample_idx = int(time_secs * SAMPLE_RATE)
        block = audio_wave[sample_idx:sample_idx+512]
        
        # Calculate active track activity RMS levels (subtracting block means to align with AC coupling)
        growl_blk = channels["growl"][sample_idx:sample_idx+512] if sample_idx < len(channels["growl"]) else np.array([])
        bass_blk  = channels["bass"][sample_idx:sample_idx+512] if sample_idx < len(channels["bass"]) else np.array([])
        arp_blk   = channels["arp"][sample_idx:sample_idx+512] if sample_idx < len(channels["arp"]) else np.array([])
        lead_blk  = channels["lead"][sample_idx:sample_idx+512] if sample_idx < len(channels["lead"]) else np.array([])
        kick_blk  = channels["kick"][sample_idx:sample_idx+512] if sample_idx < len(channels["kick"]) else np.array([])
        snare_blk = channels["snare"][sample_idx:sample_idx+512] if sample_idx < len(channels["snare"]) else np.array([])

        features = {
            "growl": np.sqrt(np.mean((growl_blk - np.mean(growl_blk))**2)) if len(growl_blk) > 0 else 0.0,
            "bass":  np.sqrt(np.mean((bass_blk - np.mean(bass_blk))**2)) if len(bass_blk) > 0 else 0.0,
            "arp":   np.sqrt(np.mean((arp_blk - np.mean(arp_blk))**2)) if len(arp_blk) > 0 else 0.0,
            "lead":  np.sqrt(np.mean((lead_blk - np.mean(lead_blk))**2)) if len(lead_blk) > 0 else 0.0,
            "drums": np.sqrt(np.mean((kick_blk - np.mean(kick_blk))**2 + (snare_blk - np.mean(snare_blk))**2)) if len(kick_blk) > 0 else 0.0
        }
        v_gate, r_ds, temp = classifier.process_block(block, features)
        
        # Live Optical/Electrical Verification Guard: Throw fault on parameter collapse (Rule 12 protection)
        if r_ds < 1.0 or r_ds > 30.0:
            raise ValueError(f"FET Diagnostic Fault: Channel resistance {r_ds:.2f} ohms violates boundary limits!")
        if temp < 290.0 or temp > 450.0:
            raise ValueError(f"FET Diagnostic Fault: Junction temperature {temp:.2f}K violates thermal limits!")
            
        # Live Phase Cancellation Guard (exclude growl and bass due to master AC-coupling low-frequency roll-off)
        sum_features = features["lead"] + features["drums"]
        master_rms = np.sqrt(np.mean(block**2)) if len(block) > 0 else 0.0
        if sum_features > 1.2 and master_rms < 0.005 * sum_features:
            raise ValueError(f"Phase Cancellation Fault: Master mix collapsed to {master_rms:.6f} despite active channels ({sum_features:.6f})!")
            
        v_gate_array[idx] = v_gate
        r_ds_array[idx] = r_ds
        temp_array[idx] = temp

    # Precompute camera physics states for all frames sequentially (spider mount elastic suspension)
    print("[VIDEO] Precomputing camera spider mount suspension physics...")
    disp_x_array = np.zeros((7, TOTAL_FRAMES))
    disp_y_array = np.zeros((7, TOTAL_FRAMES))
    vel_x_array = np.zeros((7, TOTAL_FRAMES))
    vel_y_array = np.zeros((7, TOTAL_FRAMES))
    
    dt_phys = 1.0 / 30.0
    m_phys = 1.0
    k_phys = 220.0
    d_phys = 9.0  # sub-arachnoid fluid damping
    
    for i in range(7):
        disp_x, disp_y = 0.0, 0.0
        vel_x, vel_y = 0.0, 0.0
        
        if i == 0:
            wave_buf = audio_wave
        elif i == 1:
            wave_buf = channels["growl"] * 8.0
        elif i == 2:
            wave_buf = channels["bass"]
        elif i == 3:
            wave_buf = channels["arp"]
        elif i == 4:
            wave_buf = channels["kick"]
        elif i == 5:
            wave_buf = channels["snare"]
        elif i == 6:
            wave_buf = channels["lead"]
            
        for f in range(TOTAL_FRAMES):
            sample_idx = f * 1470
            sig_seg = wave_buf[sample_idx:sample_idx+512] if sample_idx < len(wave_buf) else np.zeros(512)
            prev_idx = max(0, sample_idx - 512)
            prev_sig_seg = wave_buf[prev_idx:prev_idx+512] if prev_idx < len(wave_buf) else np.zeros(512)
            
            rms = np.sqrt(np.mean(sig_seg**2)) if len(sig_seg) > 0 else 0.0
            prev_rms = np.sqrt(np.mean(prev_sig_seg**2)) if len(prev_sig_seg) > 0 else 0.0
            flux_val = abs(rms - prev_rms)
            
            # PMG hit impact force proportional to transient flux
            f_impact_x = flux_val * 750.0 * (1.0 if f % 2 == 0 else -1.0)
            f_impact_y = flux_val * 750.0 * (-1.0 if f % 3 == 0 else 1.0)
            
            # Pure IIR linear decay filter (non-spring dynamics) to calculate displacements organically
            target_disp_x = f_impact_x * 0.04
            target_disp_y = f_impact_y * 0.04
            
            # Smooth displacements and derive velocity from IIR differences (re-stabilizes to rest)
            new_disp_x = disp_x * 0.82 + target_disp_x * 0.18
            new_disp_y = disp_y * 0.82 + target_disp_y * 0.18
            
            vel_x = (new_disp_x - disp_x) / dt_phys
            vel_y = (new_disp_y - disp_y) / dt_phys
            
            disp_x = new_disp_x
            disp_y = new_disp_y
            
            disp_x_array[i, f] = disp_x
            disp_y_array[i, f] = disp_y
            vel_x_array[i, f] = vel_x
            vel_y_array[i, f] = vel_y

    # Spawn process pool for parallel frame generation
    with ProcessPoolExecutor(initializer=init_worker, initargs=(audio_wave, channels, v_gate_array, r_ds_array, temp_array, disp_x_array.flatten(), disp_y_array.flatten(), vel_x_array.flatten(), vel_y_array.flatten())) as executor:
        futures = {executor.submit(render_single_frame, idx): idx for idx in range(TOTAL_FRAMES)}
        
        # Write frames sequentially as they are rendered
        for idx in range(TOTAL_FRAMES):
            # Find the corresponding future
            for fut, f_idx in futures.items():
                if f_idx == idx:
                    png_bytes = fut.result()
                    process.stdin.write(png_bytes)
                    break
            
            if idx % 90 == 0:
                print(f"[VIDEO] Processed and piped frame {idx}/{TOTAL_FRAMES}...")
                
    process.stdin.close()
    process.wait()
    print("[VIDEO] Successfully rendered MP4 to assets/bionika/eye_of_the_tiger.mp4")

if __name__ == "__main__":
    os.makedirs("assets/bionika", exist_ok=True)
    audio_wave, channels = synthesize_audio()
    generate_video(audio_wave, channels)
