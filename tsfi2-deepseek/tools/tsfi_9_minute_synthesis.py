import os
import subprocess
import math

def fract(x):
    return x - math.floor(x)

def procedural_noise(px, py, time_offset):
    # The DeepSeek GLSL Hash Matrix
    dot_val = (px * 12.9898) + (py * 78.233) + time_offset
    return fract(math.sin(dot_val) * 43758.5453)

def render_9_minute_matrix():
    print("=== TSFi Extreme Duration Encoder: 9-Minute Matrix ===", flush=True)
    
    W = 1024
    H = 1536
    FPS = 30 # Reduced from 60 to prevent the .mp4 from reaching multiple gigabytes
    DURATION = 540 # 9 minutes
    TOTAL_FRAMES = FPS * DURATION
    
    output_file = "assets/atropa_9_minute_choreography.mp4"
    if not os.path.exists("assets"):
        os.makedirs("assets")
        
    print(f"[INFO] Initiating FFmpeg hardware pipeline to {output_file}...", flush=True)
    print(f"[WARN] This will synthesize {TOTAL_FRAMES} massive frames. It will take significant time.", flush=True)
    
    ffmpeg_cmd = [
        'ffmpeg', '-y', '-f', 'rawvideo', '-vcodec', 'rawvideo',
        '-s', f'{W}x{H}', '-pix_fmt', 'rgb24', '-r', str(FPS),
        '-i', '-', '-c:v', 'libx264', '-preset', 'ultrafast', '-crf', '23', '-pix_fmt', 'yuv420p',
        output_file
    ]
    
    try:
        process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"[FRACTURE] Failed to launch FFmpeg: {e}")
        return

    # Pre-allocate the memory block to prevent python garbage collection stutter
    pixels = bytearray(b'\x00' * (W * H * 3))
    
    # Cache constants for speed
    brown = bytes([91, 63, 51])
    sick_green = bytes([84, 107, 45])
    fracture_red = bytes([255, 0, 0])
    krowz_green = bytes([0, 255, 0])
    
    for frame in range(TOTAL_FRAMES):
        # We clear only the bounding box area to save millions of CPU cycles per frame
        # Actually, it's faster in python to just recreate the array or overwrite it
        pixels[:] = b'\x00' * (W * H * 3)
        
        # A massive 9-minute sine wave evolution 
        # The bear breathes, pulses, and gets progressively sicker over 9 minutes
        time_offset = frame * 0.05 
        macro_evolution = frame / TOTAL_FRAMES # 0.0 to 1.0 over 9 minutes
        pulse = math.sin(macro_evolution * math.pi * 50) # 25 full breaths over 9 minutes
        
        # Geometry Constants
        head_radius = 200 + (pulse * 15)
        pupil_radius = 20 + (abs(pulse) * 15)
        sickness_spread = 0.8 - (macro_evolution * 0.3) # The sickness threshold lowers, making the bear greener over time
        
        for y in range(100, 1100):
            row_idx = y * W * 3
            py = y / H
            
            for x in range(250, 750):
                px = x / W
                idx = row_idx + (x * 3)
                
                dist_eye1 = math.hypot(px - 0.5, py - 0.4)
                dist_eye2 = math.hypot(px - 0.5, py - 0.6) 
                
                pulse_radius = 0.02 + (pulse * 0.005)
                
                if dist_eye1 < pulse_radius or dist_eye2 < (0.045 + pulse * 0.01):
                    pixels[idx:idx+3] = krowz_green
                    
                elif math.hypot(px - 0.362, py - 0.25) < 0.075 or math.hypot(px - 0.662, py - 0.25) < 0.075:
                    pixels[idx:idx+3] = fracture_red
                    
                elif math.hypot(px - 0.5, py - 0.5) < (0.2 + (pulse * 0.01)):
                    pixels[idx:idx+3] = fracture_red
                    
                elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                    if procedural_noise(px * 100.0, py * 100.0, time_offset) > sickness_spread:
                        pixels[idx:idx+3] = sick_green
                    else:
                        pixels[idx:idx+3] = brown
                        
        process.stdin.write(pixels)
        
        if frame % 300 == 0:
            print(f"-> Generated {frame}/{TOTAL_FRAMES} Frames ({(frame/TOTAL_FRAMES)*100:.1f}%)")
        
    process.stdin.close()
    process.wait()
    
    file_size = os.path.getsize(output_file)
    print(f"\\n[ABSOLUTE SUCCESS] 9-Minute Choreography Complete.")
    print(f"  -> Generated: {output_file}")
    print(f"  -> File Mass: {file_size / 1024 / 1024:.2f} MB")

if __name__ == "__main__":
    render_9_minute_matrix()
