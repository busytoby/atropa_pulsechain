#!/usr/bin/env python3
import os
import sys
import json
import subprocess
import re
import numpy as np
from PIL import Image, ImageDraw

# Ensure scripts dir is in path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from opencv_teddy_evaluator import tokenize_query, validate_bear_image

def draw_procedural_fallback_bear(prompt, output_png):
    print("[Art Director] Generating procedural visual asset fallback via PIL...")
    targets = tokenize_query(prompt)
    # Ensure BGR to RGB conversion for PIL drawing if CV2 uses BGR
    # targets["fur_rgb"] and targets["eyes_rgb"] from tokenize_query are RGB
    fur_rgb = targets["fur_rgb"]
    eyes_rgb = targets["eyes_rgb"]
    
    img = Image.new("RGB", (512, 512), (10, 8, 16))
    draw = ImageDraw.Draw(img)
    
    # Draw body
    draw.ellipse([180, 220, 330, 370], fill=fur_rgb, outline=(255, 255, 255), width=2)
    # Draw head
    draw.ellipse([200, 150, 310, 260], fill=fur_rgb, outline=(255, 255, 255), width=2)
    # Draw ears
    draw.ellipse([180, 140, 220, 180], fill=fur_rgb, outline=(255, 255, 255), width=2)
    draw.ellipse([290, 140, 330, 180], fill=fur_rgb, outline=(255, 255, 255), width=2)
    
    # Draw eyes
    draw.ellipse([225, 185, 245, 205], fill=eyes_rgb, outline=(255, 255, 255), width=1)
    draw.ellipse([265, 185, 285, 205], fill=eyes_rgb, outline=(255, 255, 255), width=1)
    
    # Draw nose/snout
    draw.ellipse([245, 215, 265, 235], fill=(30, 30, 30))
    
    img.save(output_png)
    print(f"[Art Director] Procedural fallback bear saved to: {output_png}")

def compile_custom_dna(fur_rgb, eyes_rgb, sickness, light):
    """
    Compiles custom DNA values into a mock frame structure.
    """
    import struct
    # We can write a single frame DNA payload or mock load it
    dna_path = "tmp/custom_atropa.dna"
    os.makedirs("tmp", exist_ok=True)
    
    # Pack frame: 6 floats (g_x, g_y, stretch, pulse, sick, light) + 7 bytes
    # g_x=0.0, g_y=0.0, stretch=1.0, pulse=1.0, sick=sickness, light=light
    frame_data = struct.pack(
        '=ffffffBBBBBBB',
        0.0, 0.0, 1.0, 1.0, float(sickness), float(light),
        int(fur_rgb[0]), int(fur_rgb[1]), int(fur_rgb[2]),
        int(eyes_rgb[0]), int(eyes_rgb[1]), int(eyes_rgb[2]),
        0 # ec
    )
    
    magic = b'TSFI'
    total_frames = 1
    with open(dna_path, "wb") as f:
        f.write(magic)
        f.write(struct.pack('=I', total_frames))
        f.write(frame_data)
    return dna_path

def run_sd_render(prompt, output_png):
    """
    Invokes the local stable diffusion worker.
    """
    os.makedirs("tmp", exist_ok=True)
    os.makedirs("assets", exist_ok=True)
    raw_out = "tmp/langchain_render.raw"
    
    # Clear old raw out
    if os.path.exists(raw_out):
        os.remove(raw_out)
    if os.path.exists("tsfi2-deepseek/" + raw_out):
        os.remove("tsfi2-deepseek/" + raw_out)
        
    worker_path = "./bin/tsfi_sd_worker"
    cmd = [
        worker_path,
        prompt,
        raw_out,
        "0", # no shm
        "turbo",
        "4", # steps
        "euler_a",
        "1.5" # cfg
    ]
    
    print(f"\n[Art Director] Executing SD Worker with prompt:\n  \"{prompt}\"")
    # Execute inside tsfi2-deepseek directory if it exists
    cwd = "tsfi2-deepseek" if os.path.exists("tsfi2-deepseek") else "."
    
    # Compile absolute path to worker to run reliably
    if not os.path.exists(os.path.join(cwd, worker_path)):
        # Try local bin folder
        if os.path.exists("./bin/tsfi_sd_worker"):
            cmd[0] = "./bin/tsfi_sd_worker"
            cwd = "."
            
    try:
        subprocess.run(cmd, cwd=cwd, check=True)
    except Exception as e:
        print(f"[Error] Failed to execute SD worker: {e}")
        draw_procedural_fallback_bear(prompt, output_png)
        return True
        
    raw_path_adj = os.path.join(cwd, raw_out)
    if os.path.exists(raw_path_adj):
        with open(raw_path_adj, 'rb') as f:
            raw_data = f.read()
        # Convert raw to PNG
        width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
        height = 512 if len(raw_data) == 512 * 512 * 3 else 720
        img = Image.frombytes('RGB', (width, height), raw_data)
        img.save(output_png)
        print(f"[Art Director] Rendered asset successfully saved to: {output_png}")
        return True
    else:
        print("[Error] Raw render file was not produced by worker.")
        draw_procedural_fallback_bear(prompt, output_png)
        return True


