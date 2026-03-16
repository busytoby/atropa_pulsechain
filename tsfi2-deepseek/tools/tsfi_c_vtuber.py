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
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw, ImageFont

# --- Genie Globals ---
try:
    import torch
    from transformers import AutoModelForCausalLM, AutoProcessor
    GENIE_ENABLED = True
except ImportError:
    GENIE_ENABLED = False

genie_model = None
genie_processor = None
genie_current_thought = "GENIE IS AWAKENING..."
genie_last_observation_time = 0

def init_genie():
    global genie_model, genie_processor
    if GENIE_ENABLED and genie_model is None:
        print("[INFO] Conjuring the Moondream Genie from the Void...")
        try:
            genie_model = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
            genie_processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
            print("[SUCCESS] Genie is watching.")
        except Exception as e:
            print(f"[WARN] Genie failed to materialize: {e}")
            
# Spin it up on import
if GENIE_ENABLED:
    threading.Thread(target=init_genie, daemon=True).start()

def genie_observation_thread(frame_image):
    global genie_current_thought
    if genie_model is None or genie_processor is None: return
    try:
        enc_image = genie_model.encode_image(frame_image)
        q = "You are a mystical genie watching this stuffed animal. What is it doing? Answer in one short, poetic sentence."
        ans = genie_model.answer_question(enc_image, q, genie_processor)
        # Format the thought to fit the matrix
        genie_current_thought = "GENIE SEES: " + ans.upper()
    except:
        pass

ffmpeg_process = None
global_frame_idx = 0
current_genome = "brown teddy bear"
latest_frame_bytes = None

def broadcast_thread():
    global ffmpeg_process, latest_frame_bytes
    import time
    fps = 30.0
    frame_time = 1.0 / fps
    next_time = time.time()
    
    while True:
        if ffmpeg_process is not None and latest_frame_bytes is not None:
            try:
                ffmpeg_process.stdin.write(latest_frame_bytes)
                ffmpeg_process.stdin.flush()
            except Exception:
                print("[FRACTURE] YouTube RTMP disconnected (Broken Pipe).")
                ffmpeg_process = None
                
        next_time += frame_time
        sleep_time = next_time - time.time()
        if sleep_time > 0:
            time.sleep(sleep_time)
        else:
            # Reset if we fell behind to prevent a burst
            next_time = time.time()

threading.Thread(target=broadcast_thread, daemon=True).start()

