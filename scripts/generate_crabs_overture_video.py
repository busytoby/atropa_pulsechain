import os
import sys
import math
import random
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw

# --- Configuration ---
SAMPLE_RATE = 44100
DURATION_SECS = 390 # 6 and a half minutes
FPS = 30
WIDTH = 1280
HEIGHT = 720
TOTAL_FRAMES = DURATION_SECS * FPS # 11700 frames

# 22-EDO Scale frequencies (Root C3 = 130.81 Hz)
ROOT_FREQ = 130.81
EDO_22 = [ROOT_FREQ * (2 ** (i / 22)) for i in range(22)]

# Suite Sections Configuration (Matches Browser)
SECTIONS = [
    { "start": 0, "end": 15, "name": "Auncient Tide (Birth of Crabs)", "scale": [0, 4, 8, 12, 16], "bpm": 90, "bass": 0.8, "density": 0.1, "theme": "sea" },
    { "start": 15, "end": 30, "name": "Melancholic Horizon (Lonely Shore)", "scale": [0, 3, 7, 10, 14], "bpm": 95, "bass": 0.9, "density": 0.2, "theme": "sunset" },
    { "start": 30, "end": 45, "name": "Anticipation (First Pinch)", "scale": [0, 2, 6, 8, 12, 16], "bpm": 100, "bass": 1.0, "density": 0.4, "theme": "sunset" },
    { "start": 45, "end": 60, "name": "Paradiddle Awakening (Crawl Begins)", "scale": [0, 2, 6, 8, 14, 16], "bpm": 102, "bass": 1.1, "density": 0.5, "theme": "sea" },
    { "start": 60, "end": 75, "name": "Deep Manifold Groove (Thumpin Bass)", "scale": [0, 4, 7, 11, 15, 18], "bpm": 105, "bass": 1.6, "density": 0.6, "theme": "deep" },
    { "start": 75, "end": 90, "name": "Sub-harmonic Tension (Undercurrent)", "scale": [0, 1, 5, 8, 12, 17], "bpm": 102, "bass": 1.8, "density": 0.5, "theme": "deep" },
    { "start": 90, "end": 105, "name": "Pinch Dance (Playful Rhythm)", "scale": [0, 4, 8, 12, 16, 20], "bpm": 108, "bass": 1.2, "density": 0.7, "theme": "lava" },
    { "start": 105, "end": 120, "name": "Crying Ocean Depth (Mournful Echo)", "scale": [0, 3, 6, 9, 13, 16], "bpm": 98, "bass": 1.0, "density": 0.4, "theme": "sunset" },
    { "start": 120, "end": 135, "name": "Microtonal Reef (Heavy Sub-Bass)", "scale": [0, 4, 6, 10, 12, 18, 20], "bpm": 104, "bass": 2.1, "density": 0.6, "theme": "deep" },
    { "start": 135, "end": 150, "name": "Claw Aggression (Crabs Assemble)", "scale": [0, 2, 5, 8, 11, 14, 17], "bpm": 110, "bass": 1.8, "density": 0.8, "theme": "lava" },
    { "start": 150, "end": 165, "name": "Syrinx Synthesis (Ancient Signal)", "scale": [0, 5, 10, 15, 20], "bpm": 112, "bass": 1.4, "density": 0.7, "theme": "sea" },
    { "start": 165, "end": 180, "name": "Paradiddle Hurricane (Intense Beats)", "scale": [0, 2, 4, 8, 10, 12, 16, 20], "bpm": 118, "bass": 1.7, "density": 0.95, "theme": "lava" },
    { "start": 180, "end": 195, "name": "Echoplex Euphoria (Spiritual Wave)", "scale": [0, 4, 8, 12, 16, 21], "bpm": 105, "bass": 1.3, "density": 0.5, "theme": "sunset" },
    { "start": 195, "end": 210, "name": "Volcanic Rift (Acid Thump)", "scale": [0, 3, 6, 9, 12, 15, 18, 21], "bpm": 114, "bass": 2.3, "density": 0.85, "theme": "lava" },
    { "start": 210, "end": 225, "name": "Lava Tide Warning (Incineration)", "scale": [0, 1, 4, 7, 10, 13, 16, 19], "bpm": 116, "bass": 2.5, "density": 0.9, "theme": "lava" },
    { "start": 225, "end": 240, "name": "Tutti Climax (All Claws Flailing)", "scale": [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20], "bpm": 122, "bass": 2.8, "density": 1.0, "theme": "lava" },
    { "start": 240, "end": 255, "name": "Reef Subversion (Mysterious Calm)", "scale": [0, 4, 8, 12], "bpm": 100, "bass": 1.6, "density": 0.3, "theme": "deep" },
    { "start": 255, "end": 270, "name": "Melodic Resurgence (New Hope)", "scale": [0, 5, 8, 13, 17, 21], "bpm": 108, "bass": 1.5, "density": 0.6, "theme": "sea" },
    { "start": 270, "end": 285, "name": "Intense Contrapuntal Swarm", "scale": [0, 2, 6, 9, 12, 15, 18, 20], "bpm": 116, "bass": 1.9, "density": 0.8, "theme": "deep" },
    { "start": 285, "end": 300, "name": "Paradiddle Frenzy (Final Wave)", "scale": [0, 2, 4, 8, 10, 12, 16, 20], "bpm": 124, "bass": 2.2, "density": 0.95, "theme": "lava" },
    { "start": 300, "end": 315, "name": "Oceanic Resolve (Triumphant Return)", "scale": [0, 4, 8, 12, 16, 20], "bpm": 110, "bass": 1.6, "density": 0.7, "theme": "sea" },
    { "start": 315, "end": 330, "name": "Twilight Descending (Soft Shadows)", "scale": [0, 3, 7, 10, 14, 18], "bpm": 102, "bass": 1.2, "density": 0.4, "theme": "sunset" },
    { "start": 330, "end": 345, "name": "Incantation of the Coral", "scale": [0, 4, 8, 12, 16], "bpm": 98, "bass": 1.1, "density": 0.3, "theme": "sunset" },
    { "start": 345, "end": 360, "name": "Sub-aquatic Dream (Echoes)", "scale": [0, 4, 9, 14, 19], "bpm": 95, "bass": 0.9, "density": 0.2, "theme": "deep" },
    { "start": 360, "end": 375, "name": "Sunset Shoreline (Farewell)", "scale": [0, 4, 8, 12, 16], "bpm": 90, "bass": 0.7, "density": 0.1, "theme": "sunset" },
    { "start": 375, "end": 390, "name": "Auncient Tide (Return to Void)", "scale": [0, 4, 8, 12, 16], "bpm": 85, "bass": 0.4, "density": 0.05, "theme": "sea" }
]

