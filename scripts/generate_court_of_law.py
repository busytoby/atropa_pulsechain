import os
import json
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

MotzkinPrime = 953467954114363

# Court of Law roles
BEARS = [
    {"id": 1, "role": "Judge", "address": "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B", "exp": 440, "mod": MotzkinPrime, "pan": 0.0, "color": (217, 119, 6), "pos": (320, 95)},
    {"id": 2, "role": "Witness A", "address": "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce", "exp": 12000, "mod": MotzkinPrime, "pan": -0.6, "color": (185, 28, 28), "pos": (140, 190)},
    {"id": 3, "role": "Witness B", "address": "0xd32c39fee49391c7952d1b30b15921b0d3b42e69", "exp": 8877, "mod": MotzkinPrime, "pan": 0.6, "color": (59, 130, 246), "pos": (500, 190)},
    {"id": 4, "role": "Juror 1", "address": "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b", "exp": 4433, "mod": MotzkinPrime, "pan": -0.8, "color": (107, 114, 128), "pos": (80, 290)},
    {"id": 5, "role": "Juror 2", "address": "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e", "exp": 5522, "mod": MotzkinPrime, "pan": 0.8, "color": (4, 120, 87), "pos": (560, 290)}
]

def derive_hypotrochoid_params(address):
    address_int = int(address, 16)
    R_hyp = (address_int % 60) + 15.0
    r_hyp = (address_int % 30) + 8.0
    d_hyp = (address_int % 20) + 5.0
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

