import os
import sys
import time
import subprocess
import base64
import math
import io
import json
import urllib.request
import ctypes
import random
import torch
from PIL import Image, ImageDraw, ImageChops, ImageFilter
from transformers import AutoModelForCausalLM, AutoProcessor

# Global Stage Locks
locked_background = None
isolated_bear_epoch2 = None
isolated_bear_epoch3 = None
dna_epoch = 0

def extract_temporal_silhouette(img_a, img_b):
    diff = ImageChops.difference(img_a, img_b).convert("L")
    diff_blurred = diff.filter(ImageFilter.GaussianBlur(radius=5))
    return diff_blurred.point(lambda p: 255 if p > 15 else 0)

def init_ffmpeg_rtmp(stream_key):
    print(f"[INFO] Booting FFmpeg RTMP Muxer towards Global Ingest...")
    # YouTube RTMP URL format. Change to Twitch (rtmp://live.twitch.tv/app/) if required.
    rtmp_url = f"rtmp://a.rtmp.youtube.com/live2/{stream_key}"
    
    cmd = [
        "ffmpeg",
        "-y",
        "-f", "rawvideo",
        "-vcodec", "rawvideo",
        "-s", "512x1024",
        "-pix_fmt", "rgb24",
        "-r", "10", # 10 FPS stream pacing
        "-i", "-", # Read raw bytes from stdin
        "-f", "lavfi",
        "-i", "sine=frequency=40:sample_rate=44100", # Sovereign 40Hz Heartbeat Audio
        "-c:v", "libx264",
        "-preset", "veryfast",
        "-b:v", "3000k",
        "-maxrate", "3000k",
        "-bufsize", "6000k",
        "-pix_fmt", "yuv420p",
        "-g", "20", # Keyframe interval
        "-c:a", "aac",
        "-b:a", "128k",
        "-f", "flv",
        rtmp_url
    ]
    
    return subprocess.Popen(cmd, stdin=subprocess.PIPE)

