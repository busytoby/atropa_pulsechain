import os
import struct
import subprocess
import base64
import urllib.request
import json
from PIL import Image

def load_dna_frame(file_path, frame_idx):
    with open(file_path, "rb") as f:
        # Read header
        magic = f.read(4)
        if magic != b'TSFI':
            raise ValueError("Invalid DNA Header")
        total_frames = struct.unpack('=I', f.read(4))[0]
        
        # Seek to the frame
        # Struct size is 31 bytes
        f.seek(8 + frame_idx * 31)
        frame_data = f.read(31)
        if len(frame_data) < 31:
            raise ValueError("Frame index out of bounds")
        
        g_x, g_y, stretch, pulse, sick, light, r, g, b, er, eg, eb, ec = struct.unpack(
            '=ffffffBBBBBBB', frame_data
        )
        return {
            'g_x': g_x, 'g_y': g_y, 'stretch': stretch, 'pulse': pulse,
            'sick': sick, 'light': light, 'r': r, 'g': g, 'b': b,
            'er': er, 'eg': eg, 'eb': eb, 'ec': ec
        }

def render_and_evaluate():
    print("=== TSFi Photorealistic DNA Renderer & DeepSeek Evaluator ===")
    
    dna_path = "tsfi2-deepseek/assets/atropa.dna"
    if not os.path.exists(dna_path):
        print(f"[Error] DNA file not found at {dna_path}. Run compile_dna.py first.")
        return
        
    frame_idx = 5000
    print(f"[Director] Decoding DNA Frame {frame_idx}...")
    frame = load_dna_frame(dna_path, frame_idx)
    
    sick_percent = int(frame['sick'] * 100)
    print(f"  -> Sickness: {sick_percent}% | Light: {frame['light']:.2f}")
    print(f"  -> Fur RGB: ({frame['r']}, {frame['g']}, {frame['b']})")
    
    prompt = (
        f"A highly detailed photorealistic teddy bear plush sitting down, "
        f"fur color RGB({frame['r']},{frame['g']},{frame['b']}), glowing eyes RGB({frame['er']},{frame['eg']},{frame['eb']}), "
        f"cinematic lighting with intensity {frame['light']:.2f}, "
        f"{sick_percent} percent sickness mutation details, dark background, 8k resolution, masterpiece"
    )
    
    raw_out = "tmp/dna_render_bear.raw"
    png_out = "assets/photorealistic_bear_final.png"
    
    os.makedirs("tmp", exist_ok=True)
    os.makedirs("assets", exist_ok=True)
    
    # Run C++ Stable Diffusion worker
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
    
    print(f"[Director] Executing Stable Diffusion: {' '.join(cmd)}")
    subprocess.run(cmd, cwd="tsfi2-deepseek")
    
    raw_path_adj = "tsfi2-deepseek/" + raw_out
    if os.path.exists(raw_path_adj):
        with open(raw_path_adj, 'rb') as f:
            raw_data = f.read()
        # Detect resolution based on output size (512x512 vs 1280x720)
        width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
        height = 512 if len(raw_data) == 512 * 512 * 3 else 720
        img = Image.frombytes('RGB', (width, height), raw_data)
        img.save(png_out)
        print(f"[Director] Photorealistic bear frame ({width}x{height}) saved to: {png_out}")
        
        # Call DeepSeek VLM Evaluator
        print("[Director] Initiating DeepSeek local VLM validation...")
        
        VLM_URL = "http://127.0.0.1:11435/api/generate"
        with open(png_out, "rb") as image_file:
            b64_data = base64.b64encode(image_file.read()).decode('utf-8')
            
        payload = {
            "model": "moondream",
            "prompt": (
                "Analyze this photorealistic teddy bear frame rendered from DNA. "
                "Confirm if it matches the DNA specification: "
                f"Is the fur brown? Are the eyes glowing? Is there approximately {sick_percent}% sickness/mutation?"
            ),
            "images": [b64_data],
            "stream": False
        }
        
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
                print("\n=== DeepSeek DNA Verification Analysis ===")
                print(feedback)
                print("==========================================\n")
        except Exception as e:
            print(f"[VLM ERROR] Failed to connect to Ollama/VLM: {e}")
            print("Make sure Ollama is running on port 11435 (OLLAMA_HOST=127.0.0.1:11435) with the 'moondream' model pulled.")
            
    else:
        print("[Error] SD Worker output file not found.")

if __name__ == "__main__":
    render_and_evaluate()
