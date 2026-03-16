import os
import time
import subprocess
import base64
import math
import io
import json
import urllib.request
import ctypes
from PIL import Image, ImageDraw

def execute_ultimate_animation():
    print("=== TSFi Ultimate DNA Kinematic Animator ===", flush=True)
    os.makedirs("assets/ultimate_animation", exist_ok=True)
    
    # Load TSFi C-Kernel for Biological Stripping
    try:
        silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
        silhouette_lib.extract_silhouette.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int]
    except Exception as e:
        print(f"[FRACTURE] C-Kernel offline: {e}")
        return

    TOTAL_FRAMES = 50
    FPS = 8

    print("[ACTIVE] Synthesizing Epoch 2 (The Flawless Anchor)...")
    
    # 1. Generate the Masterpiece Anchor Bear (Epoch 2 equivalent)
    prompt = "A perfectly proportioned, highly detailed photorealistic brown teddy bear sitting on a floor, masterpiece, visible ears, visible eyes, visible snout, brightly lit studio photography"
    req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/txt2img', data=json.dumps({
        "prompt": prompt, "width": 512, "height": 512, "steps": 20, "cfg_scale": 7.0, "seed": 42
    }).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    img_bytes = base64.b64decode(json.loads(urllib.request.urlopen(req).read())['images'][0])
    
    anchor_bear = Image.open(io.BytesIO(img_bytes)).convert("RGB")
    anchor_bear.save("assets/ultimate_animation/00_anchor.png")

    print("[ACTIVE] Executing Biological Strip (Capturing Ultimate DNA)...")
    
    # 2. Extract the Ultimate DNA Mask using the C-Kernel
    w_c, h_c = 512, 512
    in_pixels = (ctypes.c_uint8 * (w_c * h_c * 3)).from_buffer_copy(anchor_bear.tobytes())
    binary_silhouette = (ctypes.c_uint8 * (w_c * h_c))()
    silhouette_lib.extract_silhouette(in_pixels, binary_silhouette, w_c, h_c)
    
    # Convert C-array to PIL Binary Mask
    ultimate_dna_mask = Image.frombuffer("L", (w_c, h_c), binary_silhouette, "raw", "L", 0, 1).point(lambda p: 255 if p > 0 else 0)
    ultimate_dna_mask.save("assets/ultimate_animation/01_ultimate_dna.png")
    
    # 3. Lock the Stage Background
    locked_background = anchor_bear.copy().convert("RGBA")
    bg_pixels = locked_background.load()
    m_pixels = ultimate_dna_mask.load()
    for y in range(h_c):
        for x in range(w_c):
            if m_pixels[x, y] == 255: bg_pixels[x, y] = (0, 0, 0, 0) # Erase Bear
    locked_background.save("assets/ultimate_animation/02_locked_stage.png")
    
    # 4. Isolate the Photorealistic Bear Pixels
    isolated_bear = anchor_bear.copy().convert("RGBA")
    bear_pixels = isolated_bear.load()
    for y in range(h_c):
        for x in range(w_c):
            if m_pixels[x, y] == 0: bear_pixels[x, y] = (0, 0, 0, 0) # Erase Room
    
    print("\\n[ACTIVE] Initiating Kinematic Affine Deformation Loop...")
    
    for f in range(TOTAL_FRAMES):
        out_path = f"assets/ultimate_animation/frame_{f:04d}.png"
        
        # Calculate Kinematics
        translate_y = 0
        translate_x = 0
        prompt_state = "sitting"
        img2img_strength = 0.40 # Base strength for temporal consistency
        
        if f > 20 and f <= 35:
            # Stand Up Phase: DO NOT STRETCH. Physically lift the 1:1 mass into the air.
            progress = (f - 20) / 15.0
            translate_y = -int(progress * 150) # Lift the bear exactly 150 pixels straight up
            prompt_state = "standing up tall on two long legs, full body visible"
            img2img_strength = 0.65 # Higher strength required so it has the freedom to hallucinate the missing legs in the void beneath it!
            
        elif f > 35:
            # Walk Off Left: Remain lifted, shift left.
            progress = (f - 35)
            translate_y = -150
            translate_x = -int(progress * 20) # Move left 20 pixels per frame
            
            # Simulate a bobbing walk cycle by oscillating the Y axis slightly
            walk_bob = int(math.sin(progress * math.pi * 4.0) * 15.0)
            translate_y += walk_bob
            
            prompt_state = "walking off screen to the left, standing on two legs, swinging arms, striding"
            img2img_strength = 0.65 
            
        # Paste the UNWARPED, perfect Ultimate DNA onto a blank canvas, applying ONLY translation
        frame_composition = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
        
        # Center the bear on the stage, apply the kinematic shifts
        base_x = 0 + translate_x
        base_y = 0 + translate_y
        frame_composition.paste(isolated_bear, (base_x, base_y))
        
        # Overlay the translated bear onto the perfectly locked Background Room
        final_init_image = Image.alpha_composite(locked_background, frame_composition).convert("RGB")
        
        # Encode for Stable Diffusion
        buffered = io.BytesIO()
        final_init_image.save(buffered, format="PNG")
        b64_mask = base64.b64encode(buffered.getvalue()).decode('utf-8')
        
        # Engage LCM img2img. The AI will see the bear floating and will draw legs to connect it to the floor.
        req_prompt = f"A photorealistic brown teddy bear {prompt_state}, masterpiece, cinematic lighting <lora:lcm_lora_sd15:1.0>"
        
        data = {
            "prompt": req_prompt,
            "negative_prompt": "abstract, deformed, bad anatomy, warped, stretched, squished, floating",
            "width": 512, "height": 512, "steps": 4, "cfg_scale": 1.5, 
            "seed": 42 + f,
            "init_images": [b64_mask],
            "denoising_strength": float(img2img_strength)
        }
        
        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
        req.add_header('Content-Type', 'application/json')
        try:
            response = urllib.request.urlopen(req)
            img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
            with open(out_path, "wb") as f_out: f_out.write(img_bytes)
            print(f"  -> Generated Frame {f}/{TOTAL_FRAMES} | Kinematic State: {prompt_state}")
        except Exception as e:
            print(f"  [FRACTURE] Frame {f} Failed: {e}")
            
    print("\\n[ACTIVE] Compiling Ultimate MP4 Matrix...")
    output_video = "assets/atropa_ultimate_kinematic_walk.mp4"
    subprocess.run(["ffmpeg", "-y", "-framerate", str(FPS), "-i", "assets/ultimate_animation/frame_%04d.png", "-c:v", "libx264", "-preset", "slow", "-crf", "20", "-pix_fmt", "yuv420p", output_video], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print(f"[ABSOLUTE SUCCESS] The Sovereign Animation is sealed: {output_video}")

if __name__ == "__main__":
    execute_ultimate_animation()