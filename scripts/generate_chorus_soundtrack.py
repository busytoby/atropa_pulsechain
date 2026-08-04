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
        
    duration_per_step = 0.08  # 80 ms per conversational step
    total_samples_per_step = int(sample_rate * duration_per_step)
    total_audio = []
    
    # LPC reflection coefficient vowel profiles from standard synthesizer
    k_ee = [0.60, -0.13, 0.34, -0.10, 0.05, -0.08, -0.04, -0.02, -0.01]
    k_ah = [0.25, -0.35, 0.20, -0.15, 0.08, -0.04, 0.02, -0.01, 0.00]
    k_oo = [0.75, -0.45, 0.50, -0.25, 0.12, -0.06, 0.03, -0.01, 0.00]
    
    # LPC recursive filter state
    delay_line = np.zeros(10)
    
    # Pre-generate noise excitation
    noise = np.random.uniform(-1.0, 1.0, total_samples_per_step * len(transitions))
    
    for idx, step in enumerate(transitions):
        step_signal = []
        
        # Select coefficient based on bear speaker personality
        if step["speaker"] == "Trusty":
            K = k_ee
        elif step["speaker"] == "Aggro":
            K = k_ah
        elif step["speaker"] == "Skeptic":
            K = k_oo
        else:
            K = [0.5 * (k_ee[i] + k_ah[i]) for i in range(9)]
            
        for s in range(total_samples_per_step):
            global_sample = idx * total_samples_per_step + s
            time_sec = global_sample / sample_rate
            
            # Voiced excitation with vibrato/sweep
            f_carrier = step["pitch"]
            f_mod = 6.0
            beta = 0.08
            vibrato = beta * math.cos(2.0 * math.pi * f_mod * time_sec)
            period = int(sample_rate / (f_carrier * (1.0 + vibrato)))
            
            # Simple glottal impulse flow excitation
            excitation = 1.0 if (global_sample % period == 0) else 0.0
            
            # Recursive Lattice Filter
            forward = excitation
            for i in range(8, -1, -1):
                forward = forward - K[i] * delay_line[i]
                delay_line[i + 1] = delay_line[i] + K[i] * forward
            delay_line[0] = forward
            
            step_signal.append(forward)
            
        total_audio.extend(step_signal)
        
    audio_np = np.array(total_audio)
    
    # Lip radiation filter differentiator
    for s in range(len(audio_np) - 1, 0, -1):
        audio_np[s] = audio_np[s] - 0.95 * audio_np[s - 1]
        
    audio_np -= np.mean(audio_np)
    max_val = np.max(np.abs(audio_np))
    if max_val > 0.0:
        audio_np /= max_val
        
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated standard LPC lattice bear chorus speech at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
