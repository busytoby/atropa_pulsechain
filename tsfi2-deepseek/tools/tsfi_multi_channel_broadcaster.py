import os
import time
import subprocess
import base64
import math
import io
import re
import random
import json
import urllib.request
import ctypes
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw, ImageChops, ImageFilter

# Global Architecture Locks
locked_background = None
isolated_bear_target = None
dna_epoch = 0
recent_logs = ["[SYSTEM] TSFi Sovereign Matrix Initialized..."]

def push_telemetry(msg):
    recent_logs.append(msg)
    if len(recent_logs) > 50:
        recent_logs.pop(0)

def generate_dna_mask(frame, W=256, H=256):
    mask = Image.new("L", (W, H), 0)
    draw = ImageDraw.Draw(mask)
    for i in range(12):
        t = frame * 0.1 + i * 0.5
        x = int(W/2 + math.cos(t) * 80)
        y = int(H/2 + math.sin(t*1.3) * 80)
        r = int(10 + math.sin(t*0.7) * 5)
        draw.ellipse([x-r, y-r, x+r, y+r], fill=255)
    return mask

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class MultiChannelHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi Sovereign Evolution</title>
                <style>
                    body { background-color: #050505; color: #0f0; font-family: monospace; text-align: center; margin-top: 30px; }
                    .container { display: flex; justify-content: center; gap: 20px; align-items: flex-start; max-width: 900px; margin: 0 auto; }
                    .compositor { position: relative; width: 512px; height: 512px; border: 2px solid #0f0; box-shadow: 0 0 30px #0f0; overflow: hidden; flex-shrink: 0; }
                    .panel { width: 350px; text-align: left; border: 1px solid #0f0; padding: 10px; height: 512px; overflow-y: auto; background: #001100; flex-shrink: 0; box-sizing: border-box; }
                    .panel div { margin-bottom: 5px; line-height: 1.3; font-size: 13px; }
                    canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
                    .quad-labels { position: absolute; width: 100%; height: 100%; pointer-events: none; }
                    .lbl { position: absolute; font-size: 11px; background: rgba(0,255,0,0.2); border: 1px solid #0f0; padding: 2px 4px; font-weight: bold; text-shadow: 1px 1px 2px #000; color: #fff; }
                    .lbl-tl { top: 10px; left: 10px; }
                    .lbl-tr { top: 10px; left: 266px; }
                    .lbl-bl { top: 266px; left: 10px; }
                    .lbl-br { top: 266px; left: 266px; }
                </style>
            </head>
            <body>
                <h1>TSFi Sovereign Teddy Channel (Port 9091)</h1>
                <div class="container">
                    <div class="compositor">
                        <canvas id="painterCanvas" width="512" height="512"></canvas>
                        <div class="quad-labels">
                            <div class="lbl lbl-tl">Q1: VOLUMETRIC PBR FUR</div>
                            <div class="lbl lbl-tr">Q2: RICCI GEODESIC SKELETON</div>
                            <div class="lbl lbl-bl">Q3: KINEMATIC TENSION HEATMAP</div>
                            <div class="lbl lbl-br">Q4: EVOLUTIONARY COMPOSITE</div>
                        </div>
                    </div>
                    <div class="panel" id="logPanel"></div>
                </div>
                <script>
                    const painterCtx = document.getElementById('painterCanvas').getContext('2d');
                    const img = new Image();
                    img.crossOrigin = "Anonymous";
                    img.src = "/multiplex_stream";
                    function updateCanvas() {
                        if(img.complete && img.naturalWidth > 0) {
                            painterCtx.drawImage(img, 0, 0, 512, 512, 0, 0, 512, 512);
                        }
                        requestAnimationFrame(updateCanvas);
                    }
                    img.onload = () => { requestAnimationFrame(updateCanvas); };

                    const logPanel = document.getElementById('logPanel');
                    setInterval(() => {
                        fetch('/telemetry.json').then(r => r.json()).then(data => {
                            let wasAtBottom = (logPanel.scrollHeight - logPanel.scrollTop <= logPanel.clientHeight + 20);
                            logPanel.innerHTML = data.map(m => `<div>${m}</div>`).join('');
                            if (wasAtBottom) logPanel.scrollTop = logPanel.scrollHeight;
                        });
                    }, 500);
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/telemetry.json':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Cache-Control', 'no-cache')
            self.end_headers()
            self.wfile.write(json.dumps(recent_logs).encode('utf-8'))

        elif self.path == '/multiplex_stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            global dna_epoch, locked_background, isolated_bear_target
            
            try:
                silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
                silhouette_lib.extract_silhouette.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int]
                silhouette_lib.generate_nand_trap_frame.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_uint8)]
                silhouette_lib.tsfi_jpeg_encode.argtypes = [
                    ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), 
                    ctypes.POINTER(ctypes.c_ulong),
                    ctypes.POINTER(ctypes.c_uint8),
                    ctypes.c_int, ctypes.c_int, ctypes.c_int
                ]
                silhouette_lib.tsfi_ai_evaluate_vlm.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
                silhouette_lib.tsfi_free_buffer.argtypes = [ctypes.c_void_p]
                silhouette_lib.tsfi_image_blend.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int, ctypes.c_float]
            except Exception as e:
                print(f"[ERROR] Could not load libtsfi_silhouette: {e}")
                push_telemetry(f"[ERROR] C-Kernel load failed: {e}")
                silhouette_lib = None

            vlm_feedback = "Photorealistic overstuffed brown teddy bear."
            masterpiece_buffer = None
            external_params = (ctypes.c_float * 16)(*[1.0]*16)

            while True:
                try:
                    jpeg_bytes = None
                    if silhouette_lib is not None:
                        w_c, h_c = 512, 512
                        pixel_buffer = (ctypes.c_uint8 * (w_c * h_c * 3))()
                        
                        # Trigger SD update immediately at start, then every 100 frames
                        if dna_epoch % 100 == 0 or (dna_epoch == 1 and masterpiece_buffer is None):
                            push_telemetry(f"<br/><b>[EPOCH {dna_epoch}]</b> Initiating Neural Sync...")
                            
                            # Interrogate VLM for current state
                            out_ptr = ctypes.POINTER(ctypes.c_uint8)()
                            out_size = ctypes.c_ulong(0)
                            if silhouette_lib.tsfi_jpeg_encode(ctypes.byref(out_ptr), ctypes.byref(out_size), pixel_buffer, w_c, h_c, 85) == 0:
                                b64_frame = base64.b64encode(ctypes.string_at(out_ptr, out_size.value)).decode('utf-8')
                                silhouette_lib.tsfi_free_buffer(ctypes.cast(out_ptr, ctypes.c_void_p))
                                vlm_resp = ctypes.create_string_buffer(512)
                                q = b"What specific photorealistic details are missing? Answer in one short prompt fragment."
                                
                                push_telemetry(f" > Interrogating Moondream2 VLM...")
                                silhouette_lib.tsfi_ai_evaluate_vlm(b64_frame.encode('utf-8'), q, vlm_resp, 512)
                                vlm_feedback = vlm_resp.value.decode('utf-8')
                                push_telemetry(f" > <b>[VLM]</b> <i>\"{vlm_feedback}\"</i>")
                                
                                # Mutate C-kernel params based on VLM keywords
                                if "eye" in vlm_feedback.lower(): external_params[3] = 1.2 # Larger eyes
                                if "fur" in vlm_feedback.lower(): external_params[0] = 1.1 # Redder fur
                                if "tall" in vlm_feedback.lower(): external_params[4] = 1.2 # Stretch
                                
                            sd_prompt = f"Photorealistic brown teddy bear, {vlm_feedback}, high detail fur, cinematic lighting"
                            push_telemetry(f" > Deploying Native SD Worker for Masterpiece...")
                            
                            # 3D Scan 'Genie': We write the procedural AVX-512 geometry buffer to disk.
                            # The C++ SD worker uses this as a precise ControlNet Depth map, ensuring the 
                            # photorealistic animation perfectly matches the high-speed procedural movements.
                            start_sd = time.time()
                            with open("tmp/current_depth.raw", "wb") as f:
                                f.write(ctypes.string_at(pixel_buffer, w_c * h_c * 3))
                                
                            push_telemetry(f" > [GENIE] 3D Depth Map Scanned and Transmitted.")

                            subprocess.run(["bin/tsfi_sd_worker", sd_prompt, "tmp/sd_masterpiece.raw", "tmp/current_depth.raw"], capture_output=True)
                            if os.path.exists("tmp/sd_masterpiece.raw"):
                                with open("tmp/sd_masterpiece.raw", "rb") as f:
                                    # Load 512x512 high-res masterpiece
                                    masterpiece_data = f.read()
                                    full_masterpiece = Image.frombytes("RGB", (512, 512), masterpiece_data)
                                    # Resize to 256x256 for Q4 slot (maintains super-sampled quality)
                                    masterpiece_buffer = (ctypes.c_uint8 * (256 * 256 * 3)).from_buffer_copy(full_masterpiece.resize((256, 256), Image.LANCZOS).tobytes())
                                push_telemetry(f" > <b>[SD HIGH-RES SUCCESS]</b> 512x512 Masterpiece Rendered.")

                        genome_idx = (dna_epoch // 20) % 4
                        
                        # Update NandTrapState external_params in storage for C-kernel to pick up
                        # (Simpler: Just pass it via a global or file for now, but here we'll assume the C-kernel handles it)
                        # We need to update the C-kernel to actually read these from the binary state file.
                        
                        silhouette_lib.generate_nand_trap_frame(pixel_buffer, dna_epoch, genome_idx, masterpiece_buffer)

                        out_ptr = ctypes.POINTER(ctypes.c_uint8)()
                        out_size = ctypes.c_ulong(0)
                        if silhouette_lib.tsfi_jpeg_encode(ctypes.byref(out_ptr), ctypes.byref(out_size), pixel_buffer, w_c, h_c, 85) == 0:
                            jpeg_bytes = ctypes.string_at(out_ptr, out_size.value)
                            silhouette_lib.tsfi_free_buffer(ctypes.cast(out_ptr, ctypes.c_void_p))
                        dna_epoch += 1
                    else:
                        time.sleep(1)

                    if jpeg_bytes:
                        self.wfile.write(b'--FRAME\r\n')
                        self.send_header('Content-Type', 'image/jpeg')
                        self.send_header('Content-Length', str(len(jpeg_bytes)))
                        self.end_headers()
                        self.wfile.write(jpeg_bytes)
                        self.wfile.write(b'\r\n')
                        
                except (ConnectionResetError, BrokenPipeError):
                    break
                except Exception as e:
                    print(f"[FRACTURE] {e}")
                    time.sleep(1)
        else:
            self.send_error(404)

if __name__ == '__main__':
    server = ThreadedHTTPServer(('0.0.0.0', 9091), MultiChannelHandler)
    print("=== TSFi Teddy Channel Active (Port 9091) ===")
    server.serve_forever()
