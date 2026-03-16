import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

spider_code = """
# ---------------------------------------------------------
# THE SPIDER'S CONTENT EVALUATOR
# ---------------------------------------------------------
def run_spider_critic(rendered_img_path, original_prompt):
    try:
        img = cv2.imread(rendered_img_path)
        if img is None: return "NONE"

        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
        brightness = np.mean(gray)
        
        lower_green = np.array([35, 40, 40])
        upper_green = np.array([85, 255, 200])
        mask_gator = cv2.inRange(hsv, lower_green, upper_green)
        gator_pixels = cv2.countNonZero(mask_gator)
        
        lower_brown = np.array([10, 50, 20])
        upper_brown = np.array([25, 255, 200])
        mask_bear = cv2.inRange(hsv, lower_brown, upper_brown)
        bear_pixels = cv2.countNonZero(mask_bear)
        
        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0
        
        if brightness < 80.0:
            return "INCREASE_LIGHTING"
        elif gator_presence < 2.0 and "alligator" in original_prompt.lower():
            return "EMPHASIZE_GATOR"
        elif bear_presence < 5.0 and "bear" in original_prompt.lower():
            return "EMPHASIZE_BEAR"
            
        return "NONE"
    except:
        return "NONE"
"""

code = code.replace(
    '# ---------------------------------------------------------\n# AUTONOMOUS VISION CRITIC (THE ALLIGATOR)',
    spider_code + '\n# ---------------------------------------------------------\n# AUTONOMOUS VISION CRITIC (THE ALLIGATOR)'
)

# Wire the Spider into the retry loop
old_retry = """                # Alligator evaluates the output
                passed, reason, advice = run_vision_critic(out_path, mask_path, decay_ratio)
                if passed:
                    break # Frame is physically valid, proceed to multiplexing.
                    
                print(f"[THE ALLIGATOR BITES] Frame {local_f_idx} failed: {reason}. Retrying (Attempt {attempt+1}/3). Action: {advice}")
                
                # Apply Dynamic Self-Improvement based on Alligator's feedback
                if advice == "INCREASE_CONTROL":
                    dynamic_control = min(1.0, dynamic_control + 0.15)
                elif advice == "SCRAMBLE_SEED":
                    divergent_seed = str(int(divergent_seed) + 777)
                elif advice == "INCREASE_STEPS":
                    cmd[cmd.index("--steps")+1] = "6"
                elif advice == "INCREASE_CFG":
                    dynamic_cfg += 0.5"""

new_retry = """                # 1. The Alligator evaluates the physical Form
                passed, reason, advice = run_vision_critic(out_path, mask_path, decay_ratio)
                
                # 2. The Spider evaluates the Content Textures
                spider_advice = "NONE"
                if passed:
                    spider_advice = run_spider_critic(out_path, prompt)
                    if spider_advice != "NONE":
                        passed = False
                        reason = f"SPIDER_REJECT: {spider_advice}"
                        advice = spider_advice

                if passed:
                    break # Frame is physically valid and texturally sound
                    
                print(f"[CRITIC BITE] Frame {local_f_idx} failed: {reason}. Retrying (Attempt {attempt+1}/3). Action: {advice}")
                
                # Apply Dynamic Self-Improvement based on Critic feedback
                if advice == "INCREASE_CONTROL":
                    dynamic_control = min(1.0, dynamic_control + 0.15)
                elif advice == "SCRAMBLE_SEED":
                    divergent_seed = str(int(divergent_seed) + 777)
                elif advice == "INCREASE_STEPS":
                    cmd[cmd.index("--steps")+1] = "6"
                elif advice == "INCREASE_CFG":
                    dynamic_cfg += 0.5
                elif advice == "INCREASE_LIGHTING":
                    # DeepSeek-style prompt injection for immediate correction
                    prompt = "EXTREMELY BRIGHT BLINDING SOFTBOX LIGHTING, OVEREXPOSED, " + prompt
                elif advice == "EMPHASIZE_GATOR":
                    prompt = "MASSIVE BRIGHT GREEN ALLIGATOR PLUSH, " + prompt
                elif advice == "EMPHASIZE_BEAR":
                    prompt = "MASSIVE BROWN TEDDY BEAR PLUSH, " + prompt"""

code = code.replace(old_retry, new_retry)

with open(filepath, 'w') as f:
    f.write(code)

