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
    
    audio_np = np.zeros(total_samples, dtype=np.float32)
    noise_state = 12345
    
    # Backing Drum & Bass Track (120 BPM, 4/4)
    bpm = 120
    beat_dur = 60.0 / bpm
    step_dur = beat_dur / 4.0
    step_samples = int(sample_rate * step_dur)
    
    for s in range(total_samples):
        t_curr = s * dt
        step_idx = s // step_samples
        beat_idx = step_idx // 4
        step_in_beat = step_idx % 4
        sample_in_step = s % step_samples
        
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
    step_samples_turn = int(sample_rate * duration_per_step)
    
    hammond_ratios = [0.5, 1.498, 1.0, 2.0, 2.996, 4.0, 5.039, 6.0]
    
    for idx, step in enumerate(transitions):
        start_sample = idx * step_samples_turn
        end_sample = start_sample + step_samples_turn
        
        name = step["speaker"]
        voltage = step["voltage"]
        pitch = step["pitch"]
        
        for s in range(step_samples_turn):
            global_sample = start_sample + s
            if global_sample >= total_samples:
                break
            t_local = s * dt
            t_global = global_sample * dt
            
            # Map emotional voltage to synthesizer modulation depth
            slalom_time = min(0.3, voltage * 0.05) # highly emotional voltage results in slower pitch sweeps (slalom)
            enable_perc = (voltage > 2.5) # click percussion active when bear is excited
            
            if name in ["Trusty", "Coop"]:
                # --- Farfisa Organ Module ---
                # Drawbars: [16', 8', 4', 2-2/3']
                drawbars = [1.0, 0.8, 0.4, 0.2] if name == "Trusty" else [0.8, 0.8, 0.8, 0.0]
                
                mixed_val = 0.0
                # 8' Fundamental with Slalom pitch sweep
                active_freq = pitch
                if t_local < slalom_time:
                    start_freq = pitch * math.pow(2.0, -3.0/12.0)
                    active_freq = start_freq + (pitch - start_freq) * (t_local / slalom_time)
                    
                # 8' wave
                phase = (active_freq * t_local) % 1.0
                mixed_val += (2.0 * phase - 1.0) * drawbars[1] * 0.25
                
                # 16' Sub-Octave
                phase16 = ((active_freq * 0.5) * t_local) % 1.0
                mixed_val += (2.0 * phase16 - 1.0) * drawbars[0] * 0.20
                
                # 4' Octave
                phase4 = ((active_freq * 2.0) * t_local) % 1.0
                mixed_val += (2.0 * phase4 - 1.0) * drawbars[2] * 0.15
                
                # 2-2/3' Quint
                phase2 = ((active_freq * 3.0) * t_local) % 1.0
                mixed_val += (2.0 * phase2 - 1.0) * drawbars[3] * 0.10
                
                # Farfisa Percussion Click
                if enable_perc and t_local < 0.04:
                    perc_freq = pitch * 4.0
                    p_phase = (perc_freq * t_local) % 1.0
                    tri = abs(2.0 * p_phase - 1.0) * 2.0 - 1.0
                    decay = math.exp(-t_local / 0.012)
                    mixed_val += tri * 0.20 * decay
                    
            else:
                # --- Hammond Tonewheel Module ---
                db_profile = [8, 5, 8, 8, 8, 8, 8, 8] if name == "Aggro" else [8, 0, 0, 0, 0, 0, 8, 8]
                contact_wear = 0.05 * (voltage / 10.0)
                
                mixed_val = 0.0
                base_freq = pitch
                for i in range(8):
                    if db_profile[i] <= 0:
                        continue
                    # Add contact wear grit
                    noise_val = (random.random() - 0.5) * contact_wear
                    level = max(0.0, (db_profile[i] / 8.0) + noise_val)
                    mixed_val += level * math.sin(2.0 * math.pi * (base_freq * hammond_ratios[i]) * t_local)
                    
            env = 1.0
            if s < 1000:
                env = s / 1000.0
            elif s > step_samples_turn - 1000:
                env = (step_samples_turn - s) / 1000.0
                
            audio_np[global_sample] += math.tanh(mixed_val * 0.4) * 0.35 * env
            
    # Normalize output
    max_val = np.max(np.abs(audio_np))
    if max_val > 1.0:
        audio_np /= max_val
        
    audio_int = (audio_np * 32767).astype(np.int16)
    
    with wave.open(audio_path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        w.writeframes(struct.pack("<{}h".format(len(audio_int)), *audio_int))
        
    print(f"Generated multi-synth bear chorus soundtrack at '{audio_path}'")

if __name__ == "__main__":
    generate_audio()
