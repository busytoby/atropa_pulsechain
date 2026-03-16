import time
import urllib.request
import json
import base64
import math
import io
from PIL import Image

def generate_dna_mask():
    img = Image.new("RGB", (512, 512), (0, 0, 0))
    pixels = img.load()
    for y in range(512):
        for x in range(512):
            if math.hypot(x - 256, y - 256) < 100:
                pixels[x, y] = (76, 29, 14)
    return img

def test_fast_inference():
    print("=== TSFi Fast img2img Validation ===")
    
    # Generate mock abstract DNA frame
    img = generate_dna_mask()
    buffered = io.BytesIO()
    img.save(buffered, format="PNG")
    # Must prefix with data URI for standard A1111 compatibility, or test without. We will try without first, then with.
    b64_img = base64.b64encode(buffered.getvalue()).decode('utf-8')
    
    data = {
        "prompt": "A highly detailed photorealistic Sick Teddy Bear, matted brown fur",
        "width": 512,
        "height": 512,
        "steps": 4, # Absolute minimum for speed
        "cfg_scale": 2.0,
        "seed": 42,
        "init_images": [b64_img],
        "denoising_strength": 0.85
    }
    
    req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        start_time = time.time()
        print("[ACTIVE] Dispatching Base64 Geometry Mask to GPU...")
        response = urllib.request.urlopen(req)
        img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
        end_time = time.time()
        
        with open("assets/fast_img2img_test.png", "wb") as f:
            f.write(img_bytes)
            
        print(f"[ABSOLUTE SUCCESS] GPU Photorealism Generated in: {(end_time - start_time):.2f} Seconds")
    except Exception as e:
        print(f"[FRACTURE] Payload Rejected: {e}")

if __name__ == "__main__":
    test_fast_inference()
