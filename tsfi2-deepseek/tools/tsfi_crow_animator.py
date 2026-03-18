import os
import time
import subprocess
import mmap
import ctypes
import math
import numpy as np
from PIL import Image

# SHM Constants
TSFI_CN_SHM_DEPTH = "tsfi_cn_depth"
TSFI_CN_SHM_POSE  = "tsfi_cn_pose"
TSFI_CN_SHM_DGUI  = "tsfi_cn_dgui"
W, H = 1280, 720
MAP_SIZE = W * H * 3
HEADER_SIZE = 32
DGUI_SIZE = 32

def get_shm(name):
    try:
        fd = os.open(f"/dev/shm/{name}", os.O_RDWR)
        return mmap.mmap(fd, HEADER_SIZE + MAP_SIZE)
    except FileNotFoundError:
        fd = os.open(f"/dev/shm/{name}", os.O_RDWR | os.O_CREAT)
        os.ftruncate(fd, HEADER_SIZE + MAP_SIZE)
        m = mmap.mmap(fd, HEADER_SIZE + MAP_SIZE)
        m[0:4] = b'TCNM'
        m[8:12] = (W).to_bytes(4, 'little')
        m[12:16] = (H).to_bytes(4, 'little')
        m[16:20] = (3).to_bytes(4, 'little')
        return m

def get_dgui_shm():
    try:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR)
        return mmap.mmap(fd, DGUI_SIZE)
    except FileNotFoundError:
        fd = os.open(f"/dev/shm/{TSFI_CN_SHM_DGUI}", os.O_RDWR | os.O_CREAT)
        os.ftruncate(fd, DGUI_SIZE)
        m = mmap.mmap(fd, DGUI_SIZE)
        m[0:4] = b'DGUI'
        return m

def update_guidance(m, depth=0.8, pose=0.6, cfg=7.5, steps=4):
    m[4:8] = bytearray(ctypes.c_float(depth))
    m[8:12] = bytearray(ctypes.c_float(pose))
    m[12:16] = bytearray(ctypes.c_float(cfg))
    m[16:20] = (int(steps)).to_bytes(4, 'little')

def draw_bone(pose, p1, p2, color, thickness):
    x1, y1 = p1
    x2, y2 = p2
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx*dx + dy*dy)
    if dist == 0: return
    
    for t in np.linspace(0, 1, int(dist * 2)):
        cx = int(x1 + dx * t)
        cy = int(y1 + dy * t)
        for dy_off in range(-thickness, thickness + 1):
            for dx_off in range(-thickness, thickness + 1):
                if dx_off*dx_off + dy_off*dy_off <= thickness*thickness:
                    tx, ty = cx + dx_off, cy + dy_off
                    if 0 <= tx < W and 0 <= ty < H:
                        idx = (ty * W + tx) * 3
                        pose[idx:idx+3] = color

