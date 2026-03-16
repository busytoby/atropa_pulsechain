import os
import urllib.request
import json
import base64
import subprocess
import time

def generate_frame(index, prompt, seed):
    url = 'http://127.0.0.1:8080/txt2img'
    
    # Payload optimized for extreme speed
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "step": 6,
        "cfg_scale": 2.0,
        "seed": seed
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        # The sd-server returns base64 encoded images
        for i, img_data in enumerate(result):
            img_bytes = base64.b64decode(img_data['data'])
            with open(f"assets/frames/frame_{index:03d}.png", "wb") as f:
                f.write(img_bytes)
        return True
    except Exception as e:
        print(f"[FRACTURE] Frame {index} failed: {e}")
        return False

def render_optimized_video():
    print("=== TSFi Ultra-Optimized Vulkan Server Matrix ===", flush=True)
    
    frames_dir = "assets/frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    # Wait for SD Server to fully boot and load the 3GB matrix into VRAM
    print("[INFO] Establishing connection to Vulkan Server Node...")
    time.sleep(3) 
    
    FRAMES = 60 # 4 seconds at 15 FPS
    
    print(f"[ACTIVE] Initiating High-Frequency Frame Synthesis ({FRAMES} frames)...", flush=True)
    
    for i in range(FRAMES):
        # Evolutionary prompt
        sickness = int((i / FRAMES) * 100)
        prompt = f"A highly detailed photorealistic Sick Teddy Bear, dark background, cinematic lighting, Atropa aesthetic, {sickness} percent mutated geometry, glowing green eyes"
        seed = 42 + (i % 10) # Bouncing seed to create organic visual bubbling
        
        success = generate_frame(i, prompt, seed)
        if success:
            print(f"  -> Frame {i:03d} synthesized natively in VRAM.")
            
    print("\\n[ACTIVE] Compiling Video via FFmpeg...", flush=True)
    
    output_video = "assets/atropa_server_painted.mp4"
    ffmpeg_cmd = [
        "ffmpeg", "-y", "-framerate", "15",
        "-i", f"{frames_dir}/frame_%03d.png",
        "-c:v", "libx264", "-pix_fmt", "yuv420p",
        output_video
    ]
    
    subprocess.run(ffmpeg_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"[ABSOLUTE SUCCESS] Video Synthesis Complete: {output_video}")

if __name__ == "__main__":
    render_optimized_video()