def build_openpose_skeleton(f_idx, phase="sit"):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Render Unicode DNA Matrix Background
    try:
        font = ImageFont.truetype("assets/fonts/Hack-Regular.ttf", 14)
    except IOError:
        font = ImageFont.load_default()
        
    # The Genie's thought streams down the screen
    thought = genie_current_thought + " " + genie_current_thought + " "
    
    for column in range(0, W, 14):
        # Stagger the matrix drops based on the frame index to create a falling effect
        offset = (f_idx * 5 + column * 13) % H
        
        # Determine which character of the thought to pull based on the column
        char_idx_base = column % len(thought)
        
        for row in range(0, H, 14):
            y_pos = (row + offset) % H
            if y_pos > H - 50:
                continue # Fade out at bottom
                
            char_idx = (char_idx_base + (row // 14)) % len(thought)
            char = thought[char_idx]
            
            # Bright green matrix rain
            intensity = int(255 * (1.0 - (y_pos / float(H))))
            draw.text((column, y_pos), char, font=font, fill=(0, intensity, 0))

    center_x = W // 2
    base_y = int(H * 0.75) 
    
    swing = 0.0
    scale_y = 1.0
    translate_x = 0
    translate_y = 0
    sit_arm_wave = 0.0
    
    if phase == "walk":
        # Full stand immediately for walking epoch
        scale_y = 1.5
        
        # Walk in a circle (360 degrees over 200 frames)
        # Epoch is 200 frames. Complete exactly 1 full circle.
        theta = (f_idx / 200.0) * math.pi * 2.0
        radius_x = 180.0
        radius_y = 60.0
        
        # Start at the back of the circle, walk forward/around
        translate_x = int(math.sin(theta) * radius_x)
        translate_y = -80 + int(math.cos(theta) * radius_y)
        
        # Scale up slightly as it comes closer to the camera (bottom of the circle)
        depth_scale = 1.0 + (math.cos(theta) * 0.3)
        scale_y = 1.5 * depth_scale
        
        # Rapid gait cycle for the legs/arms
        cycle = (f_idx % 16) / 16.0 
        swing = math.sin(cycle * math.pi * 2.0)
    elif phase == "sit":
        t = f_idx / 30.0 
        # Dampening factor that reaches exactly 0.0 before the end of the 200 frame epoch (10 seconds)
        # f_idx goes from 0 to 199. At f_idx=0, decay is 1.0. At f_idx=150, decay is 0.0.
        decay = max(0.0, 1.0 - (f_idx / 150.0))
        
        sit_arm_wave = (math.sin(t * 1.5) * 0.5 + math.sin(t * 4.1) * 0.2 + math.sin(t * 0.8) * 0.3) * decay
        sit_leg_twitch = (math.cos(t * 2.3) * 0.5 + math.sin(t * 3.7) * 0.3) * decay
            
    cx = center_x + translate_x
    cy = base_y + translate_y
    
    torso_len = int(120 * scale_y)
    arm_len = int(80 * scale_y)
    leg_len = int(90 * scale_y)
    
    neck = (cx, cy - torso_len)
    nose = (cx, neck[1] - 40)
    r_eye = (cx - 15, neck[1] - 55)
    l_eye = (cx + 15, neck[1] - 55)
    r_ear = (cx - 35, neck[1] - 65)
    l_ear = (cx + 35, neck[1] - 65)
    
    r_shoulder = (neck[0] - 40, neck[1])
    l_shoulder = (neck[0] + 40, neck[1])
    r_hip = (cx - 30, cy)
    l_hip = (cx + 30, cy)
    
    if phase == "sit":
        # Best sitting form: facing the camera, legs significantly bent and squished outwards
        # Arms bent and close to the body, waking up/stirring
        
        # Upper arms rest close to torso, forearms stir/twitch
        r_elbow_x = r_shoulder[0] - 20
        r_elbow_y = r_shoulder[1] + int(arm_len * 0.4) - int(sit_arm_wave * 10)
        r_wrist_x = r_elbow_x - 10 - int(sit_arm_wave * 15)
        r_wrist_y = r_elbow_y + int(arm_len * 0.5) - int(sit_arm_wave * 25)
        
        l_elbow_x = l_shoulder[0] + 20
        l_elbow_y = l_shoulder[1] + int(arm_len * 0.4) + int(sit_arm_wave * 8)
        l_wrist_x = l_elbow_x + 10 + int(sit_arm_wave * 12)
        l_wrist_y = l_elbow_y + int(arm_len * 0.5) + int(sit_arm_wave * 20)
        
        # Legs squished outwards and bent sharply, with feet twitching
        r_knee = (r_hip[0] - 50, r_hip[1] + 30)
        r_ankle = (r_hip[0] - 80 - int(sit_leg_twitch * 10), r_hip[1] + 70 - int(sit_leg_twitch * 15))
        l_knee = (l_hip[0] + 50, l_hip[1] + 30)
        l_ankle = (l_hip[0] + 80 + int(sit_leg_twitch * 8), l_hip[1] + 70 + int(sit_leg_twitch * 12))
    else:
        r_elbow_x = r_shoulder[0] - 20
        r_elbow_y = r_shoulder[1] + int(arm_len * 0.8) + int(swing * 30)
        r_wrist_x = r_elbow_x - 10
        r_wrist_y = r_elbow_y + arm_len + int(swing * 40)
        
        l_elbow_x = l_shoulder[0] + 20
        l_elbow_y = l_shoulder[1] + int(arm_len * 0.8) - int(swing * 30)
        l_wrist_x = l_elbow_x + 10
        l_wrist_y = l_elbow_y + arm_len - int(swing * 40)
        
        r_knee_y_offset = leg_len - int(abs(swing) * 20) if swing > 0 else leg_len
        r_knee = (r_hip[0] - int(swing * 30), r_hip[1] + r_knee_y_offset)
        r_ankle = (r_hip[0] - int(swing * 50), r_knee[1] + leg_len - int(abs(swing) * 10))
        
        l_knee_y_offset = leg_len - int(abs(swing) * 20) if swing < 0 else leg_len
        l_knee = (l_hip[0] + int(swing * 30), l_hip[1] + l_knee_y_offset)
        l_ankle = (l_hip[0] + int(swing * 50), l_knee[1] + leg_len - int(abs(swing) * 10))
        
    th = 8
    draw.line([neck, (cx, cy)], fill=(255, 0, 0), width=th)
    draw.line([neck, nose], fill=(255, 0, 0), width=th)
    draw.line([nose, r_eye], fill=(255, 0, 255), width=th)
    draw.line([nose, l_eye], fill=(255, 0, 255), width=th)
    draw.line([r_eye, r_ear], fill=(255, 255, 0), width=th)
    draw.line([l_eye, l_ear], fill=(255, 255, 0), width=th)
    draw.line([neck, r_shoulder], fill=(255, 85, 0), width=th)
    draw.line([r_shoulder, (r_elbow_x, r_elbow_y)], fill=(255, 170, 0), width=th)
    draw.line([(r_elbow_x, r_elbow_y), (r_wrist_x, r_wrist_y)], fill=(255, 255, 0), width=th)
    draw.line([neck, l_shoulder], fill=(85, 255, 0), width=th)
    draw.line([l_shoulder, (l_elbow_x, l_elbow_y)], fill=(0, 255, 0), width=th)
    draw.line([(l_elbow_x, l_elbow_y), (l_wrist_x, l_wrist_y)], fill=(0, 255, 85), width=th)
    
    draw.line([(cx, cy), r_hip], fill=(0, 255, 170), width=th)
    draw.line([r_hip, r_knee], fill=(0, 85, 255), width=th)
    draw.line([r_knee, r_ankle], fill=(0, 0, 255), width=th)
    draw.line([(cx, cy), l_hip], fill=(85, 0, 255), width=th)
    draw.line([l_hip, l_knee], fill=(170, 0, 255), width=th)
    draw.line([l_knee, l_ankle], fill=(255, 0, 255), width=th)
    
    return img

def init_ffmpeg_rtmp(stream_key):
    stream_key = stream_key.strip()
    print(f"\n[INFO] Engaging Global FFmpeg RTMP Muxer (30 FPS)...")
    rtmp_url = f"rtmp://a.rtmp.youtube.com/live2/{stream_key}"
    cmd = [
        "ffmpeg", "-y", 
        "-thread_queue_size", "1024", 
        "-f", "rawvideo", "-vcodec", "rawvideo",
        "-s", "512x1024", "-pix_fmt", "rgb24", "-framerate", "30", 
        "-i", "-", 
        "-f", "lavfi", "-i", "sine=frequency=40:sample_rate=44100", 
        "-vf", "scale=1080:1920:force_original_aspect_ratio=decrease,pad=1080:1920:-1:-1:color=black",
        "-c:v", "libx264", "-preset", "ultrafast", "-tune", "zerolatency", 
        "-b:v", "6800k", "-maxrate", "6800k", "-minrate", "6800k", "-bufsize", "13600k", "-nal-hrd", "cbr",
        "-pix_fmt", "yuv420p", 
        "-r", "30", 
        "-g", "60", 
        "-c:a", "aac", "-b:a", "128k", 
        "-f", "flv", rtmp_url
    ]
    return subprocess.Popen(cmd, stdin=subprocess.PIPE)

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class MultiChannelHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        global ffmpeg_process
        if self.path == '/rtmp_init':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                stream_key = data.get("key", "").strip()
                if stream_key:
                    if ffmpeg_process is None:
                        ffmpeg_process = init_ffmpeg_rtmp(stream_key)
                    self.send_response(200)
                    self.end_headers()
                    self.wfile.write(b"RTMP Link Established")
                else:
                    self.send_response(400)
                    self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()

    def do_GET(self):
        global global_frame_idx, current_genome, ffmpeg_process
        
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi VTUBE Control Center</title>
                <style>
                    body { background-color: #050505; color: #0f0; font-family: monospace; text-align: center; margin-top: 20px; }
                    .compositor { position: relative; width: 512px; height: 512px; margin: 0 auto; border: 2px solid #0f0; box-shadow: 0 0 30px #0f0; overflow: hidden; }
                    canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
                    #telemetry { margin-top: 10px; color: #ff0; font-size: 1.2em; font-weight: bold; }
                    .control-panel { margin-top: 20px; padding: 20px; border: 1px solid #333; display: inline-block; background: #111; }
                    input[type="password"] { padding: 10px; font-family: monospace; width: 300px; background: #000; color: #0f0; border: 1px solid #0f0; }
                    button { padding: 10px 20px; background: #f00; color: #fff; font-family: monospace; font-weight: bold; cursor: pointer; border: none; }
                    button:hover { background: #ff3333; }
                </style>
            </head>
            <body>
                <h1>TSFi Sovereign Fast OpenPose Broadcast</h1>
                <div class="compositor">
                    <canvas id="painterCanvas" width="512" height="512"></canvas>
                </div>
                <div id="telemetry">Initializing Neural Lifecycle...</div>
                
                <div class="control-panel">
                    <h3>Global VTUBE Broadcast Matrix</h3>
                    <input type="password" id="rtmpKey" placeholder="Paste YouTube RTMP Stream Key">
                    <button onclick="goLive()">GO LIVE ON YOUTUBE</button>
                    <p id="rtmpStatus" style="color:#aaa;"></p>
                </div>
                
                <script>
                    const painterCtx = document.getElementById('painterCanvas').getContext('2d');
                    const img = new Image();
                    img.crossOrigin = "Anonymous";
                    img.src = "/multiplex_stream";
                    function updateCanvas() {
                        if(img.complete && img.naturalWidth > 0) {
                            painterCtx.drawImage(img, 0, 512, 512, 512, 0, 0, 512, 512);
                        }
                        requestAnimationFrame(updateCanvas);
                    }
                    img.onload = () => { requestAnimationFrame(updateCanvas); };
                    
                    setInterval(() => {
                        fetch('/telemetry').then(r => r.text()).then(t => {
                            document.getElementById('telemetry').innerText = t;
                        });
                    }, 1000);
                    
                    function goLive() {
                        const key = document.getElementById('rtmpKey').value;
                        if(!key) { alert("Matrix demands a Stream Key."); return; }
                        document.getElementById('rtmpStatus').innerText = "Initiating RTMP Handshake...";
                        fetch('/rtmp_init', {
                            method: 'POST', headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ key: key })
                        }).then(r => {
                            if(r.ok) { document.getElementById('rtmpStatus').innerText = "BROADCAST LIVE. YouTube Ingesting..."; document.getElementById('rtmpStatus').style.color = '#0f0'; }
                            else { document.getElementById('rtmpStatus').innerText = "RTMP Fracture. Check console."; document.getElementById('rtmpStatus').style.color = '#f00'; }
                        });
                    }
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/telemetry':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            
            phase_str = "Sitting (Decay)"
            if global_frame_idx > 150: phase_str = "Standing Up & Walking"
            
            score = f"[LIVE] Entity: {current_genome} | Phase: {phase_str} | Epoch {global_frame_idx}/200"
            self.wfile.write(score.encode('utf-8'))

        elif self.path == '/multiplex_stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            bear_types = [
                "brown teddy bear", "pink plush teddy bear", "gothic black patchwork teddy bear", 
                "polar bear plushie", "vintage 1920s woven teddy bear", "neon cyberpunk plush bear",
                "stuffed bunny rabbit", "plush elephant", "stuffed penguin", "stuffed tiger", "plush koala"
            ]

            # Track the physical phase state explicitly
            current_phase_state = 0 
            
            while True:
                try:
                    # Epoch Settings: 40 continuous frames of the exact same bear. 
                    # At 20 frames per second, this equals exactly 2 seconds of hyper-fast animation before a complete epoch shift.
                    FRAMES_PER_EPOCH = 40
                    
                    if global_frame_idx >= FRAMES_PER_EPOCH:
                        global_frame_idx = 0
                        current_genome = random.choice(bear_types)
                        current_phase_state = 1 - current_phase_state # Toggle explicitly 0/1
                        time.sleep(0.5)
                        continue

                    # Lock the physics seed so the 2-second animation is the exact same bear
                    epoch_seed = str(42 + hash(current_genome) % 10000)
                    
                    # Alternate behavior every epoch intentionally
                    phase = "sit" if current_phase_state == 0 else "walk"
                    local_f_idx = global_frame_idx
                    
                    # 1. Generate OpenPose Skeleton Math
                    skeleton = build_openpose_skeleton(local_f_idx, phase)
                    mask_path = f"assets/mpx_skel.png"
                    skeleton.save(mask_path)
                    
                    b64_mask = base64.b64encode(open(mask_path, "rb").read()).decode('utf-8')
                    
                    sickness = int((global_frame_idx / float(FRAMES_PER_EPOCH)) * 100)
                    
                    if phase == "sit":
                        action_text = "sitting still like a companion pet, facing the camera directly, front view, making eye contact"
                    else:
                        action_text = "standing up on two legs and walking forward aggressively, full body, upright stance"
                        
                    prompt = f"A photorealistic macro photography shot of an overstuffed plush 16 inch tall {current_genome} {action_text}, NO HUMANS, strictly non-human stuffed animal, accompanied by a 3 inch tall fuzzy hawk moth plush flying beside it, masterpiece, 8k resolution, cinematic lighting, strictly non-human animal shape, fat round torso, plush fabric material, short stuffed animal limbs, {sickness} percent mutated, both covered in glowing green Kr0wZ sickness <lora:lcm_lora_sd15:1.0>"
                    
                    out_path = "assets/mpx_out.png"
                    
                    # 2. ControlNet NATIVE C CLI Pass
                    if local_f_idx == 0:
                        # Frame 0: Generate the bear perfectly from text
                        cmd = [
                            "assets/sd_cpp/build/bin/sd-cli",
                            "-m", "assets/models/sd15.safetensors",
                            "--vae", "assets/models/taesd.safetensors",
                            "--lora-model-dir", "assets/models",
                            "--control-net", "assets/models/control_openpose.safetensors",
                            "--control-image", mask_path,
                            "--control-strength", "0.85", # Reduced to allow fat plush proportions
                            "-p", prompt,
                            "-n", "human, person, man, woman, child, people, human face, human hands, human body, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",
                            "-W", "512", "-H", "512",
                            "--steps", "4", 
                "--threads", "4",# Solid 4 steps for epoch genesis
                            "--cfg-scale", "1.0", 
                            "--seed", epoch_seed,
                            "-o", out_path
                        ]
                    else:
                        # Frame 1+: Generate the next frame by evolving the previous frame via img2img
                        # Moderate strength 0.35 means "keep 65% of the old image, physically mutate the other 35%"
                        # This allows the bear to move its arms without completely changing its face or fur every frame.
                        cmd = [
                            "assets/sd_cpp/build/bin/sd-cli",
                            "-m", "assets/models/sd15.safetensors",
                            "--vae", "assets/models/taesd.safetensors",
                            "--lora-model-dir", "assets/models",
                            "--control-net", "assets/models/control_openpose.safetensors",
                            "--control-image", mask_path,
                            "--control-strength", "0.85",
                            "-i", "assets/mpx_out.png", # Evolve the previous frame
                            "--strength", "0.35",
                            "-p", prompt,
                            "-n", "human, person, man, woman, child, people, human face, human hands, human body, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",
                            "-W", "512", "-H", "512",
                            "--steps", "2", # Fast evolution
                            "--cfg-scale", "1.0", 
                            "--seed", epoch_seed, # Lock the seed so the fur texture remains consistent across the epoch
                            "-o", out_path
                        ]
                        
                    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    time.sleep(0.5) # Aggressive throttle to ~60% duty cycle to prevent full system stuttering
                    
                    painter_img = Image.open(out_path).convert("RGB")
                    
                    # 3. Multiplexing
                    mpx_img = Image.new("RGB", (512, 1024))
                    mpx_img.paste(painter_img, (0, 0)) # Top half is the Final Generation
                    mpx_img.paste(skeleton, (0, 512)) # Bottom half is the Math Bone Matrix
                    
                    jpeg_io = io.BytesIO()
                    mpx_img.save(jpeg_io, format="JPEG", quality=80)
                    
                    # Trigger the Genie asynchronously every 4 seconds (80 frames at 20fps)
                    global genie_last_observation_time
                    current_time = time.time()
                    if GENIE_ENABLED and (current_time - genie_last_observation_time > 4.0) and local_f_idx > 10:
                        genie_last_observation_time = current_time
                        # Pass a scaled down copy of the painter frame to the Genie so it runs faster
                        obs_img = painter_img.copy().resize((256, 256))
                        threading.Thread(target=genie_observation_thread, args=(obs_img,), daemon=True).start()
                    jpeg_bytes = jpeg_io.getvalue()
                    
                    global latest_frame_bytes
                    latest_frame_bytes = mpx_img.tobytes()
                    
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(jpeg_bytes)))
                    self.end_headers()
                    self.wfile.write(jpeg_bytes)
                    self.wfile.write(b'\r\n')
                    
                    global_frame_idx += 1
                        
                except (ConnectionResetError, BrokenPipeError):
                    print("[INFO] Web UI disconnected.")
                    break
                except Exception as e:
                    print(f"[FRACTURE] Stream broke: {e}")
                    time.sleep(2)
        else:
            self.send_error(404)

if __name__ == '__main__':
    os.makedirs("assets", exist_ok=True)
    server = ThreadedHTTPServer(('0.0.0.0', 9091), MultiChannelHandler)
    print("=== TSFi VTUBE Control Center Active ===")
    print("-> Open a web browser to http://127.0.0.1:9091")
    server.serve_forever()