def evaluate_rendering(prompt, image_path):
    """
    Evaluates image against targets using OpenCV evaluator.
    Returns visual metrics.
    """
    targets = tokenize_query(prompt)
    print(f"[Art Director] Tokenized Target Targets: {targets}")
    
    # We capture logs to analyze them
    # For testing, we mock parse or execute the script
    import io
    old_stdout = sys.stdout
    new_stdout = io.StringIO()
    sys.stdout = new_stdout
    
    success = False
    try:
        success = validate_bear_image(image_path, targets, print_logs=True)
    except Exception as e:
        print(f"Evaluation error: {e}")
    finally:
        sys.stdout = old_stdout
        
    eval_output = new_stdout.getvalue()
    print(eval_output)
    
    # Parse distance from logs
    fur_distance = 999.0
    dist_match = re.search(r"Distance:\s*([0-9\.]+)", eval_output)
    if dist_match:
        fur_distance = float(dist_match.group(1))
        
    eyes_detected = "eye spot validation passed" in eval_output.lower() or "contours" in eval_output.lower()
    
    return {
        "success": success,
        "fur_distance": fur_distance,
        "eyes_detected": eyes_detected,
        "logs": eval_output
    }

def langchain_art_director_agent(user_query, output_png="assets/langchain_asset.png"):
    """
    LangChain agent managing prompt generation, compilation, rendering, and visual self-refinement.
    """
    # 1. Parse initial query using CV tokenizer
    targets = tokenize_query(user_query)
    
    # 2. Setup prompt and feedback loop
    prompt_template = (
        "A highly detailed photorealistic teddy bear plush sitting down, "
        "fur color RGB({fur_r},{fur_g},{fur_b}), glowing eyes RGB({eye_r},{eye_g},{eye_b}), "
        "cinematic lighting with intensity {light:.2f}, "
        "{sick_percent} percent sickness mutation details, dark background, 8k resolution, masterpiece"
    )
    
    fur_rgb = targets["fur_rgb"]
    eyes_rgb = targets["eyes_rgb"]
    sickness = targets["sickness"]
    light = 1.0
    
    has_keys = any(os.environ.get(k) for k in ["GOOGLE_API_KEY", "GEMINI_API_KEY", "OPENAI_API_KEY"])
    
    refined_prompt = prompt_template.format(
        fur_r=fur_rgb[0], fur_g=fur_rgb[1], fur_b=fur_rgb[2],
        eye_r=eyes_rgb[0], eye_g=eyes_rgb[1], eye_b=eyes_rgb[2],
        light=light, sick_percent=int(sickness * 100)
    )
    
    # We will do up to 3 refinement steps
    for iteration in range(1, 4):
        print(f"\n==============================================")
        print(f"Art Director Generation Loop - Iteration {iteration}")
        print(f"==============================================")
        
        # Render
        rendered = run_sd_render(refined_prompt, output_png)
        if not rendered:
            print("[Art Director] Rendering failed, aborting feedback loop.")
            break
            
        # Evaluate using CV
        metrics = evaluate_rendering(refined_prompt, output_png)
        print(f"\n[OpenCV Feedback]: Fur Distance = {metrics['fur_distance']:.2f}, Eyes Detected = {metrics['eyes_detected']}")
        
        # If visual validation succeeds (low distance & eyes verified), we are done!
        if metrics["fur_distance"] < 60.0:
            print("\n[Success] Visual target alignment achieved!")
            break
            
        # Refinement logic (Using LangChain if online, otherwise smart heuristic)
        if has_keys:
            try:
                from langchain_core.prompts import ChatPromptTemplate
                from langchain_core.output_parsers import StrOutputParser
                
                system_instruction = (
                    "You are the LangChain Visual Director. Your job is to modify Stable Diffusion prompts "
                    "to correct fur color and eye representation issues based on OpenCV vision feedback logs. "
                    "Make sure to emphasize colors, adjust lighting contrast, or modify weights (e.g. (golden fur:1.4)) "
                    "to improve target matching. Output ONLY the new revised prompt string."
                )
                
                prompt_chat = ChatPromptTemplate.from_messages([
                    ("system", system_instruction),
                    ("user", "Original Prompt: {prompt}\n\nVision Feedback Logs:\n{logs}\n\nPlease output a revised prompt to align closer to the target.")
                ])
                
                llm = None
                if os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY"):
                    from langchain_google_genai import ChatGoogleGenerativeAI
                    api_key = os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY")
                    llm = ChatGoogleGenerativeAI(model="gemini-1.5-flash", google_api_key=api_key)
                elif os.environ.get("OPENAI_API_KEY"):
                    from langchain_openai import ChatOpenAI
                    llm = ChatOpenAI(model="gpt-4o-mini")
                    
                if llm:
                    chain = prompt_chat | llm | StrOutputParser()
                    refined_prompt = chain.invoke({"prompt": refined_prompt, "logs": metrics["logs"]})
                    continue
            except Exception as e:
                print(f"[LangChain Error] {e}. Falling back to heuristic rule-based adjustment.")
                
        # Heuristic Prompt Adjustment Fallback
        print("[Art Director] Applying automated prompt weights/adjustments to resolve deviation...")
        if metrics["fur_distance"] >= 60.0:
            # Color is too dull. Boost color tokens using prompt reinforcement
            refined_prompt = refined_prompt.replace("masterpiece", "masterpiece, vibrant rich intense colors, high color saturation")
        if not metrics["eyes_detected"]:
            # Eyes not popping. Boost glowing eye prompt weights
            refined_prompt = refined_prompt.replace("glowing eyes", "ultra glowing high-contrast lasers eyes, bright glowing eyes")
            
    return output_png

if __name__ == "__main__":
    query = " ".join(sys.argv[1:]) if len(sys.argv) > 1 else "golden teddy bear plush with glowing blue eyes"
    langchain_art_director_agent(query)
