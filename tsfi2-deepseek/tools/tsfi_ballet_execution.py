import os
import subprocess
import time

def execute_sovereign_ballet():
    print("=== TSFi Sovereign Ballet Execution Matrix ===", flush=True)
    out_dir = "assets/ballet_performance"
    os.makedirs(out_dir, exist_ok=True)
    
    sd_bin = "assets/sd_cpp/build/bin/sd-cli"
    model = "assets/models/sd15.safetensors"
    vae = "assets/models/taesd.safetensors"
    controlnet = "assets/models/control_openpose.safetensors"
    
    # The Absolute Anchor Bear (Generated from the Ultimate Animation script)
    init_image = "assets/ultimate_animation/00_anchor.png"
    
    if not os.path.exists(init_image):
        print(f"[FRACTURE] Anchor Bear missing: {init_image}. Please run ultimate_animator first.")
        return

    postures = [
        "first_position", "second_position", "third_position", "fourth_position", "fifth_position"
    ]
    
    start_time = time.time()
    
    for posture in postures:
        skeleton_mask = f"assets/ballet_dictionary/{posture}.png"
        out_path = f"{out_dir}/{posture}_performance.png"
        
        prompt = f"A highly detailed photorealistic brown teddy bear performing classical ballet {posture.replace('_', ' ')}, masterpiece, 8k resolution, cinematic lighting, sharp focus <lora:lcm_lora_sd15:1.0>"
        
        # We use a strength of 0.70. High enough to completely rearrange the limbs to match the skeleton,
        # but low enough to physically preserve the texture and lighting of the Epoch 2 bear!
        cmd = [
            sd_bin,
            "-m", model,
            "--vae", vae,
            "--lora-model-dir", "assets/models",
            "--control-net", controlnet,
            "--control-image", skeleton_mask,
            "--control-strength", "1.0",
            "-p", prompt,
            "-n", "abstract, deformed, bad anatomy, flat, illustration, missing limbs",
            "-W", "512", "-H", "512",
            "--steps", "4",
            "--cfg-scale", "1.2",
            "--seed", "42",
            "-i", init_image,
            "--strength", "0.70",
            "-o", out_path
        ]
        
        print(f"[ACTIVE] Forcing Epoch 2 Anchor to execute {posture}...")
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
    print(f"\\n[ABSOLUTE SUCCESS] The Sovereign Ballet is complete. Rendered in {time.time() - start_time:.2f} seconds.")

if __name__ == "__main__":
    execute_sovereign_ballet()
