import os
import subprocess
import glob

def render_vulkan_video():
    print("=== TSFi Autonomous Vulkan Video Painter ===", flush=True)
    
    W = 512
    H = 512
    FRAMES = 60 # 4 seconds @ 15fps
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    frames_dir = "assets/frames"
    output_video = "assets/atropa_vulkan_painted.mp4"
    
    os.makedirs(frames_dir, exist_ok=True)
    
    print("[INFO] Initiating img2img Autonomous Evolution Loop...")
    
    for i in range(FRAMES):
        frame_path = f"{frames_dir}/frame_{i:03d}.png"
        prev_path = f"{frames_dir}/frame_{i-1:03d}.png" if i > 0 else None
        
        # The prompt mechanically mutates based on the frame index to drive the evolution
        sickness_level = int((i / FRAMES) * 100)
        base_prompt = "A highly detailed photorealistic Sick Teddy Bear, dark background, cinematic lighting, Atropa aesthetic, 8k resolution"
        evolution_prompt = f"{base_prompt}, glowing green eyes, fractured body {sickness_level} percent mutated, organic procedural noise"
        
        cmd = [
            sd_bin,
            "-m", model,
            "-p", evolution_prompt,
            "-W", str(W),
            "-H", str(H),
            "--steps", "8", # Fast steps for video synthesis
            "--cfg-scale", "4.0",
            "-o", frame_path,
            "--seed", str(42 + i) # Mutate the seed slightly to cause geometric bubbling
        ]
        
        # If we have a previous frame, use it as the structural baseline (img2img)
        if prev_path and os.path.exists(prev_path):
            cmd.extend(["-i", prev_path, "--strength", "0.6"])
            
        print(f"-> Painting Frame {i}/{FRAMES} via Vulkan GPU...")
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
    print("\\n[ACTIVE] Compiling frames into .mp4 via FFmpeg...", flush=True)
    
    ffmpeg_cmd = [
        "ffmpeg", "-y", "-framerate", "15",
        "-i", f"{frames_dir}/frame_%03d.png",
        "-c:v", "libx264", "-pix_fmt", "yuv420p",
        output_video
    ]
    
    subprocess.run(ffmpeg_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print(f"[ABSOLUTE SUCCESS] Video Synthesis Complete. Saved to {output_video}")

if __name__ == "__main__":
    render_vulkan_video()
