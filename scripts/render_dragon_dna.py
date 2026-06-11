import os
import struct
import subprocess
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

def render_dragon_frame(frame_idx=700):
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
    
    # Construct cel-shaded Don Bluth prompt with values from DNA record
    prompt = (
        f"A fierce animated red dragon, skin color RGB({frame['r']},{frame['g']},{frame['b']}), "
        f"glowing eyes RGB({frame['er']},{frame['eg']},{frame['eb']}), "
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
            
        width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
        height = 512 if len(raw_data) == 512 * 512 * 3 else 720
        img = Image.frombytes('RGB', (width, height), raw_data)
        img.save(png_out)
        print(f"[Director] Animated Dragon DNA frame ({width}x{height}) successfully saved to: {png_out}")
        os.remove(raw_path_adj)
    else:
        print("[Error] SD Worker output raw file not found.")

if __name__ == "__main__":
    import sys
    frame_to_render = 700
    if len(sys.argv) > 1:
        frame_to_render = int(sys.argv[1])
    render_dragon_frame(frame_to_render)
