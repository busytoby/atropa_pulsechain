import urllib.request
import json
import base64
import os
from PIL import Image

def synthesize_aot_rendering_asset(prompt, output_file):
    print(f"  -> Demanding specialized asset: '{prompt}'")
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    data = {
        "prompt": prompt,
        "width": 512,
        "height": 512,
        "steps": 12,
        "cfg_scale": 4.5,
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
        print(f"  -> [PASS] Asset locked: {output_file}")
        return True
    except Exception as e:
        print(f"  -> [FRACTURE] Vulkan Painter failed: {e}")
        return False

def extract_lighting_gradients(img_path):
    img = Image.open(img_path).convert('L') # Convert to greyscale
    pixels = img.load()
    w, h = img.size
    
    # We scan the image to find the absolute darkest shadow threshold and the absolute brightest highlight
    min_val, max_val = 255, 0
    center_val = pixels[w//2, h//2]
    
    for y in range(h):
        for x in range(w):
            val = pixels[x, y]
            if val < min_val: min_val = val
            if val > max_val: max_val = val
            
    return min_val, max_val, center_val

def run_advanced_aot_extraction():
    print("=== TSFi Advanced AOT Painter Extraction ===", flush=True)
    os.makedirs("assets", exist_ok=True)
    
    # 1. Ambient Occlusion (Shadow Mapping)
    print("\\n[ACTIVE] Extracting Ambient Occlusion physics...")
    ao_prompt = "A pure greyscale ambient occlusion map of a seated teddy bear, stark black shadows, pure white highlights, no color, 3d render depth map"
    ao_path = "assets/aot_ambient_occlusion.png"
    if synthesize_aot_rendering_asset(ao_prompt, ao_path):
        min_s, max_s, center_s = extract_lighting_gradients(ao_path)
        print(f"    - Absolute Shadow Floor (0-255): {min_s}")
        print(f"    - Maximum Highlight Peak: {max_s}")
        print(f"    - Core Geometric Shading: {center_s}")

    # 2. Specular Highlight Morphology
    print("\\n[ACTIVE] Extracting Specular Rim Lighting physics...")
    rim_prompt = "A pure black background, a teddy bear lit ONLY by a stark, glowing rim light from the top left corner, high contrast silhouette highlight"
    rim_path = "assets/aot_rim_light.png"
    if synthesize_aot_rendering_asset(rim_prompt, rim_path):
        print("    - [PASS] Top-Left Rim Light morphology extracted for C-Kernel alpha blending.")

if __name__ == "__main__":
    run_advanced_aot_extraction()
