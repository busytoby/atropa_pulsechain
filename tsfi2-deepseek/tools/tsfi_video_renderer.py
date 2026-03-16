import os
import subprocess
import math

def fract(x):
    return x - math.floor(x)

def procedural_noise(px, py, time_offset):
    # Replicating DeepSeek's GLSL hash noise: fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
    # We add the frame time offset to make the noise organically boil across the 240 frames
    dot_val = (px * 12.9898) + (py * 78.233) + time_offset
    return fract(math.sin(dot_val) * 43758.5453)

def render_headless_video():
    print("=== TSFi Headless Video Encoder: DeepSeek GLSL Simulation ===", flush=True)
    
    W = 1024
    H = 1536
    FPS = 60
    DURATION = 4 # seconds
    TOTAL_FRAMES = FPS * DURATION
    
    output_file = "assets/atropa_glsl_phase3.mp4"
    if not os.path.exists("assets"):
        os.makedirs("assets")
        
    print(f"[INFO] Initiating FFmpeg hardware pipeline to {output_file}...", flush=True)
    
    ffmpeg_cmd = [
        'ffmpeg', '-y', '-f', 'rawvideo', '-vcodec', 'rawvideo',
        '-s', f'{W}x{H}', '-pix_fmt', 'rgb24', '-r', str(FPS),
        '-i', '-', '-c:v', 'libx264', '-preset', 'fast', '-pix_fmt', 'yuv420p',
        output_file
    ]
    
    try:
        process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE, stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"[FRACTURE] Failed to launch FFmpeg: {e}")
        return

    print(f"[ACTIVE] Synthesizing GLSL Procedural Matrices ({TOTAL_FRAMES} frames)...", flush=True)
    
    for frame in range(TOTAL_FRAMES):
        pixels = bytearray(b'\x00' * (W * H * 3))
        
        time_offset = frame * 0.1 # Forces the noise to evolve per frame
        pulse = math.sin((frame / TOTAL_FRAMES) * math.pi * 2) 
        
        # Shader replication
        for y in range(H):
            row_idx = y * W * 3
            # Normalized coordinates (like the GLSL shader p = vec2(storePos) / vec2(1024, 1536))
            py = y / H
            for x in range(W):
                px = x / W
                idx = row_idx + (x * 3)
                
                # Z-Ordering Execution
                color = None
                
                # 1. Eyes (Top Layer)
                # DeepSeek wrote: distance(p, vec2(0.5, 0.4)) < 0.02
                dist_eye1 = math.hypot(px - 0.5, py - 0.4)
                dist_eye2 = math.hypot(px - 0.5, py - 0.6) # Wait, DeepSeek placed the second eye at Y=0.6! It's a vertical cyclops! We must render its exact logic.
                
                # We add the pulse to the pupil radius to make it blink
                pulse_radius = 0.02 + (pulse * 0.005)
                if dist_eye1 < pulse_radius or dist_eye2 < (0.045 + pulse * 0.01):
                    color = (0, 255, 0) # Green Eye
                    
                # 2. Ears
                elif math.hypot(px - 0.362, py - 0.25) < 0.075 or math.hypot(px - 0.662, py - 0.25) < 0.075:
                    color = (255, 0, 0) # DeepSeek hallucinated Red ears
                    
                # 3. Cranium
                elif math.hypot(px - 0.5, py - 0.5) < 0.2:
                    color = (255, 0, 0) # DeepSeek hallucinated Red head
                    
                # 4. Sick Body
                elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                    if procedural_noise(px * 100.0, py * 100.0, time_offset) > 0.8:
                        color = (84, 107, 45) # Sick Green (0.33, 0.42, 0.18)
                    else:
                        color = (91, 63, 51) # Brown (0.36, 0.25, 0.20)
                        
                if color:
                    pixels[idx:idx+3] = bytes(color)
                        
        process.stdin.write(pixels)
        
    process.stdin.close()
    process.wait()
    
    file_size = os.path.getsize(output_file)
    print(f"\n[ABSOLUTE SUCCESS] Phase 3 GLSL Rendering Complete.")
    print(f"  -> Generated: {output_file}")
    print(f"  -> File Mass: {file_size / 1024 / 1024:.2f} MB")

if __name__ == "__main__":
    render_headless_video()
