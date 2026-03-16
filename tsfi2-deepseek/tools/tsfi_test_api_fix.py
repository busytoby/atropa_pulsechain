import time
import urllib.request
import json
import base64
import math
import io
from PIL import Image

def generate_dna_mask():
    img = Image.new("RGB", (256, 256), (0, 0, 0))
    pixels = img.load()
    for y in range(256):
        for x in range(256):
            if math.hypot(x - 128, y - 128) < 50:
                pixels[x, y] = (76, 29, 14)
    return img

def test_api_fix():
    print("=== TSFi Server img2img API Fix Validation ===")
    
    img = generate_dna_mask()
    buffered = io.BytesIO()
    img.save(buffered, format="PNG")
    
    # Test 1: Try passing the strict AUTOMATIC1111 data URI format
    b64_str = base64.b64encode(buffered.getvalue()).decode('utf-8')
    b64_uri = f"data:image/png;base64,{b64_str}"
    
    data = {
        "prompt": "A sick teddy bear",
        "width": 256,
        "height": 256,
        "steps": 4,
        "cfg_scale": 1.0,
        "seed": 42,
        "init_images": [b64_str], # Trying raw base64 first
        "denoising_strength": 0.85
    }
    
    req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        urllib.request.urlopen(req)
        print("[PASS] Raw Base64 Accepted!")
    except Exception as e:
        print(f"[FAIL] Raw Base64 Rejected: {e}")
        
        # Test 2: Try Data URI
        print("[ACTIVE] Attempting Data URI Format...")
        data["init_images"] = [b64_uri]
        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
        req.add_header('Content-Type', 'application/json')
        try:
            urllib.request.urlopen(req)
            print("[PASS] Data URI Accepted!")
        except Exception as e2:
            print(f"[FAIL] Data URI Rejected: {e2}")

if __name__ == "__main__":
    test_api_fix()
