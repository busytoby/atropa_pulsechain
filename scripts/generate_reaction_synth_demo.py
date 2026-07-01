import os
import json
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# Auncient prime and constants
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

def render_frame(frame_idx, left_freq, right_freq, eta_val, kappa_val, note_name):
    # Create high-quality black canvas
    img = Image.new("RGBA", (640, 360), (3, 5, 8, 255))
    draw = ImageDraw.Draw(img)

    # Draw grid lines (Visual Aesthetics)
    for x in range(0, 640, 40):
        draw.line([(x, 0), (x, 360)], fill=(251, 191, 36, 8), width=1)
    for y in range(0, 360, 40):
        draw.line([(0, y), (640, y)], fill=(251, 191, 36, 8), width=1)

    # Draw product interface borders
    draw.rectangle([(10, 10), (630, 350)], outline=(251, 191, 36, 38), width=2)
    
    # UI Header text
    draw.text((20, 20), "THE TEDDY BEAR REACTION SYNTHESIZER RS-131", fill="#fbbf24")
    draw.text((20, 40), f"Active Note: {note_name} | L Freq: {left_freq:.1f}Hz | R Freq: {right_freq:.1f}Hz", fill="#9ca3af")

    # Draw secondary locks
    draw.text((20, 310), f"Eta Register (Left Path):  0x{eta_val:012X}", fill="#fbbf24")
    draw.text((20, 330), f"Kappa Register (Right Path): 0x{kappa_val:012X}", fill="#10b981")

    # Draw Teddy Bear silhouette/geometry reactively pulsing
    pulse = 1.0 + 0.15 * math.sin(frame_idx * 0.2)
    bear_color = (251, 191, 36) if frame_idx % 2 == 0 else (16, 185, 129)
    cx, cy = 480, 180
    
    # Render organic bear segment outlines (head, ears, body)
    draw.ellipse([(cx - 40 * pulse, cy - 40 * pulse), (cx + 40 * pulse, cy + 40 * pulse)], outline=bear_color, width=2) # Head
    draw.ellipse([(cx - 55 * pulse, cy - 55 * pulse), (cx - 30 * pulse, cy - 30 * pulse)], outline=bear_color, width=2) # Left Ear
    draw.ellipse([(cx + 30 * pulse, cy - 55 * pulse), (cx + 55 * pulse, cy - 30 * pulse)], outline=bear_color, width=2) # Right Ear
    draw.ellipse([(cx - 15 * pulse, cy - 10 * pulse), (cx - 5 * pulse, cy)], fill=bear_color) # Eye L
    draw.ellipse([(cx + 5 * pulse, cy - 10 * pulse), (cx + 15 * pulse, cy)], fill=bear_color) # Eye R
    draw.polygon([(cx - 5 * pulse, cy + 5 * pulse), (cx + 5 * pulse, cy + 5 * pulse), (cx, cy + 12 * pulse)], fill=bear_color) # Nose

    # Draw Lissajous trajectory for Auncient register modulation
    pts = []
    centerX, centerY = 200, 180
    scaleX, scaleY = 120, 100
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
    
    # Read .bio score
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
    
    # We render the first 8 patterns (64 beats) for a 48 second demo
    num_patterns = min(8, len(arr))
    total_beats = num_patterns * 8
    duration = total_beats * (60.0 / tempo)
    total_frames = int(duration * fps)
    total_samples = int(duration * sample_rate)

    print(f"  -> Duration: {duration:.2f}s | Frames: {total_frames} | Samples: {total_samples}")

    # Synthesize Audio arrays
    audio_data = np.zeros((total_samples, 2), dtype=np.float32)

    # Note frequencies lookup
    notes_freq = {
        "C4": 261.63, "C#4": 277.18, "D4": 293.66, "D#4": 311.13, "E4": 329.63,
        "F4": 349.23, "F#4": 369.99, "G4": 392.00, "G#4": 415.30, "A4": 440.00,
        "A#4": 466.16, "B4": 493.88, "C5": 523.25,
        "Bb3": 233.08, "A3": 220.00, "F3": 174.61, "Eb4": 311.13, "G3": 196.00
    }

    # Tracking note timeline
    seconds_per_pattern = 8.0 * (60.0 / tempo)
    seconds_per_step = seconds_per_pattern / 32.0

    print("[RS-131] Synthesizing Reaction Binaural Audio pathway...")
    for p_idx in range(num_patterns):
        pat_name = arr[p_idx]
        pat = patterns.get(pat_name, {})
        lead = pat.get("lead", {})
        seq = lead.get("sequence", ["REST"]*32)

        p_start_sec = p_idx * seconds_per_pattern
        for step in range(32):
            note = seq[step]
            if note == "REST" or note not in notes_freq:
                continue

            # Compute step time boundaries
            step_start_sec = p_start_sec + step * seconds_per_step
            step_end_sec = step_start_sec + seconds_per_step

            # Synthesize reaction outputs
            f_base = notes_freq[note]
            Pi = int(f_base)
            base_offset = 12000
            channel_offset = 440

            Eta = mod_exp(Pi, channel_offset, base_offset)
            Kappa = mod_exp(Pi, base_offset, channel_offset)

            f_left = (Eta % 800) + 100
            f_right = (Kappa % 800) + 100

            # Synthesize sine waves into stereo audio buffer
            start_sample = int(step_start_sec * sample_rate)
            end_sample = int(step_end_sec * sample_rate)
            
            t_vals = np.arange(end_sample - start_sample) / sample_rate
            # Apply quick fade in/out envelope to prevent clipping
            env = np.ones_like(t_vals)
            fade_len = int(0.01 * sample_rate)
            if len(t_vals) > 2 * fade_len:
                env[:fade_len] = np.linspace(0, 1, fade_len)
                env[-fade_len:] = np.linspace(1, 0, fade_len)

            audio_data[start_sample:end_sample, 0] += np.sin(2 * np.pi * f_left * t_vals) * 0.35 * env
            audio_data[start_sample:end_sample, 1] += np.sin(2 * np.pi * f_right * t_vals) * 0.35 * env

    # Write WAV file
    wav_path = "assets/bionika/reaction_synth_rs131_demo.wav"
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        # Convert floats to 16-bit PCM bytes
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
        lead = pat.get("lead", {})
        seq = lead.get("sequence", ["REST"]*32)

        local_sec = time_sec % seconds_per_pattern
        step_idx = min(31, int(local_sec / seconds_per_step))
        note = seq[step_idx]

        f_left, f_right = 440.0, 440.0
        Eta, Kappa = 0, 0
        if note != "REST" and note in notes_freq:
            Pi = int(notes_freq[note])
            Eta = mod_exp(Pi, 440, 12000)
            Kappa = mod_exp(Pi, 12000, 440)
            f_left = (Eta % 800) + 100
            f_right = (Kappa % 800) + 100

        img = render_frame(f_idx, f_left, f_right, Eta, Kappa, note)
        img.convert("RGB").save(process.stdin, "PNG")

        if f_idx % 90 == 0:
            print(f"  -> Piped frame {f_idx}/{total_frames}...")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Demo video rendered successfully to {mp4_path}")

if __name__ == "__main__":
    os.makedirs("assets/bionika", exist_ok=True)
    main()
