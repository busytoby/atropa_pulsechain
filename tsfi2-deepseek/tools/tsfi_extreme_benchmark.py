import time
import urllib.request
import json

def run_extreme_turbo_benchmark():
    print("=== TSFi Extreme 1-Step Hardware Benchmark (SD-Turbo + TAESD) ===")
    
    url = 'http://127.0.0.1:8080/sdapi/v1/txt2img'
    
    # Absolute physical floor: 1 Step, Zero CFG Scale (Turbo models do not use CFG)
    data = {
        "prompt": "A highly detailed photorealistic Sick Teddy Bear, matted brown fur, glowing green eyes",
        "width": 512,
        "height": 512,
        "steps": 1,
        "cfg_scale": 1.0, 
        "seed": 42
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        print("[INFO] Executing Warmup Pass (allocating Vulkan cache)...")
        urllib.request.urlopen(req)
        
        print("\\n[ACTIVE] Initiating 5-Frame High-Speed Stress Test...")
        iterations = 5
        start_time = time.time()
        
        for i in range(iterations):
            iter_start = time.time()
            urllib.request.urlopen(req)
            iter_end = time.time()
            print(f"  -> Frame {i+1} Synthesized in: {(iter_end - iter_start) * 1000:.2f} ms")
            
        end_time = time.time()
        
        total_time = end_time - start_time
        avg_ms = (total_time / iterations) * 1000
        print(f"\\n[ABSOLUTE SUCCESS] 1-Step Vulkan Generation Average: {avg_ms:.2f} ms per frame.")
        
    except Exception as e:
        print(f"[FRACTURE] API Server offline: {e}")

if __name__ == "__main__":
    run_extreme_turbo_benchmark()
