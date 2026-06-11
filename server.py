import os
import sys
import json
import subprocess
import mmap
import ctypes
import base64
import io
from http.server import SimpleHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image

TSFI_CN_SHM_DEPTH = "tsfi_cn_depth"
TSFI_CN_SHM_POSE = "tsfi_cn_pose"
TSFI_CN_SHM_DGUI = "tsfi_cn_dgui"
W, H = 512, 512
HEADER_SIZE = 32
MAP_SIZE = W * H * 3
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

def update_guidance(m, depth=0.8, pose=0.0, cfg=7.5, steps=4):
    m[4:8] = bytearray(ctypes.c_float(depth))
    m[8:12] = bytearray(ctypes.c_float(pose))
    m[12:16] = bytearray(ctypes.c_float(cfg))
    m[16:20] = (int(steps)).to_bytes(4, 'little')

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    pass

class CustomHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(204)
        self.end_headers()

    def translate_path(self, path):
        import urllib.parse
        # Decode URL-encoded characters
        path = urllib.parse.unquote(path)
        # Split off query string / fragment
        path = path.split('?', 1)[0].split('#', 1)[0]
        
        # Strip leading slash for relative joining
        rel_path = path.lstrip('/')
        if rel_path.startswith('frontend/'):
            rel_path = rel_path[len('frontend/'):]
            
        # Candidates to check:
        # 1. ./frontend/rel_path
        # 2. ./rel_path
        # and for each, if it doesn't have an extension or does not exist, try with .html extension
        
        cwd = os.getcwd()
        
        # Determine if we should try appending .html
        base, ext = os.path.splitext(rel_path)
        
        paths_to_try = [rel_path]
        if not ext or ext.lower() not in ['.html', '.js', '.css', '.png', '.jpg', '.jpeg', '.json', '.svg', '.gif', '.wav']:
            paths_to_try.append(rel_path + '.html')
            
        for p in paths_to_try:
            # check in frontend directory
            cand_frontend = os.path.join(cwd, 'frontend', p)
            if os.path.isfile(cand_frontend):
                return cand_frontend
            # check in root directory
            cand_root = os.path.join(cwd, p)
            if os.path.isfile(cand_root):
                return cand_root
                
        # If nothing matched, fallback to standard behavior of SimpleHTTPRequestHandler
        return super().translate_path(path)

    def do_POST(self):
        if self.path == '/generate-bear':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            params = json.loads(post_data.decode('utf-8'))
            
            prompt = params.get('prompt', 'A photorealistic brown teddy bear')
            profile = params.get('profile', 'turbo') # 'turbo' or 'sd15'
            steps = int(params.get('steps', 4))
            cfg = float(params.get('cfg', 1.5))
            controlnet_image = params.get('controlnet_image', None)
            controlnet_pose = params.get('controlnet_pose', None)
            use_controlnet = params.get('use_controlnet', False)
            
            # Create a tmp directory if not exist
            os.makedirs('tmp', exist_ok=True)
            
            raw_out = 'tmp/test_bear.raw'
            png_out = 'tmp/test_bear.png'
            
            raw_out_path = 'tsfi2-deepseek/' + raw_out
            # Clean up old files
            if os.path.exists(raw_out_path): os.remove(raw_out_path)
            if os.path.exists(png_out): os.remove(png_out)
            
            use_shm_arg = "0"
            if use_controlnet and controlnet_image:
                try:
                    header, encoded = controlnet_image.split(",", 1)
                    img_data = base64.b64decode(encoded)
                    img = Image.open(io.BytesIO(img_data)).convert('RGB').resize((W, H))
                    raw_depth = img.tobytes()
                    
                    shm_depth = get_shm(TSFI_CN_SHM_DEPTH, HEADER_SIZE + MAP_SIZE)
                    shm_depth.seek(HEADER_SIZE)
                    shm_depth.write(raw_depth)
                    
                    pose_strength = 0.0
                    if controlnet_pose:
                        try:
                            p_header, p_encoded = controlnet_pose.split(",", 1)
                            p_img_data = base64.b64decode(p_encoded)
                            p_img = Image.open(io.BytesIO(p_img_data)).convert('RGB').resize((W, H))
                            raw_pose = p_img.tobytes()
                            
                            shm_pose = get_shm(TSFI_CN_SHM_POSE, HEADER_SIZE + MAP_SIZE)
                            shm_pose.seek(HEADER_SIZE)
                            shm_pose.write(raw_pose)
                            pose_strength = 0.75
                            print("[Server] ControlNet OpenPose SHM initialized.")
                        except Exception as p_ex:
                            print(f"[Server] Failed to setup ControlNet OpenPose SHM: {p_ex}")
                    
                    dgui = get_shm(TSFI_CN_SHM_DGUI, DGUI_SIZE)
                    update_guidance(dgui, depth=0.85, pose=pose_strength, cfg=cfg, steps=steps)
                    
                    use_shm_arg = "1"
                    print("[Server] ControlNet SHM initialized and enabled.")
                except Exception as ex:
                    print(f"[Server] Failed to setup ControlNet SHM: {ex}")
            
            # Execute compiled bin/tsfi_sd_worker inside the tsfi2-deepseek directory
            worker_path = './bin/tsfi_sd_worker'
            
            cmd = [
                worker_path,
                prompt,
                raw_out,  # Relative to cwd (tsfi2-deepseek/tmp/test_bear.raw)
                use_shm_arg,  # "1" if ControlNet SHM is active, "0" otherwise
                profile,
                str(steps),
                "euler_a",
                str(cfg)
            ]
            
            print(f"[Server] Executing command: {' '.join(cmd)}")
            try:
                res = subprocess.run(cmd, capture_output=True, text=True, timeout=60, cwd='tsfi2-deepseek')
                print(f"[Server] STDOUT: {res.stdout}")
                print(f"[Server] STDERR: {res.stderr}")
                
                raw_out_path = 'tsfi2-deepseek/' + raw_out
                if os.path.exists(raw_out_path):
                    with open(raw_out_path, 'rb') as f:
                        raw_data = f.read()
                    
                    # Detect resolution based on output size (512x512 vs 1280x720)
                    width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
                    height = 512 if len(raw_data) == 512 * 512 * 3 else 720
                    
                    img = Image.frombytes('RGB', (width, height), raw_data)
                    img.save(png_out)
                    
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps({'status': 'success', 'url': 'tmp/test_bear.png'}).encode('utf-8'))
                    return
                else:
                    self.send_response(500)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps({'status': 'error', 'message': 'Output raw file not generated. ' + res.stderr}).encode('utf-8'))
                    return
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'status': 'error', 'message': str(e)}).encode('utf-8'))
                return
        else:
            super().do_POST()

if __name__ == '__main__':
    server = ThreadedHTTPServer(('127.0.0.1', 8000), CustomHandler)
    print("[Server] Custom Dynamic Server active on http://127.0.0.1:8000")
    server.serve_forever()
