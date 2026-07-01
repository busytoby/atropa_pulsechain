import os
import json
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

MotzkinPrime = 953467954114363

# Define the 9 basic emotional chords (voltage input values mapping)
EMOTIONS = {
    "JOY": {"voltage": 1009, "desc": "Perfect fourth harmonics"},
    "SORROW": {"voltage": 1543, "desc": "Low-frequency sub-harmonic drone"},
    "ANGER": {"voltage": 2237, "desc": "Asymmetric growling ring-modulation"},
    "FEAR": {"voltage": 3109, "desc": "Rapid FM jitter/vibrato"},
    "MELANCHOLY": {"voltage": 4423, "desc": "Breathy filtered wind noise"},
    "EUPHORIA": {"voltage": 5519, "desc": "Evolving multi-loop rosettes"},
    "SUSPENSE": {"voltage": 6701, "desc": "Dissonant off-phase intervals"},
    "TRANQUILITY": {"voltage": 7883, "desc": "Pure symmetrical sine balance"},
    "CONFUSION": {"voltage": 8923, "desc": "Chaotic non-repeating boundaries"}
}

BEARS = [
    {"id": 1, "role": "Judge", "address": "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B", "exp": 440, "mod": MotzkinPrime, "color": (217, 119, 6), "pos": (320, 110)},
    {"id": 2, "role": "Witness A", "address": "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce", "exp": 12000, "mod": MotzkinPrime, "color": (185, 28, 28), "pos": (140, 200)},
    {"id": 3, "role": "Witness B", "address": "0xd32c39fee49391c7952d1b30b15921b0d3b42e69", "exp": 8877, "mod": MotzkinPrime, "color": (59, 130, 246), "pos": (500, 200)},
    {"id": 4, "role": "Juror 1", "address": "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b", "exp": 4433, "mod": MotzkinPrime, "color": (107, 114, 128), "pos": (100, 300)},
    {"id": 5, "role": "Juror 2", "address": "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e", "exp": 5522, "mod": MotzkinPrime, "color": (4, 120, 87), "pos": (540, 300)}
]

def derive_hypotrochoid_params(address):
    address_int = int(address, 16)
    R_hyp = (address_int % 50) + 15.0
    r_hyp = (address_int % 25) + 8.0
    d_hyp = (address_int % 15) + 5.0
    return R_hyp, r_hyp, d_hyp

def mod_exp(base, exp, mod):
    res = 1
    base = base % mod
    while exp > 0:
        if exp % 2 == 1:
            res = (res * base) % mod
        base = (base * base) % mod
        exp //= 2
    return res

def lpf_one_pole(x, cutoff, sample_rate=44100):
    y = np.zeros_like(x)
    val = 0.0
    alpha = 2.0 * np.pi * cutoff / (2.0 * np.pi * cutoff + sample_rate)
    for idx in range(len(x)):
        val = val + alpha * (x[idx] - val)
        y[idx] = val
    return y

