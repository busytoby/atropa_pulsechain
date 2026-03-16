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
from PIL import Image, ImageDraw

ffmpeg_process = None
global_frame_idx = 0
current_genome = "brown teddy bear"
latest_frame_bytes = None

def broadcast_thread():
    global ffmpeg_process, latest_frame_bytes
    while True:
        if ffmpeg_process is not None and latest_frame_bytes is not None:
            try:
                ffmpeg_process.stdin.write(latest_frame_bytes)
                ffmpeg_process.stdin.flush()
            except Exception:
                print("[FRACTURE] YouTube RTMP disconnected (Broken Pipe).")
                ffmpeg_process = None
        time.sleep(1.0 / 2.0)

threading.Thread(target=broadcast_thread, daemon=True).start()

# ---------------------------------------------------------
# OPENPOSE SKELETAL GENERATOR
# ---------------------------------------------------------
def build_openpose_skeleton(f_idx, phase="sit"):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    t = f_idx / 30.0
    decay = max(0.0, 1.0 - (f_idx / 60.0))
    anim_wave = (math.sin(t * 1.5) * 0.5) * decay
    
    th = 6
    
    def dl(x0, y0, x1, y1, r, g, b):
        draw.line([(x0, y0), (x1, y1)], fill=(r, g, b), width=th)
        
    # 1. The Registry Spider (Left side/Center) - A small pet
    sx = 200
    sy = 280
    
    dl(sx, sy, sx, sy - 20, 255, 0, 0)
    
    leg_spread = 30 + anim_wave * 5
    
    dl(sx, sy, sx - leg_spread, sy - 10, 255, 85, 0)
    dl(sx, sy, sx - leg_spread - 5, sy + 10, 255, 170, 0)
    dl(sx, sy, sx - leg_spread + 5, sy + 30, 255, 255, 0)
    
    dl(sx, sy, sx + leg_spread, sy - 10, 85, 255, 0)
    dl(sx, sy, sx + leg_spread + 10, sy + 10, 0, 255, 0)
    dl(sx, sy, sx + leg_spread, sy + 30, 0, 255, 85)
    
    # 2. The Memory Alligator (Right side/Underneath) - A large master
    ax = 320
    ay = 340
    
    tail_wave = math.sin(t * 3.0) * 20.0 * decay
    jaw_snap = max(0.0, math.sin(t * 4.0) * 10.0 * decay)
    
    dl(ax, ay, ax + 120, ay, 0, 255, 170)
    dl(ax + 120, ay, ax + 180, ay - tail_wave, 0, 85, 255)
    
    dl(ax, ay, ax - 60, ay - 10 - jaw_snap, 255, 0, 0)
    dl(ax, ay, ax - 60, ay + 10 + jaw_snap, 255, 0, 0)
    
    dl(ax + 20, ay, ax + 20, ay + 30, 85, 0, 255)
    dl(ax + 20, ay + 30, ax + 10, ay + 40, 170, 0, 255)
    
    dl(ax + 100, ay, ax + 100, ay + 30, 85, 0, 255)
    dl(ax + 100, ay + 30, ax + 90, ay + 40, 170, 0, 255)
    
    return img

def init_ffmpeg_rtmp(stream_key):
    stream_key = stream_key.strip()
    print(f"\n[INFO] Engaging Global FFmpeg RTMP Muxer (2 FPS)...")
    rtmp_url = f"rtmp://a.rtmp.youtube.com/live2/{stream_key}"
    cmd = [
        "ffmpeg", "-y", "-re", "-f", "rawvideo", "-vcodec", "rawvideo",
        "-s", "512x1024", "-pix_fmt", "rgb24", "-r", "2", 
        "-i", "-", 
        "-f", "lavfi", "-i", "sine=frequency=40:sample_rate=44100", 
        "-vf", "scale=-2:1920,pad=1080:1920:(ow-iw)/2:(oh-ih)/2:black",
        "-c:v", "libx264", "-preset", "veryfast", "-b:v", "2500k", "-maxrate", "2500k",
        "-bufsize", "5000k", "-pix_fmt", "yuv420p", "-r", "30", "-g", "60", 
        "-c:a", "aac", "-b:a", "128k", "-f", "flv", rtmp_url
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
                <h1>TSFi Sovereign OpenPose Broadcast</h1>
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
            
            score = f"[LIVE] Entity: {current_genome} | Phase: {phase_str} | Epoch {global_frame_idx}/366"
            self.wfile.write(score.encode('utf-8'))

        elif self.path == '/multiplex_stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            bear_types = [
                "large green alligator plush toy and its pet fuzzy spider plush toy",
                "large swamp alligator plush and its pet black widow spider plush",
                "brown leather alligator plush and a neon cyberpunk spider plush",
                "cute pastel alligator plush toy and a tiny pink spider plush toy"
            ]

            while True:
                try:
                    # Epoch Settings: 20 continuous frames of the exact same bear, then switch.
                    # 20 frames @ 2 FPS = 10 second videos per epoch.
                    FRAMES_PER_EPOCH = 20
                    
                    if global_frame_idx >= FRAMES_PER_EPOCH:
                        global_frame_idx = 0
                        current_genome = random.choice(bear_types)
                        time.sleep(1)
                        continue

                    # The physical seed MUST be locked across the 20 frame epoch to ensure it's the 
                    # exact same teddy bear just moving its limbs, rather than generating a brand new bear.
                    epoch_seed = str(42 + hash(current_genome) % 10000)
                    
                    phase = "sit" # Always sit and stir
                    local_f_idx = global_frame_idx
                    
                    # 1. Generate OpenPose Skeleton Math
                    skeleton = build_openpose_skeleton(local_f_idx, phase)
                    mask_path = f"assets/mpx_skel.png"
                    skeleton.save(mask_path)
                    
                    b64_mask = base64.b64encode(open(mask_path, "rb").read()).decode('utf-8')
                    
                    action_text = "sitting closely together"
                    prompt = f"A photorealistic macro photography shot of two stuffed plush toys, a {current_genome} {action_text}, masterpiece, cute, 8k resolution, cinematic lighting, highly detailed anatomy, covered in glowing green Kr0wZ sickness <lora:lcm_lora_sd15:1.0>"
                    
                    # 2. ControlNet CLI Pass
                    out_path = "assets/mpx_out.png"
                    cmd = [
                        "assets/sd_cpp/build/bin/sd-cli",
                        "-m", "assets/models/LCM_Dreamshaper_v7.safetensors",
                        "--vae", "assets/models/taesd.safetensors",
                        "--lora-model-dir", "assets/models",
                        "--control-net", "assets/models/control_openpose.safetensors",
                        "--control-image", mask_path,
                        "--control-strength", "0.5",
                        "-p", prompt,
                        "-n", "human, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation",
                        "-W", "512", "-H", "512",
                        "--steps", "6",
                        "--cfg-scale", "1.5", 
                        "--seed", epoch_seed, # Static seed per epoch!
                        "-o", out_path
                    ]
                    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(0.5) # Aggressive throttle to ~60% duty cycle to prevent full system stuttering
                    
                    painter_img = Image.open(out_path).convert("RGB")
                    
                    # 3. Multiplexing
                    mpx_img = Image.new("RGB", (512, 1024))
                    mpx_img.paste(skeleton, (0, 0)) # Top half is the Math Bone Matrix
                    mpx_img.paste(painter_img, (0, 512)) # Bottom half is the Final Generation
                    
                    jpeg_io = io.BytesIO()
                    mpx_img.save(jpeg_io, format="JPEG", quality=80)
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
