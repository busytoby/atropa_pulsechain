import os
import math
import numpy as np
from PIL import Image, ImageDraw, ImageFont

SAMPLE_RATE = 44100
FPS = 30
DURATION = 6.0
TOTAL_SAMPLES = int(SAMPLE_RATE * DURATION)
TOTAL_FRAMES = int(FPS * DURATION)

def generate_audio():
    print("[DSP] Synthesizing Auncient Temple soundscape & vocoder drone...")
    t = np.linspace(0, DURATION, TOTAL_SAMPLES, endpoint=False)
    
    # 1. 808-style kick pattern (Four-on-the-floor sub)
    kick = np.zeros_like(t)
    beat_len = int(SAMPLE_RATE * 0.5) # 120 BPM
    for i in range(12):
        onset = i * beat_len
        age = (np.arange(TOTAL_SAMPLES) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < 0.4)
        if np.any(mask):
            k_age = age[mask]
            pitch_env = np.exp(-k_age / 0.04)
            freq = 48.0 + 100.0 * pitch_env
            phase = 2.0 * np.pi * np.cumsum(freq) / SAMPLE_RATE
            # Limit phase to the masked slice
            phase_slice = 2.0 * np.pi * freq * k_age
            kick_val = np.sin(phase_slice) * np.exp(-k_age / 0.35)
            # Add click
            click_mask = k_age < 0.005
            kick_val[click_mask] += (np.random.rand(np.sum(click_mask)) * 2 - 1) * 0.15
            kick[mask] += np.tanh(kick_val * 1.5)

    # 2. 303-style arpeggiated bassline
    bass = np.zeros_like(t)
    notes = [65.41, 130.81, 77.78, 98.00, 116.54, 130.81] # C2, C3, Eb2, G2, Bb2, C3
    step_len = int(SAMPLE_RATE * 0.25) # 8th notes
    for i in range(24):
        onset = i * step_len
        note_freq = notes[i % len(notes)]
        age = (np.arange(TOTAL_SAMPLES) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < 0.2)
        if np.any(mask):
            b_age = age[mask]
            # Sawtooth wave phase
            phase = (note_freq * b_age) % 1.0
            saw = 2.0 * phase - 1.0
            # Lowpass sweep
            env = np.exp(-b_age / 0.08)
            cutoff = 200.0 + 1200.0 * env
            # Simple digital lowpass filter
            coeff = (2.0 * np.pi * cutoff) / SAMPLE_RATE
            filtered = np.zeros_like(saw)
            p = 0.0
            for j in range(len(saw)):
                p += coeff[j] * (saw[j] - p)
                filtered[j] = p
            bass[mask] += np.tanh(filtered * 2.0) * 0.25

    # 3. Vocoded "Auncient Voice" Carrier
    # Synthesizes formant filters for vocalization simulation "A-U-N-C-I-E-N-T"
    voice = np.zeros_like(t)
    carrier_freq = 82.41 # E2 deep voice fundamental
    phase_voice = (carrier_freq * t) % 1.0
    saw_voice = 2.0 * phase_voice - 1.0
    
    # Formants for vowel sound transition
    f1 = 450.0 + 150.0 * np.sin(2.0 * np.pi * t * 0.3)
    f2 = 1400.0 + 400.0 * np.cos(2.0 * np.pi * t * 0.3)
    f3 = 2400.0 + 200.0 * np.sin(2.0 * np.pi * t * 0.5)

    coeff1 = (2.0 * np.pi * f1) / SAMPLE_RATE
    coeff2 = (2.0 * np.pi * f2) / SAMPLE_RATE
    coeff3 = (2.0 * np.pi * f3) / SAMPLE_RATE

    p1, p2, p3 = 0.0, 0.0, 0.0
    for j in range(TOTAL_SAMPLES):
        p1 += coeff1[j] * (saw_voice[j] - p1)
        p2 += coeff2[j] * (saw_voice[j] - p2)
        p3 += coeff3[j] * (saw_voice[j] - p3)
        voice[j] = (p1 + p2 + p3) / 3.0

    # Smooth gate envelope for voice triggers
    gate = 0.5 + 0.5 * np.sin(2.0 * np.pi * t * 0.6)
    voice_out = np.tanh(voice * gate * 3.5) * 0.2

    # Mix down
    mixed = (kick * 0.45) + (bass * 0.35) + (voice_out * 0.2)
    final_audio = np.tanh(mixed)
    return final_audio

