import os
import time
import subprocess
import base64
import math
import io
import json
import random
import ctypes
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw, ImageFilter

# Global State for the World Serpent
distribution_active = True
recent_signals = ["[SYSTEM] World Serpent Distribution Initialized (Port 9093)"]
isomorphism_lock = "STABILIZING..."
current_atom = [3.0, 420.463, 420.463, 23.487, 0.414, 0.999, 1.20, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

def push_signal(msg):
    recent_signals.append(msg)
    if len(recent_signals) > 50:
        recent_signals.pop(0)

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class WorldSerpentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi World Serpent Distribution</title>
                <style>
                    body { background-color: #000; color: #0ff; font-family: 'Courier New', monospace; text-align: center; margin-top: 30px; }
                    .container { display: flex; justify-content: center; gap: 20px; align-items: flex-start; max-width: 1000px; margin: 0 auto; }
                    .manifold { position: relative; width: 640px; height: 360px; border: 2px solid #0ff; box-shadow: 0 0 40px #044; overflow: hidden; background: #000; }
                    .sidebar { width: 300px; text-align: left; border: 1px solid #0ff; padding: 15px; height: 360px; overflow-y: auto; background: #000505; }
                    .sidebar div { margin-bottom: 8px; line-height: 1.2; font-size: 12px; }
                    .atom-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 5px; margin-top: 20px; }
                    .atom-cell { border: 1px solid #088; padding: 2px; font-size: 10px; color: #fff; }
                    h1 { text-shadow: 0 0 10px #0ff; font-size: 24px; }
                    .status-box { margin-top: 20px; padding: 10px; border: 1px solid #fff; display: inline-block; color: #fff; background: #300; }
                </style>
            </head>
            <body>
                <h1>TSFi World Serpent: Subjective Distribution (Port 9093)</h1>
                <div class="container">
                    <div class="manifold">
                        <img src="/serpent_stream" style="width:100%; height:100%;">
                    </div>
                    <div class="sidebar" id="signalPanel"></div>
                </div>
                <div class="status-box">
                    SIGNAL: <span id="isomorphismStatus">LOADING...</span>
                </div>
                <div class="container" style="margin-top:20px;">
                    <div style="width: 100%; text-align: left;">
                        <h3>Canonical Taste Atom (64-byte Resonance)</h3>
                        <div class="atom-grid" id="atomGrid"></div>
                    </div>
                </div>
                <script>
                    const signalPanel = document.getElementById('signalPanel');
                    const isoStatus = document.getElementById('isomorphismStatus');
                    const atomGrid = document.getElementById('atomGrid');

                    setInterval(() => {
                        fetch('/distribution_meta.json').then(r => r.json()).then(data => {
                            signalPanel.innerHTML = data.signals.map(m => `<div>${m}</div>`).join('');
                            isoStatus.innerText = data.lock;
                            if (data.lock.includes("STABILIZED")) {
                                document.querySelector('.status-box').style.background = "#030";
                            }
                            atomGrid.innerHTML = data.atom.map((v, i) => `<div class="atom-cell">S[${i}]: ${v.toFixed(3)}</div>`).join('');
                        });
                    }, 1000);
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/distribution_meta.json':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            meta = {
                "signals": recent_signals,
                "lock": isomorphism_lock,
                "atom": current_atom
            }
            self.wfile.write(json.dumps(meta).encode('utf-8'))

        elif self.path == '/serpent_stream':
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            frame_idx = 0
            while True:
                try:
                    # RENDER THE SUBJECTIVE CROW
                    img = Image.new("RGB", (640, 360), (0, 5, 5))
                    draw = ImageDraw.Draw(img)
                    
                    # 1. THE BODY (Manifold Center)
                    bx, by = 320, 200
                    draw.ellipse([bx-40, by-30, bx+40, by+30], fill=(5, 5, 10), outline=(0, 150, 150))
                    
                    # 2. THE HEAD & BEAK (ICPC 414A Clicking)
                    hx, hy = bx + 50, by - 40
                    draw.ellipse([hx-20, hy-20, hx+20, hy+20], fill=(5, 5, 10), outline=(0, 200, 200))
                    
                    # Beak Articulation (GCD Resonance)
                    click = math.sin(frame_idx * 0.8) * 15 # Staccato rhythm
                    draw.polygon([(hx+15, hy), (hx+60, hy-5+click), (hx+60, hy+5-click)], fill=(20, 20, 0), outline=(255, 255, 0))
                    
                    # 3. THE WINGS (ICPC 120F Spastic Flight)
                    flap = math.sin(frame_idx * 0.4) * 60
                    # Left Wing
                    draw.polygon([(bx-10, by-10), (bx-80, by-80+flap), (bx-40, by)], fill=(10, 10, 20), outline=(0, 255, 255))
                    # Right Wing
                    draw.polygon([(bx+10, by-10), (bx+80, by-80+flap), (bx+40, by)], fill=(10, 10, 20), outline=(0, 255, 255))
                    
                    # 4. THE SICKNESS (ICPC 414C Fracture)
                    for _ in range(5):
                        sx = bx + random.randint(-50, 50)
                        sy = by + random.randint(-40, 40)
                        draw.ellipse([sx-5, sy-5, sx+5, sy+5], fill=(0, 255, 0)) # Neon Green Sickness
                    
                    # 5. SUBJECTIVE SOFTNESS (Simulated MSAA/Guardband)
                    img = img.filter(ImageFilter.GaussianBlur(radius=1))
                    
                    buf = io.BytesIO()
                    img.save(buf, format='JPEG')
                    jpeg_bytes = buf.getvalue()

                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(jpeg_bytes)))
                    self.end_headers()
                    self.wfile.write(jpeg_bytes)
                    self.wfile.write(b'\r\n')
                    
                    frame_idx += 1
                    time.sleep(0.033)
                except (ConnectionResetError, BrokenPipeError):
                    break

if __name__ == '__main__':
    isomorphism_lock = "CROW_ISOMORPHISM_STABILIZED_12S"
    current_atom = [3.0, 420.463, 420.463, 23.487, 0.414, 0.999, 1.20, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    push_signal("[REALIZER] Terminal Identity Lock Achieved.")
    push_signal("[ALLIGATOR] Grade 1 Truth Verified.")
    push_signal("[SERPENT] Broadcaster Attached to Port 9093.")
    
    print("-> Attempting to bind to 0.0.0.0:9093...")
    try:
        server = ThreadedHTTPServer(('0.0.0.0', 9093), WorldSerpentHandler)
        print("=== TSFi World Serpent Active (Port 9093) ===")
        server.serve_forever()
    except Exception as e:
        print(f"[FATAL] Server crash: {e}")
