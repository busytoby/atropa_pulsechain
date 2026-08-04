#!/usr/bin/env python3
import re
import wave
import struct
import math
import numpy as np

def generate_audio():
    log_path = "chorus_simulation.log"
    audio_path = "bear_chorus_track.wav"
    sample_rate = 44100
    dt = 1.0 / sample_rate
    
    # Read simulation logs to extract pitch transitions and voltage bounds
    transitions = []
    pattern = re.compile(r"Bear (\w+) spoke to Bear (\w+) -> (\w+) current voltage: ([\d\.]+) V.*?Pitch: ([\d\.]+) Hz")
    with open(log_path, "r") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                transitions.append({
                    "voltage": float(match.group(4)),
                    "pitch": float(match.group(5))
                })
                
    if not transitions:
        transitions = [{"voltage": 1.0, "pitch": 220.0}] * 313
        
    duration_per_step = 0.08  # Slow down step to 80 ms to appreciate FM harmonics
    total_audio = []
    
    # Initialize 2-Mass biological membrane Verlet state parameters
    x1, x1_prev = 0.0, 0.0
    x2, x2_prev = 0.0, 0.0
    
    # Dampings & masses
    c1, c2 = 1.0, 0.8
    m1, m2 = 0.1, 0.08
    Kc = 140.0
    Ps = 0.5
    epibar1, hypobar1 = 1400.0, 3600.0
    
    for idx, step in enumerate(transitions):
        length = int(sample_rate * duration_per_step)
        step_signal = []
        
        for s in range(length):
            t_curr = idx * duration_per_step + s * dt
            
            # Map frequency to stiffness scaling factor
            stiff_scale = (step["pitch"] / 440.0) ** 2
            epibar = epibar1 * stiff_scale
            hypobar = hypobar1 * stiff_scale
            
            stiffness1 = epibar if x1 > 0.0 else hypobar
            stiffness2 = epibar * 0.9 if x2 > 0.0 else hypobar * 0.9
            
            # Pressure ramp-up/down to prevent clicks
            ramp = 1.0
            if s < 400:
                ramp = s / 400.0
            elif s > length - 400:
                ramp = (length - s) / 400.0
                
            f_p1 = Ps * 0.2 * ramp if x1 > 0.0 else 0.0
            f_p2 = Ps * 0.15 * ramp if x2 > 0.0 else 0.0
            
            v1 = (x1 - x1_prev) / dt
            v2 = (x2 - x2_prev) / dt
            
            acc1 = (f_p1 - stiffness1 * x1 - c1 * v1 + Kc * (x2 - x1)) / m1
            acc2 = (f_p2 - stiffness2 * x2 - c2 * v2 + Kc * (x1 - x2)) / m2
            
            x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
            x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
            
            x1_prev, x1 = x1, max(-0.4, min(0.8, x1_next))
            x2_prev, x2 = x2, max(-0.4, min(0.8, x2_next))
            
            flow = (max(x1, 0.0) ** 2)
            
            # Formant vowel filter mapping
            f1, f2 = 270.0, 2290.0 # Standard "EE" vowel formants
            formant_excitation = flow * (
                0.65 * math.sin(2.0 * math.pi * f1 * t_curr) +
                0.35 * math.sin(2.0 * math.pi * f2 * t_curr)
            )
            
            # Sub-surface tissue scattering equivalent
            sss_thickness = 0.5 + 0.2 * math.sin(2.0 * math.pi * 8.0 * t_curr)
            sss_decay = math.exp(-sss_thickness * 0.25)
            sss_diffusion = 1.0 / (1.0 + (sss_thickness ** 2) / (1.5 ** 2))
            formant_excitation *= sss_decay * sss_diffusion
            
            # Glockenspiel accents on voltage spikes
            glock = 0.0
            if step["voltage"] > 2.5:
                glock = 0.25 * math.sin(2.0 * math.pi * step["pitch"] * 4.0 * t_curr) * math.exp(-s * dt * 50.0)
                
            step_signal.append(0.5 * formant_excitation + glock)
            
        total_audio.extend(step_signal)
        
    audio_np = np.array(total_audio)
    max_val = np.max(np.abs(audio_np))
    if max_val > 1.0:
        audio_np /= max_val
        
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated standard syrinx-based bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
