import os
import struct
import subprocess
import base64
import urllib.request
import json
from PIL import Image

def load_dna_frame(file_path, frame_idx):
    with open(file_path, "rb") as f:
        magic = f.read(4)
        if magic != b'TSFI':
            raise ValueError("Invalid DNA Header")
        total_frames = struct.unpack('=I', f.read(4))[0]
        
        # Seek to the frame (31 bytes struct size)
        f.seek(8 + frame_idx * 31)
        frame_data = f.read(31)
        if len(frame_data) < 31:
            raise ValueError("Frame index out of bounds")
        
        g_x, g_y, stretch, pulse, fire, light, r, g, b, er, eg, eb, ec = struct.unpack(
            '=ffffffBBBBBBB', frame_data
        )
        return {
            'g_x': g_x, 'g_y': g_y, 'stretch': stretch, 'pulse': pulse,
            'fire': fire, 'light': light, 'r': r, 'g': g, 'b': b,
            'er': er, 'eg': eg, 'eb': eb, 'ec': ec, 'total': total_frames
        }

def render_dragon_frame(frame_idx=700, steps_override=None, cfg_override=None, prompt_override=None):
    print("=== TSFi Dragon's Lair Style DNA Renderer ===")
    
    dna_path = "tsfi2-deepseek/assets/dragon.dna"
    if not os.path.exists(dna_path):
        print(f"[Error] DNA file not found at {dna_path}. Running compiler...")
        subprocess.run(["python3", "scripts/compile_dragon_dna.py"])
        if not os.path.exists(dna_path):
            print("[Error] Failed to compile DNA.")
            return

    print(f"[Director] Decoding DNA Frame {frame_idx}...")
    frame = load_dna_frame(dna_path, frame_idx)
    
    fire_percent = int(frame['fire'] * 100)
    print(f"  -> Fire intensity: {fire_percent}% | Light contrast: {frame['light']:.2f}")
    print(f"  -> Dragon RGB: ({frame['r']}, {frame['g']}, {frame['b']})")
    
    # Prompt Setup
    if prompt_override and prompt_override.strip():
        prompt = prompt_override
    else:
        eye_desc = "two glowing eyes"
        if frame['ec'] == 1:
            eye_desc = "one single glowing eye on its face"
        elif frame['ec'] == 3:
            eye_desc = "three glowing eyes arranged in a mystical pattern on its forehead"
        elif frame['ec'] == 4:
            eye_desc = "four glowing eyes arranged symmetrically"
        elif frame['ec'] == 0:
            eye_desc = "no eyes, blind face"

        prompt = (
            f"A fierce animated red dragon, skin color RGB({frame['r']},{frame['g']},{frame['b']}), "
            f"having {eye_desc} of color RGB({frame['er']},{frame['eg']},{frame['eb']}), "
            f"1980s cel animation style, classic Don Bluth aesthetic, bold black ink outlines, "
            f"vibrant hand-painted gouache coloration, dramatic lighting with contrast {frame['light']:.2f}, "
        )
        
        if frame_idx >= 600 and frame_idx < 900:
            prompt += f"breathing a massive burst of fiery orange dragon fire, hot golden flame eruption with intensity {frame['fire']:.2f}, dark fantasy dungeon background"
        else:
            prompt += f"crouched down inside a dark fantasy castle dungeon hallway, embers in the air with intensity {frame['fire']:.2f}"
        
    raw_out = "tmp/dna_render_dragon.raw"
    png_out = "assets/storybook/page_dragon_dna.png"
    
    os.makedirs("tmp", exist_ok=True)
    os.makedirs("assets/storybook", exist_ok=True)
    
    steps = str(steps_override) if steps_override is not None else "4"
    cfg = str(cfg_override) if cfg_override is not None else "1.5"

    # Run C++ Stable Diffusion worker
    worker_path = "./bin/tsfi_sd_worker"
    cmd = [
        worker_path,
        prompt,
        raw_out,
        "0", # no shm
        "turbo",
        steps,
        "euler_a",
        cfg
    ]
    
    print(f"[Director] Executing Stable Diffusion: {' '.join(cmd)}")
    subprocess.run(cmd, cwd="tsfi2-deepseek")
    
    raw_path_adj = "tsfi2-deepseek/" + raw_out
    if os.path.exists(raw_path_adj):
        with open(raw_path_adj, 'rb') as f:
            raw_data = f.read()
            
        width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
        height = 512 if len(raw_data) == 512 * 512 * 3 else 720
        img = Image.frombytes('RGB', (width, height), raw_data)
        img.save(png_out)
        print(f"[Director] Animated Dragon DNA frame ({width}x{height}) successfully saved to: {png_out}")
        os.remove(raw_path_adj)
        
        # Local Moondream VLM Verification
        print("[Director] Initiating local VLM verification...")
        VLM_URL = "http://127.0.0.1:11435/api/generate"
        with open(png_out, "rb") as image_file:
            b64_data = base64.b64encode(image_file.read()).decode('utf-8')
            
        payload = {
            "model": "moondream",
            "prompt": (
                "Analyze this 2D animated dragon frame rendered from DNA. "
                "Confirm if it matches the Dragon's Lair style specification: "
                "Is the dragon red with large yellow/orange eyes? "
                "Is the character drawn in an expressive 1980s cel animation style with bold contours?"
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
                print("\n=== Moondream DNA Verification Analysis ===")
                print(feedback)
                print("===========================================\n")
        except Exception as e:
            print(f"[VLM ERROR] Failed to connect to Ollama/VLM: {e}")
            print("Make sure Ollama is running on port 11435.")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="TSFi Dragon DNA Renderer")
    parser.add_argument("frame", type=int, nargs="?", default=700, help="Frame index to render")
    parser.add_argument("--steps", type=int, default=None, help="Stable Diffusion steps override")
    parser.add_argument("--cfg", type=float, default=None, help="Stable Diffusion CFG scale override")
    parser.add_argument("--prompt", type=str, default=None, help="Custom prompt override")
    args = parser.parse_args()
    
    render_dragon_frame(args.frame, args.steps, args.cfg, args.prompt)
