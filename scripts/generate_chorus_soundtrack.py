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
        
    duration_per_step = 0.5  # Polite response timing: 500 ms per conversational turn
    total_samples = int(sample_rate * duration_per_step * len(transitions))
    
    # 1. Steady Drum & Bass Backing Soundtrack (120 BPM, 4/4)
    bpm = 120
    beat_dur = 60.0 / bpm
    step_dur = beat_dur / 4.0
    step_samples = int(sample_rate * step_dur)
    
    # Harmonic ratios for standard drawbars: sub-octave, 5th, octave, 12th, 15th, etc.
    ratios = [0.5, 1.498, 1.0, 2.0, 2.996, 4.0, 5.039, 6.0]
    
    # Define drawbar profiles for the 5 bears
    bear_drawbars = {
        "Trusty":  [8, 8, 8, 4, 0, 0, 0, 0],
        "Aggro":   [8, 5, 8, 8, 8, 8, 8, 8],
        "Skeptic": [0, 8, 0, 8, 0, 8, 0, 8],
        "Eerie":   [8, 0, 0, 0, 0, 0, 8, 8],
        "Coop":    [8, 8, 8, 8, 8, 0, 0, 0]
    }
    
    audio_np = np.zeros(total_samples, dtype=np.float32)
    noise_state = 12345
    
    for s in range(total_samples):
        t_curr = s * dt
        step_idx = s // step_samples
        beat_idx = step_idx // 4
        step_in_beat = step_idx % 4
        sample_in_step = s % step_samples
        
        # 1a. Kick drum on beats 0 and 2
        kick = 0.0
        if step_in_beat == 0:
            t_kick = sample_in_step * dt
            if t_kick < 0.15:
                k_freq = 110.0 * math.exp(-t_kick * 30.0) + 40.0
                kick = 0.4 * math.sin(2.0 * math.pi * k_freq * t_kick) * math.exp(-t_kick * 9.0)
                
        # 1b. Snare drum on beats 1 and 3 (white noise)
        snare = 0.0
        if step_in_beat == 2:
            t_snare = sample_in_step * dt
            if t_snare < 0.2:
                noise_state = (noise_state * 1103515245 + 12345) & 0x7fffffff
                noise_val = ((noise_state / 0x7fffffff) * 2.0) - 1.0
                snare = 0.25 * noise_val * math.exp(-t_snare * 14.0)
                
        # 1c. Steady rolling backing Bassline
        bass_freq = 55.0 # Low G/A baseline
        if beat_idx % 4 == 1:
            bass_freq = 65.4 # C
        elif beat_idx % 4 == 2:
            bass_freq = 73.4 # D
        elif beat_idx % 4 == 3:
            bass_freq = 58.2 # Bb
            
        bass_phase = 2.0 * math.pi * bass_freq * t_curr
        bass = 0.3 * math.sin(bass_phase) + 0.1 * math.sin(bass_phase * 2.0)
        
        audio_np[s] += (kick + snare + bass) * 0.4
        
    # 2. Add polite conversational tonewheel dialogue voices
    # Each bear only speaks during their active step window
    step_samples_turn = int(sample_rate * duration_per_step)
    
    for idx, step in enumerate(transitions):
        start_sample = idx * step_samples_turn
        end_sample = start_sample + step_samples_turn
        
        db = bear_drawbars.get(step["speaker"], [8, 8, 8, 4, 0, 0, 0, 0])
        contact_wear = 0.05 * (step["voltage"] / 10.0)
        
        for s in range(step_samples_turn):
            global_sample = start_sample + s
            if global_sample >= total_samples:
                break
            t_curr = global_sample * dt
            
            # Spin up to the speaker's pitch
            base_freq = step["pitch"]
            
            # Mix drawbars
            mixed_val = 0.0
            for i in range(8):
                if db[i] <= 0:
                    continue
                # Add contact wear grit
                noise_val = (random.random() - 0.5) * contact_wear
                level = max(0.0, (db[i] / 8.0) + noise_val)
                mixed_val += level * math.sin(2.0 * math.pi * (base_freq * ratios[i]) * t_curr)
                
            # Apply smooth polite ADSR fade in/out envelope
            env = 1.0
            if s < 1000:
                env = s / 1000.0
            elif s > step_samples_turn - 1000:
                env = (step_samples_turn - s) / 1000.0
                
            audio_np[global_sample] += math.tanh(mixed_val * 0.4) * 0.35 * env
            
    # Normalize final mixed output
    max_val = np.max(np.abs(audio_np))
    if max_val > 1.0:
        audio_np /= max_val
        
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated drum-and-bass tonewheel bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
