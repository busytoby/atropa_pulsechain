import os
import sys
import json
import subprocess
import mmap
import ctypes
import base64
import io
import math
import numpy as np
from http.server import SimpleHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image

def draw_bone(pose, p1, p2, color, thickness, W=512, H=512):
    x1, y1 = p1
    x2, y2 = p2
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx*dx + dy*dy)
    if dist == 0: return
    
    for t in np.linspace(0, 1, max(2, int(dist * 2))):
        cx = int(x1 + dx * t)
        cy = int(y1 + dy * t)
        for dy_off in range(-thickness, thickness + 1):
            for dx_off in range(-thickness, thickness + 1):
                if dx_off*dx_off + dy_off*dy_off <= thickness*thickness:
                    tx, ty = cx + dx_off, cy + dy_off
                    if 0 <= tx < W and 0 <= ty < H:
                        idx = (ty * W + tx) * 3
                        pose[idx:idx+3] = color

def draw_sphere_depth(d_map, center, radius, z_val, W=512, H=512):
    cx, cy = center
    r2 = radius**2
    for y in range(int(cy - radius), int(cy + radius)):
        if not (0 <= y < H): continue
        for x in range(int(cx - radius), int(cx + radius)):
            if not (0 <= x < W): continue
            dist2 = (x - cx)**2 + (y - cy)**2
            if dist2 <= r2:
                idx = (y * W + x) * 3
                val = min(255, max(0, int(z_val + math.sqrt(r2 - dist2))))
                if val > d_map[idx]:
                    d_map[idx:idx+3] = [val, val, val]

