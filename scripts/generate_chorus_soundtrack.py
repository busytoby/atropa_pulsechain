#!/usr/bin/env python3
import re
import wave
import struct
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
                    "voltage": float(match.group(4)),
                    "pitch": float(match.group(5))
                })
                
    if not transitions:
        transitions = [{"voltage": 1.0, "pitch": 220.0}] * 313
        
    duration_per_step = 0.08  # Slow down step to 80 ms to appreciate FM harmonics
    total_audio = []
    
    # progression major chords index
    notes = [130.8, 164.8, 196.0, 261.6] # C3, E3, G3, C4
    
    for idx, step in enumerate(transitions):
        t = np.linspace(0, duration_per_step, int(sample_rate * duration_per_step), endpoint=False)
        
        # 1. Warm bass swell modulated by voltage
        base_freq = notes[idx % len(notes)]
        env_bass = (1.0 - np.cos(np.pi * t / duration_per_step)) * np.exp(-t * 2.0)
        sine_bass = np.sin(2.0 * np.pi * base_freq * t)
        
        # 2. Pizzicato pluck with growl attack FM modulator
        fm_mod = np.sin(2.0 * np.pi * (step["pitch"] * 1.5) * t) * 1.2
        marimba = np.sin(2.0 * np.pi * step["pitch"] * t + fm_mod) * np.exp(-t * 30.0)
        
        # 3. Woodwind run
        ww_freq = step["pitch"] * 2.0
        flute = np.sin(2.0 * np.pi * ww_freq * t) * np.exp(-t * 12.0)
        
        # 4. Glockenspiel highlights on voltage spikes
        glock = 0.0
        if step["voltage"] > 2.5:
            glock = np.sin(2.0 * np.pi * step["pitch"] * 4.0 * t) * np.exp(-t * 50.0)
            
        mixed = (0.35 * sine_bass * env_bass) + (0.28 * marimba) + (0.15 * flute) + (0.22 * glock)
        total_audio.extend(mixed)
        
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
        
    print(f"Generated upgraded Pixar-quality bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
