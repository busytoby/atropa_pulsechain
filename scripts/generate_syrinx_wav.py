import os
import math
import wave
import struct
import numpy as np

SAMPLE_RATE = 44100
DURATION = 32.0  # Let's match the 32 seconds compositor timeline
num_samples = int(SAMPLE_RATE * DURATION)
dt = 1.0 / SAMPLE_RATE

# Dictionary of biological syrinx preset parameters derived from sweep
PRESETS = {
    # Lower mass/dampings to prevent clipping clicks
    "whistle": {"m1": 0.06, "m2": 0.05, "epibar1": 1500.0, "hypobar1": 450.0, "Kc": 5.0, "c1": 0.8, "Ps": 0.30},
    "warbler": {"m1": 0.08, "m2": 0.06, "epibar1": 1200.0, "hypobar1": 1000.0, "Kc": 8.0, "c1": 0.9, "Ps": 0.40},
    "thrush":  {"m1": 0.07, "m2": 0.05, "epibar1": 600.0, "hypobar1": 450.0, "Kc": 10.0, "c1": 1.0, "Ps": 0.35},
    "growl":   {"m1": 0.30, "m2": 0.25, "epibar1": 1800.0, "hypobar1": 500.0, "Kc": 100.0, "c1": 2.5, "Ps": 0.50}
}

# Phonetic formant transitions to spell "A-U-N-C-I-E-N-T" (EE -> AH -> OO -> EE vowels)
def get_vowel_formants(t_sec):
    # Map time to specific vowel transitions
    cycle = t_sec % 8.0
    if cycle < 2.0: # "EE"
        return 270.0, 2290.0
    elif cycle < 4.0: # "AH"
        return 730.0, 1090.0
    elif cycle < 6.0: # "OO"
        return 300.0, 870.0
    else: # "EE"
        return 270.0, 2290.0

def simulate_syrinx(preset_name, start_t, end_t, pitch_mod_freq=0.0, base_melody_freq=440.0):
    p = PRESETS[preset_name]
    start_s = int(start_t * SAMPLE_RATE)
    end_s = int(end_t * SAMPLE_RATE)
    length = end_s - start_s
    
    out_signal = np.zeros(length)
    
    # 2-Mass coupled biological membrane Verlet state (initialized softly to prevent initial click)
    x1, x1_prev = 0.0, 0.0
    x2, x2_prev = 0.0, 0.0
    
    # Dampings
    c1 = p["c1"]
    c2 = c1 * 0.8
    
    # Acoustic tract load
    r_tract = 0.95
    y_tract, y_tract_prev = 0.0, 0.0
    
    for s in range(length):
        t_curr = start_t + s * dt
        
        # Smooth pitch/melody sweep: sliding frequency
        # Creates thrush/warbler bird song melodies (chirps) instead of static notes
        vibrato = 0.25 * math.sin(2.0 * math.pi * pitch_mod_freq * t_curr)
        freq_target = base_melody_freq * (1.0 + vibrato)
        
        # Map frequency to stiffness scaling factor
        stiff_scale = (freq_target / 440.0) ** 2
        epibar = p["epibar1"] * stiff_scale
        hypobar = p["hypobar1"] * stiff_scale
        
        stiffness1 = epibar if x1 > 0.0 else hypobar
        stiffness2 = epibar * 0.9 if x2 > 0.0 else hypobar * 0.9
        
        # Soft pressure ramp-up/down at boundaries to eliminate click pops
        ramp = 1.0
        if s < 2000: # 45ms ramp up
            ramp = s / 2000.0
        elif s > length - 2000: # 45ms ramp down
            ramp = (length - s) / 2000.0
            
        f_p1 = p["Ps"] * 0.2 * ramp if x1 > 0.0 else 0.0
        f_p2 = p["Ps"] * 0.15 * ramp if x2 > 0.0 else 0.0
        
        # Velocities
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        
        # Coupled accelerations
        acc1 = (f_p1 - stiffness1 * x1 - c1 * v1 + p["Kc"] * (x2 - x1)) / p["m1"]
        acc2 = (f_p2 - stiffness2 * x2 - c2 * v2 + p["Kc"] * (x1 - x2)) / p["m2"]
        
        # Verlet Integration step (softer boundary limits to prevent hard clipping clicks)
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.4, min(0.8, x1_next))
        x2_prev, x2 = x2, max(-0.4, min(0.8, x2_next))
        
        # Output flow
        flow = (max(x1, 0.0) ** 2)
        
        # Formant filter mapping vowels/words (EE -> AH -> OO -> EE)
        f1, f2 = get_vowel_formants(t_curr)
        formant_excitation = flow * (
            0.65 * math.sin(2.0 * math.pi * f1 * t_curr) +
            0.35 * math.sin(2.0 * math.pi * f2 * t_curr)
        )
        
        # Sub-surface tissue scattering equivalent (vocal tract tissue transmissive scattering)
        # scatter_radius = 1.5, absorption_coeff = 0.25
        sss_thickness = 0.5 + 0.2 * math.sin(2.0 * math.pi * 8.0 * t_curr)
        sss_decay = math.exp(-sss_thickness * 0.25)
        sss_diffusion = 1.0 / (1.0 + (sss_thickness ** 2) / (1.5 ** 2))
        formant_excitation *= sss_decay * sss_diffusion
        
        # Simple waveguide mouth acoustic resonance filtering
        y_next = formant_excitation + r_tract * y_tract - 0.9 * y_tract_prev
        y_tract_prev, y_tract = y_tract, y_next
        
        out_signal[s] = y_next
        
    # High-pass filter to make bird calls bright
    for s in range(length - 1, 0, -1):
        out_signal[s] = out_signal[s] - 0.98 * out_signal[s-1]
        
    out_signal -= np.mean(out_signal)
    if np.max(np.abs(out_signal)) > 0:
        out_signal /= np.max(np.abs(out_signal))
        
    return out_signal

