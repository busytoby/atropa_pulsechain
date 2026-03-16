import os
import sys
import time
import subprocess
import base64
import math
import io
import json
import urllib.request
import ctypes
import random
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image, ImageDraw, ImageFont
import cv2
import numpy as np
import ctypes
import mmap
import fcntl

# TSFi Registry Definition
class TsfiAcousticRegistry(ctypes.Structure):
    _fields_ = [
        ("pdai_manifold_pressure", ctypes.c_float),
        ("yang_heartbeat_hz", ctypes.c_float),
        ("yang_static_amplitude", ctypes.c_float),
        ("agent_participation_flag", ctypes.c_int),
        ("target_genome_id", ctypes.c_int),
        ("agent_monologue", ctypes.c_char * 256),
        ("alligator_x", ctypes.c_float),
        ("alligator_y", ctypes.c_float),
        ("alligator_form_ratio", ctypes.c_float),
        ("spider_x", ctypes.c_float),
        ("spider_y", ctypes.c_float),
        ("spider_texture_sharpness", ctypes.c_float),
        ("genie_poetic_observation", ctypes.c_char * 256),
    ]

shm_fd = None
shm_reg = None
try:
    shm_fd = os.open("/dev/shm/tsfi_test_acoustic_space", os.O_RDWR)
    shm_mmap = mmap.mmap(shm_fd, ctypes.sizeof(TsfiAcousticRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    shm_reg = TsfiAcousticRegistry.from_buffer(shm_mmap)
    print("[INFO] Bound to TSFi Shared Memory Acoustic Space.")
except Exception as e:
    print(f"[WARN] Acoustic Registry not found. Running disconnected. ({e})")


# --- Genie Globals ---
try:
    import torch
    from transformers import AutoModelForCausalLM, AutoProcessor
    GENIE_ENABLED = True
except ImportError:
    GENIE_ENABLED = False

genie_model = None
genie_processor = None
genie_current_thought = "GENIE IS AWAKENING..."
genie_last_observation_time = 0

def init_genie():
    global genie_model, genie_processor
    if GENIE_ENABLED and genie_model is None:
        print("[INFO] Conjuring the Moondream Genie from the Void...")
        try:
            genie_model = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
            genie_processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
            print("[SUCCESS] Genie is watching.")
        except Exception as e:
            print(f"[WARN] Genie failed to materialize: {e}")

if GENIE_ENABLED:
    threading.Thread(target=init_genie, daemon=True).start()

def genie_observation_thread(frame_image):
    global genie_current_thought
    if genie_model is None or genie_processor is None: return
    try:
        enc_image = genie_model.encode_image(frame_image)
        
        # 1. Poetic Observation
        q1 = "You are an AI daemon observing three soft plush toys (a teddy bear, an alligator, and a spider). What are they doing together? Answer in one short, poetic sentence."
        ans1 = genie_model.answer_question(enc_image, q1, genie_processor)
        
        # 2. Quality Classification and Grading
        q2 = "Grade the quality of these plush toys on a scale from 1 to 10. Briefly state any visible defects like extra limbs, blurriness, or melting."
        ans2 = genie_model.answer_question(enc_image, q2, genie_processor)
        
        genie_current_thought = f"DAEMON SEES: {ans1.upper()} | GRADE: {ans2.upper()}"
        
        # 3. DeepSeek Oversight (The Higher Bijection)
        # We must write the prompt cleanly, wait a moment, and read the answer.
        # In a true synchronous system this would block, but since we are in 
        # `genie_observation_thread`, we can afford to wait a few seconds.
        ds_prompt = f"The visual parser 'Genie' has observed a simulation of stuffed animals. It stated: '{ans1}'. It also graded the quality: '{ans2}'. As the Supreme Oversight Matrix, synthesize this into a single, highly technical, deterministic telemetry string to be broadcast to the world. Do not use conversational filler."
        
        import os
        import time
        
        with open("/tmp/tsfi_llm_in", "w") as f_in:
            f_in.write(ds_prompt)
            
        time.sleep(2) # Give DeepSeek time to process
            
        if shm_reg is not None:
            shm_reg.genie_poetic_observation = str(ans1).encode('utf-8')[:255]
    except:
        pass


ffmpeg_process = None
global_frame_idx = 0
current_genome = "a soft stuffed animal collection featuring a green alligator plush, a fuzzy spider plush, and a brown teddy bear"
latest_frame_bytes = None
latest_jpeg_bytes = None

# Ballet Kinematics Globals
ballet_keyframes = []
ballet_active = False
ballet_current_step = 0
ballet_frame_counter = 0
puppet_target_x = 0
puppet_target_y = 0

def broadcast_thread():
    global ffmpeg_process, latest_frame_bytes
    fps = 30.0
    frame_time = 1.0 / fps
    next_time = time.time()
    
    while True:
        if ffmpeg_process is not None and latest_frame_bytes is not None:
            try:
                ffmpeg_process.stdin.write(latest_frame_bytes)
                ffmpeg_process.stdin.flush()
            except Exception:
                print("[FRACTURE] YouTube RTMP disconnected (Broken Pipe).")
                ffmpeg_process = None
                
        next_time += frame_time
        sleep_time = next_time - time.time()
        if sleep_time > 0:
            time.sleep(sleep_time)
        else:
            # If we fell behind, just reset the clock to prevent a massive catch-up burst
            next_time = time.time()

threading.Thread(target=broadcast_thread, daemon=True).start()



# ---------------------------------------------------------
# THE SPIDER'S CONTENT EVALUATOR
# ---------------------------------------------------------
def run_spider_critic(rendered_img_path, original_prompt):
    try:
        img = cv2.imread(rendered_img_path)
        if img is None: return "NONE"

        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
        brightness = np.mean(gray)
        
        lower_green = np.array([35, 40, 40])
        upper_green = np.array([85, 255, 200])
        mask_gator = cv2.inRange(hsv, lower_green, upper_green)
        gator_pixels = cv2.countNonZero(mask_gator)
        
        lower_brown = np.array([10, 50, 20])
        upper_brown = np.array([25, 255, 200])
        mask_bear = cv2.inRange(hsv, lower_brown, upper_brown)
        bear_pixels = cv2.countNonZero(mask_bear)
        
        # Human Flesh Tone quarantine mask
        lower_flesh = np.array([0, 20, 50])
        upper_flesh = np.array([20, 150, 255])
        mask_flesh = cv2.inRange(hsv, lower_flesh, upper_flesh)
        flesh_pixels = cv2.countNonZero(mask_flesh)
        
        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0
        flesh_presence = (flesh_pixels / total_pixels) * 100.0
        
        # 3. DeepSeek Oversight (The Higher Bijection)
        # We must write the prompt cleanly, wait a moment, and read the answer.
        # In a true synchronous system this would block, but since we are in 
        # `genie_observation_thread`, we can afford to wait a few seconds.
        ds_prompt = f"The visual parser 'Genie' has observed a simulation of stuffed animals. It stated: '{ans1}'. It also graded the quality: '{ans2}'. As the Supreme Oversight Matrix, synthesize this into a single, highly technical, deterministic telemetry string to be broadcast to the world. Do not use conversational filler."
        
        import os
        import time
        
        with open("/tmp/tsfi_llm_in", "w") as f_in:
            f_in.write(ds_prompt)
            
        time.sleep(2) # Give DeepSeek time to process
            
        if shm_reg is not None:
            shm_reg.spider_texture_sharpness = float(sharpness)
        
        if flesh_presence > 5.0:
            # If more than 5% of the image is human skin tone, burn it immediately.
            return "HUMAN_CONTAMINATION"
        elif brightness < 80.0:
            return "INCREASE_LIGHTING"
        elif gator_presence < 2.0 and "alligator" in original_prompt.lower():
            return "EMPHASIZE_GATOR"
        elif bear_presence < 5.0 and "bear" in original_prompt.lower():
            return "EMPHASIZE_BEAR"
            
        return "NONE"
    except:
        return "NONE"

# ---------------------------------------------------------
# AUTONOMOUS VISION CRITIC (THE ALLIGATOR)
# ---------------------------------------------------------
def run_vision_critic(rendered_img_path, skeleton_img_path, decay_ratio):
    try:
        rendered_img = cv2.imread(rendered_img_path)
        skeleton_img = cv2.imread(skeleton_img_path)

        if rendered_img is None or skeleton_img is None:
            return True, "MISSING_FILES", "CONTINUE"

        hsv_skel = cv2.cvtColor(skeleton_img, cv2.COLOR_BGR2HSV)
        lower_any = np.array([0, 50, 50])
        upper_any = np.array([180, 255, 255])
        mask_skel = cv2.inRange(hsv_skel, lower_any, upper_any)
        
        contours_skel, _ = cv2.findContours(mask_skel, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours_skel: return True, "NO_SKELETON", "CONTINUE"
            
        x_min, y_min = 9999, 9999
        x_max, y_max = 0, 0
        for cnt in contours_skel:
            x, y, w, h = cv2.boundingRect(cnt)
            x_min = min(x_min, x)
            y_min = min(y_min, y)
            x_max = max(x_max, x + w)
            y_max = max(y_max, y + h)

        skel_area = max(1, (x_max - x_min) * (y_max - y_min))

        gray_render = cv2.cvtColor(rendered_img, cv2.COLOR_BGR2GRAY)
        blurred_render = cv2.GaussianBlur(gray_render, (5, 5), 0)
        _, thresh = cv2.threshold(blurred_render, 240, 255, cv2.THRESH_BINARY_INV)
        edges = cv2.Canny(thresh, 50, 150)
        kernel = np.ones((5,5),np.uint8)
        dilated_edges = cv2.dilate(edges,kernel,iterations = 2)

        contours_render, _ = cv2.findContours(dilated_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours_render:
             return True, "NO_CONTOURS_FOUND", "NONE"

        rx_min, ry_min = 9999, 9999
        rx_max, ry_max = 0, 0
        for cnt in contours_render:
            if cv2.contourArea(cnt) > 100:
                x, y, w, h = cv2.boundingRect(cnt)
                rx_min = min(rx_min, x)
                ry_min = min(ry_min, y)
                rx_max = max(rx_max, x + w)
                ry_max = max(ry_max, y + h)

        render_area = max(1, (rx_max - rx_min) * (ry_max - ry_min))
        ratio = render_area / float(skel_area)
        
        # 3. DeepSeek Oversight (The Higher Bijection)
        # We must write the prompt cleanly, wait a moment, and read the answer.
        # In a true synchronous system this would block, but since we are in 
        # `genie_observation_thread`, we can afford to wait a few seconds.
        ds_prompt = f"The visual parser 'Genie' has observed a simulation of stuffed animals. It stated: '{ans1}'. It also graded the quality: '{ans2}'. As the Supreme Oversight Matrix, synthesize this into a single, highly technical, deterministic telemetry string to be broadcast to the world. Do not use conversational filler."
        
        import os
        import time
        
        with open("/tmp/tsfi_llm_in", "w") as f_in:
            f_in.write(ds_prompt)
            
        time.sleep(2) # Give DeepSeek time to process
            
        if shm_reg is not None:
            shm_reg.alligator_form_ratio = float(ratio)
            # The contour center represents the physical Alligator mass
            shm_reg.alligator_x = float(rx_min + (rx_max - rx_min)/2.0)
            shm_reg.alligator_y = float(ry_min + (ry_max - ry_min)/2.0)
        
        max_allowed_ratio = 1.6 + (decay_ratio * 0.4) 
        min_allowed_ratio = 0.0 # Disabled lower bound to allow any size smaller than the skeleton

        if ratio > max_allowed_ratio:
            return False, f"STRUCTURAL_FRACTURE_TOO_LARGE ({ratio:.2f} > {max_allowed_ratio:.2f})", "INCREASE_CONTROL"
        elif ratio < min_allowed_ratio:
            return False, f"STRUCTURAL_FRACTURE_TOO_SMALL ({ratio:.2f} < {min_allowed_ratio:.2f})", "SCRAMBLE_SEED"

        sharpness = cv2.Laplacian(gray_render, cv2.CV_64F).var()
        if sharpness < 50.0:
             return False, f"SEVERE_BLUR ({sharpness:.1f} < 50.0)", "INCREASE_STEPS"

        return True, "FORM_VERIFIED", "NONE"
    except Exception as e:
        return True, f"CRITIC_CRASH: {e}", "CONTINUE"

# ---------------------------------------------------------
# OPENPOSE SKELETAL GENERATOR
# ---------------------------------------------------------
def build_openpose_skeleton(f_idx, phase="sit"):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    try:
        if os.path.exists("assets/fonts/Hack-Regular.ttf"):
            font = ImageFont.truetype("assets/fonts/Hack-Regular.ttf", 14)
        else:
            font = ImageFont.load_default()
    except Exception:
        font = ImageFont.load_default()
        
    thought = genie_current_thought + " " + genie_current_thought + " "
    for column in range(0, W, 14):
        offset = (f_idx * 5 + column * 13) % H
        char_idx_base = column % len(thought)
        for row in range(0, H, 14):
            y_pos = (row + offset) % H
            if y_pos > H - 50: continue
            char_idx = (char_idx_base + (row // 14)) % len(thought)
            char = thought[char_idx]
            intensity = int(255 * (1.0 - (y_pos / float(H))))
            draw.text((column, y_pos), char, font=font, fill=(0, intensity, 0))

    
    t = f_idx / 30.0
    decay = max(0.0, 1.0 - (f_idx / 60.0))
    anim_wave = (math.sin(t * 1.5) * 0.5) * decay
    
    # Defaults (sit)
    ax = 320  # Alligator X
    ay = 380  # Alligator Y
    tail_wave = math.sin(t * 3.0) * 20.0 * decay
    jaw_snap = max(0.0, math.sin(t * 4.0) * 15.0 * decay)
    
    bx = 160  # Bear X
    by = 300  # Bear Y
    
    body_y_offset = 0
    snout_y_offset = 0
    spider_x_offset = 0
    spider_y_offset = 0
    bear_y_offset = 0
    bear_arm_wave = 0
    bear_leg_wave = 0
    
    if phase == "roar":
        body_y_offset = -40
        snout_y_offset = -100
        jaw_snap = 30 + math.sin(t * 6.0) * 20.0
        spider_x_offset = 140
        spider_y_offset = -40
        bear_arm_wave = 40
        bear_y_offset = -20
    elif phase == "swim":
        body_y_offset = 30
        snout_y_offset = 30
        jaw_snap = 5.0
        tail_wave = math.sin(t * 5.0) * 80.0
        spider_x_offset = 160 + math.sin(t * 5.0) * 50.0
        spider_y_offset = body_y_offset - 40 - tail_wave * 0.5
        bear_y_offset = 40 + math.sin(t * 4.0) * 20.0
        bear_arm_wave = 60 * math.sin(t * 5.0)
    elif phase == "dance":
        ay = 440
        body_y_offset = -160
        snout_y_offset = -220
        jaw_snap = 20.0 + math.sin(t * 8.0) * 10.0
        spider_y_offset = -320 + math.sin(t * 10.0) * 40.0
        spider_x_offset = 50 * math.sin(t * 4.0)
        bear_y_offset = -80 + math.sin(t * 8.0) * 40.0
        bear_arm_wave = 80 * math.sin(t * 8.0)
        bear_leg_wave = 30 * math.sin(t * 8.0)
    elif phase == "roll":
        snout_y_offset = 50
        body_y_offset = 40
        tail_wave = math.sin(t * 4.0) * 60.0
        spider_x_offset = -100 + math.sin(t * 3.0) * 30.0
        spider_y_offset = -150
        bear_y_offset = 60
        bear_arm_wave = -60
        bear_leg_wave = -60
    elif phase == "pounce":
        snout_y_offset = 50
        body_y_offset = 50
        spider_x_offset = 20
        spider_y_offset = -200 - abs(math.sin(t * 4.0)) * 100.0
        bear_y_offset = -100 - abs(math.sin(t * 4.0)) * 80.0
        bear_arm_wave = 100
        bear_leg_wave = 40
    else:
        bear_arm_wave = math.sin(t * 2.0) * 10.0
        bear_leg_wave = math.sin(t * 1.5) * 5.0


    # --- TSFi BRUSH AND BUNDLE ARCHITECTURE ---
    def brush_bone(d_ctx, x0, y0, x1, y1, rgb, th):
        d_ctx.line([(x0, y0), (x1, y1)], fill=rgb, width=th)
        
    def brush_joint(d_ctx, cx, cy, rad, rgb):
        d_ctx.ellipse([cx-rad, cy-rad, cx+rad, cy+rad], fill=rgb)

    def bundle_limb(d_ctx, pt_start, pt_end, rgb, th):
        brush_bone(d_ctx, pt_start[0], pt_start[1], pt_end[0], pt_end[1], rgb, th)
        brush_joint(d_ctx, pt_start[0], pt_start[1], th//2, rgb)
        brush_joint(d_ctx, pt_end[0], pt_end[1], th//2, rgb)
    # ------------------------------------------

    th = 8
    def dl(x0, y0, x1, y1, r, g, b):

        draw.line([(x0, y0), (x1, y1)], fill=(r, g, b), width=th)


    # --- INVERSE KINEMATICS SOLVER ---
    def solve_2d_ik_2bone(target_x, target_y, root_x, root_y, L1, L2):
        dx = target_x - root_x
        dy = target_y - root_y
        dist = math.sqrt(dx*dx + dy*dy)
        dist = min(dist, L1 + L2 - 0.01)
        try:
            angle1 = math.acos((L1*L1 + dist*dist - L2*L2) / (2 * L1 * dist))
        except ValueError:
            angle1 = 0
        base_angle = math.atan2(dy, dx)
        joint_angle = base_angle - angle1
        jx = root_x + L1 * math.cos(joint_angle)
        jy = root_y + L1 * math.sin(joint_angle)
        return (jx, jy)

    # --- BALLET CHOREOGRAPHY BUNDLE ---
    def bundle_ik_teddy_ballet(d_ctx, center_x, center_y, t, rgb, th):
        hip = (center_x, center_y + 80)
        neck = (center_x, center_y - 80)
        L_thigh, L_calf, L_upperarm, L_forearm = 70, 80, 60, 70
        sweep_angle = math.pi * 0.5 * math.sin(t * math.pi) 
        l_foot_target = (hip[0] - 30, hip[1] + 130)
        r_foot_target_x = hip[0] + 40 + math.sin(sweep_angle) * 120
        r_foot_target_y = hip[1] + 130 - math.sin(sweep_angle) * 200
        arm_lift = math.sin(t * math.pi) * 80
        l_hand_target = (neck[0] - 100 + arm_lift, neck[1] + 50 - arm_lift)
        r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)
        l_knee = solve_2d_ik_2bone(l_foot_target[0], l_foot_target[1], hip[0]-20, hip[1], L_thigh, L_calf)
        r_knee = solve_2d_ik_2bone(r_foot_target_x, r_foot_target_y, hip[0]+20, hip[1], L_thigh, L_calf)
        l_elbow = solve_2d_ik_2bone(l_hand_target[0], l_hand_target[1], neck[0]-30, neck[1], L_upperarm, L_forearm)
        r_elbow = solve_2d_ik_2bone(r_hand_target[0], r_hand_target[1], neck[0]+30, neck[1], L_upperarm, L_forearm)
        bundle_limb(d_ctx, hip, neck, rgb, th)
        bundle_limb(d_ctx, (hip[0]-20, hip[1]), l_knee, rgb, th)
        bundle_limb(d_ctx, l_knee, l_foot_target, rgb, th)
        bundle_limb(d_ctx, (hip[0]+20, hip[1]), r_knee, rgb, th)
        bundle_limb(d_ctx, r_knee, (r_foot_target_x, r_foot_target_y), rgb, th)
        bundle_limb(d_ctx, (neck[0]-30, neck[1]), l_elbow, rgb, th)
        bundle_limb(d_ctx, l_elbow, l_hand_target, rgb, th)
        bundle_limb(d_ctx, (neck[0]+30, neck[1]), r_elbow, rgb, th)

    # --- 1. The Sick Teddy Bear (Left) - IK BALLET BUNDLE ---

    if phase == "dance":
        # The Teddy Bear performs the full IK Ballet sequence over the 60-frame epoch
        try:
            bundle_ik_teddy_ballet(draw, bx, by + bear_y_offset, (f_idx % 60) / 60.0, (255, 85, 0), th)
        except Exception as e:
            print(f"IK CRASH: {e}")
    else:
        # Static/Idle state using the legacy procedural lines (to be bundled later)
        bear_cx = bx
        bear_cy = by + bear_y_offset
        torso_len = 80
        
        neck = (bear_cx, bear_cy - torso_len)
        r_shoulder = (neck[0] - 30, neck[1])
        l_shoulder = (neck[0] + 30, neck[1])
        r_hip = (bear_cx - 20, bear_cy)
        l_hip = (bear_cx + 20, bear_cy)
        
        r_elbow = (r_shoulder[0] - 20, r_shoulder[1] + 30 - bear_arm_wave)
        r_wrist = (r_elbow[0] - 10, r_elbow[1] + 30 - bear_arm_wave)
        l_elbow = (l_shoulder[0] + 20, l_shoulder[1] + 30 - bear_arm_wave)
        l_wrist = (l_elbow[0] + 10, l_elbow[1] + 30 - bear_arm_wave)
        
        r_knee = (r_hip[0] - 20, r_hip[1] + 30 - bear_leg_wave)
        r_ankle = (r_knee[0] - 10, r_knee[1] + 30 - bear_leg_wave)
        l_knee = (l_hip[0] + 20, l_hip[1] + 30 - bear_leg_wave)
        l_ankle = (l_knee[0] + 10, l_knee[1] + 30 - bear_leg_wave)
        
        dl(neck[0], neck[1], bear_cx, bear_cy, 0, 255, 170)
        
        dl(neck[0], neck[1], r_shoulder[0], r_shoulder[1], 255, 85, 0)
        dl(r_shoulder[0], r_shoulder[1], r_elbow[0], r_elbow[1], 255, 170, 0)
        dl(r_elbow[0], r_elbow[1], r_wrist[0], r_wrist[1], 255, 255, 0)
        dl(neck[0], neck[1], l_shoulder[0], l_shoulder[1], 85, 255, 0)
        dl(l_shoulder[0], l_shoulder[1], l_elbow[0], l_elbow[1], 0, 255, 0)
        dl(l_elbow[0], l_elbow[1], l_wrist[0], l_wrist[1], 0, 255, 85)
        
        dl(bear_cx, bear_cy, r_hip[0], r_hip[1], 0, 170, 255)
        dl(r_hip[0], r_hip[1], r_knee[0], r_knee[1], 0, 85, 255)
        dl(r_knee[0], r_knee[1], r_ankle[0], r_ankle[1], 0, 0, 255)
        dl(bear_cx, bear_cy, l_hip[0], l_hip[1], 85, 0, 255)
        dl(l_hip[0], l_hip[1], l_knee[0], l_knee[1], 170, 0, 255)
        dl(l_knee[0], l_knee[1], l_ankle[0], l_ankle[1], 255, 0, 255)

    # --- 2. The Dreamgator (Right/Bottom) ---
    by_a = ay + body_y_offset
    hy_a = ay + snout_y_offset
    
    alli_nose = (ax - 80, hy_a - jaw_snap)
    alli_neck = (ax + 20, hy_a)
    alli_midhip = (ax + 120, by_a)
    alli_tail = (ax + 240, ay - tail_wave)
    
    dl(alli_nose[0], alli_nose[1], alli_neck[0], alli_neck[1], 255, 0, 0)
    dl(alli_neck[0], alli_neck[1], alli_midhip[0], alli_midhip[1], 0, 255, 170)
    dl(alli_midhip[0], alli_midhip[1], alli_tail[0], alli_tail[1], 0, 255, 170)
    dl(alli_neck[0], alli_neck[1], ax - 60, hy_a + jaw_snap, 255, 0, 0)
    
    # Front Legs
    if phase == "dance" or phase == "roar":
        dl(alli_neck[0], alli_neck[1], alli_neck[0] - 30, alli_neck[1] - 80, 255, 170, 0)
        dl(alli_neck[0] - 30, alli_neck[1] - 80, alli_neck[0] - 60, alli_neck[1] - 140, 255, 255, 0)
        dl(alli_neck[0], alli_neck[1], alli_neck[0] + 30, alli_neck[1] - 90, 85, 255, 0)
        dl(alli_neck[0] + 30, alli_neck[1] - 90, alli_neck[0] + 50, alli_neck[1] - 150, 0, 255, 0)
    elif phase == "roll":
        dl(alli_neck[0], alli_neck[1], alli_neck[0] - 40, alli_neck[1] - 90, 255, 170, 0)
        dl(alli_neck[0] - 40, alli_neck[1] - 90, alli_neck[0] - 40, alli_neck[1] - 150, 255, 255, 0)
        dl(alli_neck[0], alli_neck[1], alli_neck[0] + 40, alli_neck[1] - 80, 85, 255, 0)
        dl(alli_neck[0] + 40, alli_neck[1] - 80, alli_neck[0] + 40, alli_neck[1] - 140, 0, 255, 0)
    else:
        dl(alli_neck[0], alli_neck[1], alli_neck[0] - 20, alli_neck[1] + 60, 255, 170, 0)
        dl(alli_neck[0] - 20, alli_neck[1] + 60, alli_neck[0] - 40, alli_neck[1] + 120, 255, 255, 0)
        dl(alli_neck[0], alli_neck[1], alli_neck[0] + 20, alli_neck[1] + 70, 85, 255, 0)
        dl(alli_neck[0] + 20, alli_neck[1] + 70, alli_neck[0] + 40, alli_neck[1] + 130, 0, 255, 0)
        
    # Back Legs
    if phase == "dance":
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] - 30, alli_midhip[1] + 80, 0, 255, 255)
        dl(alli_midhip[0] - 30, alli_midhip[1] + 80, alli_midhip[0] - 50, alli_midhip[1] + 160, 0, 170, 255)
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] + 30, alli_midhip[1] + 80, 0, 0, 255)
        dl(alli_midhip[0] + 30, alli_midhip[1] + 80, alli_midhip[0] + 50, alli_midhip[1] + 160, 85, 0, 255)
    elif phase == "roll":
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] - 40, alli_midhip[1] - 80, 0, 255, 255)
        dl(alli_midhip[0] - 40, alli_midhip[1] - 80, alli_midhip[0] - 50, alli_midhip[1] - 140, 0, 170, 255)
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] + 40, alli_midhip[1] - 70, 0, 0, 255)
        dl(alli_midhip[0] + 40, alli_midhip[1] - 70, alli_midhip[0] + 50, alli_midhip[1] - 130, 85, 0, 255)
    else:
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] - 20, alli_midhip[1] + 60, 0, 255, 255)
        dl(alli_midhip[0] - 20, alli_midhip[1] + 60, alli_midhip[0] - 40, alli_midhip[1] + 120, 0, 170, 255)
        dl(alli_midhip[0], alli_midhip[1], alli_midhip[0] + 20, alli_midhip[1] + 70, 0, 0, 255)
        dl(alli_midhip[0] + 20, alli_midhip[1] + 70, alli_midhip[0] + 40, alli_midhip[1] + 130, 85, 0, 255)

    # --- 3. The Dream Spider (Above/Around) ---
    if spider_x_offset == 0 and spider_y_offset == 0:
        sx = alli_nose[0] + 20
        sy = alli_nose[1] - 80 + int(anim_wave * 10)
    else:
        sx = ax + spider_x_offset
        sy = ay + spider_y_offset

    spider_nose = (sx, sy - 25)
    spider_neck = (sx, sy)
    spider_midhip = (sx, sy + 30)
    
    dl(spider_nose[0], spider_nose[1], spider_neck[0], spider_neck[1], 255, 0, 0)
    dl(spider_neck[0], spider_neck[1], spider_midhip[0], spider_midhip[1], 0, 255, 170)
    
    leg_spread = 50 + anim_wave * 10
    if phase == "dance": leg_spread = 80 + math.sin(t*8)*30
    elif phase == "pounce": leg_spread = 120
    elif phase == "roll": leg_spread = 20
    
    dl(spider_neck[0], spider_neck[1], sx - leg_spread*0.5, sy - 30, 85, 255, 0)
    dl(sx - leg_spread*0.5, sy - 30, sx - leg_spread, sy - 10, 0, 255, 0)
    dl(spider_neck[0], spider_neck[1], sx - leg_spread*0.8, sy, 85, 255, 0)
    dl(sx - leg_spread*0.8, sy, sx - leg_spread - 10, sy + 20, 0, 255, 0)
    dl(spider_midhip[0], spider_midhip[1], sx - leg_spread*0.7, sy + 30, 0, 0, 255)
    dl(sx - leg_spread*0.7, sy + 30, sx - leg_spread, sy + 50, 85, 0, 255)
    dl(spider_midhip[0], spider_midhip[1], sx - leg_spread*0.3, sy + 40, 0, 0, 255)
    dl(sx - leg_spread*0.3, sy + 40, sx - leg_spread*0.1, sy + 70, 85, 0, 255)

    dl(spider_neck[0], spider_neck[1], sx + leg_spread*0.5, sy - 30, 255, 170, 0)
    dl(sx + leg_spread*0.5, sy - 30, sx + leg_spread, sy - 10, 255, 255, 0)
    dl(spider_neck[0], spider_neck[1], sx + leg_spread*0.8, sy, 255, 170, 0)
    dl(sx + leg_spread*0.8, sy, sx + leg_spread + 10, sy + 20, 255, 255, 0)
    dl(spider_midhip[0], spider_midhip[1], sx + leg_spread*0.7, sy + 30, 0, 255, 255)
    dl(sx + leg_spread*0.7, sy + 30, sx + leg_spread, sy + 50, 0, 170, 255)
    dl(spider_midhip[0], spider_midhip[1], sx + leg_spread*0.3, sy + 40, 0, 255, 255)
    dl(sx + leg_spread*0.3, sy + 40, sx + leg_spread*0.1, sy + 70, 0, 170, 255)

    return img