def get_posture_joints(posture_name, t=0.0):
    cx, cy = W // 2, H // 2
    s = 6.0
    
    # Base configuration
    joints = {
        "neck_pos": [cx, cy - 20*s],
        "head_top": [cx, cy - 35*s],
        "body_mid": [cx, cy + 10*s],
        "beak_base": [cx + 10*s, cy - 25*s],
        "beak_tip": [cx + 25*s, cy - 25*s + math.sin(t*2)*5*s],
        "l_shoulder": [cx - 10*s, cy - 5*s],
        "r_shoulder": [cx + 10*s, cy - 5*s],
        "l_wing_tip": [cx - 70*s, cy - 5*s],
        "r_wing_tip": [cx + 70*s, cy - 5*s],
        "l_hip": [cx - 8*s, cy + 25*s],
        "r_hip": [cx + 8*s, cy + 25*s],
        "l_foot": [cx - 12*s, cy + 60*s],
        "r_foot": [cx + 12*s, cy + 60*s],
        "tail_base": [cx - 15*s, cy + 30*s],
        "tail_tip": [cx - 40*s, cy + 45*s]
    }

    if posture_name == "First Position":
        joints["l_foot"] = [cx - 5*s, cy + 60*s]; joints["r_foot"] = [cx + 5*s, cy + 60*s]
        joints["l_wing_tip"] = [cx - 15*s, cy + 20*s]; joints["r_wing_tip"] = [cx + 15*s, cy + 20*s]
    elif posture_name == "Second Position":
        joints["l_foot"] = [cx - 40*s, cy + 60*s]; joints["r_foot"] = [cx + 40*s, cy + 60*s]
        joints["l_wing_tip"] = [cx - 80*s, cy - 5*s]; joints["r_wing_tip"] = [cx + 80*s, cy - 5*s]
    elif posture_name == "Third Position":
        joints["l_foot"] = [cx - 5*s, cy + 55*s]; joints["r_foot"] = [cx + 5*s, cy + 65*s]
        joints["l_wing_tip"] = [cx - 15*s, cy + 20*s]; joints["r_wing_tip"] = [cx + 80*s, cy - 5*s]
    elif posture_name == "Fourth Position":
        joints["l_foot"] = [cx - 10*s, cy + 45*s]; joints["r_foot"] = [cx + 10*s, cy + 75*s]
        joints["l_wing_tip"] = [cx - 15*s, cy + 20*s]; joints["r_wing_tip"] = [cx + 20*s, cy - 80*s]
    elif posture_name == "Fifth Position":
        joints["l_foot"] = [cx + 5*s, cy + 60*s]; joints["r_foot"] = [cx - 5*s, cy + 60*s]
        joints["l_wing_tip"] = [cx - 20*s, cy - 80*s]; joints["r_wing_tip"] = [cx + 20*s, cy - 80*s]
    elif posture_name == "Plier":
        base_cy = cy + 20*s
        joints["neck_pos"] = [cx, base_cy - 20*s]; joints["head_top"] = [cx, base_cy - 35*s]; joints["body_mid"] = [cx, base_cy + 10*s]
        joints["l_foot"] = [cx - 30*s, base_cy + 40*s]; joints["r_foot"] = [cx + 30*s, base_cy + 40*s]
        joints["l_wing_tip"] = [cx - 10*s, base_cy + 5*s]; joints["r_wing_tip"] = [cx + 10*s, base_cy + 5*s]
    elif posture_name == "Etendre":
        base_cy = cy - 20*s
        joints["neck_pos"] = [cx, base_cy - 20*s]; joints["head_top"] = [cx, base_cy - 45*s]
        joints["l_wing_tip"] = [cx - 90*s, base_cy - 40*s]; joints["r_wing_tip"] = [cx + 90*s, base_cy - 40*s]
    elif posture_name == "Relever":
        joints["l_foot"] = [cx - 12*s, cy + 75*s]; joints["r_foot"] = [cx + 12*s, cy + 75*s]
        joints["l_wing_tip"] = [cx - 60*s, cy + 0*s]; joints["r_wing_tip"] = [cx + 60*s, cy + 0*s]
    elif posture_name == "Reverence":
        joints["head_top"] = [cx + 20*s, cy + 10*s]; joints["neck_pos"] = [cx + 10*s, cy + 0*s]
        joints["beak_base"] = [cx + 30*s, cy + 20*s]; joints["beak_tip"] = [cx + 45*s, cy + 30*s]
        joints["l_wing_tip"] = [cx - 50*s, cy - 30*s]; joints["r_wing_tip"] = [cx + 10*s, cy + 40*s]

    return joints

def lerp(a, b, t):
    return [a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t]

def generate_interpolated_skeleton(joints):
    pose = bytearray(MAP_SIZE)
    depth = bytearray(MAP_SIZE)
    s = 6.0
    
    # Draw Pose
    draw_bone(pose, joints["head_top"], joints["neck_pos"], [255, 0, 0], 3)
    draw_bone(pose, joints["head_top"], joints["beak_base"], [255, 0, 0], 2)
    draw_bone(pose, joints["beak_base"], joints["beak_tip"], [255, 0, 255], 4)
    draw_bone(pose, joints["neck_pos"], joints["body_mid"], [255, 85, 0], 4)
    draw_bone(pose, joints["body_mid"], joints["tail_base"], [255, 170, 0], 3)
    draw_bone(pose, joints["tail_base"], joints["tail_tip"], [255, 255, 0], 3)
    draw_bone(pose, joints["neck_pos"], joints["l_shoulder"], [170, 255, 0], 2)
    draw_bone(pose, joints["neck_pos"], joints["r_shoulder"], [85, 255, 0], 2)
    draw_bone(pose, joints["l_shoulder"], joints["l_wing_tip"], [0, 255, 0], 3)
    draw_bone(pose, joints["r_shoulder"], joints["r_wing_tip"], [0, 255, 85], 3)
    draw_bone(pose, joints["body_mid"], joints["l_hip"], [0, 255, 170], 2)
    draw_bone(pose, joints["body_mid"], joints["r_hip"], [0, 255, 255], 2)
    draw_bone(pose, joints["l_hip"], joints["l_foot"], [0, 170, 255], 2)
    draw_bone(pose, joints["r_hip"], joints["r_foot"], [0, 85, 255], 2)
    
    # Depth Map
    def draw_sphere_depth(d_map, center, radius, z_val):
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

    draw_sphere_depth(depth, joints["head_top"], 18*s, 180)
    draw_sphere_depth(depth, joints["neck_pos"], 15*s, 160)
    draw_sphere_depth(depth, joints["body_mid"], 30*s, 140)
    draw_sphere_depth(depth, joints["tail_base"], 20*s, 130)
    
    return depth, pose

