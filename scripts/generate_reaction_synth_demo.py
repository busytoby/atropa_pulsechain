import os
import json
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

MotzkinPrime = 953467954114363

def mod_exp(base, exp, mod):
    res = 1
    base = base % mod
    while exp > 0:
        if exp % 2 == 1:
            res = (res * base) % mod
        base = (base * base) % mod
        exp //= 2
    return res

def note_to_freq(note):
    if note == "REST" or not note:
        return 0.0
    names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    try:
        if len(note) == 3:
            name, acc, octave = note[0], note[1], int(note[2])
            name = name + acc
        else:
            name, octave = note[0], int(note[1:])
        
        semis = names.index(name)
        midi = 12 * (octave + 1) + semis
        return 440.0 * (2.0 ** ((midi - 69) / 12.0))
    except Exception:
        return 0.0

def render_frame(frame_idx, left_freq, right_freq, eta_val, kappa_val, note_name, current_bass, current_kick, current_snare):
    img = Image.new("RGBA", (640, 360), (3, 5, 8, 255))
    draw = ImageDraw.Draw(img)

    # Grid background
    for x in range(0, 640, 40):
        draw.line([(x, 0), (x, 360)], fill=(251, 191, 36, 8), width=1)
    for y in range(0, 360, 40):
        draw.line([(0, y), (640, y)], fill=(251, 191, 36, 8), width=1)

    draw.rectangle([(10, 10), (630, 350)], outline=(251, 191, 36, 38), width=2)
    
    # Text displays
    draw.text((20, 20), "THE TEDDY BEAR REACTION SYNTHESIZER RS-131", fill="#fbbf24")
    draw.text((20, 40), f"Lead Note: {note_name} | Bass Note: {current_bass}", fill="#9ca3af")
    draw.text((20, 60), f"Left Freq: {left_freq:.1f}Hz | Right Freq: {right_freq:.1f}Hz", fill="#9ca3af")

    draw.text((20, 310), f"Eta Register (Left Path):  0x{eta_val:012X}", fill="#fbbf24")
    draw.text((20, 330), f"Kappa Register (Right Path): 0x{kappa_val:012X}", fill="#10b981")

    # Pulsing Teddy Bear geometry
    pulse = 1.0 + 0.15 * math.sin(frame_idx * 0.2) + 0.1 * current_kick
    bear_color = (251, 191, 36) if current_kick > 0.1 else (16, 185, 129)
    cx, cy = 480, 180
    
    draw.ellipse([(cx - 40 * pulse, cy - 40 * pulse), (cx + 40 * pulse, cy + 40 * pulse)], outline=bear_color, width=2)
    draw.ellipse([(cx - 55 * pulse, cy - 55 * pulse), (cx - 30 * pulse, cy - 30 * pulse)], outline=bear_color, width=2)
    draw.ellipse([(cx + 30 * pulse, cy - 55 * pulse), (cx + 55 * pulse, cy - 30 * pulse)], outline=bear_color, width=2)
    draw.ellipse([(cx - 15 * pulse, cy - 10 * pulse), (cx - 5 * pulse, cy)], fill=bear_color)
    draw.ellipse([(cx + 5 * pulse, cy - 10 * pulse), (cx + 15 * pulse, cy)], fill=bear_color)
    draw.polygon([(cx - 5 * pulse, cy + 5 * pulse), (cx + 5 * pulse, cy + 5 * pulse), (cx, cy + 12 * pulse)], fill=bear_color)

    # 1. Render Complex Hypotrochoid behind the Lissajous wave (Auncient geometric signatures)
    centerX, centerY = 200, 180
    R = 75.0 + (eta_val % 30)
    r = 25.0 + (kappa_val % 15)
    d = 20.0 + ((eta_val + kappa_val) % 25)
    
    hyp_pts = []
    # Loop theta through multiple revolutions to form the spirograph rosette
    for i in range(180):
        theta = (i / 180.0) * 8.0 * math.pi
        hx = centerX + (R - r) * math.cos(theta) + d * math.cos(((R - r) / r) * theta)
        hy = centerY + (R - r) * math.sin(theta) - d * math.sin(((R - r) / r) * theta)
        hyp_pts.append((hx, hy))

    if len(hyp_pts) > 1:
        draw.line(hyp_pts, fill=(16, 185, 129, 60), width=1)

    # 2. Render Lissajous trajectory mapping active panned channels
    pts = []
    scaleX, scaleY = 110, 90
    for i in range(120):
        t = (i / 120.0) * 2.0 * math.pi
        x = centerX + math.sin(t * (left_freq / 100.0) + frame_idx * 0.05) * scaleX
        y = centerY + math.cos(t * (right_freq / 100.0)) * scaleY
        pts.append((x, y))

    if len(pts) > 1:
        draw.line(pts, fill="#fbbf24", width=2)

    return img