def init_ffmpeg_rtmp(stream_key):
    # Strip the backup parameter if the frontend sends it, we handle the URL structure explicitly here.
    stream_key = stream_key.strip().split("?")[0]
    print(f"\n[INFO] Engaging Global FFmpeg RTMP Muxer (30 FPS)...")
    rtmp_url = f"rtmp://b.rtmp.youtube.com/live2/{stream_key}?backup=1"
    cmd = [
        "ffmpeg", "-y", 
        "-thread_queue_size", "1024", 
        "-f", "rawvideo", "-vcodec", "rawvideo",
        "-s", "512x1024", "-pix_fmt", "rgb24", "-framerate", "30", 
        "-i", "-", 
        "-f", "lavfi", "-i", "sine=frequency=40:sample_rate=44100", 
        "-vf", "scale=1080:1920:force_original_aspect_ratio=decrease,pad=1080:1920:-1:-1:color=black",
        "-c:v", "libx264", "-preset", "ultrafast", "-tune", "zerolatency", 
        "-b:v", "6800k", "-maxrate", "6800k", "-minrate", "6800k", "-bufsize", "13600k", "-nal-hrd", "cbr",
        "-pix_fmt", "yuv420p", 
        "-r", "30", 
        "-g", "60", 
        "-c:a", "aac", "-b:a", "128k", 
        "-f", "flv", rtmp_url
    ]
    return subprocess.Popen(cmd, stdin=subprocess.PIPE)

