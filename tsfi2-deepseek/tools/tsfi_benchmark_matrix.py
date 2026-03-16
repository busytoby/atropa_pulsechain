import time
import math
import urllib.request
import json
import base64
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def benchmark_sdf_generation():
    print("[BENCHMARK] Testing Pure Python SDF Geometry Generation (512x512)...")
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    start_time = time.time()
    iterations = 10
    
    for i in range(iterations):
        for y in range(H):
            py = y / H
            for x in range(W):
                px = x / W
                dist = math.hypot(px - 0.5, py - 0.5)
                if dist < 0.2:
                    pixels[x, y] = (255, 0, 0)
                    
    end_time = time.time()
    total_time = end_time - start_time
    avg_ms = (total_time / iterations) * 1000
    print(f"  -> Result: {avg_ms:.2f} ms per frame.")
    return avg_ms

def benchmark_vulkan_painter():
    print("\\n[BENCHMARK] Testing RX 9070 XT Vulkan API Throughput (SD1.5, 6 Steps)...")
    url = 'http://127.0.0.1:8080/txt2img'
    data = {
        "prompt": "A Sick Teddy Bear",
        "width": 512,
        "height": 512,
        "steps": 6,
        "cfg_scale": 2.5,
        "seed": 42
    }
    
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    
    try:
        # Warmup
        urllib.request.urlopen(req)
        
        start_time = time.time()
        iterations = 3
        for _ in range(iterations):
            urllib.request.urlopen(req)
        end_time = time.time()
        
        total_time = end_time - start_time
        avg_ms = (total_time / iterations) * 1000
        print(f"  -> Result: {avg_ms:.2f} ms per frame.")
        return avg_ms
    except Exception as e:
        print(f"  -> [FRACTURE] Vulkan Server unreachable: {e}")
        return -1

def benchmark_moondream_cpu():
    print("\\n[BENCHMARK] Testing Moondream2 VLM CPU Inference (OpenMP Parallel)...")
    try:
        import os
        torch.set_num_threads(os.cpu_count())
        
        model_id = "vikhyatk/moondream2"
        model = AutoModelForCausalLM.from_pretrained(
            model_id, trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained(model_id, trust_remote_code=True, revision="2024-04-02")
        
        img = Image.new('RGB', (512, 512), color='black')
        
        # Warmup
        enc = model.encode_image(img)
        model.answer_question(enc, "Test", processor)
        
        start_time = time.time()
        iterations = 2
        for _ in range(iterations):
            enc = model.encode_image(img)
            model.answer_question(enc, "Is the bear standing?", processor)
        end_time = time.time()
        
        total_time = end_time - start_time
        avg_ms = (total_time / iterations) * 1000
        print(f"  -> Result: {avg_ms:.2f} ms per visual logic check.")
        return avg_ms
    except Exception as e:
        print(f"  -> [FRACTURE] VLM load failed: {e}")
        return -1

if __name__ == "__main__":
    print("=== TSFi Unified Architecture Benchmark ===")
    sdf_ms = benchmark_sdf_generation()
    gpu_ms = benchmark_vulkan_painter()
    # Skip VLM benchmark if the user doesn't want to wait 2 minutes, but we'll run it.
    vlm_ms = benchmark_moondream_cpu()
    
    print("\\n=== FINAL THROUGHPUT ANALYSIS ===")
    if gpu_ms > 0:
        max_fps = 1000.0 / (sdf_ms + gpu_ms)
        print(f"Theoretical Max Generation Rate (No VLM): {max_fps:.2f} FPS")