def render_emotional_frame(frame_idx, active_emotion, current_speaker, text_logs):
    flicker = 1.0 + 0.04 * math.sin(frame_idx * 0.4)
    img = Image.new("RGBA", (640, 360), (int(5 * flicker), int(3 * flicker), int(3 * flicker), 255))
    draw = ImageDraw.Draw(img)

    # Print the announced court emotion
    draw.text((20, 20), f"EMOTIONAL CHORD ROUND: {active_emotion}", fill="#ef4444")
    
    # Draw logging subtitles of observations
    for idx, log in enumerate(text_logs[-3:]):
        draw.text((20, 45 + idx * 15), log, fill="#9ca3af")

    # Render bears in a courtroom circle
    for b in BEARS:
        active = 1.0 if b["role"] == current_speaker else 0.0
        pulse = 1.0 + (0.25 * active) + 0.05 * math.sin(frame_idx * 0.2 + b["id"])
        
        color = b["color"] if active > 0.05 else (40, 20, 20)
        bx, by = b["pos"]

        draw.ellipse([(bx - 16 * pulse, by - 16 * pulse), (bx + 16 * pulse, by + 16 * pulse)], outline=color, width=2)
        draw.ellipse([(bx - 23 * pulse, by - 23 * pulse), (bx - 12 * pulse, by - 12 * pulse)], outline=color, width=2)
        draw.ellipse([(bx + 12 * pulse, by - 23 * pulse), (bx + 23 * pulse, by - 12 * pulse)], outline=color, width=2)

        eye_color = b["color"] if active > 0.05 else (60, 15, 15)
        draw.ellipse([(bx - 5 * pulse, by - 4 * pulse), (bx - 2 * pulse, by - 1 * pulse)], fill=eye_color)
        draw.ellipse([(bx + 2 * pulse, by - 4 * pulse), (bx + 5 * pulse, by - 1 * pulse)], fill=eye_color)
        
        draw.line([(bx - 6 * pulse, by + 5 * pulse), (bx + 6 * pulse, by + 5 * pulse)], fill=color, width=1)
        
        if active > 0.01:
            R_hyp, r_hyp, d_hyp = derive_hypotrochoid_params(b["address"])
            pts = []
            rot_y = frame_idx * 0.04
            for j in range(60):
                theta = (j / 60.0) * 4.0 * math.pi
                hx = bx + ((R_hyp - r_hyp) * math.cos(theta + rot_y) + d_hyp * math.cos(((R_hyp - r_hyp)/r_hyp)*theta)) * 0.4
                hy = (by - 32) + ((R_hyp - r_hyp) * math.sin(theta + rot_y) - d_hyp * math.sin(((R_hyp - r_hyp)/r_hyp)*theta)) * 0.4
                pts.append((hx, hy))
            if len(pts) > 1:
                draw.line(pts, fill=b["color"] + (int(active * 180),), width=1)

    return img