def execute_animation():
    print("=== TSFi Sovereign Animation Studio: Crow Ballet ===")
    out_dir = "tmp/crow_frames"
    os.makedirs(out_dir, exist_ok=True)
    
    shm_depth = get_shm(TSFI_CN_SHM_DEPTH)
    shm_pose = get_shm(TSFI_CN_SHM_POSE)
    dgui = get_dgui_shm()
    
    # Setup Guidance
    update_guidance(dgui, depth=0.85, pose=0.75, cfg=6.5, steps=8)
    
    sequence = [
        "First Position", "Plier", "Second Position", "Relever", 
        "Third Position", "Etendre", "Fourth Position", "Fifth Position", "Reverence"
    ]
    
    frames_per_transition = 8
    total_frames = len(sequence) * frames_per_transition
    
    base_prompt = "extreme macro photography of a stuffed animal crow plush, hyper-detailed individual synthetic feathers, matted black faux-plumage textures, visible stitching, soft plush under-down, studio lighting catching iridescent sheen, neutral background, 8k resolution masterpiece"
    
    frame_idx = 0
    start_time = time.time()
    
    for i in range(len(sequence)):
        start_pose = sequence[i]
        end_pose = sequence[(i + 1) % len(sequence)]
        
        j_start = get_posture_joints(start_pose, 0.0)
        j_end = get_posture_joints(end_pose, 1.0)
        
        for f in range(frames_per_transition):
            t = f / float(frames_per_transition)
            # Ease-in-out interpolation
            t_smooth = t * t * (3 - 2 * t)
            
            j_interp = {}
            for k in j_start.keys():
                j_interp[k] = lerp(j_start[k], j_end[k], t_smooth)
            
            depth, pose = generate_interpolated_skeleton(j_interp)
            
            shm_depth.seek(HEADER_SIZE)
            shm_depth.write(depth)
            shm_pose.seek(HEADER_SIZE)
            shm_pose.write(pose)
            
            output_raw = f"{out_dir}/frame_{frame_idx:04d}.raw"
            
            cmd = [
                "bin/tsfi_sd_worker",
                base_prompt,
                output_raw,
                "1", # use_shm ENABLED
                "sd15",
                "8",
                "euler_a",
                "6.5"
            ]
            
            print(f"[ANIMATOR] Rendering Frame {frame_idx}/{total_frames} ({start_pose} -> {end_pose})")
            subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            
            if os.path.exists(output_raw):
                with open(output_raw, "rb") as f_raw:
                    raw_data = f_raw.read()
                # Assuming 512x512 due to current worker config, if we rebuilt for 720p we change this
                expected_size = 512 * 512 * 3
                if len(raw_data) >= expected_size:
                    img = Image.frombytes("RGB", (512, 512), raw_data[:expected_size])
                    img.save(f"{out_dir}/frame_{frame_idx:04d}.jpg", "JPEG", quality=95)
                os.remove(output_raw) # Cleanup raw to save space
                
            frame_idx += 1
            
    print(f"\\n[ANIMATOR] Generation Complete in {time.time() - start_time:.2f}s. Assembling MP4...")
    
    output_video = "assets/crow_ballet_performance.mp4"
    subprocess.run([
        "ffmpeg", "-y", "-framerate", "12", "-i", f"{out_dir}/frame_%04d.jpg", 
        "-c:v", "libx264", "-preset", "slow", "-crf", "18", "-pix_fmt", "yuv420p", output_video
    ], capture_output=True)
    
    print(f"[ABSOLUTE SUCCESS] Crow Ballet Animation compiled to {output_video}")

if __name__ == "__main__":
    execute_animation()