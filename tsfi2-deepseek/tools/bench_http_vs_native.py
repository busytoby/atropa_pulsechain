import time
import base64
import json
import urllib.request
import math
from PIL import Image
import os

def bench_base64_overhead():
    print("=== TSFi Latency Benchmark: Serialization Overhead ===")
    
    # Generate a dummy 512x512 image
    img = Image.new("RGB", (512, 512), "black")
    temp_path = "assets/bench_tmp.png"
    img.save(temp_path)
    
    with open(temp_path, "rb") as f:
        raw_bytes = f.read()
        
    start_time = time.time()
    iterations = 100
    
    for _ in range(iterations):
        # The exact serialization penalty happening every frame
        b64_img = base64.b64encode(raw_bytes).decode('utf-8')
        data = { "image_data": [{"data": b64_img, "id": 1}] }
        payload = json.dumps(data).encode('utf-8')
        # Simulate decoding on server side
        decoded = json.loads(payload.decode('utf-8'))
        _ = base64.b64decode(decoded['image_data'][0]['data'])
        
    end_time = time.time()
    total = (end_time - start_time) * 1000
    avg_ms = total / iterations
    
    print(f"[RESULT] Pure Base64/JSON Serialization overhead: {avg_ms:.2f} ms per frame.")
    return avg_ms

def bench_gpu_inference():
    print("\\n=== TSFi Latency Benchmark: Pure GPU Inference ===")
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    
    # 1 Step inference (absolute physical minimum limit of the Vulkan hardware)
    data = {
        "prompt": "A teddy bear",
        "width": 512,
        "height": 512,
        "steps": 1,
        "cfg_scale": 1.0,
        "seed": 42
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        # Warmup
        urllib.request.urlopen(req)
        
        start_time = time.time()
        iterations = 5
        for _ in range(iterations):
            urllib.request.urlopen(req)
        end_time = time.time()
        
        total = (end_time - start_time) * 1000
        avg_ms = total / iterations
        print(f"[RESULT] 1-Step Vulkan GPU Generation (Including TCP overhead): {avg_ms:.2f} ms per frame.")
        return avg_ms
    except Exception as e:
        print(f"[FRACTURE] API Server offline: {e}")
        return -1

if __name__ == "__main__":
    os.makedirs("assets", exist_ok=True)
    ser_ms = bench_base64_overhead()
    gpu_ms = bench_gpu_inference()
    
    print("\\n=== ARCHITECTURAL CONCLUSION ===")
    if gpu_ms > 0:
        bottleneck_ratio = (ser_ms / gpu_ms) * 100
        print(f"-> Base64 Serialization accounts for roughly {bottleneck_ratio:.2f}% of the physical generation time even at 1-step speeds.")
        print("-> To achieve 60 FPS, all JSON/Base64/HTTP layers must be violently stripped from the architecture.")