# --- Audio Synthesizer Engine ---
def generate_audio():
    print("[AUDIO] Starting 6.5-minute microtonal synthesis...")
    total_samples = int(SAMPLE_RATE * DURATION_SECS)
    audio = np.zeros(total_samples, dtype=np.float32)
    
    # Pre-generate noise buffers for drums
    snare_noise = np.random.normal(0, 0.25, int(SAMPLE_RATE * 0.15))
    hat_noise = np.random.normal(0, 0.12, int(SAMPLE_RATE * 0.04))
    
    # Paradiddle patterns (R=Kick, L=Snare)
    r_rhythm = [1, 0, 1, 1, 0, 1, 0, 0]
    l_rhythm = [0, 1, 0, 0, 1, 0, 1, 1]
    
    current_sample = 0
    step_idx = 0
    
    while current_sample < total_samples:
        time_secs = current_sample / SAMPLE_RATE
        section = next((s for s in SECTIONS if time_secs >= s["start"] and time_secs < s["end"]), SECTIONS[-1])
        scale = section["scale"]
        
        bpm = section["bpm"]
        step_dur = 60 / bpm / 4
        step_samples = int(SAMPLE_RATE * step_dur)
        
        if current_sample + step_samples > total_samples:
            break
            
        sub_step = step_idx % 16
        
        # Sub-bass Note Trigger
        if sub_step in (0, 8) or (sub_step in (4, 12) and section["bass"] > 1.2):
            freq = EDO_22[scale[0]] / 2
            dur_samples = int(step_samples * 6)
            t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
            # Triangle + Saw blend
            sub_sig = 0.45 * (2 * np.abs(2 * (t * freq - np.floor(t * freq + 0.5))) - 1)
            sub_sig += 0.1 * (2 * (t * freq - np.floor(t * freq)) - 1)
            env = np.exp(-5 * t)
            audio[current_sample:current_sample+len(t)] += sub_sig * env * section["bass"]

        # Melodic Lead
        if sub_step % 2 == 0 and random.random() > 0.4:
            note_step = scale[random.randint(0, len(scale) - 1)]
            freq = EDO_22[note_step] * 2
            dur_samples = int(step_samples * 1.5)
            t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
            vibrato = np.sin(2 * np.pi * 6 * t) * 4
            lead_sig = 0.15 * np.sin(2 * np.pi * freq * t + vibrato)
            env = np.exp(-8 * t)
            audio[current_sample:current_sample+len(t)] += lead_sig * env

        # Drums (Paradiddle) scaled by density
        is_r = r_rhythm[step_idx % 8]
        is_l = l_rhythm[step_idx % 8]
        
        if random.random() <= section["density"]:
            # Kick (Right)
            if is_r:
                dur_samples = int(SAMPLE_RATE * 0.15)
                t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
                freq_sweep = 120 * np.exp(-25 * t)
                kick_sig = 0.5 * np.sin(2 * np.pi * freq_sweep * t)
                env = np.exp(-12 * t)
                audio[current_sample:current_sample+len(t)] += kick_sig * env
                
            # Snare (Left)
            if is_l:
                dur_samples = len(snare_noise)
                t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
                env = np.exp(-15 * t)
                audio[current_sample:current_sample+len(t)] += snare_noise[:len(t)] * env
                
            # Hi-hat
            if step_idx % 2 == 0:
                dur_samples = len(hat_noise)
                t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
                env = np.exp(-50 * t)
                audio[current_sample:current_sample+len(t)] += hat_noise[:len(t)] * env
        else:
            # ambient sparse hihats
            if step_idx % 4 == 0 and random.random() > 0.4:
                dur_samples = len(hat_noise)
                t = np.arange(min(dur_samples, total_samples - current_sample)) / SAMPLE_RATE
                env = np.exp(-50 * t)
                audio[current_sample:current_sample+len(t)] += hat_noise[:len(t)] * env

        current_sample += step_samples
        step_idx += 1
                
    # Normalize & convert to 16-bit PCM
    audio = np.clip(audio, -1.0, 1.0)
    audio_pcm = (audio * 32767).astype(np.int16)
    
    # Save temporary wav
    audio_path = "crabs_overture_temp.wav"
    with wave.open(audio_path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio_pcm.tobytes())
    return audio_path

