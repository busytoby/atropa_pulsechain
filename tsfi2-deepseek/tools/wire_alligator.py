import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Add the import at the top
code = code.replace(
    'from PIL import Image, ImageDraw, ImageFont',
    'from PIL import Image, ImageDraw, ImageFont\nimport cv2\nimport numpy as np'
)

# Paste the Alligator code directly into the vtuber script so it has native access
alligator_code = """
# ---------------------------------------------------------
# AUTONOMOUS VISION CRITIC (THE ALLIGATOR)
# ---------------------------------------------------------
def run_vision_critic(rendered_img_path, skeleton_img_path, decay_ratio):
    try:
        rendered_img = cv2.imread(rendered_img_path)
        skeleton_img = cv2.imread(skeleton_img_path)

        if rendered_img is None or skeleton_img is None:
            return True, "MISSING_FILES", "CONTINUE"

        hsv_skel = cv2.cvtColor(skeleton_img, cv2.COLOR_BGR2HSV)
        lower_any = np.array([0, 50, 50])
        upper_any = np.array([180, 255, 255])
        mask_skel = cv2.inRange(hsv_skel, lower_any, upper_any)
        
        contours_skel, _ = cv2.findContours(mask_skel, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours_skel: return True, "NO_SKELETON", "CONTINUE"
            
        x_min, y_min = 9999, 9999
        x_max, y_max = 0, 0
        for cnt in contours_skel:
            x, y, w, h = cv2.boundingRect(cnt)
            x_min = min(x_min, x)
            y_min = min(y_min, y)
            x_max = max(x_max, x + w)
            y_max = max(y_max, y + h)

        skel_area = max(1, (x_max - x_min) * (y_max - y_min))

        gray_render = cv2.cvtColor(rendered_img, cv2.COLOR_BGR2GRAY)
        blurred_render = cv2.GaussianBlur(gray_render, (5, 5), 0)
        _, thresh = cv2.threshold(blurred_render, 240, 255, cv2.THRESH_BINARY_INV)
        edges = cv2.Canny(thresh, 50, 150)
        kernel = np.ones((5,5),np.uint8)
        dilated_edges = cv2.dilate(edges,kernel,iterations = 2)

        contours_render, _ = cv2.findContours(dilated_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours_render:
             return False, "TOTAL_COLLAPSE", "INCREASE_CFG"

        rx_min, ry_min = 9999, 9999
        rx_max, ry_max = 0, 0
        for cnt in contours_render:
            if cv2.contourArea(cnt) > 100:
                x, y, w, h = cv2.boundingRect(cnt)
                rx_min = min(rx_min, x)
                ry_min = min(ry_min, y)
                rx_max = max(rx_max, x + w)
                ry_max = max(ry_max, y + h)

        render_area = max(1, (rx_max - rx_min) * (ry_max - ry_min))
        ratio = render_area / float(skel_area)
        
        max_allowed_ratio = 1.6 + (decay_ratio * 0.4) 
        min_allowed_ratio = 0.5 - (decay_ratio * 0.2)

        if ratio > max_allowed_ratio:
            return False, f"STRUCTURAL_FRACTURE_TOO_LARGE ({ratio:.2f} > {max_allowed_ratio:.2f})", "INCREASE_CONTROL"
        elif ratio < min_allowed_ratio:
            return False, f"STRUCTURAL_FRACTURE_TOO_SMALL ({ratio:.2f} < {min_allowed_ratio:.2f})", "SCRAMBLE_SEED"

        sharpness = cv2.Laplacian(gray_render, cv2.CV_64F).var()
        if sharpness < 50.0:
             return False, f"SEVERE_BLUR ({sharpness:.1f} < 50.0)", "INCREASE_STEPS"

        return True, "FORM_VERIFIED", "NONE"
    except Exception as e:
        return True, f"CRITIC_CRASH: {e}", "CONTINUE"
"""

code = code.replace(
    '# ---------------------------------------------------------\n# OPENPOSE SKELETAL GENERATOR\n# ---------------------------------------------------------',
    alligator_code + '\n# ---------------------------------------------------------\n# OPENPOSE SKELETAL GENERATOR\n# ---------------------------------------------------------'
)

# Update the generation loop to use the Critic
old_gen = """            # Fresh 4-step generation strictly bound to the OpenPose matrix without temporal deep-frying
            cmd = [
                "assets/sd_cpp/build/bin/sd-cli",
                "-m", "assets/models/sd15.safetensors",
                "--vae", "assets/models/taesd.safetensors",
                "--lora-model-dir", "assets/models",
                "--control-net", "assets/models/control_openpose.safetensors",
                "--control-image", mask_path,
                "--control-strength", f"{dynamic_control:.2f}", 
                "-p", prompt,
                "-n", "human, person, man, woman, child, people, human face, human hands, human body, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",
                "-W", "512", "-H", "512",
                "--steps", "4", 
                "--threads", "4",
                "--cfg-scale", f"{dynamic_cfg:.1f}", 
                "--seed", divergent_seed, 
                "-o", out_path
            ]
            
            subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(0.5) # Aggressive throttle to ~60% duty cycle to prevent full system stuttering"""

new_gen = """            # THE ALLIGATOR CRITIC LOOP
            # Maximum 3 attempts to generate a structurally sound frame
            for attempt in range(3):
                cmd = [
                    "assets/sd_cpp/build/bin/sd-cli",
                    "-m", "assets/models/sd15.safetensors",
                    "--vae", "assets/models/taesd.safetensors",
                    "--lora-model-dir", "assets/models",
                    "--control-net", "assets/models/control_openpose.safetensors",
                    "--control-image", mask_path,
                    "--control-strength", f"{dynamic_control:.2f}", 
                    "-p", prompt,
                    "-n", "human, person, man, woman, child, people, human face, human hands, human body, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",
                    "-W", "512", "-H", "512",
                    "--steps", "4", 
                    "--threads", "4",
                    "--cfg-scale", f"{dynamic_cfg:.1f}", 
                    "--seed", divergent_seed, 
                    "-o", out_path
                ]
                
                subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                time.sleep(0.5) # GPU throttle
                
                # Alligator evaluates the output
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

code = code.replace(old_gen, new_gen)

with open(filepath, 'w') as f:
    f.write(code)