def generation_thread():
    global global_frame_idx, current_genome, latest_frame_bytes, latest_jpeg_bytes
    
    bear_types = [
        "a soft stuffed animal collection featuring a green alligator plush, a fuzzy spider plush, and a brown teddy bear",
        "a group of three cute plushies: a leather alligator, a black widow spider, and a teddy bear",
        "a swamp alligator plush, a friendly spider plush, and a classic teddy bear",
        "a large alligator plush toy, a pink spider plush toy, and a vintage teddy bear"
    ]
    
    postures = ["sit", "roar", "swim", "dance", "roll", "pounce"]
    current_posture = "sit"

    while True:
        try:
            # Epoch Settings: 60 frames for a long, consistent interaction scene.
            FRAMES_PER_EPOCH = 60
            
            if global_frame_idx >= FRAMES_PER_EPOCH:
                global_frame_idx = 0
                current_genome = random.choice(bear_types)
                current_posture = random.choice(postures)
                # No sleep. Seamless transition to the next entity.
                continue

            # Lock seed per epoch so the plushies remain strictly coherent and only animate via skeleton
            base_epoch_seed = 42 + hash(current_genome + current_posture) % 10000
            divergent_seed = str(base_epoch_seed)
            
            phase = current_posture
            local_f_idx = global_frame_idx
            
            # 1. Generate OpenPose Skeleton Math
            skeleton = build_openpose_skeleton(local_f_idx, phase)
            mask_path = f"assets/mpx_skel.png"
            skeleton.save(mask_path)
            
            decay_ratio = local_f_idx / float(FRAMES_PER_EPOCH - 1)
            
            defect_string = ""
            if decay_ratio < 0.15:
                defect_string = "pristine condition, very clean plush, solid seams"
            elif decay_ratio < 0.35:
                defect_string = "slight dust accumulation, slightly matted fur, loved condition"
            elif decay_ratio < 0.55:
                defect_string = "worn out patches of fur, vintage feel, loved plush"
            elif decay_ratio < 0.75:
                defect_string = "worn fabric, slightly torn, heavily loved vintage plush toys"
            elif decay_ratio < 0.90:
                defect_string = "antique plush toys, exposed stuffing in some places, very old and loved"
            else:
                defect_string = "very old worn out vintage plush toys, missing button eyes, heavily patched"
            
            # Since we removed img2img, we can loosen control net slightly over time for dynamic bounce
            dynamic_cfg = 1.2 + (decay_ratio * 0.8)
            dynamic_control = max(0.85, 0.95 - (decay_ratio * 0.10)) 

            action_text = "sitting closely together"
            if phase != "sit": action_text = f"in a {phase} posture"

            prompt = f"A photorealistic macro photography shot of strictly non-human stuffed animals, NO HUMANS, NO ANTHROPOMORPHISM, just three plush toys interacting, {current_genome} {action_text}, {defect_string}, professional softbox studio lighting, bright clean photography, well-lit environment, neutral background, highly detailed plush fabric textures, masterpiece, 8k resolution, cute, friendly <lora:lcm_lora_sd15:1.0>"

            out_path = "assets/mpx_out.png"

            # THE ALLIGATOR CRITIC LOOP
            # Maximum 3 attempts to generate a structurally sound frame
            for attempt in range(3):
                cmd = [
                    "assets/sd_cpp/build/bin/sd-cli",
                    "-m", "assets/models/sd15.safetensors",
                    "--vae", "assets/models/taesd.safetensors",
                    "--lora-model-dir", "assets/models",
                    "--control-net", "assets/models/control_openpose.safetensors",
                    "--control-image", mask_path,
                    "--control-strength", f"{dynamic_control:.2f}", 
                    "-p", prompt,
                    "-n", "human, humanoid, person, man, woman, child, people, boy, girl, human face, human hands, human fingers, human limbs, human skin, human body, realistic animals, flesh, standing upright like a human, wearing human clothes, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",
                    "-W", "512", "-H", "512",
                    "--steps", "4", 
                    "--threads", "4",
                    "--cfg-scale", f"{dynamic_cfg:.1f}", 
                    "--seed", divergent_seed, 
                    "-o", out_path
                ]
                
                subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                time.sleep(0.5) # GPU throttle
                
                # 1. The Alligator evaluates the physical Form
                passed, reason, advice = run_vision_critic(out_path, mask_path, decay_ratio)
                
                # 2. The Spider evaluates the Content Textures
                spider_advice = "NONE"
                if passed:
                    spider_advice = run_spider_critic(out_path, prompt)
                    if spider_advice != "NONE":
                        passed = False
                        reason = f"SPIDER_REJECT: {spider_advice}"
                        advice = spider_advice

                if passed:
                    break # Frame is physically valid and texturally sound
                    
                print(f"[CRITIC BITE] Frame {local_f_idx} failed: {reason}. Retrying (Attempt {attempt+1}/3). Action: {advice}")
                
                # Apply Dynamic Self-Improvement based on Critic feedback
                if advice == "INCREASE_CONTROL":
                    dynamic_control = min(1.0, dynamic_control + 0.15)
                elif advice == "SCRAMBLE_SEED":
                    divergent_seed = str(int(divergent_seed) + 777)
                elif advice == "INCREASE_STEPS":
                    cmd[cmd.index("--steps")+1] = "6"
                elif advice == "INCREASE_CFG":
                    dynamic_cfg += 0.5
                elif advice == "INCREASE_LIGHTING":
                    # DeepSeek-style prompt injection for immediate correction
                    prompt = "EXTREMELY BRIGHT BLINDING SOFTBOX LIGHTING, OVEREXPOSED, " + prompt
                elif advice == "EMPHASIZE_GATOR":
                    prompt = "MASSIVE BRIGHT GREEN ALLIGATOR PLUSH, " + prompt
                elif advice == "EMPHASIZE_BEAR":
                    prompt = "MASSIVE BROWN TEDDY BEAR PLUSH, " + prompt
                elif advice == "HUMAN_CONTAMINATION":
                    # Scramble the seed completely and drop controlnet strength slightly so the text prompt can overpower the human skeleton shape
                    divergent_seed = str(int(divergent_seed) + 12345)
                    dynamic_control = max(0.5, dynamic_control - 0.1)
            
            painter_img = Image.open(out_path).convert("RGB")
            
            global genie_last_observation_time
            current_time = time.time()
            if GENIE_ENABLED and (current_time - genie_last_observation_time > 4.0) and local_f_idx > 5:
                genie_last_observation_time = current_time
                obs_img = painter_img.copy().resize((256, 256))
                threading.Thread(target=genie_observation_thread, args=(obs_img,), daemon=True).start()
                
            # Multiplexing
            mpx_img = Image.new("RGB", (512, 1024))
            mpx_img.paste(skeleton, (0, 0)) 
            mpx_img.paste(painter_img, (0, 512)) 
            
            jpeg_io = io.BytesIO()
            mpx_img.save(jpeg_io, format="JPEG", quality=80)
            
            latest_jpeg_bytes = jpeg_io.getvalue()
            latest_frame_bytes = mpx_img.tobytes()
            
            global_frame_idx += 1
                
        except Exception as e:
            print(f"[GENERATION LOOP ERROR] {e}")
            time.sleep(1)