def get_motion_joints(motion, frame_idx, num_frames):
    cx, cy = 256, 256
    s = 2.5
    phase = 2 * math.pi * frame_idx / num_frames
    
    # Base joints
    joints = {
        "neck_pos": [cx, cy - 20*s],
        "head_top": [cx, cy - 35*s],
        "body_mid": [cx, cy + 10*s],
        "beak_base": [cx + 10*s, cy - 25*s],
        "beak_tip": [cx + 25*s, cy - 25*s],
        "l_shoulder": [cx - 15*s, cy - 5*s],
        "r_shoulder": [cx + 15*s, cy - 5*s],
        "l_wing_tip": [cx - 60*s, cy - 5*s],
        "r_wing_tip": [cx + 60*s, cy - 5*s],
        "l_hip": [cx - 10*s, cy + 25*s],
        "r_hip": [cx + 10*s, cy + 25*s],
        "l_foot": [cx - 15*s, cy + 60*s],
        "r_foot": [cx + 15*s, cy + 60*s],
        "tail_base": [cx - 15*s, cy + 30*s],
        "tail_tip": [cx - 40*s, cy + 45*s]
    }
    
    if motion == "walk":
        # Body bobs up and down twice per cycle
        bob = abs(math.sin(phase * 2)) * 4 * s
        for k in joints:
            joints[k][1] += bob
            
        # Legs move back and forth
        stride = 12 * s
        lift = 10 * s
        
        joints["l_foot"][0] += math.sin(phase) * stride
        joints["l_foot"][1] -= abs(math.cos(phase)) * lift
        
        joints["r_foot"][0] += math.sin(phase + math.pi) * stride
        joints["r_foot"][1] -= abs(math.cos(phase + math.pi)) * lift
        
        # Wings swing gently
        joints["l_wing_tip"][1] += math.cos(phase) * 5 * s
        joints["r_wing_tip"][1] -= math.cos(phase) * 5 * s
        
    elif motion == "jump":
        # Jump cycle: crouch -> launch -> apex -> land
        if phase < math.pi / 2: # Crouch
            t = phase / (math.pi / 2)
            crouch = t * 15 * s
            joints["neck_pos"][1] += crouch
            joints["head_top"][1] += crouch
            joints["body_mid"][1] += crouch * 0.7
            joints["l_wing_tip"][0] -= crouch * 0.5
            joints["r_wing_tip"][0] += crouch * 0.5
        elif phase < math.pi: # Launch
            t = (phase - math.pi / 2) / (math.pi / 2)
            crouch = (1 - t) * 15 * s
            jump_up = t * 30 * s
            joints["neck_pos"][1] += crouch - jump_up
            joints["head_top"][1] += crouch - jump_up
            joints["body_mid"][1] += crouch * 0.7 - jump_up
            joints["l_foot"][1] += jump_up * 0.2
            joints["r_foot"][1] += jump_up * 0.2
        elif phase < 3 * math.pi / 2: # Apex
            t = (phase - math.pi) / (math.pi / 2)
            jump_up = (1 - t) * 30 * s + t * 20 * s
            joints["neck_pos"][1] -= jump_up
            joints["head_top"][1] -= jump_up
            joints["body_mid"][1] -= jump_up
            joints["l_foot"][1] -= 10 * s
            joints["r_foot"][1] -= 10 * s
        else: # Land
            t = (phase - 3 * math.pi / 2) / (math.pi / 2)
            jump_up = (1 - t) * 20 * s
            joints["neck_pos"][1] -= jump_up
            joints["head_top"][1] -= jump_up
            joints["body_mid"][1] -= jump_up
            
    elif motion == "wave":
        wave_y = math.sin(phase * 3) * 15 * s
        wave_x = math.cos(phase * 3) * 10 * s
        joints["r_wing_tip"] = [cx + 40*s + wave_x, cy - 30*s + wave_y]
        joints["l_wing_tip"] = [cx - 40*s, cy + 20*s]
        
    elif motion == "ballet":
        p_idx = int((phase / (2 * math.pi)) * 4) % 4
        t = (phase / (2 * math.pi) * 4) % 1.0
        t_smooth = t * t * (3 - 2 * t)
        
        poses = ["First Position", "Plier", "Relever", "Reverence"]
        p1 = poses[p_idx]
        p2 = poses[(p_idx + 1) % 4]
        
        def get_ballet_posture(name):
            j = {
                "neck_pos": [cx, cy - 20*s],
                "head_top": [cx, cy - 35*s],
                "body_mid": [cx, cy + 10*s],
                "beak_base": [cx + 10*s, cy - 25*s],
                "beak_tip": [cx + 25*s, cy - 25*s],
                "l_shoulder": [cx - 10*s, cy - 5*s],
                "r_shoulder": [cx + 10*s, cy - 5*s],
                "l_wing_tip": [cx - 60*s, cy - 5*s],
                "r_wing_tip": [cx + 60*s, cy - 5*s],
                "l_hip": [cx - 8*s, cy + 25*s],
                "r_hip": [cx + 8*s, cy + 25*s],
                "l_foot": [cx - 12*s, cy + 60*s],
                "r_foot": [cx + 12*s, cy + 60*s],
                "tail_base": [cx - 15*s, cy + 30*s],
                "tail_tip": [cx - 40*s, cy + 45*s]
            }
            if name == "First Position":
                j["l_foot"] = [cx - 5*s, cy + 60*s]; j["r_foot"] = [cx + 5*s, cy + 60*s]
                j["l_wing_tip"] = [cx - 15*s, cy + 20*s]; j["r_wing_tip"] = [cx + 15*s, cy + 20*s]
            elif name == "Plier":
                base_cy = cy + 10*s
                j["neck_pos"] = [cx, base_cy - 20*s]; j["head_top"] = [cx, base_cy - 35*s]; j["body_mid"] = [cx, base_cy + 10*s]
                j["l_foot"] = [cx - 20*s, base_cy + 40*s]; j["r_foot"] = [cx + 20*s, base_cy + 40*s]
                j["l_wing_tip"] = [cx - 10*s, base_cy + 5*s]; j["r_wing_tip"] = [cx + 10*s, base_cy + 5*s]
            elif name == "Relever":
                j["l_foot"] = [cx - 8*s, cy + 70*s]; j["r_foot"] = [cx + 8*s, cy + 70*s]
                j["l_wing_tip"] = [cx - 40*s, cy + 0*s]; j["r_wing_tip"] = [cx + 40*s, cy + 0*s]
            elif name == "Reverence":
                j["head_top"] = [cx + 10*s, cy + 5*s]; j["neck_pos"] = [cx + 5*s, cy - 5*s]
                j["beak_base"] = [cx + 15*s, cy + 10*s]; j["beak_tip"] = [cx + 25*s, cy + 15*s]
                j["l_wing_tip"] = [cx - 30*s, cy - 15*s]; j["r_wing_tip"] = [cx + 5*s, cy + 20*s]
            return j

        j1 = get_ballet_posture(p1)
        j2 = get_ballet_posture(p2)
        for k in joints:
            joints[k] = [
                j1[k][0] + (j2[k][0] - j1[k][0]) * t_smooth,
                j1[k][1] + (j2[k][1] - j1[k][1]) * t_smooth
            ]
                
    return joints