# --- Crab Class for visual tracking ---
class RenderCrab:
    def __init__(self, x, y, size, color):
        self.x = x
        self.y = y
        self.size = size
        self.color = color
        self.vx = random.uniform(-2.0, 2.0)
        self.vy = random.uniform(-0.5, 0.5)
        self.pinch_l = 0.0
        self.pinch_r = 0.0

    def update(self, is_beat):
        self.x += self.vx
        self.y += self.vy
        if self.x < 100 or self.x > WIDTH - 100: self.vx = -self.vx
        if self.y < 380 or self.y > HEIGHT - 80: self.vy = -self.vy
        
        self.pinch_l = max(0.0, self.pinch_l - 0.1)
        self.pinch_r = max(0.0, self.pinch_r - 0.1)
        if is_beat:
            if random.random() > 0.4: self.pinch_l = 1.0
            if random.random() > 0.4: self.pinch_r = 1.0

# --- Video Assembler ---
def main():
    print("=== CRABS OVERTURE 6.5-MINUTE VIDEO GENERATOR ===")
    audio_file = generate_audio()
    
    crabs = [
        RenderCrab(300, 420, 28, (255, 94, 98)),
        RenderCrab(550, 480, 36, (255, 160, 122)),
        RenderCrab(800, 450, 24, (255, 107, 129)),
        RenderCrab(450, 580, 32, (230, 92, 0)),
        RenderCrab(700, 520, 40, (255, 71, 87)),
        RenderCrab(950, 500, 30, (255, 127, 80))
    ]

    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_file,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        "crabs_overture_final.mp4"
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    print(f"[VIDEO] Rendering {TOTAL_FRAMES} frames of Crabs Visuals...")
    for frame_idx in range(TOTAL_FRAMES):
        time_secs = frame_idx / FPS
        section = next((s for s in SECTIONS if time_secs >= s["start"] and time_secs < s["end"]), SECTIONS[-1])
        theme = section["theme"]
        
        # Determine active theme colors
        if theme == "sunset":
            ocean_colors = [(44, 0, 62), (254, 52, 110)]
            sand_color = (75, 18, 72)
            ray_color = (255, 94, 98, 18)
            wave_color = (254, 52, 110, 40)
        elif theme == "deep":
            ocean_colors = [(2, 8, 19), (8, 27, 51)]
            sand_color = (14, 22, 38)
            ray_color = (57, 255, 20, 12)
            wave_color = (57, 255, 20, 25)
        elif theme == "lava":
            ocean_colors = [(26, 5, 5), (128, 12, 12)]
            sand_color = (28, 12, 12)
            ray_color = (255, 0, 0, 30)
            wave_color = (255, 94, 98, 60)
        else: # sea
            ocean_colors = [(1, 22, 39), (3, 50, 70)]
            sand_color = (66, 40, 6)
            ray_color = (0, 242, 254, 18)
            wave_color = (0, 242, 254, 40)

        # Dynamic beat check based on active BPM
        is_beat = (frame_idx % max(1, int(FPS * (60 / section["bpm"] / 4))) == 0)
        
        # Draw background frame
        img = Image.new("RGB", (WIDTH, HEIGHT), color=ocean_colors[0])
        draw = ImageDraw.Draw(img, "RGBA")
        
        # Draw sky/water
        draw.rectangle([0, 0, WIDTH, 360], fill=ocean_colors[1])
        
        # Volumetric light rays
        for i in range(4):
            ray_offset = math.sin(frame_idx * 0.015 + i) * 120
            draw.polygon([
                (WIDTH//2 + ray_offset, 0),
                (WIDTH//2 - 200 + ray_offset * 1.5, 360),
                (WIDTH//2 + 200 + ray_offset * 1.5, 360)
            ], fill=ray_color)
            
        # Sandy Beach
        draw.rectangle([0, 360, WIDTH, HEIGHT], fill=sand_color)
        
        # Ocean Shoreline Tide Wave
        wave_y = 350 + math.sin(frame_idx * 0.04) * 16
        wave_coords = [(0, wave_y)]
        for x in range(0, WIDTH + 40, 40):
            wave_coords.append((x, wave_y + math.sin(x * 0.03 + frame_idx * 0.1) * 8))
        wave_coords.append((WIDTH, HEIGHT))
        wave_coords.append((0, HEIGHT))
        draw.polygon(wave_coords, fill=wave_color)
        
        # Update and draw crabs
        for c in crabs:
            c.update(is_beat)
            # Legs
            for side in [-1, 1]:
                for leg in range(3):
                    angle = 0.2 + leg * 0.3 + math.sin(frame_idx * 0.15 + c.x + leg) * 0.15
                    lx = c.x + side * (c.size * 1.2)
                    ly = c.y + c.size * 0.8
                    draw.line([(c.x + side * (c.size/2), c.y), (lx, ly)], fill=c.color, width=int(c.size/5))
            
            # Shell Body
            draw.ellipse([c.x - c.size, c.y - int(c.size * 0.7), c.x + c.size, c.y + int(c.size * 0.7)], fill=c.color)
            
            # Claws
            cl_y = c.y - int(c.size * 0.8) - int(c.pinch_l * c.size * 0.3)
            draw.line([(c.x - int(c.size * 0.6), c.y), (c.x - c.size, cl_y)], fill=c.color, width=int(c.size/5))
            draw.ellipse([c.x - c.size - 6, cl_y - 6, c.x - c.size + 6, cl_y + 6], fill=c.color)
            
            cr_y = c.y - int(c.size * 0.8) - int(c.pinch_r * c.size * 0.3)
            draw.line([(c.x + int(c.size * 0.6), c.y), (c.x + c.size, cr_y)], fill=c.color, width=int(c.size/5))
            draw.ellipse([c.x + c.size - 6, cr_y - 6, c.x + c.size + 6, cr_y + 6], fill=c.color)
            
            # Eyes
            draw.ellipse([c.x - int(c.size * 0.5), c.y - int(c.size * 0.8), c.x - int(c.size * 0.2), c.y - int(c.size * 0.5)], fill=(255,255,255))
            draw.ellipse([c.x + int(c.size * 0.2), c.y - int(c.size * 0.8), c.x + int(c.size * 0.5), c.y - int(c.size * 0.5)], fill=(255,255,255))
            draw.ellipse([c.x - int(c.size * 0.45), c.y - int(c.size * 0.75), c.x - int(c.size * 0.35), c.y - int(c.size * 0.65)], fill=(0,0,0))
            draw.ellipse([c.x + int(c.size * 0.35), c.y - int(c.size * 0.75), c.x + int(c.size * 0.45), c.y - int(c.size * 0.65)], fill=(0,0,0))
            
        # Draw dynamic HUD text (displays active emotion & progress)
        draw.text((20, 20), f"CRABS OVERTURE - SECTION: {section['name'].upper()}", fill=(255, 255, 255))
        draw.text((20, 40), f"TIME: {int(time_secs//60):02}:{int(time_secs%60):02} / 06:30 | BPM: {bpm}", fill=(0, 242, 254))

        # CRT Scanlines overlay
        for y in range(0, HEIGHT, 4):
            draw.rectangle([0, y, WIDTH, y+1], fill=(0,0,0,30))
            
        img.save(process.stdin, "PNG")
        
        if frame_idx % 1800 == 0:
            print(f"[VIDEO] Rendered {frame_idx}/{TOTAL_FRAMES} frames ({int(time_secs)}s elapsed)...")
            
    process.stdin.close()
    process.wait()
    if os.path.exists(audio_file):
        os.remove(audio_file)
    print("=== VIDEO COMPILATION SUCCESSFULLY FINISHED: crabs_overture_final.mp4 ===")

if __name__ == "__main__":
    main()
