import os
import sys
import time
import subprocess
import base64
import json
import mmap
import ctypes
import io
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image

W, H = 512, 512

TSFI_CN_SHM_DEPTH = "tsfi_cn_depth"
TSFI_CN_SHM_POSE = "tsfi_cn_pose"
TSFI_CN_SHM_DGUI = "tsfi_cn_dgui"
MAP_SIZE = W * H * 3
HEADER_SIZE = 32
DGUI_SIZE = 32

def get_shm(name, size):
    try:
        fd = os.open(f"/dev/shm/{name}", os.O_RDWR)
        return mmap.mmap(fd, size)
    except FileNotFoundError:
        fd = os.open(f"/dev/shm/{name}", os.O_RDWR | os.O_CREAT)
        os.ftruncate(fd, size)
        m = mmap.mmap(fd, size)
        if name != TSFI_CN_SHM_DGUI:
            m[0:4] = (0x54434E4D).to_bytes(4, 'little')
            m[8:12] = (W).to_bytes(4, 'little')
            m[12:16] = (H).to_bytes(4, 'little')
            m[16:20] = (3).to_bytes(4, 'little')
        else:
            m[0:4] = (0x44475549).to_bytes(4, 'little')
        return m

def get_dgui_shm():
    try:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR)
        return mmap.mmap(fd, DGUI_SIZE)
    except FileNotFoundError:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR | os.O_CREAT)
        os.ftruncate(fd, DGUI_SIZE)
        m = mmap.mmap(fd, DGUI_SIZE)
        m[0:4] = (0x44475549).to_bytes(4, 'little')
        return m

def update_guidance(m, depth=0.8, pose=0.6, cfg=7.5, steps=4):
    m[4:8] = bytearray(ctypes.c_float(depth))
    m[8:12] = bytearray(ctypes.c_float(pose))
    m[12:16] = bytearray(ctypes.c_float(cfg))
    m[16:20] = (int(steps)).to_bytes(4, 'little')

