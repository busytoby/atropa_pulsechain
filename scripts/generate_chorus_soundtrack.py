#!/usr/bin/env python3
import re
import wave
import struct
import numpy as np

def generate_audio():
    log_path = "chorus_simulation.log"
    audio_path = "bear_chorus_track.wav"
    sample_rate = 44100
    
    # Read simulation logs to extract pitch transitions
    pitches = []
    pattern = re.compile(r"\[Conversation\].*?Pitch:\s*([\d\.]+)\s*Hz")
    with open(log_path, "r") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                pitches.append(float(match.group(1)))
                
    if not pitches:
        pitches = [220.0] * 313
        
    duration_per_step = 0.05  # 50 ms per step
    total_audio = []
    
    # Synthesize sine waves modulated by each conversational step's pitch
    for step_pitch in pitches:
        t = np.linspace(0, duration_per_step, int(sample_rate * duration_per_step), endpoint=False)
        wave_data = np.sin(2 * np.pi * step_pitch * t)
        # Apply smooth envelope
        envelope = np.sin(np.pi * np.linspace(0, 1, len(wave_data)))
        total_audio.extend(wave_data * envelope * 0.3)
        
    audio_np = np.array(total_audio)
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
