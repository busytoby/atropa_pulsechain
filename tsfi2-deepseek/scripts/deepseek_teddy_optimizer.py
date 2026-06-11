import json
import base64
import urllib.request
import urllib.error
import sys
import os

# Configuration endpoints
VLM_URL = "http://127.0.0.1:11435/api/generate"
VLM_MODEL = "moondream"  # Can be swapped to deepseek-vl or similar

def optimize_teddy_genome(image_path, prompt_aesthetic):
    print(f"[Optimizer] Loading image for DeepSeek analysis: {image_path}")
    if not os.path.exists(image_path):
        print(f"[Error] Image not found at {image_path}")
        return None
        
    try:
        # 1. Encode the synthesized image to Base64
        with open(image_path, "rb") as image_file:
            b64_data = base64.b64encode(image_file.read()).decode('utf-8')
    except Exception as e:
        print(f"[Error] Failed to read image: {e}")
        return None

    # 2. Build the payload matching the tsfi_ai_core.c specifications
    payload = {
        "model": VLM_MODEL,
        "prompt": (
            f"Analyze this teddy bear image. {prompt_aesthetic} "
            "Suggest adjustments for the following slider values (0.0 to 1.0/3.0) and colors in JSON format: "
            "{\"sickness\": float, \"rim_light\": float, \"base_color_hex\": string, \"reason\": string}"
        ),
        "images": [b64_data],
        "stream": False
    }
    
    print(f"[Optimizer] Sending image and aesthetic query to VLM ({VLM_MODEL})...")
    
    try:
        req = urllib.request.Request(
            VLM_URL,
            data=json.dumps(payload).encode('utf-8'),
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        
        with urllib.request.urlopen(req, timeout=30) as response:
            res_data = response.read().decode('utf-8')
            res_json = json.loads(res_data)
            
            feedback = res_json.get("response", "")
            print("\n=== DeepSeek Quality Analysis Output ===")
            print(feedback)
            print("========================================\n")
            return feedback
            
    except urllib.error.URLError as e:
        print(f"[Error] Connection to local VLM failed. Ensure server runs on 127.0.0.1:11435.")
        print(f"Details: {e}")
        return None
    except Exception as e:
        print(f"[Error] Unexpected error during request: {e}")
        return None

if __name__ == "__main__":
    # Standard test image generated in the artifacts folder
    test_img = "/home/mariarahel/.gemini/antigravity-cli/brain/6f2057ff-bccd-47e3-b6ed-8e254157b6b6/deepseek_golden_masterpiece_bear_1781210074719.png"
    
    query = "Evaluate the fur texture detail and lighting contrast. Is the nose appropriately shaped and leathery?"
    
    print("=== DeepSeek Sovereign Teddy Optimizer active ===")
    optimize_teddy_genome(test_img, query)