def make_skeleton_maps(joints, W=512, H=512):
    pose = bytearray(W * H * 3)
    depth = bytearray(W * H * 3)
    s = 2.5
    
    draw_bone(pose, joints["head_top"], joints["neck_pos"], [255, 0, 0], 3, W, H)
    draw_bone(pose, joints["head_top"], joints["beak_base"], [255, 0, 0], 2, W, H)
    draw_bone(pose, joints["beak_base"], joints["beak_tip"], [255, 0, 255], 4, W, H)
    draw_bone(pose, joints["neck_pos"], joints["body_mid"], [255, 85, 0], 4, W, H)
    draw_bone(pose, joints["body_mid"], joints["tail_base"], [255, 170, 0], 3, W, H)
    draw_bone(pose, joints["tail_base"], joints["tail_tip"], [255, 255, 0], 3, W, H)
    draw_bone(pose, joints["neck_pos"], joints["l_shoulder"], [170, 255, 0], 2, W, H)
    draw_bone(pose, joints["neck_pos"], joints["r_shoulder"], [85, 255, 0], 2, W, H)
    draw_bone(pose, joints["l_shoulder"], joints["l_wing_tip"], [0, 255, 0], 3, W, H)
    draw_bone(pose, joints["r_shoulder"], joints["r_wing_tip"], [0, 255, 85], 3, W, H)
    draw_bone(pose, joints["body_mid"], joints["l_hip"], [0, 255, 170], 2, W, H)
    draw_bone(pose, joints["body_mid"], joints["r_hip"], [0, 255, 255], 2, W, H)
    draw_bone(pose, joints["l_hip"], joints["l_foot"], [0, 170, 255], 2, W, H)
    draw_bone(pose, joints["r_hip"], joints["r_foot"], [0, 85, 255], 2, W, H)
    
    draw_sphere_depth(depth, joints["head_top"], 18*s, 180, W, H)
    draw_sphere_depth(depth, joints["neck_pos"], 15*s, 160, W, H)
    draw_sphere_depth(depth, joints["body_mid"], 30*s, 140, W, H)
    draw_sphere_depth(depth, joints["tail_base"], 20*s, 130, W, H)
    
    return depth, pose

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

import threading

