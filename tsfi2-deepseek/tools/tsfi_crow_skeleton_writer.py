import os
import math
import time
import mmap
import numpy as np

# SHM Constants
TSFI_CN_SHM_DEPTH = "tsfi_cn_depth"
TSFI_CN_SHM_POSE  = "tsfi_cn_pose"
# Distribution Resolution: 1280x720 (as per TSFI_VISUAL_TEST_PLAN.md)
W, H = 1280, 720
MAP_SIZE = W * H * 3
HEADER_SIZE = 32

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

def generate_crow_skeleton(t, posture_name=None):
    pose = bytearray(MAP_SIZE)
    depth = bytearray(MAP_SIZE)
    
    cx, cy = W // 2, H // 2
    s = 6.0 # Scaling for 720p
    
    # Base Anchors (Defaults)
    neck_pos = [cx, cy - 20*s]
    head_top = [cx, cy - 35*s]
    body_mid = [cx, cy + 10*s]
    beak_base = [cx + 10*s, cy - 25*s]
    beak_tip = [cx + 25*s, cy - 25*s + math.sin(t*2)*5*s]
    
    l_shoulder = [cx - 10*s, cy - 5*s]
    r_shoulder = [cx + 10*s, cy - 5*s]
    
    wing_flap = math.sin(t * 4) * 40 * s
    l_wing_tip = [cx - 70*s, cy - 5*s + wing_flap]
    r_wing_tip = [cx + 70*s, cy - 5*s + wing_flap]
    
    l_hip = [cx - 8*s, cy + 25*s]
    r_hip = [cx + 8*s, cy + 25*s]
    l_foot = [cx - 12*s, cy + 60*s + math.cos(t)*5*s]
    r_foot = [cx + 12*s, cy + 60*s + math.sin(t)*5*s]
    
    tail_base = [cx - 15*s, cy + 30*s]
    tail_tip = [cx - 40*s, cy + 45*s + math.cos(t)*10*s]

    # Apply Posture Overrides
    if posture_name == "First Position":
        l_foot = [cx - 5*s, cy + 60*s]; r_foot = [cx + 5*s, cy + 60*s]
        l_wing_tip = [cx - 15*s, cy + 20*s]; r_wing_tip = [cx + 15*s, cy + 20*s]
    elif posture_name == "Second Position":
        l_foot = [cx - 40*s, cy + 60*s]; r_foot = [cx + 40*s, cy + 60*s]
        l_wing_tip = [cx - 80*s, cy - 5*s]; r_wing_tip = [cx + 80*s, cy - 5*s]
    elif posture_name == "Third Position":
        l_foot = [cx - 5*s, cy + 55*s]; r_foot = [cx + 5*s, cy + 65*s]
        l_wing_tip = [cx - 15*s, cy + 20*s]; r_wing_tip = [cx + 80*s, cy - 5*s]
    elif posture_name == "Fourth Position":
        l_foot = [cx - 10*s, cy + 45*s]; r_foot = [cx + 10*s, cy + 75*s]
        l_wing_tip = [cx - 15*s, cy + 20*s]; r_wing_tip = [cx + 20*s, cy - 80*s]
    elif posture_name == "Fifth Position":
        l_foot = [cx + 5*s, cy + 60*s]; r_foot = [cx - 5*s, cy + 60*s]
        l_wing_tip = [cx - 20*s, cy - 80*s]; r_wing_tip = [cx + 20*s, cy - 80*s]
    elif posture_name == "Plier":
        base_cy = cy + 20*s
        neck_pos = [cx, base_cy - 20*s]; head_top = [cx, base_cy - 35*s]; body_mid = [cx, base_cy + 10*s]
        l_foot = [cx - 30*s, base_cy + 40*s]; r_foot = [cx + 30*s, base_cy + 40*s]
        l_wing_tip = [cx - 10*s, base_cy + 5*s]; r_wing_tip = [cx + 10*s, base_cy + 5*s]
    elif posture_name == "Etendre":
        base_cy = cy - 20*s
        neck_pos = [cx, base_cy - 20*s]; head_top = [cx, base_cy - 45*s]
        l_wing_tip = [cx - 90*s, base_cy - 40*s]; r_wing_tip = [cx + 90*s, base_cy - 40*s]
    elif posture_name == "Relever":
        l_foot = [cx - 12*s, cy + 75*s]; r_foot = [cx + 12*s, cy + 75*s]
        l_wing_tip = [cx - 60*s, cy + 0*s]; r_wing_tip = [cx + 60*s, cy + 0*s]
    elif posture_name == "Reverence":
        head_top = [cx + 20*s, cy + 10*s]; neck_pos = [cx + 10*s, cy + 0*s]
        beak_base = [cx + 30*s, cy + 20*s]; beak_tip = [cx + 45*s, cy + 30*s]
        l_wing_tip = [cx - 50*s, cy - 30*s]; r_wing_tip = [cx + 10*s, cy + 40*s]

    # Draw Pose (OpenPose Colors)
    draw_bone(pose, head_top, neck_pos, [255, 0, 0], 3)
    draw_bone(pose, head_top, beak_base, [255, 0, 0], 2)
    draw_bone(pose, beak_base, beak_tip, [255, 0, 255], 4)
    draw_bone(pose, neck_pos, body_mid, [255, 85, 0], 4)
    draw_bone(pose, body_mid, tail_base, [255, 170, 0], 3)
    draw_bone(pose, tail_base, tail_tip, [255, 255, 0], 3)
    draw_bone(pose, neck_pos, l_shoulder, [170, 255, 0], 2)
    draw_bone(pose, neck_pos, r_shoulder, [85, 255, 0], 2)
    draw_bone(pose, l_shoulder, l_wing_tip, [0, 255, 0], 3)
    draw_bone(pose, r_shoulder, r_wing_tip, [0, 255, 85], 3)
    draw_bone(pose, body_mid, l_hip, [0, 255, 170], 2)
    draw_bone(pose, body_mid, r_hip, [0, 255, 255], 2)
    draw_bone(pose, l_hip, l_foot, [0, 170, 255], 2)
    draw_bone(pose, r_hip, r_foot, [0, 85, 255], 2)
    
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

    draw_sphere_depth(depth, head_top, 18*s, 180)
    draw_sphere_depth(depth, neck_pos, 15*s, 160)
    draw_sphere_depth(depth, body_mid, 30*s, 140)
    draw_sphere_depth(depth, tail_base, 20*s, 130)
    
    return depth, pose

if __name__ == "__main__":
    shm_depth = get_shm(TSFI_CN_SHM_DEPTH)
    shm_pose = get_shm(TSFI_CN_SHM_POSE)
    
    postures = [None, "First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position", "Plier", "Etendre", "Relever", "Reverence"]
    
    print("=== TSFi Enriched Crow Ballet Skeleton Provider (1280x720) ===")
    t = 0
    p_idx = 0
    last_p_change = time.time()
    
    while True:
        if time.time() - last_p_change > 5.0: # Change posture every 5 seconds
            p_idx = (p_idx + 1) % len(postures)
            last_p_change = time.time()
            print(f"[SKELETON] Switching to: {postures[p_idx] if postures[p_idx] else 'Dynamic Idle'}")

        depth, pose = generate_crow_skeleton(t, postures[p_idx])
        shm_depth.seek(HEADER_SIZE)
        shm_depth.write(depth)
        shm_pose.seek(HEADER_SIZE)
        shm_pose.write(pose)
        t += 0.05
        time.sleep(0.033)
