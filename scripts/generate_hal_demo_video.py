import os
import sys
import math
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
DURATION_SECS = 45
FPS = 30
WIDTH = 1280
HEIGHT = 720
TOTAL_FRAMES = DURATION_SECS * FPS

def generate_drum_soundtrack(filename):
    print("[AUDIO] Synthesizing 2-instrument drum soundtrack (Kick & Snare)...")
    num_samples = SAMPLE_RATE * DURATION_SECS
    with wave.open(filename, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        samples = []
        for i in range(num_samples):
            t = i / SAMPLE_RATE
            
            # Drum pattern: Kick on beat 1 & 3, Snare on beat 2 & 4 (BPM = 120, 2 beats per second)
            beat_idx = int(t * 2.0)
            beat_time = t % 0.5
            
            drum_val = 0.0
            
            # Kick Drum (Instrument 1) on beats 0 and 2 (of 4 beats)
            if (beat_idx % 4 == 0 or beat_idx % 4 == 2):
                # Pitch decay
                freq = 150.0 * math.exp(-35.0 * beat_time) + 45.0
                amp = math.exp(-15.0 * beat_time)
                drum_val += math.sin(2.0 * math.pi * freq * beat_time) * amp * 0.8
                
            # Snare Drum (Instrument 2) on beats 1 and 3
            if (beat_idx % 4 == 1 or beat_idx % 4 == 3):
                # High-frequency noise burst
                noise = np.random.uniform(-1.0, 1.0)
                amp = math.exp(-22.0 * beat_time)
                drum_val += noise * amp * 0.4
                
            # Basic synth line to bind the tempo
            bass_freq = 55.0 if (beat_idx % 8 < 4) else 65.4
            synth_line = math.sin(2.0 * math.pi * bass_freq * t) * 0.15
            
            mixed = drum_val + synth_line
            # Soft-clip output
            if mixed > 1.0: mixed = 1.0
            if mixed < -1.0: mixed = -1.0
            
            sample_val = int(mixed * 16384.0)
            samples.append(struct.pack("<h", sample_val))
        wf.writeframes(b"".join(samples))
    print("[AUDIO] Drum soundtrack synthesis completed.")

def main():
    print("=== STARTING HAL DEMO VIDEO GENERATION ===")
    audio_path = "temp_drum_soundtrack.wav"
    generate_drum_soundtrack(audio_path)
    
    artifact_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/dd413f30-1457-4127-8d11-415844cbbc2d"
    os.makedirs(artifact_dir, exist_ok=True)
    video_output = os.path.join(artifact_dir, "hal_hbridge_demo.mp4")
    
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(FPS),
        "-i", "-",
        "-i", audio_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        video_output
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    
    # Pre-render a list of instruction lines representing the EDSAC compiler trace
    instructions = [
        "EDSAC INIT: BOOTING INITIAL ORDERS 1",
        "HAL_READ_REG: PORT_TAPE_READ (16400) -> 0x41 ('A')",
        "HAL_WRITE_REG: CMD_BUFFER (65416) <- 0x41",
        "HAL_READ_REG: PORT_TAPE_READ (16400) -> 0x54 ('T')",
        "HAL_WRITE_REG: CMD_BUFFER (65417) <- 0x54",
        "HAL_READ_REG: ACCUMULATOR -> 102",
        "HAL_WRITE_REG: RAU_DITHER_IN (16416) <- 120",
        "HAL_WRITE_REG: RAU_DITHER_ERR (16480) <- 20",
        "S/370 EMULATOR: EXECUTE DITH R4, R2",
        "HAL_WRITE_REG: RAU_DITHER_OUT (16448) <- 102",
        "HAL_WRITE_REG: FAULT_REGISTER (65404) <- 0",
        "HBRIDGE STATUS: MOTOR_COAST ENFORCED",
    ]
    
    for frame_idx in range(TOTAL_FRAMES):
        t = frame_idx / FPS
        beat_time = t % 0.5
        pulse_scale = 1.0 + 0.15 * math.exp(-15.0 * beat_time)
        
        # Color palette: Dark deep cyan to slate blue background
        img = Image.new("RGB", (WIDTH, HEIGHT), color=(8, 12, 20))
        draw = ImageDraw.Draw(img)
        
        # Draw tech matrix background lines
        for y in range(0, HEIGHT, 40):
            draw.line([(0, y), (WIDTH, y)], fill=(12, 22, 38), width=1)
        for x in range(0, WIDTH, 40):
            draw.line([(x, 0), (x, HEIGHT)], fill=(12, 22, 38), width=1)
            
        # Title Card
        draw.rectangle([40, 30, WIDTH - 40, 90], outline=(0, 255, 200), width=2)
        draw.text((60, 45), "AUNCIENT DYSNOMIA VM: HAL REGISTER TRACE", fill=(0, 255, 200))
        
        # Draw simulated Register Allocation Units (RAU) status blocks
        draw.text((80, 120), "REGISTER ALLOCATION UNITS (RAU)", fill=(0, 180, 255))
        for r_idx, reg_name in enumerate(["DITHER_IN", "DITHER_OUT", "DITHER_ERR"]):
            reg_val = 120 if r_idx == 0 else (102 if r_idx == 1 else 0)
            box_x = 80 + r_idx * 260
            box_y = 150
            
            draw.rectangle([box_x, box_y, box_x + 230, box_y + 100], outline=(0, 180, 255), width=2)
            draw.text((box_x + 15, box_y + 20), reg_name, fill=(200, 230, 255))
            draw.text((box_x + 15, box_y + 55), f"VAL: {reg_val}", fill=(0, 255, 200))
            
        # Draw Scratchpad / Delay-line status blocks
        draw.text((80, 280), "MERCURY DELAY-LINE SCRATCHPAD", fill=(255, 180, 0))
        for s_idx in range(6):
            pulse_val = int(math.sin(t * 4.0 + s_idx) * 127 + 128)
            box_x = 80 + s_idx * 130
            box_y = 310
            draw.rectangle([box_x, box_y, box_x + 110, box_y + 70], outline=(255, 180, 0), width=1)
            draw.text((box_x + 10, box_y + 15), f"SEC_{s_idx:02d}", fill=(255, 210, 100))
            draw.text((box_x + 10, box_y + 40), f"P: {pulse_val:03d}", fill=(255, 255, 255))
            
        # Draw Trace Logs representing instruction parsing
        draw.text((80, 410), "EDSAC INITIAL ORDERS 1 PARSING LOG", fill=(0, 255, 100))
        draw.rectangle([80, 440, 680, 680], outline=(0, 255, 100), width=1)
        
        visible_lines = min(len(instructions), 1 + int(t * 0.5) % len(instructions))
        for line_idx in range(visible_lines):
            log_line = instructions[line_idx]
            draw.text((100, 460 + line_idx * 18), f"> {log_line}", fill=(180, 255, 180))
            
        # Render dynamic Lissajous quaternion wireframe mapping geometric offsets
        center_x = 980
        center_y = 480
        radius = 160.0 * pulse_scale
        points = []
        for a_idx in range(32):
            angle_val = a_idx * (2.0 * math.pi / 32.0)
            x_offset = math.cos(angle_val * 3.0 + t * 1.5) * math.sin(angle_val * 2.0) * radius
            y_offset = math.sin(angle_val * 4.0 + t * 2.0) * math.cos(angle_val * 1.0) * radius
            points.append((int(center_x + x_offset), int(center_y + y_offset)))
            
        for p_idx in range(len(points)):
            p1 = points[p_idx]
            p2 = points[(p_idx + 1) % len(points)]
            p3 = points[(p_idx + 8) % len(points)] # draw interior chord lines
            draw.line([p1, p2], fill=(0, 255, 200), width=2)
            draw.line([p1, p3], fill=(0, 128, 128), width=1)
            
        # Render a frame image buffer to the FFmpeg pipe
        img.save(process.stdin, "PNG")
        
    process.stdin.close()
    process.wait()
    os.remove(audio_path)
    print("=== HAL DEMO VIDEO GENERATION COMPLETED ===")

if __name__ == "__main__":
    main()