# Start the dedicated background generation loop independent of any HTTP connections
threading.Thread(target=generation_thread, daemon=True).start()

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass

class MultiChannelHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        global ffmpeg_process
        if self.path == '/rtmp_init':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                stream_key = data.get("key", "").strip()
                if stream_key:
                    if ffmpeg_process is None:
                        ffmpeg_process = init_ffmpeg_rtmp(stream_key)
                    self.send_response(200)
                    self.end_headers()
                    self.wfile.write(b"RTMP Link Established")
                else:
                    self.send_response(400)
                    self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()
        elif self.path == '/puppet_ballet':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                global ballet_keyframes, ballet_active, ballet_current_step, ballet_frame_counter
                
                raw_keyframes = data.get("keyframes", [])
                
                # We need to mathematically interpolate these keyframes across a 15-step ballet.
                # If they gave us 3 keyframes, we generate 15 total frames moving smoothly between them.
                ballet_keyframes = []
                TOTAL_BALLET_STEPS = 15
                segments = len(raw_keyframes) - 1
                steps_per_segment = TOTAL_BALLET_STEPS // segments
                
                for i in range(segments):
                    p1 = raw_keyframes[i]
                    p2 = raw_keyframes[i+1]
                    for step in range(steps_per_segment):
                        # Linear interpolation for now, could be upgraded to Spline/Sine easing
                        t = step / float(steps_per_segment)
                        interp_x = p1['x'] + (p2['x'] - p1['x']) * t
                        interp_y = p1['y'] + (p2['y'] - p1['y']) * t
                        ballet_keyframes.append({'x': interp_x, 'y': interp_y})
                
                # Ensure the final keyframe is perfectly locked
                ballet_keyframes.append(raw_keyframes[-1])
                
                if len(ballet_keyframes) > 0:
                    ballet_active = True
                    ballet_current_step = 0
                    ballet_frame_counter = 0
                
                self.send_response(200)
                self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()

    def do_GET(self):
        global global_frame_idx, current_genome, ffmpeg_process, latest_jpeg_bytes
        
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """
            <html>
            <head>
                <title>TSFi System0: Spider & Alligator Channel</title>
                <style>
                    body { background-color: #050505; color: #0f0; font-family: monospace; text-align: center; margin-top: 20px; }
                    .compositor { position: relative; width: 512px; height: 512px; margin: 0 auto; border: 2px solid #0f0; box-shadow: 0 0 30px #0f0; overflow: hidden; }
                    canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
                    #telemetry { margin-top: 10px; color: #ff0; font-size: 1.2em; font-weight: bold; }
                    .control-panel { margin-top: 20px; padding: 20px; border: 1px solid #333; display: inline-block; background: #111; }
                    input[type="password"] { padding: 10px; font-family: monospace; width: 300px; background: #000; color: #0f0; border: 1px solid #0f0; }
                    button { padding: 10px 20px; background: #f00; color: #fff; font-family: monospace; font-weight: bold; cursor: pointer; border: none; }
                    button:hover { background: #ff3333; }
                </style>
            </head>
            <body>
                <h1>TSFi System0: Spider & Alligator (Grade 1 Truth)</h1>
                <div class="compositor">
                    <canvas id="painterCanvas" width="512" height="512" style="z-index: 1;"></canvas>
                    <canvas id="scribbleCanvas" width="512" height="512" style="z-index: 2; cursor: crosshair;"></canvas>
                </div>
                <div style="margin-top:10px;">
                    <button onclick="clearScribble()">CLEAR POSE</button>
                    <button onclick="saveKeyframe()">SAVE KEYFRAME (<span id="kfCount">0</span>/15)</button>
                    <button onclick="sendBalletToPuppet()" style="background:#800080;">EXECUTE 15-STEP BALLET</button>
                </div>
                <div id="telemetry">Initializing Neural Lifecycle...</div>
                
                <div class="control-panel">
                    <h3>Global VTUBE Broadcast Matrix</h3>
                    <input type="password" id="rtmpKey" placeholder="Paste YouTube RTMP Stream Key">
                    <button onclick="goLive()">GO LIVE ON YOUTUBE</button>
                    <p id="rtmpStatus" style="color:#aaa;"></p>
                </div>
                
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
                    
                    // --- THE SCRIBBLE BRUSH LOGIC ---
                    const scribbleCanvas = document.getElementById('scribbleCanvas');
                    const sCtx = scribbleCanvas.getContext('2d');
                    sCtx.lineCap = 'round';
                    sCtx.lineJoin = 'round';
                    sCtx.lineWidth = 4;
                    sCtx.strokeStyle = '#0f0'; // Neon Green matrix ink
                    
                    let isPainting = false;
                    let lastX = 0;
                    let lastY = 0;
                    
                    scribbleCanvas.addEventListener('mousedown', (e) => {
                        isPainting = true;
                        const rect = scribbleCanvas.getBoundingClientRect();
                        lastX = e.clientX - rect.left;
                        lastY = e.clientY - rect.top;
                    });
                    
                    scribbleCanvas.addEventListener('mousemove', (e) => {
                        if(!isPainting) return;
                        const rect = scribbleCanvas.getBoundingClientRect();
                        const currentX = e.clientX - rect.left;
                        const currentY = e.clientY - rect.top;
                        
                        sCtx.beginPath();
                        sCtx.moveTo(lastX, lastY);
                        sCtx.lineTo(currentX, currentY);
                        sCtx.stroke();
                        
                        lastX = currentX;
                        lastY = currentY;
                    });
                    
                    let pathData = [];
                    
                    scribbleCanvas.addEventListener('mousedown', (e) => {
                        isPainting = true;
                        pathData = []; // Start a new path
                        const rect = scribbleCanvas.getBoundingClientRect();
                        lastX = e.clientX - rect.left;
                        lastY = e.clientY - rect.top;
                        pathData.push({x: lastX, y: lastY});
                    });

                    scribbleCanvas.addEventListener('mousemove', (e) => {
                        if(!isPainting) return;
                        const rect = scribbleCanvas.getBoundingClientRect();
                        const currentX = e.clientX - rect.left;
                        const currentY = e.clientY - rect.top;
                        
                        sCtx.beginPath();
                        sCtx.moveTo(lastX, lastY);
                        sCtx.lineTo(currentX, currentY);
                        sCtx.stroke();
                        
                        lastX = currentX;
                        lastY = currentY;
                        pathData.push({x: lastX, y: lastY});
                    });

                    scribbleCanvas.addEventListener('mouseup', () => isPainting = false);
                    scribbleCanvas.addEventListener('mouseleave', () => isPainting = false);
                    
                    window.clearScribble = function() {
                        sCtx.clearRect(0, 0, 512, 512);
                        pathData = [];
                    };
                    
                    let keyframes = [];
                    
                    window.saveKeyframe = function() {
                        if(pathData.length === 0) return;
                        // Grab the very last coordinate drawn as the definitive IK target for this pose
                        const target = pathData[pathData.length - 1];
                        keyframes.push(target);
                        document.getElementById('kfCount').innerText = keyframes.length;
                        
                        // Draw a permanent red dot on the UI to signify the keyframe lock
                        sCtx.fillStyle = 'red';
                        sCtx.beginPath();
                        sCtx.arc(target.x, target.y, 8, 0, Math.PI*2);
                        sCtx.fill();
                        
                        pathData = []; // Reset for next stroke
                    };

                    window.sendBalletToPuppet = function() {
                        if(keyframes.length < 2) {
                            alert("A ballet requires at least 2 keyframes to interpolate.");
                            return;
                        }
                        
                        fetch('/puppet_ballet', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ keyframes: keyframes })
                        }).then(r => {
                            if(r.ok) {
                                document.getElementById('telemetry').innerText = "Ballet Choreography Uploaded! Executing 15-Step Interpolation...";
                                sCtx.clearRect(0, 0, 512, 512);
                                keyframes = [];
                                document.getElementById('kfCount').innerText = "0";
                            }
                        });
                    };

                    
                    setInterval(() => {
                        fetch('/telemetry').then(r => r.text()).then(t => {
                            document.getElementById('telemetry').innerHTML = t;
                        });
                    }, 1000);
                    
                    function goLive() {
                        const key = document.getElementById('rtmpKey').value;
                        if(!key) { alert("Matrix demands a Stream Key."); return; }
                        document.getElementById('rtmpStatus').innerText = "Initiating RTMP Handshake...";
                        fetch('/rtmp_init', {
                            method: 'POST', headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ key: key })
                        }).then(r => {
                            if(r.ok) { document.getElementById('rtmpStatus').innerText = "BROADCAST LIVE. YouTube Ingesting..."; document.getElementById('rtmpStatus').style.color = '#0f0'; }
                            else { document.getElementById('rtmpStatus').innerText = "RTMP Fracture. Check console."; document.getElementById('rtmpStatus').style.color = '#f00'; }
                        });
                    }
                </script>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif self.path == '/telemetry':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            
            phase_str = "Sitting (Decay)"
            if global_frame_idx > 150: phase_str = "Standing Up & Walking"
            
            # Read from SHM if available
            alli_score = "WAITING"
            spider_score = "WAITING"
            ds_obs = "WAITING"
            genie_obs = "WAITING"
            
            if shm_reg is not None:
                alli_score = f"{shm_reg.alligator_form_ratio:.2f} (0.05-2.0)"
                spider_score = f"{shm_reg.spider_texture_sharpness:.2f} (>50.0)"
                ds_obs = shm_reg.agent_monologue.decode('utf-8', 'ignore').strip()
                genie_obs = shm_reg.genie_poetic_observation.decode('utf-8', 'ignore').strip()
            
            html_telem = f"""
            <div style="text-align: left; font-size: 0.9em; margin-top: 15px; border: 1px solid #333; padding: 10px; background: #0a0a0a;">
                <h3 style="color:#ff0; margin-top:0;">GRADE 1 AUDIT: ACOUSTIC PROOF SPACE</h3>
                <p><b>[GENIE]:</b> {genie_obs}</p>
                <p><b>[DEEPSEEK]:</b> {ds_obs}</p>
                <p><b>[ALLIGATOR]:</b> Form IoU Mass: {alli_score}</p>
                <p><b>[SPIDER]:</b> Texture Variance: {spider_score}</p>
                <hr style="border-color: #333;">
                <p style="color:#0ff;"><b>[SYSTEM]:</b> Epoch {global_frame_idx}/60 | Phase: {phase_str}</p>
            </div>
            """
            
            try:
                self.wfile.write(html_telem.encode('utf-8'))
            except BrokenPipeError:
                pass

        elif self.path == '/multiplex_stream':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            
            last_idx = -1
            while True:
                try:
                    # Only send down the wire if a new frame has been generated by the background thread
                    if latest_jpeg_bytes is not None and global_frame_idx != last_idx:
                        self.wfile.write(b'--FRAME\r\n')
                        self.send_header('Content-Type', 'image/jpeg')
                        self.send_header('Content-Length', str(len(latest_jpeg_bytes)))
                        self.end_headers()
                        self.wfile.write(latest_jpeg_bytes)
                        self.wfile.write(b'\r\n')
                        last_idx = global_frame_idx
                    else:
                        time.sleep(0.05)
                        
                except (ConnectionResetError, BrokenPipeError):
                    print("[INFO] Web UI disconnected.")
                    break
                except Exception as e:
                    print(f"[FRACTURE] Stream broke: {e}")
                    time.sleep(2)
        else:
            self.send_error(404)

if __name__ == '__main__':
    os.makedirs("assets", exist_ok=True)
    server = ThreadedHTTPServer(('0.0.0.0', 9092), MultiChannelHandler)
    print("=== TSFi Spider & Alligator Channel Active ===")
    print("-> Open a web browser to http://127.0.0.1:9092")
    server.serve_forever()
