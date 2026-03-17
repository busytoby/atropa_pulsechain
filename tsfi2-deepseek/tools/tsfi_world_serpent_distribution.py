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

# --- TSFi Neural Realization Bridge ---
def generate_photorealistic_prompt(subjective_hash, active_style):
    base_prompt = "hyper-photorealistic crow stuffed animal, highly detailed stitching, studio lighting, 8k resolution"
    if active_style == "0xGOLD": style_prompt = "liquid metal gold chrome surface, mirror reflections"
    elif active_style == "0xVOID": style_prompt = "glowing cyan particles, fragmented geometry"
    elif active_style == "0xMOSS": style_prompt = "thick green velvet moss, earthy textures"
    elif active_style == "0xPLUSH": style_prompt = "vintage beige fabric, worn plush texture"
    elif active_style == "0xSICK": style_prompt = "pulsing neon green rot, intense glow"
    elif active_style == "0xREAL": style_prompt = "matted black feathers, subtle iridescence"
    else: style_prompt = "clean toy aesthetic, sharp edges"
    return f"{base_prompt}, {style_prompt}"

# Global State for the World Serpent
active_style = "0xREAL"
recent_signals = ["[SYSTEM] Interactive Puppetry Active (Port 9093)"]
isomorphism_lock = "CROW_ISOMORPHISM_STABILIZED_12S"

# Joint State (Managed by User + Spider)
cx, cy = 640, 400
joints = {
    "body": [cx, cy], "neck": [cx, cy - 60], "head": [cx, cy - 120],
    "beak_u": [cx + 80, cy - 130], "beak_l": [cx + 80, cy - 110],
    "l_shoulder": [cx - 40, cy - 20], "r_shoulder": [cx + 40, cy - 20],
    "l_wing_tip": [cx - 240, cy - 100], "r_wing_tip": [cx + 240, cy - 100],
    "l_hip": [cx - 30, cy + 60], "r_hip": [cx + 30, cy + 60],
    "l_foot": [cx - 40, cy + 140], "r_foot": [cx + 40, cy + 140],
    "tail": [cx - 120, cy + 100]
}

