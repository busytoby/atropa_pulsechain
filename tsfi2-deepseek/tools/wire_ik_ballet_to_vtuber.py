import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

ik_solver_code = """
import math

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
    # Anchor the skeleton
    hip = (center_x, center_y + 80)
    neck = (center_x, center_y - 80)
    
    L_thigh = 70
    L_calf = 80
    L_upperarm = 60
    L_forearm = 70

    # Choreography: Grand Battement (Right leg sweeps up)
    sweep_angle = math.pi * 0.5 * math.sin(t * math.pi) 
    
    l_foot_target = (hip[0] - 30, hip[1] + 130) # En pointe
    r_foot_target_x = hip[0] + 40 + math.sin(sweep_angle) * 120
    r_foot_target_y = hip[1] + 130 - math.sin(sweep_angle) * 200
    
    # Port de bras (Arms lift)
    arm_lift = math.sin(t * math.pi) * 80
    l_hand_target = (neck[0] - 100 + arm_lift, neck[1] + 50 - arm_lift)
    r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)

    # Solve IK
    l_knee = solve_2d_ik_2bone(l_foot_target[0], l_foot_target[1], hip[0]-20, hip[1], L_thigh, L_calf)
    r_knee = solve_2d_ik_2bone(r_foot_target_x, r_foot_target_y, hip[0]+20, hip[1], L_thigh, L_calf)
    l_elbow = solve_2d_ik_2bone(l_hand_target[0], l_hand_target[1], neck[0]-30, neck[1], L_upperarm, L_forearm)
    r_elbow = solve_2d_ik_2bone(r_hand_target[0], r_hand_target[1], neck[0]+30, neck[1], L_upperarm, L_forearm)

    # Draw Matrix
    bundle_limb(d_ctx, hip, neck, rgb, th)
    bundle_limb(d_ctx, (hip[0]-20, hip[1]), l_knee, rgb, th)
    bundle_limb(d_ctx, l_knee, l_foot_target, rgb, th)
    bundle_limb(d_ctx, (hip[0]+20, hip[1]), r_knee, rgb, th)
    bundle_limb(d_ctx, r_knee, (r_foot_target_x, r_foot_target_y), rgb, th)
    bundle_limb(d_ctx, (neck[0]-30, neck[1]), l_elbow, rgb, th)
    bundle_limb(d_ctx, l_elbow, l_hand_target, rgb, th)
    bundle_limb(d_ctx, (neck[0]+30, neck[1]), r_elbow, rgb, th)
"""

code = code.replace(
    '    # --- TSFi BRUSH AND BUNDLE ARCHITECTURE ---',
    ik_solver_code + '\n    # --- TSFi BRUSH AND BUNDLE ARCHITECTURE ---'
)

# Replace the old procedural Teddy Bear with the IK Ballet Teddy Bear
old_teddy = """    # --- 1. The Sick Teddy Bear (Left) ---
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
    
    dl(neck[0], neck[1], bear_cx, bear_cy, 0, 255, 170)
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
    
    dl(bear_cx, bear_cy, r_hip[0], r_hip[1], 0, 170, 255)
    dl(r_hip[0], r_hip[1], r_knee[0], r_knee[1], 0, 85, 255)
    dl(r_knee[0], r_knee[1], r_ankle[0], r_ankle[1], 0, 0, 255)
    dl(bear_cx, bear_cy, l_hip[0], l_hip[1], 85, 0, 255)
    dl(l_hip[0], l_hip[1], l_knee[0], l_knee[1], 170, 0, 255)
    dl(l_knee[0], l_knee[1], l_ankle[0], l_ankle[1], 255, 0, 255)"""

new_teddy = """    # --- 1. The Sick Teddy Bear (Left) - IK BALLET BUNDLE ---
    if phase == "dance":
        # The Teddy Bear performs the full IK Ballet sequence over the 60-frame epoch
        bundle_ik_teddy_ballet(draw, bx, by + bear_y_offset, (f_idx % 60) / 60.0, (255, 85, 0), th)
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
        dl(l_knee[0], l_knee[1], l_ankle[0], l_ankle[1], 255, 0, 255)"""

code = code.replace(old_teddy, new_teddy)

with open(filepath, 'w') as f:
    f.write(code)

