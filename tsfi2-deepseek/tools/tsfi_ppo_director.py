import os
import time
import subprocess
import random
import re
import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor
import math

# Use the established mathematical mask generator
def generate_base_mask(W, H):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    for y in range(H):
        for x in range(W):
            if math.hypot(x - (W/2), y - (H/2)) < (W * 0.2):
                pixels[x, y] = (76, 29, 14)
    return img

class TSFiPPODirector:
    def __init__(self):
        print("[INFO] Booting Moondream2 VLM (The Reward Oracle)...")
        torch.set_num_threads(os.cpu_count())
        self.vlm = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        self.processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        
        self.champion_params = {
            "steps": 4,
            "res": 256,
            "cfg": 1.0
        }
        self.champion_reward = -9999.0
        
    def mutate_parameters(self):
        # Stochastic exploration of the Action Space
        chal = self.champion_params.copy()
        
        mutation_target = random.choice(["steps", "res", "cfg"])
        if mutation_target == "steps":
            chal["steps"] = max(1, min(8, chal["steps"] + random.choice([-1, 1])))
        elif mutation_target == "res":
            # Must remain a multiple of 64 for the VAE
            resolutions = [128, 192, 256, 320, 384]
            current_idx = resolutions.index(chal["res"])
            new_idx = max(0, min(len(resolutions)-1, current_idx + random.choice([-1, 1])))
            chal["res"] = resolutions[new_idx]
        elif mutation_target == "cfg":
            chal["cfg"] = max(1.0, min(5.0, chal["cfg"] + random.choice([-0.5, 0.5])))
            
        return chal

    def evaluate_quality(self, img_path):
        try:
            img = Image.open(img_path).convert("RGB")
            enc_image = self.vlm.encode_image(img)
            q = "Rate the photorealism, structural detail, and visual quality of this image on a scale from 0 to 100. Reply with exactly one integer."
            ans = self.vlm.answer_question(enc_image, q, self.processor)
            
            # Extract the first number found in the VLM's response
            match = re.search(r'\d+', ans)
            if match:
                return float(match.group())
            return 10.0 # Heavy penalty if the VLM hallucinates non-numbers
        except Exception as e:
            print(f"  [!] VLM Fracture: {e}")
            return 0.0

    def execute_epoch(self, epoch):
        print(f"\\n=== PPO EPOCH {epoch} ===")
        params = self.mutate_parameters()
        print(f"[ACTIVE] Testing Parameters: Steps={params['steps']}, Res={params['res']}, CFG={params['cfg']:.1f}")
        
        mask_path = "assets/ppo_mask.png"
        out_path = "assets/ppo_output.png"
        generate_base_mask(params['res'], params['res']).save(mask_path)
        
        cmd = [
            "assets/sd_cpp/build/bin/sd-cli",
            "-m", "assets/models/sd15.safetensors",
            "--vae", "assets/models/taesd.safetensors",
            "-p", "A highly detailed photorealistic Sick Teddy Bear, matted brown fur, masterpiece, 8k resolution",
            "-W", str(params['res']), "-H", str(params['res']),
            "--steps", str(params['steps']),
            "--cfg-scale", str(params['cfg']),
            "--type", "f16",
            "--seed", "42",
            "-i", mask_path,
            "--strength", "0.85",
            "-o", out_path
        ]
        
        # 1. Measure Latency
        start_time = time.time()
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        latency = time.time() - start_time
        
        # 2. Measure Quality
        quality = self.evaluate_quality(out_path)
        
        # 3. Calculate Final Reward
        # Reward = Quality (0-100) - (Latency_Seconds * 5.0)
        # E.g., Quality 80, Time 2s -> 80 - 10 = 70 Reward
        # E.g., Quality 90, Time 15s -> 90 - 75 = 15 Reward
        latency_penalty = latency * 5.0
        reward = quality - latency_penalty
        
        print(f"  -> Quality Score:  {quality:.1f} / 100")
        print(f"  -> Latency:        {latency:.2f} seconds")
        print(f"  -> Total Reward:   {reward:.2f}")
        
        if reward > self.champion_reward:
            print(f"  [+++] NEW CHAMPION ARCHITECTURE SECURED!")
            self.champion_reward = reward
            self.champion_params = params

def run_training_loop():
    os.makedirs("assets", exist_ok=True)
    director = TSFiPPODirector()
    
    print("\\n[ACTIVE] Initiating Autonomous TSFi Optimization Matrix...")
    for i in range(1, 6): # Run 5 epochs to demonstrate the architecture
        director.execute_epoch(i)
        
    print("\\n=== PPO TRAINING COMPLETE ===")
    print(f"Optimal Hardware Parameters Discovered:")
    print(f"-> Steps: {director.champion_params['steps']}")
    print(f"-> Resolution: {director.champion_params['res']}x{director.champion_params['res']}")
    print(f"-> CFG Scale: {director.champion_params['cfg']}")
    print(f"-> Peak Reward: {director.champion_reward:.2f}")

if __name__ == "__main__":
    run_training_loop()