class IconEditorHandler(BaseHTTPRequestHandler):
    last_error = "No errors recorded."

    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            
            html = """
            <!DOCTYPE html>
            <html>
            <head>
                <title>TSFi Stylized Icon Editor</title>
                <style>
                    body { background-color: #111; color: #eee; font-family: monospace; text-align: center; }
                    .container { display: flex; justify-content: center; gap: 20px; margin-top: 20px; }
                    .canvas-container { position: relative; width: 512px; height: 512px; border: 2px solid #444; }
                    canvas { position: absolute; top: 0; left: 0; }
                    #drawCanvas { z-index: 1; cursor: crosshair; }
                    #poseCanvas { z-index: 2; pointer-events: none; }
                    #interactionCanvas { z-index: 3; cursor: grab; }
                    .result-box { width: 512px; height: 512px; border: 2px solid #444; background: #222; display: flex; align-items: center; justify-content: center; }
                    .result-box img { max-width: 100%; max-height: 100%; }
                    button, input, select { background: #333; color: #fff; border: 1px solid #555; padding: 10px; font-family: monospace; margin: 5px; }
                    button { cursor: pointer; }
                    button:hover { background: #555; }
                    .controls { margin-top: 20px; padding: 15px; background: #222; border-radius: 8px; display: inline-block; }
                    input[type="text"] { width: 350px; }
                    .slider-container { display: flex; justify-content: space-around; gap: 10px; margin-top: 10px; font-size: 12px; }
                </style>
            </head>
            <body>
                <h2>TSFi Stylized Line Art Editor (Dream Spider Edition)</h2>
                
                <div class="controls">
                    <div>
                        <input type="text" id="promptInput" placeholder="Describe the icon (e.g., 'a cute stuffed animal crow')" value="a stuffed animal crow">
                        <select id="styleSelect">
                            <option value="minimalist">Minimalist Line Art</option>
                            <option value="auncient">Auncient Retro Cybernetic</option>
                            <option value="sigil">Hypotrochoid Vector Sigil</option>
                            <option value="steampunk">Steampunk Mechanical</option>
                        </select>
                        <button onclick="generateIcon()">Generate Icon</button>
                        <button onclick="clearCanvas()">Clear Scribble</button>
                        <button onclick="togglePoseMode()" id="modeBtn">Mode: Scribble</button>
                        <button style="background: #a00; color: white;" onclick="terminateTest()">FAIL</button>
                    </div>
                    <div class="slider-container">
                        <div>
                            <label>Steps: <span id="stepsVal">20</span></label><br>
                            <input type="range" id="stepsSlider" min="5" max="40" value="20" oninput="document.getElementById('stepsVal').innerText=this.value">
                        </div>
                        <div>
                            <label>CFG Scale: <span id="cfgVal">7.5</span></label><br>
                            <input type="range" id="cfgSlider" min="1.0" max="15.0" step="0.5" value="7.5" oninput="document.getElementById('cfgVal').innerText=this.value">
                        </div>
                        <div>
                            <label>Depth Weight: <span id="depthVal">0.85</span></label><br>
                            <input type="range" id="depthSlider" min="0.0" max="1.0" step="0.05" value="0.85" oninput="document.getElementById('depthVal').innerText=this.value">
                        </div>
                    </div>
                </div>

                <div class="container">
                    <div class="canvas-container">
                        <!-- Layer 0: Background for Scribble (Black) -->
                        <canvas id="bgCanvas" width="512" height="512"></canvas>
                        <!-- Layer 1: The Scribble Pad -->
                        <canvas id="drawCanvas" width="512" height="512"></canvas>
                        <!-- Layer 2: The Rendered OpenPose Skeleton -->
                        <canvas id="poseCanvas" width="512" height="512"></canvas>
                        <!-- Layer 3: Invisible hit-detection for joints -->
                        <canvas id="interactionCanvas" width="512" height="512"></canvas>
                    </div>
                    <div>
                        <div class="result-box" id="resultBox">
                            <span>Waiting for generation...</span>
                        </div>
                    </div>
                </div>

                <script>
                    let mode = 'scribble'; // 'scribble' or 'pose'
                    
                    const bgCanvas = document.getElementById('bgCanvas');
                    const bgCtx = bgCanvas.getContext('2d');
                    bgCtx.fillStyle = 'black';
                    bgCtx.fillRect(0, 0, 512, 512);

                    const drawCanvas = document.getElementById('drawCanvas');
                    const drawCtx = drawCanvas.getContext('2d');
                    drawCtx.lineCap = 'round';
                    drawCtx.lineJoin = 'round';
                    drawCtx.lineWidth = 8;
                    drawCtx.strokeStyle = 'white';
                    
                    const poseCanvas = document.getElementById('poseCanvas');
                    const poseCtx = poseCanvas.getContext('2d');
                    
                    const intCanvas = document.getElementById('interactionCanvas');
                    
                    // --- Scribble Logic ---
                    let isDrawing = false;
                    let lastX = 0, lastY = 0;
                    
                    intCanvas.addEventListener('mousedown', e => {
                        const rect = intCanvas.getBoundingClientRect();
                        const x = e.clientX - rect.left; 
                        const y = e.clientY - rect.top;
                        
                        if (mode === 'scribble') {
                            isDrawing = true;
                            lastX = x; lastY = y;
                        } else if (mode === 'pose') {
                            // Find nearest joint
                            let minDist = 20; // 20px grab radius
                            let hit = -1;
                            for (let i = 0; i < joints.length; i++) {
                                const dx = joints[i].x - x;
                                const dy = joints[i].y - y;
                                const dist = Math.sqrt(dx*dx + dy*dy);
                                if (dist < minDist) { minDist = dist; hit = i; }
                            }
                            if (hit !== -1) { draggingJoint = hit; }
                        }
                    });
                    
                    intCanvas.addEventListener('mousemove', e => {
                        const rect = intCanvas.getBoundingClientRect();
                        const x = e.clientX - rect.left; 
                        const y = e.clientY - rect.top;
                        
                        if (mode === 'scribble' && isDrawing) {
                            drawCtx.beginPath();
                            drawCtx.moveTo(lastX, lastY);
                            drawCtx.lineTo(x, y);
                            drawCtx.stroke();
                            lastX = x; lastY = y;
                        } else if (mode === 'pose' && draggingJoint !== -1) {
                            joints[draggingJoint].x = x;
                            joints[draggingJoint].y = y;
                            renderPose();
                        }
                    });
                    
                    window.addEventListener('mouseup', () => { isDrawing = false; draggingJoint = -1; });
                    
                    function clearCanvas() {
                        drawCtx.clearRect(0, 0, 512, 512);
                    }

                    function togglePoseMode() {
                        mode = (mode === 'scribble') ? 'pose' : 'scribble';
                        document.getElementById('modeBtn').innerText = 'Mode: ' + (mode === 'scribble' ? 'Scribble' : 'Pose');
                        intCanvas.style.cursor = (mode === 'scribble') ? 'crosshair' : 'grab';
                        // Dim scribble when posing
                        drawCanvas.style.opacity = (mode === 'pose') ? '0.3' : '1.0';
                    }

                    // --- Dream Spider Pose Logic ---
                    const joints = [
                        {x: 256, y: 150}, // 0: Nose
                        {x: 256, y: 200}, // 1: Neck
                        {x: 256, y: 300}, // 2: MidHip
                        {x: 150, y: 150}, // 3: L1
                        {x: 100, y: 200}, // 4: L2
                        {x: 120, y: 300}, // 5: L3
                        {x: 180, y: 380}, // 6: L4
                        {x: 362, y: 150}, // 7: R1
                        {x: 412, y: 200}, // 8: R2
                        {x: 392, y: 300}, // 9: R3
                        {x: 332, y: 380}  // 10: R4
                    ];
                    let draggingJoint = -1;

                    function renderPose() {
                        poseCtx.clearRect(0, 0, 512, 512);
                        
                        function dl(ctx, j1, j2, r, g, b) {
                            ctx.beginPath();
                            ctx.moveTo(joints[j1].x, joints[j1].y);
                            ctx.lineTo(joints[j2].x, joints[j2].y);
                            ctx.strokeStyle = `rgb(${r},${g},${b})`;
                            ctx.lineWidth = 14;
                            ctx.stroke();
                        }
                        
                        dl(poseCtx, 0, 1, 255, 0, 0); // Nose to Neck
                        dl(poseCtx, 1, 2, 0, 255, 170); // Neck to MidHip
                        
                        dl(poseCtx, 1, 3, 85, 255, 0);
                        dl(poseCtx, 1, 4, 0, 255, 0);
                        dl(poseCtx, 2, 5, 0, 0, 255);
                        dl(poseCtx, 2, 6, 85, 0, 255);
                        
                        dl(poseCtx, 1, 7, 255, 170, 0);
                        dl(poseCtx, 1, 8, 255, 255, 0);
                        dl(poseCtx, 2, 9, 255, 0, 0);
                        dl(poseCtx, 2, 10, 255, 85, 0);

                        for (let i = 0; i < joints.length; i++) {
                            poseCtx.beginPath();
                            poseCtx.arc(joints[i].x, joints[i].y, 6, 0, 2 * Math.PI);
                            poseCtx.fillStyle = 'white';
                            poseCtx.fill();
                        }
                    }
                    renderPose(); // Initial draw
                    
                    // --- Orchestration ---
                    async function generateIcon() {
                        const tempCanvas = document.createElement('canvas');
                        tempCanvas.width = 512; tempCanvas.height = 512;
                        const tCtx = tempCanvas.getContext('2d');
                        
                        tCtx.fillStyle = 'black';
                        tCtx.fillRect(0, 0, 512, 512);
                        tCtx.drawImage(drawCanvas, 0, 0);
                        
                        function dlT(j1, j2) {
                            tCtx.beginPath();
                            tCtx.moveTo(joints[j1].x, joints[j1].y);
                            tCtx.lineTo(joints[j2].x, joints[j2].y);
                            tCtx.strokeStyle = 'white';
                            tCtx.lineWidth = 14;
                            tCtx.stroke();
                        }
                        
                        if (mode === 'pose' || draggingJoint !== -1 || joints[0].x !== 256) {
                            dlT(0, 1); dlT(1, 2);
                            dlT(1, 3); dlT(1, 4); dlT(2, 5); dlT(2, 6);
                            dlT(1, 7); dlT(1, 8); dlT(2, 9); dlT(2, 10);
                            
                            for (let i = 0; i < joints.length; i++) {
                                tCtx.beginPath();
                                tCtx.arc(joints[i].x, joints[i].y, 6, 0, 2 * Math.PI);
                                tCtx.fillStyle = 'white';
                                tCtx.fill();
                            }
                        }
                        
                        const depthDataURL = tempCanvas.toDataURL('image/png');
                        const userPrompt = document.getElementById('promptInput').value;
                        const selectedStyle = document.getElementById('styleSelect').value;
                        const steps = document.getElementById('stepsSlider').value;
                        const cfg = document.getElementById('cfgSlider').value;
                        const depthWeight = document.getElementById('depthSlider').value;
                        
                        const resultBox = document.getElementById('resultBox');
                        resultBox.innerHTML = '<span>Synthesizing neural geometry...</span>';
                        
                        const response = await fetch('/generate', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ 
                                depth: depthDataURL, 
                                prompt: userPrompt,
                                style: selectedStyle,
                                steps: steps,
                                cfg: cfg,
                                depth_weight: depthWeight
                            })
                        });
                        
                        const result = await response.json();
                        if (result.success) {
                            resultBox.innerHTML = '<img src="data:image/jpeg;base64,' + result.image + '">';
                        } else {
                            resultBox.innerHTML = '<span style="color:red">Generation Failed</span>';
                        }
                    }

                    async function terminateTest() {
                        const response = await fetch('/fail', { method: 'POST' });
                        const result = await response.json();
                        document.body.innerHTML = '<h2>Test Terminated.</h2><pre style="text-align: left; background: #000; padding: 20px;">' + result.errors + '</pre>';
                    }
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
        else:
            self.send_error(404)

    def do_POST(self):
        if self.path == '/fail':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'errors': IconEditorHandler.last_error}).encode('utf-8'))
            print("[SYSTEM] FAIL triggered. Terminating test...")
            def kill_server():
                time.sleep(1)
                os._exit(1)
            import threading
            threading.Thread(target=kill_server).start()
            return
            
        if self.path == '/generate':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            req = json.loads(post_data.decode('utf-8'))
            
            # --- Depth Scribble Setup ---
            data_url = req['depth']
            user_prompt = req.get('prompt', 'a stuffed animal crow')
            style = req.get('style', 'minimalist')
            steps = int(req.get('steps', 20))
            cfg = float(req.get('cfg', 7.5))
            depth_weight = float(req.get('depth_weight', 0.85))
            
            header, encoded = data_url.split(",", 1)
            img_data = base64.b64decode(encoded)
            img = Image.open(io.BytesIO(img_data)).convert('RGB').resize((W, H))
            raw_depth = img.tobytes()
            
            shm_depth = get_shm(TSFI_CN_SHM_DEPTH, HEADER_SIZE + MAP_SIZE)
            shm_depth[0:4] = (0x54434E4D).to_bytes(4, 'little')
            shm_depth[4:8] = (1).to_bytes(4, 'little')
            shm_depth[8:12] = (W).to_bytes(4, 'little')
            shm_depth[12:16] = (H).to_bytes(4, 'little')
            shm_depth[16:20] = (3).to_bytes(4, 'little')
            shm_depth.seek(HEADER_SIZE)
            shm_depth.write(raw_depth)
            
            # --- DGUI Setup ---
            dgui = get_dgui_shm()
            update_guidance(dgui, depth=depth_weight, pose=0.0, cfg=cfg, steps=steps)
            
            # Select style positive prompt
            if style == 'minimalist':
                base_prompt = "clean minimalist line art icon, elegant ink drawing, bold black outlines on pure white background, vector graphics style, sharp edges, no shading, high contrast, perfect logo design"
            elif style == 'auncient':
                base_prompt = "Auncient retro cybernetic hardware style, glowing neon wireframe envelope, Lissajous vector projections, computer terminal glow, dark background, premium HSL tailored palette, highly detailed"
            elif style == 'sigil':
                base_prompt = "glowing cyber-fantasy magical sigil, sacred geometry, complex hypotrochoid curves, glowing vector orbits on pure dark background, high contrast, radiant vector lines"
            else: # steampunk
                base_prompt = "steampunk machinery icon, complex brass gears, mechanical clockwork loops, polished copper pipes, high details, dark slate background"
                
            prompt = f"{user_prompt}, {base_prompt}"
            out_raw = "tmp/icon_out.raw"
            
            try: os.unlink("/dev/shm/tsfi_cn_pose")
            except: pass
            try: os.unlink("/dev/shm/tsfi_cn_init")
            except: pass

            cmd = [
                "bin/tsfi_sd_worker",
                prompt,
                out_raw,
                "1", # use_shm
                "sd15",
                str(steps),
                "euler_a",
                str(cfg)
            ]
            
            env = os.environ.copy()
            env["GGML_VK_FORCE_MAX_BUFFER_SIZE"] = "1073741824"
            
            print("[EDITOR] Synthesizing stylized icon...")
            res = subprocess.run(cmd, capture_output=True, env=env)
            
            if res.returncode == 0 and os.path.exists(out_raw):
                with open(out_raw, "rb") as f:
                    out_data = f.read()
                
                expected_sz = W * H * 3
                if len(out_data) >= expected_sz:
                    res_img = Image.frombytes("RGB", (W, H), out_data[:expected_sz])
                    
                    buf = io.BytesIO()
                    res_img.save(buf, format='JPEG', quality=90)
                    b64_res = base64.b64encode(buf.getvalue()).decode('utf-8')
                    
                    self.send_response(200)
                    self.send_header('Content-type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps({'success': True, 'image': b64_res}).encode('utf-8'))
                    return
            
            err_msg = res.stderr.decode('utf-8')
            IconEditorHandler.last_error = f"Worker Exit Code: {res.returncode}\n{err_msg}"
            print(f"[ERROR] Generation failed. SD Worker Output: {err_msg}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(json.dumps({'success': False}).encode('utf-8'))

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True

if __name__ == '__main__':
    os.makedirs("tmp", exist_ok=True)
    server = ThreadedHTTPServer(('0.0.0.0', 9094), IconEditorHandler)
    print("=== TSFi Stylized Line Art Icon Editor ===")
    print("-> Open a web browser to http://127.0.0.1:9094")
    server.serve_forever()