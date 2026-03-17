import os
import time
import subprocess
import base64
import math
import io
import json
import random
import numpy as np
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw, ImageFilter

# --- TSFi Neural Realizer Configuration ---
SD_WORKER_BIN = "bin/tsfi_sd_worker"
OUTPUT_FRAME = "/tmp/tsfi_neural_out.raw"
SHM_DEPTH = "/dev/shm/tsfi_cn_depth"
SHM_POSE = "/dev/shm/tsfi_cn_pose"

def generate_photorealistic_prompt(active_style):
    base_prompt = "hyper-photorealistic macro photography of a crow stuffed animal, hand-stitched matte black fabric, oily iridescent matted feather texture, glowing green glass bead eyes, detailed stitching seams, 8k resolution, cinematic lighting"
    if active_style == "0xGOLD": style_prompt = "solid liquid gold chrome, mirror reflections"
    elif active_style == "0xSICK": style_prompt = "pulsing toxic green decay, glowing fractures"
    else: style_prompt = "worn plush fabric, vintage aesthetic"
    return f"{base_prompt}, {style_prompt}"

# Global State
active_style = "0xREAL"
recent_signals = ["[SYSTEM] Neural Masterpiece Active (Port 9093)"]
joints = {
    "body": [640, 450], "neck": [640, 360], "head": [640, 280],
    "beak_u": [720, 270], "beak_l": [720, 290],
    "l_shoulder": [580, 400], "r_shoulder": [700, 400],
    "l_wing_tip": [450, 350], "r_wing_tip": [830, 350],
    "l_hip": [610, 520], "r_hip": [670, 520],
    "l_foot": [600, 600], "r_foot": [680, 600],
    "tail": [520, 550]
}