sd_lock = threading.Lock()

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

    def do_GET(self):
        if self.path == '/api/config':
            config_path = os.path.join(os.getcwd(), 'config/user_config.json')
            if os.path.exists(config_path):
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                with open(config_path, 'r') as f:
                    self.wfile.write(f.read().encode('utf-8'))
            else:
                self.send_response(404)
                self.end_headers()
            return
        super().do_GET()

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
            
            with sd_lock:
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
        elif self.path == '/generate-animation':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            params = json.loads(post_data.decode('utf-8'))
            
            with sd_lock:
                prompt = params.get('prompt', 'cute stuffed dragon plush toy')
                motion = params.get('motion', 'walk')
                profile = params.get('profile', 'turbo')
                steps = int(params.get('steps', 4))
                cfg = float(params.get('cfg', 1.5))
                num_frames = int(params.get('num_frames', 8))
                
                os.makedirs('tmp', exist_ok=True)
                os.makedirs('tsfi2-deepseek/tmp', exist_ok=True)
                
                shm_depth = get_shm(TSFI_CN_SHM_DEPTH, HEADER_SIZE + MAP_SIZE)
                shm_pose = get_shm(TSFI_CN_SHM_POSE, HEADER_SIZE + MAP_SIZE)
                dgui = get_shm(TSFI_CN_SHM_DGUI, DGUI_SIZE)
                
                # set guidance: depth=0.85, pose=0.75
                update_guidance(dgui, depth=0.85, pose=0.75, cfg=cfg, steps=steps)
                
                frame_urls = []
                
                for f in range(num_frames):
                    joints = get_motion_joints(motion, f, num_frames)
                    depth_map, pose_map = make_skeleton_maps(joints, W, H)
                    
                    shm_depth.seek(HEADER_SIZE)
                    shm_depth.write(depth_map)
                    
                    shm_pose.seek(HEADER_SIZE)
                    shm_pose.write(pose_map)
                    
                    raw_out = f'tmp/anim_{motion}_{f}.raw'
                    png_out = f'tmp/anim_{motion}_{f}.png'
                    
                    raw_out_path = 'tsfi2-deepseek/' + raw_out
                    if os.path.exists(raw_out_path): os.remove(raw_out_path)
                    if os.path.exists(png_out): os.remove(png_out)
                    
                    worker_path = './bin/tsfi_sd_worker'
                    cmd = [
                        worker_path,
                        prompt,
                        raw_out,
                        "1", # use_shm=1
                        profile,
                        str(steps),
                        "euler_a",
                        str(cfg)
                    ]
                    
                    print(f"[Server] Synthesizing Frame {f}/{num_frames} path={raw_out}")
                    res = subprocess.run(cmd, capture_output=True, text=True, timeout=30, cwd='tsfi2-deepseek')
                    
                    if os.path.exists(raw_out_path):
                        with open(raw_out_path, 'rb') as fp:
                            raw_data = fp.read()
                        width = 512 if len(raw_data) == 512 * 512 * 3 else 1280
                        height = 512 if len(raw_data) == 512 * 512 * 3 else 720
                        img = Image.frombytes('RGB', (width, height), raw_data)
                        img.save(png_out)
                        frame_urls.append(f'tmp/anim_{motion}_{f}.png')
                        os.remove(raw_out_path)
                    else:
                        print(f"[Server] Frame {f} generation failed: {res.stderr}")
                
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'status': 'success', 'frames': frame_urls}).encode('utf-8'))
                return
        elif self.path == '/open-browser':
            # Terminate existing node browser controllers for YouTube to prevent layout/display conflicts
            subprocess.run(["pkill", "-f", "rooted_browser_controller.js.*youtube"])
            
            # Spawn the new browser controller pointing to YouTube with Atropa "this week" query
            target_url = "https://www.youtube.com/results?search_query=atropa&sp=EgQIAxAB"
            print(f"[Server] Spawning Auncient rooted browser controller for URL: {target_url}")
            subprocess.Popen(["node", "scripts/rooted_browser_controller.js", target_url])
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'status': 'success', 'message': 'Browser controller spawned successfully.'}).encode('utf-8'))
            return
        elif self.path == '/control':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            params = json.loads(post_data.decode('utf-8'))
            command = params.get('command', '')
            
            if command:
                address_path = 'tmp/wmq_address.txt'
                if os.path.exists(address_path):
                    with open(address_path, 'r') as f:
                        wmq_address = f.read().strip()
                    
                    try:
                        cmd_bytes = command.encode('utf-8')
                        if len(cmd_bytes) < 32:
                            cmd_bytes = cmd_bytes + b'\x00' * (32 - len(cmd_bytes))
                        else:
                            cmd_bytes = cmd_bytes[:32]
                        tx_data = "0xccb077a0" + cmd_bytes.hex()
                        
                        payload = {
                            "jsonrpc": "2.0",
                            "method": "eth_sendTransaction",
                            "params": [{
                                "from": "0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266",
                                "to": wmq_address,
                                "data": tx_data,
                                "gas": "0xF4240"
                            }],
                            "id": 1
                        }
                        import urllib.request
                        req = urllib.request.Request(
                            "http://127.0.0.1:8545",
                            data=json.dumps(payload).encode('utf-8'),
                            headers={'content-type': 'application/json'}
                        )
                        with urllib.request.urlopen(req, timeout=2) as response:
                            res_data = json.loads(response.read().decode('utf-8'))
                            print(f"[Server] Dispatched input command via WMQ transaction: {command}")
                    except Exception as tx_err:
                        print(f"[Server] Failed to dispatch transaction to WMQ: {tx_err}")
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'status': 'success'}).encode('utf-8'))
            return
        else:
            super().do_POST()

if __name__ == '__main__':
    server = ThreadedHTTPServer(('127.0.0.1', 8000), CustomHandler)
    print("[Server] Custom Dynamic Server active on http://127.0.0.1:8000")
    server.serve_forever()