def execute_global_broadcast():
    print("=== TSFi RTMP Global VTUBE Broadcaster ===", flush=True)
    
    if not os.path.exists("stream_key.txt"):
        print("[FRACTURE] 'stream_key.txt' not found.")
        print("-> You must create a file named 'stream_key.txt' in the project root containing your YouTube/Twitch RTMP key.")
        return
        
    with open("stream_key.txt", "r") as f:
        stream_key = f.read().strip()
        
    ffmpeg_process = init_ffmpeg_rtmp(stream_key)
    
    print("[INFO] Booting VLM Blockade Matrix...")
    torch.set_num_threads(4)
    vlm = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
    processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
    
    global dna_epoch, locked_background, isolated_bear_epoch2, isolated_bear_epoch3
    epoch2_cache = None
    epoch3_cache = None
    
    # C-Kernel Silhouette Engine
    try:
        silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
        silhouette_lib.extract_silhouette.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int]
    except:
        print("[FRACTURE] C-Kernel libtsfi_silhouette.so not found. Compile it first.")
        return

    while True:
        try:
            # Check if FFmpeg crashed (e.g. stream key invalid or network drop)
            if ffmpeg_process.poll() is not None:
                print("[WARN] FFmpeg process died. Attempting to resurrect the RTMP pipe...")
                time.sleep(5)
                ffmpeg_process = init_ffmpeg_rtmp(stream_key)
            
            if locked_background is None:
                # 1. The Stage Isolation Loop (txt2img Epoch 2 and Epoch 3)
                print(f"[STAGE SEPARATION] Searching for Absolute Perfection | Epoch {dna_epoch}")
                
                api_endpoint = "txt2img"
                prompt = "A perfectly proportioned photorealistic brown teddy bear sitting on a floor, masterpiece, visible ears, visible eyes, visible snout, brightly lit room, detailed floor"
                neg_prompt = "abstract, deformed, blurry, cartoon, bad anatomy, dark, shadow"
                
                data = {
                    "prompt": prompt, "negative_prompt": neg_prompt,
                    "width": 256, "height": 256, "steps": 20, "cfg_scale": 7.0, 
                    "seed": 42 + dna_epoch 
                }
                req = urllib.request.Request(f'http://127.0.0.1:8080/sdapi/v1/{api_endpoint}', data=json.dumps(data).encode('utf-8'))
                req.add_header('Content-Type', 'application/json')
                response = urllib.request.urlopen(req)
                img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
                current_img = Image.open(io.BytesIO(img_bytes)).convert("RGB")
                
                # Check VLM Perfection for Epoch 2 (The Baseline)
                vlm_score = 0
                enc_image = vlm.encode_image(current_img)
                if "yes" in vlm.answer_question(enc_image, "Does this image clearly show a perfect teddy bear with a head, ears, eyes, snout, and body? Answer YES or NO.", processor).lower():
                    vlm_score += 100
                    
                if vlm_score < 100:
                    print(f"  -> [REJECTED] Frame failed anatomical verification. Resampling.")
                    dna_epoch += 1
                    continue
                    
                print(f"  -> [100/100 VERIFIED] Masterpiece Locked.")
                final_output = current_img.resize((512, 512), Image.NEAREST)
                
                if epoch2_cache is None:
                    epoch2_cache = current_img
                elif epoch3_cache is None:
                    epoch3_cache = current_img
                    
                    # Extract the absolute silhouette and punch the holes
                    mask = extract_temporal_silhouette(epoch2_cache, epoch3_cache)
                    
                    locked_background = epoch2_cache.copy().convert("RGBA")
                    bg_pixels = locked_background.load()
                    m_pixels = mask.load()
                    for y in range(256):
                        for x in range(256):
                            if m_pixels[x,y] == 255: bg_pixels[x,y] = (0,0,0,0)
                            
                    isolated_bear_epoch2 = epoch2_cache.copy().convert("RGBA")
                    isolated_bear_epoch3 = epoch3_cache.copy().convert("RGBA")
                    b2_p = isolated_bear_epoch2.load()
                    b3_p = isolated_bear_epoch3.load()
                    
                    for y in range(256):
                        for x in range(256):
                            if m_pixels[x,y] == 0:
                                b2_p[x,y] = (0,0,0,0)
                                b3_p[x,y] = (0,0,0,0)
                                
                    print("[ABSOLUTE SUCCESS] Stage Background Permanently Locked.")
                    dna_epoch = 4
                else:
                    dna_epoch += 1

            else:
                # 2. The Forward Math Evolution Matrix (Morphing 3 -> 2)
                # Over 40 frames, the Bear slowly transitions from Epoch 3 (Corrupted) to Epoch 2 (Healed Masterpiece)
                evolution_progress = ((dna_epoch - 4) % 40) / 40.0
                
                morphed_bear = Image.blend(isolated_bear_epoch3, isolated_bear_epoch2, alpha=evolution_progress)
                final_stage = Image.alpha_composite(locked_background, morphed_bear)
                final_output = final_stage.convert("RGB").resize((512, 512), Image.NEAREST)
                
                dna_epoch += 1

            # 3. Multiplexing the UI
            # For the stream, we show the raw math/VLM state on top, and the final broadcast on the bottom
            mpx_img = Image.new("RGB", (512, 1024))
            
            # Draw telemetry data on the top panel
            draw = ImageDraw.Draw(mpx_img)
            draw.rectangle([0, 0, 512, 512], fill=(10, 10, 20))
            if locked_background is None:
                draw.text((20, 250), f"[HUNTING] Securing Stage Epoch {dna_epoch}", fill=(255, 255, 0))
            else:
                draw.text((20, 250), f"[BROADCASTING] Synthesizing DNA Evolution Frame {dna_epoch}", fill=(0, 255, 0))
                
            mpx_img.paste(final_output, (0, 512))
            
            # 4. Stream to FFmpeg Pipe (Raw RGB24)
            # The exact microsecond the frame is ready, it is flushed into the RTMP pipe.
            ffmpeg_process.stdin.write(mpx_img.tobytes())
            ffmpeg_process.stdin.flush()
            
        except Exception as e:
            print(f"[FRACTURE] Broadcast Loop Error: {e}")
            time.sleep(2)

if __name__ == "__main__":
    execute_global_broadcast()