def main():
    print("[COURT-EMO] Generating 9-Emotion Court of Law Video...")
    
    fps = 30
    sample_rate = 44100
    duration = 60.0
    total_frames = int(duration * fps)
    total_samples = int(duration * sample_rate)
    
    audio_data = np.zeros((total_samples, 2), dtype=np.float32)
    
    gavel_times = [1.0, 1.3, 1.6, 25.0, 57.0]
    
    # 1. Swearing in Gavel
    for gt in gavel_times:
        s_start = int(gt * sample_rate)
        s_len = int(0.2 * sample_rate)
        t_gavel = np.arange(s_len) / sample_rate
        freq = 600.0 * np.exp(-t_gavel * 60.0)
        phase = 2.0 * np.pi * np.cumsum(freq) / sample_rate
        g_wave = np.sin(phase) * np.exp(-t_gavel * 25.0) * 0.5
        audio_data[s_start:s_start+s_len, 0] += g_wave
        audio_data[s_start:s_start+s_len, 1] += g_wave

    def synthesize_voice(start_sec, end_sec, base_freq, exp, mod, growl_amt, pan):
        s_start = int(start_sec * sample_rate)
        s_end = int(end_sec * sample_rate)
        length = s_end - s_start
        t = np.arange(length) / sample_rate
        
        f_left_base = (mod_exp(int(base_freq), exp, mod) % 400) + 200
        f_right_base = (mod_exp(int(base_freq), mod, exp) % 400) + 200
        
        phase_l = 2 * np.pi * f_left_base * t
        phase_r = 2 * np.pi * f_right_base * t
        
        wave_l = np.sin(phase_l) + 0.35 * np.sin(2 * phase_l)
        wave_r = np.sin(phase_r) + 0.35 * np.sin(2 * phase_r)
        
        wave_l = lpf_one_pole(wave_l, 550.0, sample_rate)
        wave_r = lpf_one_pole(wave_r, 550.0, sample_rate)
        
        if growl_amt > 0.05:
            growl_osc = np.sin(2 * np.pi * 33.0 * t)
            wave_l = wave_l * (1.0 - growl_amt) + wave_l * growl_osc * growl_amt * 2.0
            wave_r = wave_r * (1.0 - growl_amt) + wave_r * growl_osc * growl_amt * 2.0
            
        env = np.ones_like(t)
        fade = int(0.05 * sample_rate)
        env[:fade] = np.linspace(0, 1, fade)
        env[-fade:] = np.linspace(1, 0, fade)
        
        wave_l *= env * 0.2
        wave_r *= env * 0.2
        
        audio_data[s_start:s_end, 0] += wave_l * (1.0 - pan) * 0.5
        audio_data[s_start:s_end, 1] += wave_r * (1.0 + pan) * 0.5

    # Swearing in Speech (Judge)
    synthesize_voice(2.0, 4.5, 180.0, 440, MotzkinPrime, 0.0, 0.0)

    rounds = ["JOY", "SORROW", "ANGER", "FEAR", "MELANCHOLY", "EUPHORIA", "SUSPENSE", "TRANQUILITY", "CONFUSION"]

    # Process 9 emotional rounds
    for r_idx, emo in enumerate(rounds):
        r_start = 5.0 + r_idx * 6.0
        cfg = EMOTIONS[emo]
        vol = cfg["voltage"]

        # Step 1: Judge announces emotion (1.5 seconds)
        synthesize_voice(r_start, r_start + 1.5, vol, 440, MotzkinPrime, 0.0, 0.0)
        
        # Step 2: Witness A plays its version (1.8 seconds)
        growl = 0.65 if emo in ["ANGER", "CONFUSION"] else 0.0
        synthesize_voice(r_start + 1.8, r_start + 3.6, vol, 12000, MotzkinPrime, growl, -0.6)
        
        # Step 3: Witness B plays its version (1.8 seconds)
        synthesize_voice(r_start + 3.8, r_start + 5.6, vol, 8877, MotzkinPrime, growl, 0.6)

    # Master delay return
    delay_samps = int(0.3 * sample_rate)
    for i in range(delay_samps, total_samples):
        audio_data[i, 0] += audio_data[i - delay_samps, 1] * 0.25
        audio_data[i, 1] += audio_data[i - delay_samps, 0] * 0.25
        
    max_val = np.max(np.abs(audio_data))
    if max_val > 0.95:
        audio_data = (audio_data / max_val) * 0.95

    # Write WAV
    wav_path = "assets/bionika/emotional_court.wav"
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        pcm_data = (audio_data * 32767.0).astype(np.int16)
        w.writeframes(pcm_data.tobytes())

    # Render video
    mp4_path = "assets/bionika/teddy_bear_emotional_court.mp4"
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

    print("[COURT-EMO] Compiling video frames...")
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    for f_idx in range(total_frames):
        time_sec = f_idx / fps
        
        active_emotion = "SWEARING IN SESSION"
        current_speaker = "None"
        text_logs = ["[System] Swearing in witnesses..."]

        if time_sec < 5.0:
            active_emotion = "SWEARING IN SESSION"
            current_speaker = "Judge"
            text_logs = [
                "Judge: 'All bears swear to play Follow the Leader.'",
                "Witnesses: 'We swear to observe and witness.'"
            ]
        else:
            r_idx = int((time_sec - 5.0) / 6.0)
            if r_idx < len(rounds):
                active_emotion = rounds[r_idx]
                r_start = 5.0 + r_idx * 6.0
                rel_time = time_sec - r_start
                
                if rel_time < 1.8:
                    current_speaker = "Judge"
                    text_logs = [
                        f"Judge defines baseline for: {active_emotion}",
                        f"Judge rosette resolved (LUT base 440)"
                    ]
                elif 1.8 <= rel_time < 3.8:
                    current_speaker = "Witness A"
                    witness_feeling = active_emotion
                    observer_feeling = "SORROW" if active_emotion == "JOY" else ("JOY" if active_emotion == "SORROW" else "CONFUSION")
                    
                    text_logs = [
                        f"Witness A plays version. Rosette resolved.",
                        f"Witness A logs state: {witness_feeling}",
                        f"Witness B projects shape: {observer_feeling} (LUT mismatch)"
                    ]
                else:
                    current_speaker = "Witness B"
                    witness_feeling = active_emotion
                    observer_feeling = "ANGER" if active_emotion == "TRANQUILITY" else "JOY"
                    
                    text_logs = [
                        f"Witness B plays version. Rosette resolved.",
                        f"Witness B logs state: {witness_feeling}",
                        f"Witness A projects shape: {observer_feeling} (LUT mismatch)"
                    ]

        gavel_val = 0.0
        for gt in gavel_times:
            if gt <= time_sec <= gt + 0.4:
                gavel_val = 1.0 - ((time_sec - gt) / 0.4)
                break

        img = render_emotional_frame(f_idx, active_emotion, current_speaker, text_logs)
        img.convert("RGB").save(process.stdin, "PNG")

        if f_idx % 180 == 0:
            print(f"  -> Piped court frame {f_idx}/{total_frames}...")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Emotional Court video rendered successfully to {mp4_path}")

if __name__ == "__main__":
    main()