def main():
    print("=== SYNTHESIZING BIOLOGICAL SYRINX SOUNDTRACK ===")
    
    # Pre-allocate master audio buffer (mono)
    master_audio = np.zeros(num_samples)
    
    # Arrange bird calls sequentially throughout the 32 seconds performance timeline
    # 0s - 4s: Slow Whistle slide
    print("-> Rendering Whistle slide...")
    s_whistle = simulate_syrinx("whistle", 0.0, 4.0, pitch_mod_freq=2.0, base_melody_freq=680.0)
    master_audio[0:len(s_whistle)] += s_whistle * 0.4
    
    # 6s - 12s: Thrush melodic phrase (spells vowel formants)
    print("-> Rendering Thrush melody...")
    s_thrush = simulate_syrinx("thrush", 6.0, 12.0, pitch_mod_freq=5.0, base_melody_freq=520.0)
    idx_start = int(6.0 * SAMPLE_RATE)
    master_audio[idx_start:idx_start + len(s_thrush)] += s_thrush * 0.35
    
    # 15s - 22s: Fast Warbler vibrato trill
    print("-> Rendering Warbler trill...")
    s_warbler = simulate_syrinx("warbler", 15.0, 22.0, pitch_mod_freq=18.0, base_melody_freq=880.0)
    idx_start = int(15.0 * SAMPLE_RATE)
    master_audio[idx_start:idx_start + len(s_warbler)] += s_warbler * 0.3
    
    # 24s - 30s: Low Growls
    print("-> Rendering Growl phase...")
    s_growl = simulate_syrinx("growl", 24.0, 30.0, pitch_mod_freq=1.0, base_melody_freq=180.0)
    idx_start = int(24.0 * SAMPLE_RATE)
    master_audio[idx_start:idx_start + len(s_growl)] += s_growl * 0.45

    # Normalize output
    master_audio -= np.mean(master_audio)
    if np.max(np.abs(master_audio)) > 0:
        master_audio /= np.max(np.abs(master_audio))
        
    output_wav = "syrinx_ambient_resonance.wav"
    print(f"[DSP] Writing master biological audio to {output_wav}...")
    
    # Save WAV
    byte_data = bytearray(44 + len(master_audio) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(master_audio) * 2).to_bytes(4, "little")
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
    byte_data[40:44] = int(len(master_audio) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in master_audio:
        v = int(val * 32767) if val >= 0 else int(val * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(output_wav, "wb") as f:
        f.write(byte_data)
        
    print("[SUCCESS] Syrinx bio-acoustic audio compiled successfully.")

if __name__ == "__main__":
    main()
