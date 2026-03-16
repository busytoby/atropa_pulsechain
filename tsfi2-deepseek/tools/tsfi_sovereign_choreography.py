import os
import time
import subprocess
import base64
import math
import io
import re
import random
import torch
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw
from transformers import AutoModelForCausalLM, AutoProcessor

def generate_dna_mask(epoch, genome_id, W=256, H=256):
    img = Image.new("RGB", (W, H), (180, 180, 180))
    pixels = img.load()
    pulse = math.sin(epoch * 0.1) 
    
    # Kinematics
    g_x = 0.0
    g_y = 0.0
    body_stretch = 1.0
    
    if epoch > 50 and epoch <= 80:
        stand_progress = (epoch - 50) / 30.0
        body_stretch = 1.0 + (stand_progress * 1.0)
    elif epoch > 80:
        walk_progress = (epoch - 80)
        body_stretch = 2.0
        g_x = -(walk_progress * 0.02)
        
    for y in range(H):
        py = y / H
        if py < 0.1 or py > 0.95: continue
        for x in range(W):
            px = x / W
            ax = px - g_x
            if ax < -0.2 or ax > 1.2: continue
            
            # --- GENETIC POOL SWITCH ---
            if genome_id == 0: # Genome A: Baseline
                cran_x, cran_y, cran_r = 0.50, 0.50, 0.20 + (pulse * 0.01)
                leye_x, leye_y = 0.46, 0.40
                reye_x, reye_y = 0.54, 0.40
                lear_x, lear_y = 0.35, 0.25
                rear_x, rear_y = 0.65, 0.25
                body_x, body_y, base_body_stretch = 0.50, 0.75, 1.0
            elif genome_id == 1: # Genome B: Slumped
                cran_x, cran_y, cran_r = 0.60, 0.60, 0.18 + (pulse * 0.01)
                leye_x, leye_y = 0.57, 0.52
                reye_x, reye_y = 0.63, 0.55
                lear_x, lear_y = 0.48, 0.40
                rear_x, rear_y = 0.72, 0.45
                body_x, body_y, base_body_stretch = 0.55, 0.85, 0.8
            elif genome_id == 2: # Genome C: Bipedal Tall
                cran_x, cran_y, cran_r = 0.50, 0.30, 0.15 + (pulse * 0.01)
                leye_x, leye_y = 0.46, 0.25
                reye_x, reye_y = 0.54, 0.25
                lear_x, lear_y = 0.38, 0.15
                rear_x, rear_y = 0.62, 0.15
                body_x, body_y, base_body_stretch = 0.50, 0.65, 1.8
            else: # Genome D: Micro Fetal
                cran_x, cran_y, cran_r = 0.50, 0.65, 0.25 + (pulse * 0.01)
                leye_x, leye_y = 0.43, 0.55
                reye_x, reye_y = 0.57, 0.55
                lear_x, lear_y = 0.30, 0.45
                rear_x, rear_y = 0.70, 0.45
                body_x, body_y, base_body_stretch = 0.50, 0.85, 0.5
                
            final_body_stretch = base_body_stretch * body_stretch
            
            dist_eye1 = math.hypot(ax - leye_x, py - leye_y)
            dist_eye2 = math.hypot(ax - reye_x, py - reye_y)
            pulse_radius = 0.02 + (pulse * 0.005)
            
            dist_left = math.hypot(ax - (body_x - 0.1), py - 0.780)
            dist_right = math.hypot(ax - (body_x + 0.124), py - 0.780)
            k = 50.0
            shadow_val = math.exp(-k * dist_left) + math.exp(-k * dist_right)
            shadow_dist = -math.log(shadow_val) / k if shadow_val > 0 else 1.0
            
            base_brown = (165, 95, 45)
            
            if shadow_dist < 0.08: pixels[x, y] = (50, 50, 50)
            elif dist_eye1 < pulse_radius: pixels[x, y] = (0, 255, 0)
            elif dist_eye2 < pulse_radius: 
                # The Phase 0.5 Missing Eye Mutation Constraint
                if epoch > 25: pixels[x, y] = (30, 0, 0) # Eye rots out
                else: pixels[x, y] = (0, 255, 0)
            elif math.hypot(ax - lear_x, py - lear_y) < 0.075 or math.hypot(ax - rear_x, py - rear_y) < 0.075: pixels[x, y] = base_brown
            elif math.hypot(ax - cran_x, py - cran_y) < cran_r:
                if math.hypot(ax - (cran_x - 0.05), py - (cran_y - 0.05)) < 0.08: pixels[x, y] = (255, 255, 255)
                else: pixels[x, y] = base_brown
            elif math.hypot(ax - body_x, (py - body_y) / final_body_stretch) < 0.25:
                # Procedural Noise Gradient for Crystalline Textures instead of flat paint
                noise_val = (x * y * epoch) % 100
                if noise_val < min(100, epoch * 2): 
                    intensity = int((noise_val / 100.0) * 150)
                    pixels[x, y] = (51, min(255, 65 + intensity), 0)
                else: pixels[x, y] = base_brown
    return img

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
                <title>TSFi Sovereign Choreography</title>
                <style>
                    body { background-color: #050505; color: #0f0; font-family: monospace; text-align: center; margin-top: 30px; }
                    .compositor { position: relative; width: 512px; height: 512px; margin: 0 auto; border: 2px solid #0f0; box-shadow: 0 0 30px #0f0; overflow: hidden; }
                    canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
                    #telemetry { margin-top: 20px; color: #ff0; font-size: 1.2em; font-weight: bold; }
                </style>
            </head>
            <body>
                <h1>TSFi The Sovereign Choreography (Lifecycle Matrix)</h1>
                <div class="compositor">
                    <canvas id="painterCanvas" width="512" height="512"></canvas>
                </div>
                <div id="telemetry">Initializing Neural Lifecycle...</div>
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
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/telemetry':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            try:
                with open("tools/vlm_score.txt", "r") as f: score = f.read()
            except:
                score = "Initializing Neural Weights..."
            self.wfile.write(score.encode('utf-8'))

        elif self.path == '/multiplex_stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            print("[INFO] Booting Synchronous VLM Blockade Matrix...")
            torch.set_num_threads(4)
            vlm = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
            processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
            
            dna_epoch = 0
            global anchor_bear
            anchor_bear = None
            hunting_base = None
            seed_offset = 0
            
            # The Multi-Genome Array
            genomes = [
                (0, "photorealistic overstuffed plush 16 inch tall brown teddy bear sitting straight, symmetrical, next to human furniture for scale"),
                (1, "photorealistic overstuffed plush 16 inch tall brown teddy bear slumped over to the right, asymmetric, tilted head, next to human furniture for scale"),
                (2, "photorealistic overstuffed plush 16 inch tall brown teddy bear standing up straight on two legs, next to human furniture for scale"),
                (3, "photorealistic fat round overstuffed plush 16 inch tall brown teddy bear, squished, curled up, massive head, tiny body, next to human furniture for scale")
            ]
            
            current_genome_id, current_genome_prompt = random.choice(genomes)
            
            import json, urllib.request, ctypes
            try:
                silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
                silhouette_lib.extract_silhouette.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int]
                silhouette_lib.generate_dynamic_mask.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int, ctypes.c_int]
            except:
                silhouette_lib = None
                
            # The Temporal Caching Matrix
            anchor_bear = None
            cached_epochs = []
            max_cached_epochs = 7
            
            # The Phase Tracker
            # 0: Hunting for perfect Epoch 2 anchor
            # 1: Synthesizing Epoch 3 through 7 (The Forward Decay)
            # 2: The Reverse Temporal Morph (The Live Broadcast)
            system_phase = 0 
            seed_offset = 0
            morph_progress = 0.0
            current_morph_step = 7
            
            while True:
                try:
                    if system_phase == 0:
                        # PHASE 0: The Absolute Anchor Hunt
                        with open("tools/vlm_score.txt", "w") as f: f.write(f"[HUNTING] Securing Flawless Anatomy | Pass: {seed_offset}")
                        
                        api_endpoint = "txt2img"
                        prompt = f"A perfectly proportioned, highly detailed photorealistic overstuffed plush 16 inch tall {current_bear_type} sitting on a floor, masterpiece, visible ears, visible eyes, visible snout, brightly lit studio photography"
                        neg_prompt = "muscular, human-like, abstract, deformed, blurry, cartoon, simple, bad anatomy, flat textures, disfigured, dark, shadow"
                        
                        data = {
                            "prompt": prompt, "negative_prompt": neg_prompt,
                            "width": 256, "height": 256, "steps": 20, "cfg_scale": 7.0, "seed": 42 + seed_offset
                        }
                        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/txt2img', data=json.dumps(data).encode('utf-8'))
                        req.add_header('Content-Type', 'application/json')
                        response = urllib.request.urlopen(req)
                        img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
                        
                        painter_img_small = Image.open(io.BytesIO(img_bytes)).convert("RGB")
                        
                        enc_image = vlm.encode_image(painter_img_small)
                        vlm_score = 0
                        if "yes" in vlm.answer_question(enc_image, "Does this image show a complete teddy bear with head, ears, eyes, snout? YES or NO.", processor).lower():
                            vlm_score += 50
                        if "yes" in vlm.answer_question(enc_image, "Is the bear physically textured like photorealistic fur? YES or NO.", processor).lower():
                            vlm_score += 50
                            
                        painter_large = painter_img_small.resize((512, 512), Image.NEAREST)
                        
                        if vlm_score == 100:
                            anchor_bear = painter_img_small
                            cached_epochs.append(painter_large) # Epoch 2 (Index 0 in cache)
                            system_phase = 1 # Move to Sickness Caching
                            with open("tools/vlm_score.txt", "w") as f: f.write("[100/100 PERFECT] Anchor Locked.")
                        else:
                            seed_offset += 1
                            
                        # Show hunting process
                        mpx_img = Image.new("RGB", (512, 1024))
                        mpx_img.paste(generate_dna_mask(0, current_genome_id).resize((512, 512), Image.NEAREST), (0, 0))
                        mpx_img.paste(painter_large, (0, 512))
                        
                    elif system_phase == 1:
                        # PHASE 1: AOT Generation of Epochs 3 through 7 (The Forward Decay)
                        # We use img2img on the anchor bear exactly ONCE per epoch to prevent deep-frying
                        epoch_idx = len(cached_epochs) + 2
                        sickness = min(100, int((epoch_idx / max_cached_epochs) * 100))
                        
                        with open("tools/vlm_score.txt", "w") as f: f.write(f"[SYNTHESIZING DECAY] Generating Epoch {epoch_idx}...")
                        
                        dna_img_small = generate_dna_mask(epoch_idx, current_genome_id, W=256, H=256)
                        mask_path = "assets/mpx_mask.png"
                        
                        # Apply math corruption directly over the PURE anchor bear (Never compound the noise)
                        math_corruption = Image.blend(anchor_bear, dna_img_small, alpha=0.15 + (sickness*0.003))
                        math_corruption.save(mask_path)
                        
                        payload_images = {"init_images": [base64.b64encode(open(mask_path, "rb").read()).decode('utf-8')]}
                        prompt = f"A photorealistic 16 inch tall {current_bear_type}, masterpiece, {sickness} percent glowing green sickness mutation <lora:lcm_lora_sd15:1.0>"
                        
                        data = {
                            "prompt": prompt,
                            "negative_prompt": "abstract, deformed, blurry, cartoon, simple, bad anatomy, flat textures, disfigured",
                            "width": 256, "height": 256, "steps": 4, "cfg_scale": 1.2, 
                            "seed": 42 + epoch_idx, "denoising_strength": 0.35,
                            **payload_images
                        }
                        
                        req = urllib.request.Request('http://127.0.0.1:8080/sdapi/v1/img2img', data=json.dumps(data).encode('utf-8'))
                        req.add_header('Content-Type', 'application/json')
                        response = urllib.request.urlopen(req)
                        img_bytes = base64.b64decode(json.loads(response.read())['images'][0])
                        
                        painter_img_small = Image.open(io.BytesIO(img_bytes)).convert("RGB")
                        painter_large = painter_img_small.resize((512, 512), Image.NEAREST)
                        cached_epochs.append(painter_large)
                        
                        if len(cached_epochs) >= max_cached_epochs:
                            system_phase = 2 # Trigger the Reverse Live Broadcast
                            current_morph_step = max_cached_epochs - 1 # Start at Epoch 7
                            morph_progress = 0.0
                            
                        # Broadcast caching state so the screen doesn't freeze
                        mpx_img = Image.new("RGB", (512, 1024))
                        mpx_img.paste(dna_img_small.resize((512, 512), Image.NEAREST), (0, 0))
                        mpx_img.paste(painter_large, (0, 512))
                        
                    elif system_phase == 2:
                        # PHASE 2: The Reverse Temporal Morph (Epoch 7 -> Epoch 6 -> ... -> Epoch 2)
                        # We linearly blend the photorealistic cached frames backwards to guarantee absolute convergence
                        with open("tools/vlm_score.txt", "w") as f: f.write(f"[REVERSE TEMPORAL MATRIX] Morphing Epoch {current_morph_step+2} -> {current_morph_step+1}")
                        
                        if current_morph_step > 0:
                            img_start = cached_epochs[current_morph_step] # E.g., Epoch 7
                            img_end = cached_epochs[current_morph_step - 1] # E.g., Epoch 6
                            
                            # Mathematically tween the images
                            painter_large = Image.blend(img_start, img_end, alpha=morph_progress)
                            
                            morph_progress += 0.05 # Speed of the blend
                            if morph_progress >= 1.0:
                                morph_progress = 0.0
                                current_morph_step -= 1
                        else:
                            # Reached Epoch 2! Reset the entire timeline to create a new Bear Entity!
                            system_phase = 0
                            anchor_bear = None
                            cached_epochs.clear()
                            seed_offset = 0
                            current_genome_id, current_genome_prompt = random.choice(genomes)
                            with open("tools/vlm_score.txt", "w") as f: f.write(f"[REBIRTH] Spawning Genome {current_genome_id}...")
                            time.sleep(2)
                            continue
                            
                        # Display the top DNA channel as a smooth reverse blend as well to match
                        dna_start = generate_dna_mask(current_morph_step+2, current_genome_id, W=256, H=256).resize((512, 512), Image.NEAREST)
                        dna_end = generate_dna_mask(current_morph_step+1, current_genome_id, W=256, H=256).resize((512, 512), Image.NEAREST)
                        dna_large = Image.blend(dna_start, dna_end, alpha=morph_progress)
                        
                        mpx_img = Image.new("RGB", (512, 1024))
                        mpx_img.paste(dna_large, (0, 0))
                        mpx_img.paste(painter_large, (0, 512))

                    # Broadcast universally
                    jpeg_io = io.BytesIO()
                    mpx_img.save(jpeg_io, format="JPEG", quality=80)
                    jpeg_bytes = jpeg_io.getvalue()
                    
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(jpeg_bytes)))
                    self.end_headers()
                    self.wfile.write(jpeg_bytes)
                    self.wfile.write(b'\r\n')
                    
                    if anchor_bear is not None:
                        dna_epoch += 1 # Advance the physical timeline
                        
                except (ConnectionResetError, BrokenPipeError):
                    print("[INFO] Client severed the HTTP connection.")
                    break
                except Exception as e:
                    print(f"[FRACTURE] Stream broke: {e}")
                    time.sleep(2)
        else:
            self.send_error(404)

if __name__ == '__main__':
    os.makedirs("assets", exist_ok=True)
    server = ThreadedHTTPServer(('0.0.0.0', 9091), MultiChannelHandler)
    print("=== TSFi Sovereign Choreography (Lifecycle Matrix) Active ===")
    print("-> Open a web browser to http://127.0.0.1:9091")
    server.serve_forever()
