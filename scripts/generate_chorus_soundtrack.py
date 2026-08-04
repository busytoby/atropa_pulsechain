#!/usr/bin/env python3
import re
import wave
import struct
import math
import random
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
                    "speaker": match.group(1),
                    "voltage": float(match.group(4)),
                    "pitch": float(match.group(5))
                })
                
    if not transitions:
        transitions = [{"speaker": "Trusty", "voltage": 1.0, "pitch": 220.0}] * 313
        
    duration_per_step = 0.08  # Slow down step to 80 ms to appreciate FM harmonics
    total_audio = []
    
    # Harmonic ratios for standard drawbars: sub-octave, 5th, octave, 12th, 15th, etc.
    ratios = [0.5, 1.498, 1.0, 2.0, 2.996, 4.0, 5.039, 6.0]
    
    # Define drawbar profiles for the 5 bears
    bear_drawbars = {
        "Trusty":  [8, 8, 8, 4, 0, 0, 0, 0], # Pure tone
        "Aggro":   [8, 5, 8, 8, 8, 8, 8, 8], # Rich, aggressive harmonics
        "Skeptic": [0, 8, 0, 8, 0, 8, 0, 8], # Crooked harmonics
        "Eerie":   [8, 0, 0, 0, 0, 0, 8, 8], # Uncanny whistle harmonics
        "Coop":    [8, 8, 8, 8, 8, 0, 0, 0]  # Balanced tone
    }
    
    # Tonewheel state
    shaft_velocity = 0.0
    
    for idx, step in enumerate(transitions):
        length = int(sample_rate * duration_per_step)
        step_signal = []
        
        db = bear_drawbars.get(step["speaker"], [8, 8, 8, 4, 0, 0, 0, 0])
        
        # Calculate contact wear/grit based on voltage
        contact_wear = 0.05 * (step["voltage"] / 10.0)
        
        for s in range(length):
            t_curr = idx * duration_per_step + s * dt
            
            # Electromechanical clutch spin-up / spin-down torque simulation
            target_vel = 1.0
            slip = target_vel - shaft_velocity
            torque = slip * 15.0 # fast responsiveness
            shaft_velocity += torque * dt
            if shaft_velocity > 1.0:
                shaft_velocity = 1.0
                
            base_freq = step["pitch"] * shaft_velocity
            
            # Mix drawbars with contact grit/noise
            mixed_val = 0.0
            for i in range(8):
                if db[i] <= 0:
                    continue
                # Add contact wear grit
                noise = (random.random() - 0.5) * contact_wear
                level = max(0.0, (db[i] / 8.0) + noise)
                
                # Sine wave generator for the tonewheel frequency
                mixed_val += level * math.sin(2.0 * math.pi * (base_freq * ratios[i]) * t_curr)
                
            # Saturate signal using tanhf
            saturated = math.tanh(mixed_val * 0.4) * 0.3
            step_signal.append(saturated)
            
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
        
    print(f"Generated electromechanical tonewheel bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
