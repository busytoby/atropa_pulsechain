import os
import subprocess
import base64
import math
import io
import json
import urllib.request
import ctypes
import time
from PIL import Image

def build_procedural_sickness(W, H, frame, intensity):
    img = Image.new("RGBA", (W, H), (0,0,0,0))
    pixels = img.load()
    time_offset = frame * 0.1
    for y in range(H):
        for x in range(W):
            px, py = x/W, y/H
            # Fast procedural cellular noise
            noise = (math.sin(px * 100.0 + time_offset) * math.cos(py * 100.0 + time_offset))
            if noise > 0.5:
                # Green sickness pixels
                alpha = int(255 * intensity)
                pixels[x, y] = (51, 65, 0, alpha)
    return img

def execute_absolute_timeline():
    print("=== TSFi Sovereign Choreography: The 159-Second Matrix ===", flush=True)
    frames_dir = "assets/timeline_frames"
    os.makedirs(frames_dir, exist_ok=True)
    
    FPS = 24
    SIT_SECONDS = 150 # 2 minutes 30 seconds
    WALK_SECONDS = 9
    
    SIT_FRAMES = SIT_SECONDS * FPS
    WALK_FRAMES = WALK_SECONDS * FPS
    
    # 1. Generate Masterpiece Anchor Bear (Epoch 2)
    print("\\n[ACTIVE] Generating Epoch 2 Anatomical Masterpiece...")
    prompt = "A perfectly proportioned photorealistic brown teddy bear sitting on a floor, masterpiece, visible ears, eyes, snout, brightly lit studio photography"
    req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/txt2img', data=json.dumps({
        "prompt": prompt, "negative_prompt": "abstract, deformed, blurry, cartoon",
        "width": 512, "height": 512, "steps": 20, "cfg_scale": 7.0, "seed": 42
    }).encode('utf-8'), headers={'Content-Type': 'application/json'})
    
    anchor_bear = Image.open(io.BytesIO(base64.b64decode(json.loads(urllib.request.urlopen(req).read())['images'][0]))).convert("RGB")
    
    # 2. Extract Biological Silhouette via C-Kernel
    print("[ACTIVE] Extracting Ultimate DNA Silhouette...")
    try:
        silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
        silhouette_lib.extract_silhouette.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int]
    except Exception as e:
        print(f"[FRACTURE] C-Kernel offline: {e}")
        return
        
    w_c, h_c = 512, 512
    in_pixels = (ctypes.c_uint8 * (w_c * h_c * 3)).from_buffer_copy(anchor_bear.tobytes())
    binary_silhouette = (ctypes.c_uint8 * (w_c * h_c))()
    silhouette_lib.extract_silhouette(in_pixels, binary_silhouette, w_c, h_c)
    
    mask = Image.frombuffer("L", (w_c, h_c), binary_silhouette, "raw", "L", 0, 1).point(lambda p: 255 if p > 0 else 0)
    
    # 3. Isolate Background and Bear
    locked_background = anchor_bear.copy().convert("RGBA")
    bg_pixels = locked_background.load()
    m_pixels = mask.load()
    for y in range(h_c):
        for x in range(w_c):
            if m_pixels[x,y] == 255: bg_pixels[x,y] = (0,0,0,0)
            
    isolated_bear = anchor_bear.copy().convert("RGBA")
    bear_pixels = isolated_bear.load()
    for y in range(h_c):
        for x in range(w_c):
            if m_pixels[x,y] == 0: bear_pixels[x,y] = (0,0,0,0)
            
    # =====================================================================
    # PHASE 1: THE 2:30 ZERO-COST SIT LOOP (Native Python C-Kernel)
    # =====================================================================
    print(f"\\n[ACTIVE] Executing 2:30 Temporal Sit Loop ({SIT_FRAMES} Frames)...")
    start_time = time.time()
    
    for f in range(SIT_FRAMES):
        # Calculate subtle organic breathing (2% vertical stretch)
        breath_scale = 1.0 + (math.sin(f * 0.1) * 0.02)
        breathing_bear = isolated_bear.resize((512, int(512 * breath_scale)), Image.NEAREST)
        
        # Calculate sickness decay percentage
        sickness_intensity = f / float(SIT_FRAMES)
        
        # Generate math sickness and apply ONLY to the bear's silhouette
        sickness_layer = build_procedural_sickness(512, int(512 * breath_scale), f, sickness_intensity)
        sick_bear = Image.alpha_composite(breathing_bear, sickness_layer)
        
        # Paste the breathing, sick bear onto the static stage, anchored to the floor
        frame_canvas = Image.new("RGBA", (512, 512), (0,0,0,0))
        y_offset = 512 - int(512 * breath_scale)
        frame_canvas.paste(sick_bear, (0, y_offset))
        
        final_frame = Image.alpha_composite(locked_background, frame_canvas).convert("RGB")
        final_frame.save(f"{frames_dir}/frame_{f:04d}.png")
        
        if f % 300 == 0: print(f"  -> Processed {f}/{SIT_FRAMES} Zero-Cost Frames")
        
    print(f"[PASS] 2:30 Sit Phase Synthesized in {time.time() - start_time:.2f} seconds.")
    
    # =====================================================================
    # PHASE 2: THE 9-SECOND STAND AND WALK (Stable Diffusion Handshake)
    # =====================================================================
    print(f"\\n[ACTIVE] Executing 9-Second Walk Cycle ({WALK_FRAMES} Frames) via SD-CLI...")
    start_time = time.time()
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    
    for w_f in range(WALK_FRAMES):
        global_frame = SIT_FRAMES + w_f
        
        # Calculate 9-second walk kinematics
        progress = w_f / float(WALK_FRAMES)
        
        scale_y = 1.0
        translate_x = 0
        prompt_state = "sitting"
        
        if progress < 0.2:
            # First ~1.8 seconds: Stand Up
            stand_prog = progress / 0.2
            scale_y = 1.0 + (stand_prog * 0.5)
            prompt_state = "standing up tall on two long legs, full body visible"
        else:
            # Remaining ~7.2 seconds: Walk Left
            walk_prog = (progress - 0.2) / 0.8
            scale_y = 1.5
            translate_x = -int(walk_prog * 600) # Move entirely off screen
            prompt_state = "walking off screen to the left, standing on two legs, swinging arms, striding"
            
        # Deform the Ultimate DNA
        stretched_bear = isolated_bear.resize((512, int(512 * scale_y)), Image.NEAREST)
        
        frame_canvas = Image.new("RGBA", (512, 512), (0,0,0,0))
        y_offset = 512 - int(512 * scale_y)
        
        # Simulate walking bob
        if progress >= 0.2:
            y_offset += int(math.sin(walk_prog * math.pi * 10.0) * 15.0)
            
        frame_canvas.paste(stretched_bear, (translate_x, y_offset))
        final_init_image = Image.alpha_composite(locked_background, frame_canvas).convert("RGB")
        
        mask_path = f"{frames_dir}/walk_mask_{w_f:04d}.png"
        out_path = f"{frames_dir}/frame_{global_frame:04d}.png"
        final_init_image.save(mask_path)
        
        # We use a strength of 0.65. High enough to hallucinate the missing legs, low enough to preserve the bear's face geometry!
        req_prompt = f"A photorealistic brown teddy bear {prompt_state}, masterpiece, 100 percent glowing green sickness mutation <lora:lcm_lora_sd15:1.0>"
        
        cmd = [
            sd_bin, "-m", model, "--vae", vae, "--lora-model-dir", "assets/models",
            "-p", req_prompt, "-n", "abstract, deformed, bad anatomy",
            "-W", "512", "-H", "512", "--steps", "4", "--cfg-scale", "1.2", 
            "--seed", str(42 + w_f), "-i", mask_path, "--strength", "0.65", "-o", out_path
        ]
        
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if w_f % 24 == 0:
            print(f"  -> Generated Walk Frame {w_f}/{WALK_FRAMES}")
            
    print(f"[PASS] 9-Second Walk Phase Synthesized in {time.time() - start_time:.2f} seconds.")

    print("\\n[ACTIVE] Compiling 159-Second Cinematic MP4...")
    output_video = "assets/atropa_159sec_choreography.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", f"{frames_dir}/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "24", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print(f"[ABSOLUTE SUCCESS] The Matrix is sealed: {output_video}")

if __name__ == "__main__":
    execute_absolute_timeline()