def save_wav(audio_data):
    wav_path = "temp_audio.wav"
    print(f"[DSP] Saving temporary WAV: {wav_path}...")
    byte_data = bytearray(44 + len(audio_data) * 2)
    # RIFF header
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(audio_data) * 2).to_bytes(4, "little")
    byte_data[8:12] = b"WAVE"
    byte_data[12:16] = b"fmt "
    byte_data[16:20] = int(16).to_bytes(4, "little")
    byte_data[20:22] = int(1).to_bytes(2, "little") # PCM
    byte_data[22:24] = int(1).to_bytes(2, "little") # Mono
    byte_data[24:28] = int(SAMPLE_RATE).to_bytes(4, "little")
    byte_data[28:32] = int(SAMPLE_RATE * 2).to_bytes(4, "little")
    byte_data[32:34] = int(2).to_bytes(2, "little")
    byte_data[34:36] = int(16).to_bytes(2, "little")
    byte_data[36:40] = b"data"
    byte_data[40:44] = int(len(audio_data) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in audio_data:
        s = max(-1.0, min(1.0, val))
        v = int(s * 32767) if s >= 0 else int(s * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(wav_path, "wb") as f:
        f.write(byte_data)
    return wav_path

def render_frames():
    print("[GFX] Rendering frame sequence...")
    os.makedirs("temp_frames", exist_ok=True)
    
    # Coordinates of pentagon associahedron vertices
    pts = [
        (400, 150), # Top
        (550, 260), # Right
        (490, 420), # Bottom Right
        (310, 420), # Bottom Left
        (250, 260)  # Left
    ]

    for frame in range(TOTAL_FRAMES):
        img = Image.new("RGB", (800, 600), "#0a0c16")
        draw = ImageDraw.Draw(img)
        
        # Draw background glowing grid lines
        for y in range(0, 600, 40):
            draw.line([(0, y), (800, y)], fill="#12162a", width=1)
        for x in range(0, 800, 40):
            draw.line([(x, 0), (x, 600)], fill="#12162a", width=1)

        # Title Block
        draw.text((30, 25), "TEMPLE OF THE ASSOCIAHEDRON", fill="#ff007f")
        draw.text((30, 45), "STASHEFF FLIP STATE-MACHINE VERIFICATION", fill="#00f3ff")
        
        # Draw edges of pentagon (the 1-skeleton of K4)
        for i in range(5):
            p1 = pts[i]
            p2 = pts[(i + 1) % 5]
            draw.line([p1, p2], fill="#ffd700", width=3)
            
        # Draw chords in polygon center based on active vertex frame timing
        # We simulate a diagonal flip
        chord_state = (frame // 45) % 2
        center = (400, 300)
        
        if chord_state == 0:
            # Chord from Top to Bottom Right
            draw.line([pts[0], pts[2]], fill="#ff007f", width=4)
            draw.text((450, 200), "Active Chord: [A]", fill="#ff007f")
        else:
            # Chord from Left to Bottom Right
            draw.line([pts[4], pts[2]], fill="#00f3ff", width=4)
            draw.text((450, 200), "Active Chord: [B]", fill="#00f3ff")
            
        # Draw vertices (holographic points)
        for idx, pt in enumerate(pts):
            draw.ellipse([pt[0]-8, pt[1]-8, pt[0]+8, pt[1]+8], fill="#00f3ff")
            draw.text((pt[0]+12, pt[1]-6), f"K4:{idx}", fill="#94a3b8")

        # Active telemetry feed
        draw.text((30, 500), f"BPM: 120  |  Core: YI  |  Frame: {frame}/{TOTAL_FRAMES}", fill="#ffd700")
        draw.text((30, 520), f"Signal Bus: BSY=1 | C/D=1 | REQ=1 | ACK={(frame//15)%2}", fill="#10b981")
        draw.text((30, 540), "Auncient substrate evolved.", fill="#94a3b8")

        # Save frame
        img.save(f"temp_frames/frame_{frame:04d}.png")
        
def compile_video():
    print("[FFMPEG] Compiling video...")
    cmd = (
        "ffmpeg -y -r 30 -i temp_frames/frame_%04d.png "
        "-i temp_audio.wav -c:v libx264 -pix_fmt yuv420p "
        "-c:a aac -b:a 192k temple_associahedron_demo.mp4"
    )
    os.system(cmd)
    print("[FFMPEG] Compilation complete: temple_associahedron_demo.mp4")

    # Clean up temporary resources
    print("[CLEANUP] Removing temp frames and audio...")
    for frame in range(TOTAL_FRAMES):
        try:
            os.remove(f"temp_frames/frame_{frame:04d}.png")
        except FileNotFoundError:
            pass
    try:
        os.rmdir("temp_frames")
        os.remove("temp_audio.wav")
    except Exception:
        pass

if __name__ == "__main__":
    audio = generate_audio()
    save_wav(audio)
    render_frames()
    compile_video()
