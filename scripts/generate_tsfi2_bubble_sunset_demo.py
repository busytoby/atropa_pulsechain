import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 30.0  # 30 seconds
BPM = 125
STEP_DUR = 60.0 / (BPM * 4)  # 16th note step duration (~0.12s)

# 16x16 Bubble Font Bitmaps for TSFi/2
BUBBLE_FONT = [
    # T
    [0x3FFC, 0x7FFE, 0xFFFF, 0xE3C7, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180],
    # S
    [0x3FFC, 0x7FFE, 0xC003, 0xC000, 0x7FE0, 0x3FF8, 0x01FC, 0x007E, 0x000F, 0xC007, 0xC003, 0xE007, 0x7FFE, 0x7FFE, 0x3FFC, 0x0000],
    # F
    [0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xFFF0, 0xFFF0, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0x0000],
    # i
    [0x0180, 0x03C0, 0x03C0, 0x0180, 0x0000, 0x0000, 0x0180, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180],
    # /
    [0x000F, 0x001F, 0x003E, 0x007C, 0x00F8, 0x01F0, 0x03E0, 0x07C0, 0x0F80, 0x1F00, 0x3E00, 0x7C00, 0xF800, 0xF000, 0xE000, 0x0000],
    # 2
    [0x3FFC, 0x7FFE, 0xC007, 0x000F, 0x001E, 0x003C, 0x0078, 0x00F0, 0x01E0, 0x03C0, 0x0780, 0x0F00, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000]
]

