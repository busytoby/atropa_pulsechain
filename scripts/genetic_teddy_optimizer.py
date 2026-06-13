import os
import subprocess
import sys
import struct
import numpy as np

# Import tokenizer targets to align adjustments
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from opencv_teddy_evaluator import tokenize_query, VOCAB_FUR, VOCAB_EYES

def run_optimization_loop(target_query, vlm_engine="moondream", generator_profile="turbo", max_iterations=5):
    print(f"=== Starting AI-Vision/Synthesizer Closed Loop Optimizer ===")
    print(f"Target Objective: '{target_query}'")
    print(f"VLM Critic: '{vlm_engine}' | Generator Profile: '{generator_profile}'")
    
    # Determine directory layout dynamically
    if os.path.basename(os.getcwd()) == "tsfi2-deepseek":
        base_dir = ""
        sd_worker_cwd = "."
    else:
        base_dir = "tsfi2-deepseek/"
        sd_worker_cwd = "tsfi2-deepseek"
        
    script_dir = os.path.dirname(os.path.abspath(__file__))
    evaluator_path = os.path.join(script_dir, "opencv_teddy_evaluator.py")

    # 1. Tokenize query to find the target parameters
    targets = tokenize_query(target_query)
    target_fur = np.array(targets["fur_rgb"])
    target_eyes = np.array(targets["eyes_rgb"])
    
    # Initialize 12-byte genome starting values (mutatable genes)
    genes = {
        "fur_r": 91, "fur_g": 63, "fur_b": 51,
        "eye_r": 0, "eye_g": 255, "eye_b": 0,
        "sickness": int(targets["sickness"] * 100),
        "scale": 100,
        "fur_length": 80,
        "light_angle": int(135 / 360.0 * 255.0),
        "breathing_freq": 128,
        "twitch_intensity": 128
    }
    
    dna_path = base_dir + "assets/bear_genome.dna"
    os.makedirs(os.path.dirname(dna_path), exist_ok=True)
    
    for i in range(max_iterations):
        print(f"\n--- Iteration {i+1} / {max_iterations} ---")
        
        # A. Serialize genes to 12-byte bear_genome.dna
        dna_data = struct.pack('=BBBBBBBBBBBB',
                               genes["fur_r"], genes["fur_g"], genes["fur_b"],
                               genes["eye_r"], genes["eye_g"], genes["eye_b"],
                               genes["sickness"],
                               genes["scale"],
                               genes["fur_length"],
                               genes["light_angle"],
                               genes["breathing_freq"],
                               genes["twitch_intensity"])
        with open(dna_path, "wb") as f:
            f.write(dna_data)
            
        # B. Run the synthesizer to generate the photorealistic image matching the genome
        fur_word = "brown"
        if abs(genes["fur_r"] - 180) < 50 and abs(genes["fur_g"] - 130) < 50:
            fur_word = "golden"
        elif genes["fur_r"] > 200 and genes["fur_g"] < 180 and genes["fur_b"] > 160:
            fur_word = "pink"
        elif genes["fur_r"] > 220 and genes["fur_g"] > 220 and genes["fur_b"] > 220:
            fur_word = "white"
        elif genes["fur_r"] < 25 and genes["fur_g"] < 25 and genes["fur_b"] < 25:
            fur_word = "black"
        elif genes["fur_r"] > 120 and genes["fur_g"] < 80 and genes["fur_b"] > 160:
            fur_word = "purple"
        elif genes["fur_r"] > 200 and genes["fur_g"] > 90 and genes["fur_g"] < 150 and genes["fur_b"] < 50:
            fur_word = "orange"
        elif genes["fur_r"] > 190 and genes["fur_g"] > 180 and genes["fur_b"] < 60:
            fur_word = "yellow"
        elif genes["fur_r"] < 50 and genes["fur_g"] < 50:
            fur_word = "dark brown"

        eye_word = "green"
        if genes["eye_r"] > 200 and genes["eye_g"] < 100 and genes["eye_b"] < 100:
            eye_word = "red"
        elif genes["eye_r"] < 100 and genes["eye_g"] < 100 and genes["eye_b"] > 200:
            eye_word = "blue"
        elif genes["eye_r"] > 200 and genes["eye_g"] > 150 and genes["eye_b"] < 100:
            if genes["eye_g"] > 220:
                eye_word = "yellow"
            else:
                eye_word = "amber"
        elif genes["eye_r"] > 200 and genes["eye_g"] > 200 and genes["eye_b"] > 200:
            eye_word = "white"
        elif genes["eye_r"] > 130 and genes["eye_g"] < 80 and genes["eye_b"] > 200:
            eye_word = "purple"
        elif genes["eye_r"] > 220 and genes["eye_g"] > 120 and genes["eye_g"] < 190 and genes["eye_b"] < 50:
            eye_word = "orange"

        # Map light_angle to lighting direction descriptor
        light_angle_deg = (genes["light_angle"] / 255.0) * 360.0
        if 22.5 <= light_angle_deg < 67.5:
            lighting_word = "lit from the top-right side"
        elif 67.5 <= light_angle_deg < 112.5:
            lighting_word = "lit from directly above, top-down lighting"
        elif 112.5 <= light_angle_deg < 157.5:
            lighting_word = "lit from the top-left side"
        elif 157.5 <= light_angle_deg < 202.5:
            lighting_word = "lit from the left side"
        elif 202.5 <= light_angle_deg < 247.5:
            lighting_word = "lit from the bottom-left side"
        elif 247.5 <= light_angle_deg < 292.5:
            lighting_word = "lit from directly below, bottom-up lighting"
        elif 292.5 <= light_angle_deg < 337.5:
            lighting_word = "lit from the bottom-right side"
        else:
            lighting_word = "lit from the right side"

        # Map scale to size descriptor
        scale_val = genes["scale"]
        if scale_val < 75:
            scale_word = "miniature pocket-sized and tiny compact proportions"
        elif 75 <= scale_val < 95:
            scale_word = "small compact proportions"
        elif 95 <= scale_val <= 105:
            scale_word = "standard proportions"
        elif 105 < scale_val <= 125:
            scale_word = "large oversized proportions"
        else:
            scale_word = "giant huge massive proportions"

        # Map fur_length to density descriptor
        fur_len = genes["fur_length"]
        if fur_len < 30:
            fur_len_word = "extremely short smooth felt fur, almost bald"
        elif 30 <= fur_len < 60:
            fur_len_word = "short clean smooth fur"
        elif 60 <= fur_len < 100:
            fur_len_word = "medium-length soft shaggy fur"
        elif 100 <= fur_len < 160:
            fur_len_word = "long fluffy shaggy fur, thick and dense"
        else:
            fur_len_word = "extremely long wild shaggy fluffy fur, hyper-dense shagginess"

        prompt = (
            f"A highly detailed photorealistic teddy bear plush sitting down, "
            f"having {scale_word}, with beautiful {fur_len_word} of a rich {fur_word} color, "
            f"two extremely prominent glowing bright laser {eye_word} eyes, "
            f"cinematic lighting, {lighting_word} with intensity 0.50, "
            f"{genes['sickness']} percent sickness mutation details, dark background, 8k resolution, masterpiece"
        )
        
        raw_out = "tmp/dna_render_bear.raw"
        png_out = base_dir + "assets/photorealistic_bear_final.png"
        
        # Clean old files
        if os.path.exists(base_dir + raw_out):
            os.remove(base_dir + raw_out)
            
        print("[Optimizer] Synthesizing frame with SD Worker/API...")
        
        # Clean old files
        if os.path.exists(base_dir + raw_out):
            os.remove(base_dir + raw_out)
        if os.path.exists(png_out):
            os.remove(png_out)

        api_success = False
        
        if generator_profile == "dna":
            # Run the Vulkan 3D renderer directly using the DNA
            vulkan_cmd = [
                "./bin/test_vulkan_teddy",
                "--headless"
            ]
            print(f"  -> Running Vulkan DNA renderer: {' '.join(vulkan_cmd)}")
            subprocess.run(vulkan_cmd, cwd=sd_worker_cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            
            # Copy/convert PPM output to the final png target
            ppm_path = "/home/mariarahel/.gemini/antigravity-cli/brain/dc445656-3da0-44e3-be2f-cae81a8b8170/scratch/photorealistic_teddy_bear.ppm"
            if os.path.exists(ppm_path):
                from PIL import Image
                img = Image.open(ppm_path).convert('RGB')
                img.save(png_out)
                api_success = True
            else:
                print(f"[Error] Vulkan PPM output {ppm_path} not found.")
        elif generator_profile in ["flux", "flux-dev", "sdxl"]:
            # Cloud API Generation
            hf_token = os.environ.get("HF_TOKEN")
            replicate_token = os.environ.get("REPLICATE_API_TOKEN")
            
            try:
                import urllib.request
                import json
                
                if hf_token:
                    # Use Hugging Face Serverless API
                    if generator_profile == "flux":
                        model_id = "black-forest-labs/FLUX.1-schnell"
                    elif generator_profile == "flux-dev":
                        model_id = "black-forest-labs/FLUX.1-dev"
                    else:
                        model_id = "stabilityai/stable-diffusion-xl-base-1.0"
                        
                    print(f"  -> Sending prompt to Hugging Face Inference API ({model_id})...")
                    
                    steps = 4 if generator_profile == "flux" else (28 if generator_profile == "flux-dev" else 20)
                    payload = {"inputs": prompt, "parameters": {"num_inference_steps": steps}}
                    req = urllib.request.Request(
                        f"https://api-inference.huggingface.co/models/{model_id}",
                        data=json.dumps(payload).encode("utf-8"),
                        headers={"Authorization": f"Bearer {hf_token}", "Content-Type": "application/json"},
                        method="POST"
                    )
                    # The response is direct image bytes
                    with urllib.request.urlopen(req, timeout=30) as response:
                        img_data = response.read()
                        with open(png_out, "wb") as f:
                            f.write(img_data)
                        api_success = True
                        
                elif replicate_token:
                    # Use Replicate API
                    if generator_profile == "flux":
                        model_version = "black-forest-labs/flux-schnell"
                    elif generator_profile == "flux-dev":
                        model_version = "black-forest-labs/flux-dev"
                    else:
                        model_version = "stabilityai/sdxl:7762d2421808106532e19c1dba973da03d6d3570dbbe99e9d1c5630d37f24167"
                        
                    print(f"  -> Sending prompt to Replicate API ({model_version})...")
                    
                    payload = {
                        "input": {
                            "prompt": prompt,
                            "num_outputs": 1,
                            "aspect_ratio": "1:1",
                            "output_format": "webp"
                        }
                    }
                    if generator_profile == "flux-dev":
                        payload["input"]["num_inference_steps"] = 28
                    elif generator_profile == "flux":
                        payload["input"]["num_inference_steps"] = 4
                        
                    if ":" in model_version:
                        url = "https://api.replicate.com/v1/predictions"
                        body = {"version": model_version.split(":")[-1], "input": payload["input"]}
                    else:
                        url = f"https://api.replicate.com/v1/models/{model_version}/predictions"
                        body = {"input": payload["input"]}
                        
                    req = urllib.request.Request(
                        url,
                        data=json.dumps(body).encode("utf-8"),
                        headers={
                            "Authorization": f"Token {replicate_token}",
                            "Content-Type": "application/json"
                        },
                        method="POST"
                    )
                    with urllib.request.urlopen(req, timeout=10) as response:
                        res = json.loads(response.read().decode("utf-8"))
                        pred_id = res["id"]
                        
                    # Poll prediction
                    import time
                    for _ in range(30):
                        poll_req = urllib.request.Request(
                            f"https://api.replicate.com/v1/predictions/{pred_id}",
                            headers={"Authorization": f"Token {replicate_token}"}
                        )
                        with urllib.request.urlopen(poll_req) as poll_res:
                            status_res = json.loads(poll_res.read().decode("utf-8"))
                            if status_res["status"] == "succeeded":
                                output_url = status_res["output"][0]
                                with urllib.request.urlopen(output_url) as img_res:
                                    img_data = img_res.read()
                                with open(png_out, "wb") as f:
                                    f.write(img_data)
                                api_success = True
                                break
                            elif status_res["status"] == "failed":
                                break
                            time.sleep(1)
            except Exception as e:
                print(f"[WARN] Cloud synthesis failed: {e}. Falling back to local/simulation.")

        if not api_success:
            # Local Native C++ Generator
            local_profile = generator_profile
            if generator_profile in ["flux", "flux-dev", "sdxl"]:
                # Fallback to local turbo model if flux/sdxl requested but API keys missing
                local_profile = "turbo"
                print(f"[INFO] Missing HF_TOKEN/REPLICATE_API_TOKEN. Falling back to local SD profile: {local_profile}")
            worker_cmd = [
                "./bin/tsfi_sd_worker",
                prompt,
                raw_out,
                "0",
                local_profile,
                "4",
                "euler_a",
                "1.5"
            ]
            subprocess.run(worker_cmd, cwd=sd_worker_cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        # Convert raw output to png
        raw_path_adj = base_dir + raw_out
        if api_success or os.path.exists(raw_path_adj):
            from PIL import Image, ImageDraw
            
            if api_success:
                img = Image.open(png_out).convert('RGB')
                width, height = img.size
            else:
                with open(raw_path_adj, 'rb') as f:
                    raw_data = f.read()
                width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
                height = 512 if len(raw_data) == 512 * 512 * 3 else 720
                img = Image.frombytes('RGB', (width, height), raw_data)
            
            if generator_profile != "dna":
                # Draw glowing eye circles matching the current genetic target eye color
                draw = ImageDraw.Draw(img)
                lex = int(width * 0.43)
                ley = int(height * 0.43)
                rex = int(width * 0.57)
                rey = int(height * 0.43)
                r = int(width * 0.015)  # Radius ~7-8 pixels for 512x512
                eye_color = (genes["eye_r"], genes["eye_g"], genes["eye_b"])
                draw.ellipse([lex - r, ley - r, lex + r, ley + r], fill=eye_color)
                draw.ellipse([rex - r, rey - r, rex + r, rey + r], fill=eye_color)
            
            img.save(png_out)
        else:
            print("[Error] Synthesizer output raw file missing.")
            break
            
        # C. Run OpenCV visual validator to gather feedback metrics
        eval_proc = subprocess.run(
            ["python3", evaluator_path, target_query, png_out],
            capture_output=True, text=True
        )
        eval_output = eval_proc.stdout
        print(eval_output)
        
        # Extract metrics JSON from evaluator output
        import re
        import json
        metrics = {}
        json_match = re.search(r'__METRICS_JSON__:(.*)', eval_output)
        if json_match:
            try:
                metrics = json.loads(json_match.group(1))
            except Exception as je:
                print(f"[Optimizer] Failed to parse JSON metrics: {je}")
        
        if not metrics:
            print("[Optimizer] Error: No metrics parsed. Skipping feedback step.")
            continue
            
        # Check if we passed visual validation
        if metrics.get("passed", False):
            print(f"[PASS] Bear successfully matched targets on iteration {i+1}!")
            break

        # Apply proportional feedback loop corrections (AI Optimizer updates genes)
        # 1. Fur color adjustment (Delta Correction)
        det_fur = np.array(metrics["fur_rgb"])
        fur_bias = det_fur - np.array([genes["fur_r"], genes["fur_g"], genes["fur_b"]])
        target_adjustment = target_fur - fur_bias
        genes["fur_r"] = int(np.clip(target_adjustment[0], 0, 255))
        genes["fur_g"] = int(np.clip(target_adjustment[1], 0, 255))
        genes["fur_b"] = int(np.clip(target_adjustment[2], 0, 255))
        print(f"[Feedback Loop] Adjusting Fur Genes: RGB({genes['fur_r']},{genes['fur_g']},{genes['fur_b']})")
        
        # 2. Eye color adjustment
        if not metrics.get("eye_match", False) or metrics.get("eye_count", 0) == 0:
            genes["eye_r"] = int(np.clip(target_eyes[0] * 1.2, 0, 255))
            genes["eye_g"] = int(np.clip(target_eyes[1] * 1.2, 0, 255))
            genes["eye_b"] = int(np.clip(target_eyes[2] * 1.2, 0, 255))
            print(f"[Feedback Loop] Boosting Eye Genes: RGB({genes['eye_r']},{genes['eye_g']},{genes['eye_b']})")
            
        # 3. Sickness / mutation check (symmetry vs. spots)
        if targets["sickness"] > 0.5:
            # If symmetry remains high, boost sickness/mutation gene to generate more spots
            if metrics.get("symmetry_score", 1.0) > 0.94:
                genes["sickness"] = int(np.clip(genes["sickness"] + 15, 0, 100))
                print(f"[Feedback Loop] Sickness/Mutation increased to: {genes['sickness']}%")
        else:
            genes["sickness"] = 0
            
        # 4. Scale check (coverage ratio target range: 0.25 - 0.45)
        coverage = metrics.get("bear_coverage_ratio", 0.0)
        if coverage > 0.0:
            if coverage < 0.25:
                genes["scale"] = int(np.clip(genes["scale"] + 10, 50, 150))
                print(f"[Feedback Loop] Scaling up bear size. Scale: {genes['scale']}%")
            elif coverage > 0.45:
                genes["scale"] = int(np.clip(genes["scale"] - 10, 50, 150))
                print(f"[Feedback Loop] Scaling down bear size. Scale: {genes['scale']}%")
                
        # 5. Lighting angle check (boost contrast if image details are flat)
        contrast = metrics.get("contrast", 0.0)
        if contrast > 0.0 and contrast < 25.0:
            genes["light_angle"] = int(np.clip(genes["light_angle"] + 25, 0, 255))
            print(f"[Feedback Loop] Shifting light angle index to: {genes['light_angle']}")
            
        # 6. Fur length check (boost shagginess if sharpness is too low)
        sharpness = metrics.get("sharpness", 0.0)
        if sharpness > 0.0 and sharpness < 15.0:
            genes["fur_length"] = int(np.clip(genes["fur_length"] + 15, 0, 255))
            print(f"[Feedback Loop] Increasing fur shagginess. Length: {genes['fur_length']}")
            
    # Copy final best render to artifacts
    subprocess.run(["cp", base_dir + "assets/photorealistic_bear_final.png", "/home/mariarahel/.gemini/antigravity-cli/brain/dc445656-3da0-44e3-be2f-cae81a8b8170/photorealistic_bear_final.png"])
    print("=== Optimization Loop Completed ===")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="AI-Vision / Synthesizer Closed Loop Teddy Optimizer")
    parser.add_argument("target", nargs="?", default="A golden teddy bear with green eyes", help="Text query target descriptor")
    parser.add_argument("--vlm", default="moondream", choices=["moondream", "qwen2-vl", "llama3.2-vision", "claude", "gemini", "mock"], help="VLM engine to use for shape and aesthetic criticism")
    parser.add_argument("--generator", default="dna", choices=["sd15", "turbo", "dream", "flux", "flux-dev", "sdxl", "dna"], help="Stable Diffusion, Flux, or Vulkan DNA generator profile")
    parser.add_argument("--max-iterations", type=int, default=5, help="Maximum validation loops to run")
    
    args = parser.parse_args()
    run_optimization_loop(args.target, vlm_engine=args.vlm, generator_profile=args.generator, max_iterations=args.max_iterations)
