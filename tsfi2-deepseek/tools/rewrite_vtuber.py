import re
with open("tools/tsfi_spider_alligator_vtuber.py", "r") as f:
    content = f.read()

new_skeleton = """def build_openpose_skeleton(f_idx, phase="sit"):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
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

    th = 8
    def dl(x0, y0, x1, y1, r, g, b):
        draw.line([(x0, y0), (x1, y1)], fill=(r, g, b), width=th)
        draw.ellipse([x0 - th//2, y0 - th//2, x0 + th//2, y0 + th//2], fill=(r, g, b))
        draw.ellipse([x1 - th//2, y1 - th//2, x1 + th//2, y1 + th//2], fill=(r, g, b))

    # --- 1. The Sick Teddy Bear (Left) ---
    bear_cx = bx
    bear_cy = by + bear_y_offset
    torso_len = 80
    arm_len = 60
    leg_len = 70
    
    neck = (bear_cx, bear_cy - torso_len)
    nose = (bear_cx, neck[1] - 30)
    r_eye = (bear_cx - 15, neck[1] - 45)
    l_eye = (bear_cx + 15, neck[1] - 45)
    r_ear = (bear_cx - 30, neck[1] - 55)
    l_ear = (bear_cx + 30, neck[1] - 55)
    
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
    
    dl(neck[0], neck[1], bear_cx, bear_cy, 255, 0, 0)
    dl(neck[0], neck[1], nose[0], nose[1], 255, 0, 0)
    dl(nose[0], nose[1], r_eye[0], r_eye[1], 255, 0, 255)
    dl(nose[0], nose[1], l_eye[0], l_eye[1], 255, 0, 255)
    dl(r_eye[0], r_eye[1], r_ear[0], r_ear[1], 255, 255, 0)
    dl(l_eye[0], l_eye[1], l_ear[0], l_ear[1], 255, 255, 0)
    
    dl(neck[0], neck[1], r_shoulder[0], r_shoulder[1], 255, 85, 0)
    dl(r_shoulder[0], r_shoulder[1], r_elbow[0], r_elbow[1], 255, 170, 0)
    dl(r_elbow[0], r_elbow[1], r_wrist[0], r_wrist[1], 255, 255, 0)
    dl(neck[0], neck[1], l_shoulder[0], l_shoulder[1], 85, 255, 0)
    dl(l_shoulder[0], l_shoulder[1], l_elbow[0], l_elbow[1], 0, 255, 0)
    dl(l_elbow[0], l_elbow[1], l_wrist[0], l_wrist[1], 0, 255, 85)
    
    dl(bear_cx, bear_cy, r_hip[0], r_hip[1], 0, 255, 170)
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
"""

content = re.sub(r'def build_openpose_skeleton.*?return img\n', new_skeleton, content, flags=re.DOTALL)

old_bear_types = """            bear_types = [
                "a sick dusty green alligator plush toy and a tiny sick fuzzy spider plush toy",
                "an old worn leather alligator plush and a dust covered black widow spider plush",
                "a weary swamp alligator plush and an exhausted neon spider plush",
                "a sick overgrown alligator plush toy and a tired pink spider plush toy"
            ]"""

new_bear_types = """            bear_types = [
                "a sick stuffed animal collection featuring a dusty green alligator plush, a tiny fuzzy spider plush, and a brown teddy bear",
                "a group of three sick plushies: a worn leather alligator, a black widow spider, and a tired teddy bear",
                "a weary swamp alligator plush, an exhausted neon spider plush, and a sick teddy bear",
                "a sick overgrown alligator plush toy, a tired pink spider plush toy, and an old teddy bear"
            ]"""

content = content.replace(old_bear_types, new_bear_types)

old_action_prompt = """                    action_text = "sitting closely together"
                    if phase != "sit": action_text = f"in a {phase} posture"
                    
                    # Aligning with Megalopolis Sick Teddy Bear Lore, injecting progressive damage
                    prompt = f"A photorealistic macro photography shot of two stuffed plush toys, {current_genome} {action_text}, {defect_string}, sick from eating dust and drinking swamp water, clear lucid glowing Kr0wZ green eyes, sitting in a dystopian other-worldly city, obstructive metal structured buildings, violet neon and green lighting, cracked streets, wet moldy pedestrian promenade, cobwebs everywhere, masterpiece, 8k resolution, cinematic lighting, highly detailed anatomy"
"""

new_action_prompt = """                    action_text = "sitting closely together"
                    if phase != "sit": action_text = f"in a {phase} posture"
                    
                    # Aligning with Megalopolis Sick Teddy Bear Lore, injecting progressive damage
                    prompt = f"A photorealistic macro photography shot of three stuffed plush toys interacting, {current_genome} {action_text}, {defect_string}, sick from eating dust and drinking swamp water, clear lucid glowing Kr0wZ green eyes, sitting in a dystopian other-worldly city, obstructive metal structured buildings, violet neon and green lighting, cracked streets, wet moldy pedestrian promenade, cobwebs everywhere, masterpiece, 8k resolution, cinematic lighting, highly detailed anatomy"
"""

content = content.replace(old_action_prompt, new_action_prompt)

with open("tools/tsfi_spider_alligator_vtuber.py", "w") as f:
    f.write(content)

