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
                # Limit to first 12 transitions to keep the demo video ~36 seconds long
                if len(transitions) < 12:
                    transitions.append({
                        "speaker": match.group(1),
                        "voltage": float(match.group(4)),
                        "pitch": float(match.group(5))
                    })
                
    if not transitions:
        transitions = [{"speaker": "Trusty", "voltage": 1.0, "pitch": 220.0}] * 12
        
    duration_per_step = 2.5   # Organic speaking turn: 2.5 seconds
    pause_duration = 0.5      # Cognitive pause gap: 0.5 seconds
    
    step_samples = int(sample_rate * duration_per_step)
    pause_samples = int(sample_rate * pause_duration)
    
    total_samples = (step_samples + pause_samples) * len(transitions)
    
    audio_np = np.zeros(total_samples, dtype=np.float32)
    noise_state = 12345
    
    # Backing Drum & Bass Track (120 BPM, 4/4)
    bpm = 120
    beat_dur = 60.0 / bpm
    beat_samples = int(sample_rate * beat_dur)
    
    for s in range(total_samples):
        t_curr = s * dt
        beat_idx = s // beat_samples
        sample_in_beat = s % beat_samples
        step_in_beat = int((sample_in_beat / beat_samples) * 4)
        sample_in_step = sample_in_beat % int(beat_samples / 4)
        
        kick = 0.0
        if step_in_beat == 0:
            t_kick = sample_in_step * dt
            if t_kick < 0.15:
                k_freq = 110.0 * math.exp(-t_kick * 30.0) + 40.0
                kick = 0.4 * math.sin(2.0 * math.pi * k_freq * t_kick) * math.exp(-t_kick * 9.0)
                
        snare = 0.0
        if step_in_beat == 2:
            t_snare = sample_in_step * dt
            if t_snare < 0.2:
                noise_state = (noise_state * 1103515245 + 12345) & 0x7fffffff
                noise_val = ((noise_state / 0x7fffffff) * 2.0) - 1.0
                snare = 0.25 * noise_val * math.exp(-t_snare * 14.0)
                
        bass_freq = 55.0
        if beat_idx % 4 == 1:
            bass_freq = 65.4
        elif beat_idx % 4 == 2:
            bass_freq = 73.4
        elif beat_idx % 4 == 3:
            bass_freq = 58.2
            
        bass_phase = 2.0 * math.pi * bass_freq * t_curr
        bass = 0.3 * math.sin(bass_phase) + 0.1 * math.sin(bass_phase * 2.0)
        
        audio_np[s] += (kick + snare + bass) * 0.4
        
    # Generate Bear voices utilizing Farfisa Slalom Organ and Hammond Tonewheels
    hammond_ratios = [0.5, 1.498, 1.0, 2.0, 2.996, 4.0, 5.039, 6.0]
    bear_drawbars = {
        "Trusty":  [8, 8, 8, 4, 0, 0, 0, 0],
        "Aggro":   [8, 5, 8, 8, 8, 8, 8, 8],
        "Skeptic": [0, 8, 0, 8, 0, 8, 0, 8],
        "Eerie":   [8, 0, 0, 0, 0, 0, 8, 8],
        "Coop":    [8, 8, 8, 8, 8, 0, 0, 0]
    }
    
    for idx, step in enumerate(transitions):
        start_sample = idx * (step_samples + pause_samples)
        end_sample = start_sample + step_samples
        
        name = step["speaker"]
        voltage = step["voltage"]
        pitch = step["pitch"]
        
        db = bear_drawbars.get(name, [8, 8, 8, 4, 0, 0, 0, 0])
        contact_wear = 0.05 * (voltage / 10.0)
        
        for s in range(step_samples):
            global_sample = start_sample + s
            if global_sample >= total_samples:
                break
            t_local = s * dt
            
            slalom_time = min(1.0, voltage * 0.15)
            enable_perc = (voltage > 2.5)
            
            if name in ["Trusty", "Coop"]:
                drawbars = [1.0, 0.8, 0.4, 0.2] if name == "Trusty" else [0.8, 0.8, 0.8, 0.0]
                
                mixed_val = 0.0
                active_freq = pitch
                if t_local < slalom_time:
                    start_freq = pitch * math.pow(2.0, -3.0/12.0)
                    active_freq = start_freq + (pitch - start_freq) * (t_local / slalom_time)
                    
                phase = (active_freq * t_local) % 1.0
                mixed_val += (2.0 * phase - 1.0) * drawbars[1] * 0.25
                
                phase16 = ((active_freq * 0.5) * t_local) % 1.0
                mixed_val += (2.0 * phase16 - 1.0) * drawbars[0] * 0.20
                
                phase4 = ((active_freq * 2.0) * t_local) % 1.0
                mixed_val += (2.0 * phase4 - 1.0) * drawbars[2] * 0.15
                
                phase2 = ((active_freq * 3.0) * t_local) % 1.0
                mixed_val += (2.0 * phase2 - 1.0) * drawbars[3] * 0.10
                
                if enable_perc and t_local < 0.04:
                    perc_freq = pitch * 4.0
                    p_phase = (perc_freq * t_local) % 1.0
                    tri = abs(2.0 * p_phase - 1.0) * 2.0 - 1.0
                    decay = math.exp(-t_local / 0.012)
                    mixed_val += tri * 0.20 * decay
            else:
                mixed_val = 0.0
                base_freq = pitch
                for i in range(8):
                    if db[i] <= 0:
                        continue
                    noise_val = (random.random() - 0.5) * contact_wear
                    level = max(0.0, (db[i] / 8.0) + noise_val)
                    mixed_val += level * math.sin(2.0 * math.pi * (base_freq * hammond_ratios[i]) * t_local)
                    
            # Fade envelope
            env = 1.0
            if s < 2000:
                env = s / 2000.0
            elif s > step_samples - 2000:
                env = (step_samples - s) / 2000.0
                
            audio_np[global_sample] += math.tanh(mixed_val * 0.4) * 0.35 * env
            
    max_val = np.max(np.abs(audio_np))
    if max_val > 1.0:
        audio_np /= max_val
        
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated standard biological-paced bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