# Load Shading Manifold
manifold_path = "assets/shading_manifold.bin"
manifold_data = np.fromfile(manifold_path, dtype=np.uint8).reshape((256, 256, 4)).astype(np.float32) / 255.0 if os.path.exists(manifold_path) else np.full((256, 256, 4), 0.5)

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class WorldSerpentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        global active_style, joints
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi World Serpent Distribution (Interactive)</title>
                <style>
                    body { background-color: #000; color: #0ff; font-family: 'Courier New', monospace; text-align: center; margin-top: 20px; user-select: none; }
                    .manifold { position: relative; width: 1280px; height: 720px; border: 3px solid #0ff; box-shadow: 0 0 50px #044; margin: 0 auto; cursor: crosshair; }
                    .sidebar { width: 400px; text-align: left; border: 1px solid #0ff; padding: 10px; height: 150px; overflow-y: auto; background: #000505; font-size: 11px; margin: 10px auto; }
                    .controls { display: flex; gap: 8px; justify-content: center; margin-bottom: 10px; }
                    button { background: #000; color: #0ff; border: 1px solid #0ff; padding: 8px 12px; cursor: pointer; }
                    button.active { background: #088; color: #fff; }
                </style>
            </head>
            <body>
                <h1>TSFi World Serpent: Interactive Crow Puppetry</h1>
                <div class="manifold" id="manifold">
                    <img id="serpentFrame" src="/serpent_stream" style="width:100%; height:100%;" onmousedown="startDrag(event)" onmousemove="doDrag(event)" onmouseup="stopDrag()">
                </div>
                <div class="sidebar" id="signalPanel"></div>
                <div class="controls">
                    <button id="btn_0xGOLD" onclick="setStyle('0xGOLD')">0xGOLD</button>
                    <button id="btn_0xSICK" onclick="setStyle('0xSICK')">0xSICK</button>
                    <button id="btn_0xREAL" class="active" onclick="setStyle('0xREAL')">0xREAL</button>
                    <button id="btn_0xVOID" onclick="setStyle('0xVOID')">0xVOID</button>
                </div>
                <script>
                    let dragging = false;
                    let activeJoint = null;

                    function setStyle(s) {
                        fetch('/set_style?style=' + s).then(() => {
                            document.querySelectorAll('button').forEach(b => b.classList.remove('active'));
                            document.getElementById('btn_' + s).classList.add('active');
                        });
                    }

                    function startDrag(e) {
                        const rect = e.target.getBoundingClientRect();
                        const x = (e.clientX - rect.left) * (1280 / rect.width);
                        const y = (e.clientY - rect.top) * (720 / rect.height);
                        fetch(`/puppet_grab?x=${x}&y=${y}`).then(r => r.json()).then(data => {
                            if (data.joint) { dragging = true; activeJoint = data.joint; }
                        });
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
            q = parse_qs(urlparse(self.path).query)
            tx, ty = float(q['x'][0]), float(q['y'][0])
            nearest, min_dist = None, 100000
            for name, pos in joints.items():
                d = math.sqrt((tx-pos[0])**2 + (ty-pos[1])**2)
                if d < min_dist: min_dist, nearest = d, name
            self.send_response(200); self.send_header('Content-type', 'application/json'); self.end_headers()
            self.wfile.write(json.dumps({"joint": nearest if min_dist < 100 else None}).encode())

        elif self.path.startswith('/puppet_move'):
            from urllib.parse import urlparse, parse_qs
            q = parse_qs(urlparse(self.path).query)
            name, nx, ny = q['joint'][0], float(q['x'][0]), float(q['y'][0])
            joints[name] = [nx, ny]
            self.send_response(200); self.end_headers()

        elif self.path.startswith('/set_style'):
            from urllib.parse import urlparse, parse_qs
            active_style = parse_qs(urlparse(self.path).query)['style'][0]
            self.send_response(200); self.end_headers()

        elif self.path == '/distribution_meta.json':
            self.send_response(200); self.send_header('Content-type', 'application/json'); self.end_headers()
            self.wfile.write(json.dumps({"signals": recent_signals}).encode())

        elif self.path == '/serpent_stream':
            self.send_response(200); self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME'); self.end_headers()
            f_idx = 0
            while True:
                try:
                    img = Image.new("RGB", (1280, 720), (0, 2, 2)); draw = ImageDraw.Draw(img)
                    
                    # Spider's Autonomous Jitter (Adding Taste to your Puppetry)
                    click = math.sin(f_idx * 0.8) * 15
                    
                    # Draw Bone Hierarchy
                    pairs = [("body", "neck"), ("neck", "head"), ("head", "beak_u"), ("head", "beak_l"), 
                             ("body", "l_shoulder"), ("body", "r_shoulder"), ("l_shoulder", "l_wing_tip"), 
                             ("r_shoulder", "r_wing_tip"), ("body", "l_hip"), ("body", "r_hip"), 
                             ("l_hip", "l_foot"), ("r_hip", "r_foot"), ("body", "tail")]
                    
                    color = (0, 200, 255)
                    if active_style == "0xGOLD": color = (255, 215, 0)
                    elif active_style == "0xSICK": color = (0, 255, 0)

                    for p1, p2 in pairs:
                        draw.line([tuple(joints[p1]), tuple(joints[p2])], fill=color, width=15)
                    
                    # Draw Feather Plates (Anisotropy)
                    for i in range(15):
                        off = i * 10
                        draw.polygon([tuple(joints["l_shoulder"]), (joints["l_wing_tip"][0]+off, joints["l_wing_tip"][1]-off), (joints["body"][0]-50+off, joints["body"][1])], fill=(10+i, 10, 30+i), outline=color)
                        draw.polygon([tuple(joints["r_shoulder"]), (joints["r_wing_tip"][0]-off, joints["r_wing_tip"][1]-off), (joints["body"][0]+50-off, joints["body"][1])], fill=(10+i, 10, 30+i), outline=color)

                    # Style filters
                    if active_style == "0xREAL": img = img.filter(ImageFilter.UnsharpMask(radius=4, percent=150))
                    elif active_style == "0xVOID": img = img.filter(ImageFilter.CONTOUR)

                    buf = io.BytesIO(); img.save(buf, format='JPEG')
                    self.wfile.write(b'--FRAME\r\nContent-Type: image/jpeg\r\nContent-Length: ' + str(len(buf.getvalue())).encode() + b'\r\n\r\n' + buf.getvalue() + b'\r\n')
                    f_idx += 1; time.sleep(0.033)
                except (ConnectionResetError, BrokenPipeError): break

if __name__ == '__main__':
    subprocess.run(["fuser", "-k", "9093/tcp"])
    server = ThreadedHTTPServer(('0.0.0.0', 9093), WorldSerpentHandler)
    print("=== TSFi World Serpent: Interactive Puppetry Live (Port 9093) ===")
    server.serve_forever()