def generate_audio():
    print("[DSP] Synthesizing 30-second sequenced chiptune with 11 instruments...")
    total_samples = int(SAMPLE_RATE * DURATION)
    audio = np.zeros(total_samples, dtype=np.float32)
    
    # Time array for the whole song
    t_arr = np.arange(total_samples) / float(SAMPLE_RATE)
    
    # 1. 5 Drums: Kick, Snare, Hihat, Cowbell, Rimshot
    kick_seq =   [1,0,0,0, 0,0,1,0, 1,0,0,0, 0,0,0,0]
    snare_seq =  [0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0]
    hihat_seq =  [0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,1]
    cowbell_seq =[0,0,0,0, 0,0,0,1, 0,1,0,0, 0,0,0,0]
    rim_seq =    [0,1,0,0, 0,0,0,0, 0,0,0,1, 0,0,0,0]
    
    # Synthesize Drums into the track
    step_samples = int(SAMPLE_RATE * STEP_DUR)
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        step_time = step * STEP_DUR
        
        # 16-step pattern indexes
        pat_idx = step % 16
        
        # Trigger Kick (Sine wave frequency sweep: 150Hz -> 45Hz)
        if kick_seq[pat_idx] and step_start < total_samples:
            dur = int(SAMPLE_RATE * 0.15)
            t = np.arange(min(dur, total_samples - step_start)) / float(SAMPLE_RATE)
            freqs = 45.0 + 105.0 * np.exp(-t * 30.0)
            phase = 2.0 * np.pi * np.cumsum(freqs) / float(SAMPLE_RATE)
            env = np.exp(-t * 12.0)
            audio[step_start:step_start+len(t)] += 0.5 * np.sin(phase) * env
            
        # Trigger Snare (White noise with decay envelope)
        if snare_seq[pat_idx] and step_start < total_samples:
            dur = int(SAMPLE_RATE * 0.20)
            length = min(dur, total_samples - step_start)
            noise = np.random.uniform(-1.0, 1.0, length)
            t = np.arange(length) / float(SAMPLE_RATE)
            env = np.exp(-t * 15.0)
            audio[step_start:step_start+length] += 0.35 * noise * env
            
        # Trigger Hihat (Short noise burst)
        if hihat_seq[pat_idx] and step_start < total_samples:
            dur = int(SAMPLE_RATE * 0.04)
            length = min(dur, total_samples - step_start)
            noise = np.random.uniform(-1.0, 1.0, length)
            t = np.arange(length) / float(SAMPLE_RATE)
            env = np.exp(-t * 60.0)
            audio[step_start:step_start+length] += 0.15 * noise * env

        # Trigger Cowbell (Detuned metallic dual sines)
        if cowbell_seq[pat_idx] and step_start < total_samples:
            dur = int(SAMPLE_RATE * 0.12)
            t = np.arange(min(dur, total_samples - step_start)) / float(SAMPLE_RATE)
            env = np.exp(-t * 20.0)
            cb = np.sin(2.0 * np.pi * 540.0 * t) + 0.5 * np.sin(2.0 * np.pi * 800.0 * t)
            audio[step_start:step_start+len(t)] += 0.2 * cb * env
            
        # Trigger Rimshot (High sine burst)
        if rim_seq[pat_idx] and step_start < total_samples:
            dur = int(SAMPLE_RATE * 0.03)
            t = np.arange(min(dur, total_samples - step_start)) / float(SAMPLE_RATE)
            env = np.exp(-t * 80.0)
            audio[step_start:step_start+len(t)] += 0.15 * np.sin(2.0 * np.pi * 1200.0 * t) * env

    # 2. 2 Growls: Saw-LFO Growl & Formant-Sweep Growl
    # Growl 1 (Wobble Bass: Sawtooth wave modulated by LFO)
    growl1_notes = [55.0, 65.4, 73.4, 48.9, 55.0, 65.4, 82.4, 73.4]
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if step % 8 == 0 and step_start < total_samples:
            length = min(step_samples * 8, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            freq = growl1_notes[(step // 8) % len(growl1_notes)]
            
            # Sawtooth wave
            saw = 2.0 * (t * freq - np.floor(0.5 + t * freq))
            
            # LFO modulation on lowpass filter cutoff
            lfo = 0.5 + 0.5 * np.sin(2.0 * np.pi * 6.0 * t) # 6Hz wobble
            env = np.exp(-t * 0.5)
            audio[step_start:step_start+length] += 0.25 * saw * lfo * env

    # Growl 2 (Fuzz Bass: Formant bandpass-swept triangle)
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if (step + 4) % 8 == 0 and step_start < total_samples:
            length = min(step_samples * 4, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            freq = 65.41 # C-2
            
            # Triangle wave
            tri = 2.0 * np.abs(2.0 * (t * freq - np.floor(t * freq + 0.5))) - 1.0
            
            # Formant simulation: phase sweeps from 1000Hz down to 200Hz
            formant_sweep = np.sin(2.0 * np.pi * (freq * 4.0 * t + 8.0 * np.sin(2.0 * np.pi * 2.0 * t)))
            audio[step_start:step_start+length] += 0.2 * tri * formant_sweep

    # 3. 2 Leads: Detuned Square Arpeggiator & Vibrato PWM Lead
    # Lead 1 (Fast Square Arpeggiator playing A minor)
    lead_seq = [220.0, 261.6, 293.7, 329.6, 392.0, 440.0, 523.3, 587.3]
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if step_start < total_samples:
            length = min(step_samples, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            freq = lead_seq[step % len(lead_seq)]
            
            # Detuned square waves
            sq1 = np.sign(np.sin(2.0 * np.pi * freq * t))
            sq2 = np.sign(np.sin(2.0 * np.pi * (freq + 1.5) * t))
            env = np.exp(-t * 8.0)
            audio[step_start:step_start+length] += 0.08 * (sq1 + sq2) * env

    # Lead 2 (Vibrato PWM Lead playing melody)
    melody = [440.0, 440.0, 523.3, 440.0, 392.0, 392.0, 349.2, 392.0]
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if step % 4 == 0 and step_start < total_samples:
            length = min(step_samples * 4, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            freq = melody[(step // 4) % len(melody)]
            
            # Vibrato
            vib = 1.0 + 0.015 * np.sin(2.0 * np.pi * 5.0 * t)
            phase = 2.0 * np.pi * freq * vib * t
            
            # PWM wave: compare sine against dynamic threshold
            pwm_threshold = 0.5 * np.sin(2.0 * np.pi * 1.5 * t)
            pwm = np.where(np.sin(phase) > pwm_threshold, 1.0, -1.0)
            
            env = np.exp(-t * 1.5)
            audio[step_start:step_start+length] += 0.07 * pwm * env

    # 4. 2 Rhythm Sounds: Detuned Saw Chords & Bass Line
    # Rhythm 1 (Detuned Saw Chords on offbeats)
    chords = [
        [220.0, 261.6, 329.6], # Am
        [196.0, 246.9, 293.7], # G
        [174.6, 220.0, 261.6], # F
        [196.0, 246.9, 293.7]  # G
    ]
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if (step % 4 == 2) and step_start < total_samples:
            length = min(step_samples * 2, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            chord = chords[(step // 4) % len(chords)]
            
            detuned_saws = np.zeros(length)
            for f in chord:
                detuned_saws += np.sin(2.0 * np.pi * f * t) + 0.5 * np.sin(2.0 * np.pi * (f + 0.8) * t)
                
            env = np.exp(-t * 4.0)
            audio[step_start:step_start+length] += 0.06 * detuned_saws * env

    # Rhythm 2 (Syncopated Bass line)
    bass_notes = [110.0, 110.0, 130.8, 110.0, 98.0, 98.0, 87.3, 98.0]
    for step in range(int(DURATION / STEP_DUR)):
        step_start = step * step_samples
        if (step % 2 == 0) and step_start < total_samples:
            length = min(step_samples, total_samples - step_start)
            t = np.arange(length) / float(SAMPLE_RATE)
            freq = bass_notes[(step // 2) % len(bass_notes)]
            
            # Detuned Triangle Bass
            tri = np.abs(4.0 * (t * freq - np.floor(t * freq + 0.5))) - 1.0
            env = np.exp(-t * 6.0)
            audio[step_start:step_start+length] += 0.12 * tri * env

    # Master Limiter
    max_val = np.max(np.abs(audio))
    if max_val > 0.98:
        audio = audio / max_val * 0.98
        
    return audio

def render_sunset_frame(frame, width, height):
    t_val = frame / float(FPS)
    
    # 1. Warm Sunset vertical gradient (orange/red/purple)
    img = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(img)
    
    for y in range(height):
        factor = float(y) / float(height)
        if factor < 0.6:
            t = factor / 0.6
            r = int(0x1A * (1.0 - t) + 0xFF * t)
            g = int(0x00 * (1.0 - t) + 0x55 * t)
            b = int(0x2A * (1.0 - t) + 0x00 * t)
        else:
            t = (factor - 0.6) / 0.4
            r = int(0xFF * (1.0 - t) + 0x1C * t)
            g = int(0x55 * (1.0 - t) + 0x0B * t)
            b = int(0x00 * (1.0 - t) + 0x00 * t)
        draw.line([(0, y), (width, y)], fill=(r, g, b))
        
    # 2. Draw 2D Scrolling Parallax Stars (Background)
    for i in range(35):
        # Generate stable pseudo-random star position based on star index i
        np.random.seed(i * 1234)
        base_x = np.random.randint(0, width)
        base_y = np.random.randint(0, int(height * 0.8))
        speed = 20.0 + i * 2.5
        
        # Horizontal scrolling offset
        px = int(base_x - t_val * speed) % width
        
        # Scale/brightness of stars
        brightness = int(120 + (i % 5) * 25)
        color = (brightness, int(brightness * 0.7), int(brightness * 0.2))
        
        if i % 3 == 0:
            draw.ellipse([px - 2, base_y - 2, px + 2, base_y + 2], fill=color)
        else:
            draw.ellipse([px - 1, base_y - 1, px + 1, base_y + 1], fill=color)

    # 3. Draw Sunset Yellow/Orange Mesa (Sun)
    # Drawing sunset sun in the horizon (centered)
    sun_x, sun_y = width // 2, int(height * 0.6)
    sun_r = 50 + int(4.0 * math.sin(t_val * 4.0))
    draw.ellipse([sun_x - sun_r, sun_y - sun_r, sun_x + sun_r, sun_y + sun_r], fill=(255, 204, 0))

    # 4. Draw static Western Desert Silhouette (Mesa landscape & Green Cactus)
    # Draw horizontal ground line
    draw.rectangle([0, int(height * 0.65), width, height], fill=(28, 11, 0))
    
    # Draw Cactus Silhouette in PETSCII outline style (Green outline)
    cac_x, cac_y = width // 3, int(height * 0.65)
    # Main trunk
    draw.rectangle([cac_x - 10, cac_y - 120, cac_x + 10, cac_y], fill=(0, 160, 0))
    # Left arm
    draw.rectangle([cac_x - 45, cac_y - 85, cac_x - 10, cac_y - 70], fill=(0, 160, 0))
    draw.rectangle([cac_x - 45, cac_y - 110, cac_x - 30, cac_y - 80], fill=(0, 160, 0))
    # Right arm
    draw.rectangle([cac_x + 10, cac_y - 65, cac_x + 35, cac_y - 50], fill=(0, 160, 0))
    draw.rectangle([cac_x + 20, cac_y - 90, cac_x + 35, cac_y - 60], fill=(0, 160, 0))

    # 5. Render Volumetric 3D Glossy Bubbly Font logo of "TSFi/2"
    logo_start_x = width // 2 - 40
    logo_start_y = int(height * 0.25)
    char_spacing = 72
    
    # 5a. Render Drop Shadow first
    for char_idx in range(len(BUBBLE_FONT)):
        for r in range(16):
            row_bits = BUBBLE_FONT[char_idx][r]
            for c in range(16):
                if row_bits & (1 << (15 - c)):
                    # Wobble animation
                    wobble_x = int(5.0 * math.sin(t_val * 5.0 + r * 0.5 + c * 0.2))
                    wobble_y = int(4.0 * math.cos(t_val * 4.0 + r * 0.3 + c * 0.3))
                    
                    px = logo_start_x + char_idx * char_spacing + c * 4 + wobble_x + 8
                    py = logo_start_y + r * 4 + wobble_y + 8
                    
                    draw.rectangle([px, py, px + 3, py + 3], fill=(20, 5, 0))
                    
    # 5b. Render Inflated, color cycled main body with specular glint highlights
    for char_idx in range(len(BUBBLE_FONT)):
        for r in range(16):
            row_bits = BUBBLE_FONT[char_idx][r]
            prev_row_bits = BUBBLE_FONT[char_idx][r - 1] if r > 0 else 0
            
            for c in range(16):
                if row_bits & (1 << (15 - c)):
                    # Wobble animation
                    wobble_x = int(5.0 * math.sin(t_val * 5.0 + r * 0.5 + c * 0.2))
                    wobble_y = int(4.0 * math.cos(t_val * 4.0 + r * 0.3 + c * 0.3))
                    
                    px = logo_start_x + char_idx * char_spacing + c * 4 + wobble_x
                    py = logo_start_y + r * 4 + wobble_y
                    
                    # 3D Distance field logic (inflation)
                    dist_sq = 16
                    for dr in range(-2, 3):
                        for dc in range(-2, 3):
                            nr, nc = r + dr, c + dc
                            is_inside = (0 <= nr < 16) and (0 <= nc < 16)
                            is_filled = is_inside and (BUBBLE_FONT[char_idx][nr] & (1 << (15 - nc)))
                            if not is_filled:
                                d = dr*dr + dc*dc
                                if d < dist_sq: dist_sq = d
                                
                    # Highlights
                    is_top_edge = (r == 0) or not (prev_row_bits & (1 << (15 - c)))
                    is_left_edge = (c == 0) or not (row_bits & (1 << (15 - (c - 1))))
                    is_glossy = is_top_edge and is_left_edge
                    
                    # Shading layers
                    if dist_sq > 2:
                        color = (255, 204, 0) # Gold center
                    elif dist_sq > 1:
                        color = (255, 102, 0) # Orange slope
                    else:
                        # Color cycle border
                        col_step = int(t_val * 15.0 + char_idx * 4 + c) % 8
                        color = (180 + col_step * 8, 40 + col_step * 10, 0)
                        
                    draw.rectangle([px, py, px + 3, py + 3], fill=color)
                    
                    # Glossy specular glint
                    if is_glossy:
                        draw.rectangle([px, py, px + 1, py + 1], fill=(255, 255, 255))
                        
    return img

def main():
    width, height = 720, 480
    fps = 30
    total_frames = int(fps * DURATION)
    
    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2_bubble_sunset_demo.mp4"
    audio_temp = "scripts/temp_demo_sound.wav"
    
    # 1. Synthesize Audio
    audio_data = generate_audio()
    
    # Write to temporary WAV
    import wave
    with wave.open(audio_temp, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SAMPLE_RATE)
        # Convert floats to int16 PCM
        pcm = (audio_data * 32767.0).astype(np.int16)
        w.writeframes(pcm.tobytes())
        
    # 2. Render Video frames pipeline
    print(f"[FFMPEG] Starting video pipeline to output: {video_output}")
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(fps),
        "-i", "-",
        "-i", audio_temp,
        "-c:v", "libx264",
        "-c:a", "aac",
        "-strict", "experimental",
        "-pix_fmt", "yuv420p",
        "-shortest",
        video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    for f in range(total_frames):
        if f % 90 == 0:
            print(f"[RENDER] Frame {f}/{total_frames} ({int(100.0 * f / total_frames)}% complete)")
        img = render_sunset_frame(f, width, height)
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    
    # Clean up temporary WAV
    if os.path.exists(audio_temp):
        os.remove(audio_temp)
        
    print(f"[SUCCESS] Volumetric bubble font demo rendered successfully: {video_output}")

if __name__ == "__main__":
    main()