def push_signal(msg):
    recent_signals.append(msg)
    if len(recent_signals) > 50: recent_signals.pop(0)

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class WorldSerpentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        global active_style
        if self.path == '/':
            self.send_response(200); self.send_header('Content-type', 'text/html'); self.end_headers()
            html = """
            <html>
            <head><title>TSFi World Serpent: Neural 720p</title>
            <style>body { background: #000; color: #0ff; font-family: monospace; text-align: center; } .manifold { width: 1280px; height: 720px; border: 2px solid #0ff; margin: 20px auto; } .sidebar { text-align: left; width: 1280px; margin: 0 auto; background: #011; border: 1px solid #088; padding: 10px; font-size: 11px; height: 100px; overflow-y: auto; }</style>
            </head>
            <body>
                <h1>TSFi World Serpent: Neural Masterpiece Distribution</h1>
                <div class="manifold"><img id="serpentFrame" src="/serpent_stream" style="width:100%; height:100%;" onmousedown="startDrag(event)" onmousemove="doDrag(event)" onmouseup="stopDrag()"></div>
                <div class="sidebar" id="signalPanel"></div>
                <script>
                    let dragging = false; let activeJoint = null;
                    function startDrag(e) { 
                        const rect = e.target.getBoundingClientRect();
                        const x = (e.clientX - rect.left) * (1280 / rect.width);
                        const y = (e.clientY - rect.top) * (720 / rect.height);
                        fetch(`/puppet_grab?x=${x}&y=${y}`).then(r => r.json()).then(data => { if (data.joint) { dragging = true; activeJoint = data.joint; } });
                    }
                    function doDrag(e) {
                        if (!dragging) return;
                        const rect = e.target.getBoundingClientRect();
                        const x = (e.clientX - rect.left) * (1280 / rect.width);
                        const y = (e.clientY - rect.top) * (720 / rect.height);
                        fetch(`/puppet_move?joint=${activeJoint}&x=${x}&y=${y}`);
                    }
                    function stopDrag() { dragging = false; activeJoint = null; }
                    setInterval(() => {
                        fetch('/distribution_meta.json').then(r => r.json()).then(data => {
                            document.getElementById('signalPanel').innerHTML = data.signals.map(m => `<div>${m}</div>`).join('');
                        });
                    }, 1000);
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
        elif self.path.startswith('/puppet_grab'):
            from urllib.parse import urlparse, parse_qs
            q = parse_qs(urlparse(self.path).query); tx, ty = float(q['x'][0]), float(q['y'][0])
            nearest, min_dist = None, 100000
            for name, pos in joints.items():
                d = math.sqrt((tx-pos[0])**2 + (ty-pos[1])**2)
                if d < min_dist: min_dist, nearest = d, name
            self.send_response(200); self.send_header('Content-type', 'application/json'); self.end_headers()
            self.wfile.write(json.dumps({"joint": nearest if min_dist < 100 else None}).encode())
        elif self.path.startswith('/puppet_move'):
            from urllib.parse import urlparse, parse_qs
            q = parse_qs(urlparse(self.path).query); name, nx, ny = q['joint'][0], float(q['x'][0]), float(q['y'][0])
            joints[name] = [nx, ny]; self.send_response(200); self.end_headers()
        elif self.path == '/distribution_meta.json':
            self.send_response(200); self.send_header('Content-type', 'application/json'); self.end_headers()
            self.wfile.write(json.dumps({"signals": recent_signals}).encode())
        elif self.path == '/serpent_stream':
            self.send_response(200); self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME'); self.end_headers()
            depth_shm = np.memmap(SHM_DEPTH, dtype=np.uint8, mode='w+', shape=(512, 512, 3))
            pose_shm = np.memmap(SHM_POSE, dtype=np.uint8, mode='w+', shape=(512, 512, 3))

            while True:
                try:
                    # 1. ANATOMICAL DEPTH GUIDE (512x512)
                    guide_img = Image.new("RGB", (512, 512), (0, 0, 0))
                    draw = ImageDraw.Draw(guide_img); scale = 512 / 1280.0
                    
                    # Body Volume
                    draw.ellipse([joints["body"][0]*scale-40, joints["body"][1]*scale-35, joints["body"][0]*scale+40, joints["body"][1]*scale+35], fill=(180, 180, 180))
                    # Head Volume
                    draw.ellipse([joints["head"][0]*scale-25, joints["head"][1]*scale-20, joints["head"][0]*scale+25, joints["head"][1]*scale+20], fill=(220, 220, 220))
                    # Beak
                    draw.polygon([(joints["head"][0]*scale+15, joints["head"][1]*scale), (joints["beak_u"][0]*scale, joints["beak_u"][1]*scale), (joints["beak_l"][0]*scale, joints["beak_l"][1]*scale)], fill=(255, 255, 255))
                    
                    # Wings (Anisotropic Plates)
                    for i in range(10):
                        off = i * 5
                        draw.line([(joints["l_shoulder"][0]*scale, joints["l_shoulder"][1]*scale), (joints["l_wing_tip"][0]*scale+off, joints["l_wing_tip"][1]*scale-off)], fill=(120+i*10, 120, 120), width=10)
                        draw.line([(joints["r_shoulder"][0]*scale, joints["r_shoulder"][1]*scale), (joints["r_wing_tip"][0]*scale-off, joints["r_wing_tip"][1]*scale-off)], fill=(120+i*10, 120, 120), width=10)

                    guide_img = guide_img.filter(ImageFilter.GaussianBlur(radius=6))
                    depth_shm[:] = np.array(guide_img); pose_shm[:] = np.array(guide_img)

                    # 2. TRIGGER NEURAL REALIZER
                    prompt = generate_photorealistic_prompt(active_style)
                    cmd = [SD_WORKER_BIN, prompt, OUTPUT_FRAME, "1", "turbo", "8", "euler_a", "1.5"]
                    subprocess.run(cmd, capture_output=True)

                    # 3. READ MASTERPIECE
                    if os.path.exists(OUTPUT_FRAME):
                        raw_data = np.fromfile(OUTPUT_FRAME, dtype=np.uint8).reshape((512, 512, 3))
                        img = Image.fromarray(raw_data).resize((1280, 720))
                    else: img = Image.new("RGB", (1280, 720), (20, 0, 0))

                    buf = io.BytesIO(); img.save(buf, format='JPEG')
                    self.wfile.write(b'--FRAME\r\nContent-Type: image/jpeg\r\nContent-Length: ' + str(len(buf.getvalue())).encode() + b'\r\n\r\n' + buf.getvalue() + b'\r\n')
                    time.sleep(0.016)
                except (ConnectionResetError, BrokenPipeError): break

if __name__ == '__main__':
    subprocess.run(["fuser", "-k", "9093/tcp"])
    for p in [SHM_DEPTH, SHM_POSE]:
        if not os.path.exists(p):
            with open(p, "wb") as f: f.write(b'\x00' * (512 * 512 * 3))
    server = ThreadedHTTPServer(('0.0.0.0', 9093), WorldSerpentHandler)
    print("=== TSFi World Serpent: Anatomical Neural Realization Live (Port 9093) ===")
    server.serve_forever()