def render_court_frame(frame_idx, bear_states, gavel_strike):
    flicker = 1.0 + 0.04 * math.sin(frame_idx * 0.4)
    img = Image.new("RGBA", (640, 360), (int(6 * flicker), int(4 * flicker), int(4 * flicker), 255))
    draw = ImageDraw.Draw(img)

    draw.ellipse([(290, 70), (350, 130)], fill=(217, 119, 6, 8))

    if gavel_strike > 0.0:
        radius = int(60 * (1.0 - gavel_strike))
        draw.ellipse([(320 - radius, 140 - radius), (320 + radius, 140 + radius)], outline=(217, 119, 6, int(gavel_strike * 150)), width=2)

    for b in BEARS:
        st = bear_states[b["role"]]
        active = st["active"]
        pulse = 1.0 + (0.22 * active) + 0.05 * math.sin(frame_idx * 0.2 + b["id"])
        
        color = b["color"] if active > 0.05 else (40, 20, 20)
        bx, by = b["pos"]

        draw.ellipse([(bx - 18 * pulse, by - 18 * pulse), (bx + 18 * pulse, by + 18 * pulse)], outline=color, width=2)
        draw.ellipse([(bx - 25 * pulse, by - 25 * pulse), (bx - 14 * pulse, by - 14 * pulse)], outline=color, width=2)
        draw.ellipse([(bx + 14 * pulse, by - 25 * pulse), (bx + 25 * pulse, by - 14 * pulse)], outline=color, width=2)

        eye_color = (239, 68, 68) if st["growl"] > 0.1 else (b["color"] if active > 0.05 else (60, 15, 15))
        draw.ellipse([(bx - 6 * pulse, by - 5 * pulse), (bx - 3 * pulse, by - 2 * pulse)], fill=eye_color)
        draw.ellipse([(bx + 3 * pulse, by - 5 * pulse), (bx + 6 * pulse, by - 2 * pulse)], fill=eye_color)

        draw.line([(bx - 8 * pulse, by + 5 * pulse), (bx + 8 * pulse, by + 5 * pulse)], fill=color, width=1)

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
    print("[COURT] Compiling Extended Teddy Bear Court of Law Simulator...")
    
    fps = 30
    sample_rate = 44100
    duration = 60.0
    total_frames = int(duration * fps)
    total_samples = int(duration * sample_rate)
    
    audio_data = np.zeros((total_samples, 2), dtype=np.float32)
    t_arr = np.arange(total_samples) / sample_rate
    
    print("[COURT] Rendering extended audio track...")
    
    # 1. Gavel strikes
    gavel_times = [2.0, 2.4, 2.8, 25.0, 57.0]
    for strike_time in gavel_times:
        start_idx = int(strike_time * sample_rate)
        strike_len = int(0.2 * sample_rate)
        t_strike = np.arange(strike_len) / sample_rate
        freq = 600.0 * np.exp(-t_strike * 60.0)
        phase = 2.0 * np.pi * np.cumsum(freq) / sample_rate
        gavel_wave = np.sin(phase) * np.exp(-t_strike * 25.0) * 0.6
        audio_data[start_idx:start_idx+strike_len, 0] += gavel_wave
        audio_data[start_idx:start_idx+strike_len, 1] += gavel_wave

    # 2. Voices
    def synthesize_voice(start_sec, end_sec, base_freq, exp, mod, growl_amt, pan):
        s_start = int(start_sec * sample_rate)
        s_end = int(end_sec * sample_rate)
        length = s_end - s_start
        t = np.arange(length) / sample_rate
        
        f_left_base = (mod_exp(int(base_freq), exp, mod) % 400) + 200
        f_right_base = (mod_exp(int(base_freq), mod, exp) % 400) + 200
        
        phase_l = 2 * np.pi * f_left_base * t
        phase_r = 2 * np.pi * f_right_base * t
        
        wave_l = np.sin(phase_l) + 0.3 * np.sin(2 * phase_l)
        wave_r = np.sin(phase_r) + 0.3 * np.sin(2 * phase_r)
        
        wave_l = lpf_one_pole(wave_l, 600.0, sample_rate)
        wave_r = lpf_one_pole(wave_r, 600.0, sample_rate)
        
        if growl_amt > 0.05:
            growl_osc = np.sin(2 * np.pi * 33.0 * t)
            wave_l = wave_l * (1.0 - growl_amt) + wave_l * growl_osc * growl_amt * 2.0
            wave_r = wave_r * (1.0 - growl_amt) + wave_r * growl_osc * growl_amt * 2.0
            
        env = np.ones_like(t)
        fade = int(0.05 * sample_rate)
        env[:fade] = np.linspace(0, 1, fade)
        env[-fade:] = np.linspace(1, 0, fade)
        
        wave_l *= env * 0.22
        wave_r *= env * 0.22
        
        audio_data[s_start:s_end, 0] += wave_l * (1.0 - pan) * 0.5
        audio_data[s_start:s_end, 1] += wave_r * (1.0 + pan) * 0.5

    # Segment 1 (2s - 8s): Judge speaks ("Silence in court!")
    synthesize_voice(3.2, 7.8, 180.0, 440, MotzkinPrime, 0.0, 0.0)

    # Segment 2 (10s - 24s): Witness A speaks, misaligned rules, Jurors growl aggressively
    synthesize_voice(10.0, 24.0, 320.0, 12000, MotzkinPrime, 0.7, -0.6)

    # Segment 3 (26s - 44s): Twinning/Agreement loop, growling fades, harmony emerges
    # We fade down growl manually by segments
    synthesize_voice(26.0, 32.0, 320.0, 12000, MotzkinPrime, 0.45, -0.6)
    synthesize_voice(33.0, 44.0, 320.0, 12000, MotzkinPrime, 0.0, -0.6)
    # Witness B responds in harmony during Segment 3
    synthesize_voice(35.0, 44.0, 290.0, 8877, MotzkinPrime, 0.0, 0.6)

    # Segment 4 (46s - 56s): Judge declares competency, Witness A & B overlap
    synthesize_voice(46.0, 52.0, 180.0, 440, MotzkinPrime, 0.0, 0.0)
    synthesize_voice(52.5, 56.5, 320.0, 12000, MotzkinPrime, 0.0, -0.6)
    synthesize_voice(52.5, 56.5, 290.0, 8877, MotzkinPrime, 0.0, 0.6)

    # master delay and echo
    delay_samps = int(0.3 * sample_rate)
    for i in range(delay_samps, total_samples):
        audio_data[i, 0] += audio_data[i - delay_samps, 1] * 0.25
        audio_data[i, 1] += audio_data[i - delay_samps, 0] * 0.25
        
    max_val = np.max(np.abs(audio_data))
    if max_val > 0.95:
        audio_data = (audio_data / max_val) * 0.95

    # Write WAV
    wav_path = "assets/bionika/court_of_law.wav"
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(sample_rate)
        pcm_data = (audio_data * 32767.0).astype(np.int16)
        w.writeframes(pcm_data.tobytes())

    # Render video
    mp4_path = "assets/bionika/teddy_bear_court_of_law.mp4"
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

    print("[COURT] Compiling video frames...")
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    for f_idx in range(total_frames):
        time_sec = f_idx / fps
        
        bear_states = {
            "Judge": {"active": 0.0, "growl": 0.0},
            "Witness A": {"active": 0.0, "growl": 0.0},
            "Witness B": {"active": 0.0, "growl": 0.0},
            "Juror 1": {"active": 0.0, "growl": 0.0},
            "Juror 2": {"active": 0.0, "growl": 0.0}
        }
        
        # Gavel strike visual ripples
        gavel_val = 0.0
        for gt in gavel_times:
            if gt <= time_sec <= gt + 0.4:
                gavel_val = 1.0 - ((time_sec - gt) / 0.4)
                break

        # Sequence state machine mapping
        if 3.2 <= time_sec <= 7.8 or 46.0 <= time_sec <= 52.0:
            bear_states["Judge"]["active"] = 1.0
        elif 10.0 <= time_sec <= 24.0:
            bear_states["Witness A"]["active"] = 1.0
            bear_states["Witness A"]["growl"] = 0.7
            bear_states["Juror 1"]["active"] = 0.8
            bear_states["Juror 1"]["growl"] = 0.8
            bear_states["Juror 2"]["active"] = 0.8
            bear_states["Juror 2"]["growl"] = 0.8
        elif 26.0 <= time_sec <= 32.0:
            bear_states["Witness A"]["active"] = 1.0
            bear_states["Witness A"]["growl"] = 0.45
            bear_states["Juror 1"]["active"] = 0.5
            bear_states["Juror 1"]["growl"] = 0.5
            bear_states["Juror 2"]["active"] = 0.5
            bear_states["Juror 2"]["growl"] = 0.5
        elif 33.0 <= time_sec <= 44.0:
            bear_states["Witness A"]["active"] = 1.0
            if time_sec >= 35.0:
                bear_states["Witness B"]["active"] = 1.0
        elif 52.5 <= time_sec <= 56.5:
            bear_states["Witness A"]["active"] = 1.0
            bear_states["Witness B"]["active"] = 1.0

        img = render_court_frame(f_idx, bear_states, gavel_val)
        img.convert("RGB").save(process.stdin, "PNG")

        if f_idx % 180 == 0:
            print(f"  -> Piped court frame {f_idx}/{total_frames}...")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Extended Court of Law video rendered successfully to {mp4_path}")

if __name__ == "__main__":
    os.makedirs("assets/bionika", exist_ok=True)
    main()
