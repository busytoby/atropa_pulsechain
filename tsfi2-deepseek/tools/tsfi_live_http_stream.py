import os
import time
import json
import base64
import urllib.request
import math
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image
import io

# The mathematical AOT SDF Mask generator
def generate_dna_mask(frame, W=512, H=512):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    
    pulse = math.sin(frame * 0.1) 
    
    for y in range(H):
        py = y / H
        if py < 0.1 or py > 0.95: continue
        for x in range(W):
            px = x / W
            if px < 0.1 or px > 0.9: continue
            
            dist_eye1 = math.hypot(px - 0.46, py - 0.4)
            dist_eye2 = math.hypot(px - 0.54, py - 0.4)
            pulse_radius = 0.02 + (pulse * 0.005)
            
            dist_left = math.hypot(px - 0.400, py - 0.780)
            dist_right = math.hypot(px - 0.624, py - 0.780)
            k = 50.0
            shadow_val = math.exp(-k * dist_left) + math.exp(-k * dist_right)
            shadow_dist = -math.log(shadow_val) / k if shadow_val > 0 else 1.0
            
            if shadow_dist < 0.08:
                pixels[x, y] = (19, 19, 19)
            elif dist_eye1 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif dist_eye2 < pulse_radius:
                pixels[x, y] = (0, 255, 0)
            elif math.hypot(px - 0.35, py - 0.25) < 0.075 or math.hypot(px - 0.65, py - 0.25) < 0.075:
                pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.5) < (0.2 + (pulse * 0.01)):
                if math.hypot(px - 0.45, py - 0.45) < 0.08:
                    pixels[x, y] = (252, 252, 252)
                else:
                    pixels[x, y] = (76, 29, 14)
            elif math.hypot(px - 0.5, py - 0.75) < 0.25:
                if (x * y * frame) % 100 < 50:
                    pixels[x, y] = (51, 65, 0)
                else:
                    pixels[x, y] = (76, 29, 14)
    return img

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""

class StreamingHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi Live Sovereign Choreography</title>
                <style>
                    body { background-color: #000; color: #0f0; font-family: monospace; text-align: center; margin-top: 50px; }
                    img { border: 2px solid #0f0; box-shadow: 0 0 20px #0f0; }
                </style>
            </head>
            <body>
                <h1>TSFi Unified Generation Stream</h1>
                <p>Stable Diffusion Photorealism rendering over AOT PM4 Math</p>
                <img src="/stream" width="512" height="512" />
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            frame_idx = 0
            while True:
                try:
                    # Generate the Math
                    abstract_img = generate_dna_mask(frame_idx)
                    buffered = io.BytesIO()
                    abstract_img.save(buffered, format="PNG")
                    b64_img = base64.b64encode(buffered.getvalue()).decode('utf-8')
                    
                    # Call Stable Diffusion
                    sickness = int((math.sin(frame_idx * 0.1) + 1.0) * 50)
                    prompt = f"Photorealistic sick brown teddy bear sitting, cinematic lighting, {sickness} percent mutated, dark background. Masterpiece, 8k resolution."
                    
                    data = {
                        "prompt": prompt, "width": 512, "height": 512, "steps": 6, "cfg_scale": 2.5, "seed": 42 + (frame_idx % 5),
                        "init_images": [b64_img], "denoising_strength": 0.85
                    }
                    
                    req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
                    req.add_header('Content-Type', 'application/json')
                    
                    response = urllib.request.urlopen(req)
                    img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
                    
                    # MJPEG Mandate: Browsers reject motion PNGs. We must transcode to JPEG.
                    img_pil = Image.open(io.BytesIO(img_bytes)).convert("RGB")
                    jpeg_io = io.BytesIO()
                    img_pil.save(jpeg_io, format="JPEG", quality=85)
                    jpeg_bytes = jpeg_io.getvalue()
                    
                    self.wfile.write(b'--FRAME\\r\\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(jpeg_bytes)))
                    self.end_headers()
                    self.wfile.write(jpeg_bytes)
                    self.wfile.write(b'\\r\\n')
                    
                    frame_idx += 1
                    
                except (ConnectionResetError, BrokenPipeError):
                    print("[INFO] Client severed the HTTP connection. Pausing stream.")
                    break
                except Exception as e:
                    print(f"[FRACTURE] Stream broke: {e}")
                    time.sleep(2) # Retry if SD server is busy
                    
        else:
            self.send_error(404)

if __name__ == '__main__':
    server = ThreadedHTTPServer(('0.0.0.0', 9091), StreamingHandler)
    print("=== TSFi Live Neural Broadcast Active ===")
    print("-> Open a web browser to http://127.0.0.1:9091")
    server.serve_forever()