def main():
    print("[RS-131] Generating Teddy Bear Reaction Synthesizer Demo...")
    
    bio_path = "assets/bionika/if_you_dont_know_me_by_now.bio"
    if not os.path.exists(bio_path):
        print(f"[ERROR] Score file not found: {bio_path}")
        return

    with open(bio_path, "r") as f:
        score = json.load(f)

    tempo = score.get("tempo", 80.0)
    arr = score.get("arrangement", ["intro"])
    patterns = score.get("patterns", {})

    fps = 30
    sample_rate = 44100
    
    num_patterns = min(8, len(arr))
    total_beats = num_patterns * 8
    duration = total_beats * (60.0 / tempo)
    total_frames = int(duration * fps)
    total_samples = int(duration * sample_rate)

    print(f"  -> Duration: {duration:.2f}s | Frames: {total_frames} | Samples: {total_samples}")

    audio_data = np.zeros((total_samples, 2), dtype=np.float32)
    seconds_per_pattern = 8.0 * (60.0 / tempo)
    seconds_per_step = seconds_per_pattern / 32.0

    print("[RS-131] Synthesizing Reaction, Bass, and Drum score tracks...")
    for p_idx in range(num_patterns):
        pat_name = arr[p_idx]
        pat = patterns.get(pat_name, {})
        
        lead_seq = pat.get("lead", {}).get("sequence", ["REST"]*32)
        bass_seq = pat.get("bass", {}).get("sequence", ["REST"]*32)
        drums = pat.get("drums", {})
        kick_seq = drums.get("kick", [0]*32)
        snare_seq = drums.get("snare", [0]*32)

        p_start_sec = p_idx * seconds_per_pattern
        for step in range(32):
            step_start_sec = p_start_sec + step * seconds_per_step
            step_end_sec = step_start_sec + seconds_per_step
            start_sample = int(step_start_sec * sample_rate)
            end_sample = int(step_end_sec * sample_rate)
            num_samps = end_sample - start_sample
            t_vals = np.arange(num_samps) / sample_rate

            # Envelopes
            env_std = np.ones_like(t_vals)
            fade_len = int(0.01 * sample_rate)
            if len(t_vals) > 2 * fade_len:
                env_std[:fade_len] = np.linspace(0, 1, fade_len)
                env_std[-fade_len:] = np.linspace(1, 0, fade_len)

            # 1. Upgraded Lead Reaction Instrument (BJT Saturation & Harmonics)
            lead_note = lead_seq[step]
            if lead_note != "REST":
                f_lead = note_to_freq(lead_note)
                if f_lead > 0:
                    Pi = int(f_lead)
                    Eta = mod_exp(Pi, 440, MotzkinPrime)
                    Kappa = mod_exp(Pi, 12000, MotzkinPrime)
                    f_left = (Eta % 600) + 150
                    f_right = (Kappa % 600) + 150

                    # Wave-shaped binaural triangle/sine combination
                    phase_l = 2 * np.pi * f_left * t_vals
                    phase_r = 2 * np.pi * f_right * t_vals
                    
                    wave_l = np.sin(phase_l) + 0.3 * np.sin(3 * phase_l) + 0.1 * np.sin(5 * phase_l)
                    wave_r = np.sin(phase_r) + 0.3 * np.sin(3 * phase_r) + 0.1 * np.sin(5 * phase_r)
                    
                    # Apply hyperbolic tangent warm BJT clipping/drive
                    wave_l = np.tanh(wave_l * 1.5) * 0.22
                    wave_r = np.tanh(wave_r * 1.5) * 0.22

                    audio_data[start_sample:end_sample, 0] += wave_l * env_std
                    audio_data[start_sample:end_sample, 1] += wave_r * env_std

            # 2. Synthesize Bass Track
            bass_note = bass_seq[step]
            if bass_note != "REST":
                f_bass = note_to_freq(bass_note)
                if f_bass > 0:
                    wave_b = np.sin(2 * np.pi * f_bass * t_vals) * 0.32 + np.sin(4 * np.pi * f_bass * t_vals) * 0.04
                    audio_data[start_sample:end_sample, 0] += wave_b * env_std
                    audio_data[start_sample:end_sample, 1] += wave_b * env_std

            # 3. Synthesize Kick Drum
            if kick_seq[step] == 1:
                freq_sweep = 120.0 * np.exp(-t_vals * 40.0) + 40.0
                phase_k = 2.0 * np.pi * np.cumsum(freq_sweep) / sample_rate
                wave_k = np.sin(phase_k) * 0.6 * np.exp(-t_vals * 8.0)
                audio_data[start_sample:end_sample, 0] += wave_k
                audio_data[start_sample:end_sample, 1] += wave_k

            # 4. Synthesize Snare Drum
            if snare_seq[step] == 1:
                noise = np.random.uniform(-1.0, 1.0, num_samps)
                wave_s = noise * 0.3 * np.exp(-t_vals * 12.0)
                audio_data[start_sample:end_sample, 0] += wave_s
                audio_data[start_sample:end_sample, 1] += wave_s

    # 5. Upgraded Stereo Delay Loop (Return Echo Loop Simulation)
    delay_samps = int(0.375 * sample_rate) # Dotted 8th note delay at 80 BPM
    feedback = 0.32
    for i in range(delay_samps, total_samples):
        audio_data[i, 0] += audio_data[i - delay_samps, 1] * feedback
        audio_data[i, 1] += audio_data[i - delay_samps, 0] * feedback

    # Normalize audio
    max_val = np.max(np.abs(audio_data))
    if max_val > 0.95:
        audio_data = (audio_data / max_val) * 0.95

    # Write WAV file
    wav_path = "assets/bionika/reaction_synth_rs131_demo.wav"
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        pcm_data = (audio_data * 32767.0).astype(np.int16)
        w.writeframes(pcm_data.tobytes())

    # Compile video using FFmpeg pipe
    mp4_path = "assets/bionika/reaction_synth_rs131_demo.mp4"
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(fps),
        "-i", "-",
        "-i", wav_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-shortest",
        mp4_path
    ]

    print("[RS-131] Launching FFmpeg compiler pipeline...")
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    for f_idx in range(total_frames):
        time_sec = f_idx / fps
        pat_idx = min(num_patterns - 1, int(time_sec / seconds_per_pattern))
        pat_name = arr[pat_idx]
        pat = patterns.get(pat_name, {})
        
        lead_seq = pat.get("lead", {}).get("sequence", ["REST"]*32)
        bass_seq = pat.get("bass", {}).get("sequence", ["REST"]*32)
        drums = pat.get("drums", {})
        kick_seq = drums.get("kick", [0]*32)
        snare_seq = drums.get("snare", [0]*32)

        local_sec = time_sec % seconds_per_pattern
        step_idx = min(31, int(local_sec / seconds_per_step))
        
        note = lead_seq[step_idx]
        bass_note = bass_seq[step_idx]
        kick_val = kick_seq[step_idx]
        snare_val = snare_seq[step_idx]

        f_left, f_right = 440.0, 440.0
        Eta, Kappa = 0, 0
        if note != "REST":
            f_lead = note_to_freq(note)
            if f_lead > 0:
                Pi = int(f_lead)
                Eta = mod_exp(Pi, 440, MotzkinPrime)
                Kappa = mod_exp(Pi, 12000, MotzkinPrime)
                f_left = (Eta % 600) + 150
                f_right = (Kappa % 600) + 150

        img = render_frame(f_idx, f_left, f_right, Eta, Kappa, note, bass_note, kick_val, snare_val)
        img.convert("RGB").save(process.stdin, "PNG")

        if f_idx % 90 == 0:
            print(f"  -> Piped frame {f_idx}/{total_frames}...")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Demo video rendered successfully to {mp4_path}")

if __name__ == "__main__":
    os.makedirs("assets/bionika", exist_ok=True)
    main()
