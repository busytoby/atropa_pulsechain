import urllib.request
import json
import base64
import os
from PIL import Image

def generate_texture(prompt, output_file):
    print(f"  -> Sending prompt to Vulkan Painter: '{prompt}'")
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "steps": 15, # Higher steps for intricate texture details
        "cfg_scale": 5.0,
        "seed": 42
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        response = urllib.request.urlopen(req)
        result = json.loads(response.read())
        img_bytes = base64.b64decode(result['images'][0])
        with open(output_file, "wb") as f:
            f.write(img_bytes)
        print(f"  -> [PASS] Texture serialized to {output_file}")
        return True
    except Exception as e:
        print(f"  -> [FRACTURE] Failed to synthesize texture: {e}")
        return False

def analyze_texture_matrix(img_path):
    # Extracts the mathematical standard deviation of the RGB channels to determine the procedural noise amplitude needed in the C-Kernel
    img = Image.open(img_path).convert('RGB')
    pixels = img.load()
    w, h = img.size
    
    r_sum, g_sum, b_sum = 0, 0, 0
    for y in range(h):
        for x in range(w):
            r, g, b = pixels[x, y]
            r_sum += r
            g_sum += g
            b_sum += b
            
    r_avg = r_sum / (w * h)
    g_avg = g_sum / (w * h)
    b_avg = b_sum / (w * h)
    
    r_var, g_var, b_var = 0, 0, 0
    for y in range(h):
        for x in range(w):
            r, g, b = pixels[x, y]
            r_var += (r - r_avg) ** 2
            g_var += (g - g_avg) ** 2
            b_var += (b - b_avg) ** 2
            
    r_std = (r_var / (w * h)) ** 0.5
    g_std = (g_var / (w * h)) ** 0.5
    b_std = (b_var / (w * h)) ** 0.5
    
    return {
        "avg_color": (int(r_avg), int(g_avg), int(b_avg)),
        "noise_amplitude": (int(r_std), int(g_std), int(b_std))
    }

def run_texture_miner():
    print("=== TSFi Stable Diffusion Texture Interrogation ===", flush=True)
    os.makedirs("assets", exist_ok=True)
    
    print("\\n[ACTIVE] Synthesizing Base Fur Geometry...")
    fur_prompt = "A perfectly flat, seamless texture of matted, dark brown teddy bear fur, photorealistic, macro photography"
    fur_path = "assets/texture_fur.png"
    if generate_texture(fur_prompt, fur_path):
        fur_stats = analyze_texture_matrix(fur_path)
        print(f"    - Base Color: {fur_stats['avg_color']}")
        print(f"    - Noise Amplitude (Variance): {fur_stats['noise_amplitude']}")
        
    print("\\n[ACTIVE] Synthesizing Sickness Mutation Geometry...")
    sick_prompt = "A perfectly flat, seamless texture of glowing, dark rotting neon green sickness mutation, organic decay, macro photography"
    sick_path = "assets/texture_sick.png"
    if generate_texture(sick_prompt, sick_path):
        sick_stats = analyze_texture_matrix(sick_path)
        print(f"    - Base Color: {sick_stats['avg_color']}")
        print(f"    - Noise Amplitude (Variance): {sick_stats['noise_amplitude']}")

if __name__ == "__main__":
    run_texture_miner()
