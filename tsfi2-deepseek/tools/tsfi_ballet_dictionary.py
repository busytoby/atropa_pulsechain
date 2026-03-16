import os
import math
from PIL import Image, ImageDraw

def render_ballet_posture(position_name, W=512, H=512):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Base Anchors
    cx = W // 2
    cy = int(H * 0.6) # Hips slightly higher to allow leg extension
    
    torso_len = 100
    arm_len = 70
    leg_len = 90
    
    # Spine & Head
    neck = (cx, cy - torso_len)
    nose = (cx, neck[1] - 40)
    
    # Shoulders & Hips (Fixed core)
    r_shoulder = (neck[0] - 35, neck[1])
    l_shoulder = (neck[0] + 35, neck[1])
    r_hip = (cx - 25, cy)
    l_hip = (cx + 25, cy)
    
    # Kinematic Variables (Angles in degrees, 0 is straight down, 90 is straight right, -90 is straight left)
    # Default T-Pose variables:
    r_arm_ang = 0
    r_elbow_ang = 0
    l_arm_ang = 0
    l_elbow_ang = 0
    
    r_leg_ang = 0
    r_knee_ang = 0
    l_leg_ang = 0
    l_knee_ang = 0
    
    if position_name == "First Position":
        # Arms curved down in front of navel
        r_arm_ang = -30; r_elbow_ang = 45
        l_arm_ang = 30; l_elbow_ang = -45
        # Heels touching, toes out
        r_leg_ang = -10; r_knee_ang = 0
        l_leg_ang = 10; l_knee_ang = 0
        
    elif position_name == "Second Position":
        # Arms extended to sides, slightly curved down
        r_arm_ang = -80; r_elbow_ang = 10
        l_arm_ang = 80; l_elbow_ang = -10
        # Wide stance
        r_leg_ang = -25; r_knee_ang = 0
        l_leg_ang = 25; l_knee_ang = 0
        
    elif position_name == "Third Position":
        # One arm curved down, one arm extended out
        r_arm_ang = -30; r_elbow_ang = 45 # Down
        l_arm_ang = 80; l_elbow_ang = -10 # Out
        # One foot slightly in front (offset)
        r_leg_ang = -10; r_knee_ang = 0
        l_leg_ang = 0; l_knee_ang = 0
        
    elif position_name == "Fourth Position":
        # One arm raised above head, one curved in front
        r_arm_ang = -160; r_elbow_ang = 45 # Raised
        l_arm_ang = 30; l_elbow_ang = -45  # Down
        # Legs simulating forward/backward depth (one bent slightly)
        r_leg_ang = -15; r_knee_ang = -10
        l_leg_ang = 15; l_knee_ang = 20
        
    elif position_name == "Fifth Position":
        # Both arms raised perfectly above head, hands nearly touching
        r_arm_ang = -150; r_elbow_ang = 45
        l_arm_ang = 150; l_elbow_ang = -45
        # Feet tightly crossed (ankles overlapping)
        r_leg_ang = -5; r_knee_ang = 10
        l_leg_ang = 5; l_knee_ang = -10

    # Mathematical Vector Calculation (Degrees to Radians)
    def calc_joint(origin, angle_deg, length):
        rad = math.radians(angle_deg + 90) # Offset so 0 is down
        return (origin[0] + int(math.cos(rad) * length), origin[1] + int(math.sin(rad) * length))
        
    # Arm Physics
    r_elbow = calc_joint(r_shoulder, r_arm_ang, arm_len)
    r_wrist = calc_joint(r_elbow, r_arm_ang + r_elbow_ang, arm_len)
    
    l_elbow = calc_joint(l_shoulder, l_arm_ang, arm_len)
    l_wrist = calc_joint(l_elbow, l_arm_ang + l_elbow_ang, arm_len)
    
    # Leg Physics
    r_knee = calc_joint(r_hip, r_leg_ang, leg_len)
    r_ankle = calc_joint(r_knee, r_leg_ang + r_knee_ang, leg_len)
    
    l_knee = calc_joint(l_hip, l_leg_ang, leg_len)
    l_ankle = calc_joint(l_knee, l_leg_ang + l_knee_ang, leg_len)
    
    # ---------------------------------------------------------
    # Draw OpenPose Standard Colors (Thickness ~8)
    # ---------------------------------------------------------
    th = 8
    
    # Spine & Head
    draw.line([neck, (cx, cy)], fill=(255, 0, 0), width=th)
    draw.line([neck, nose], fill=(255, 0, 0), width=th)
    
    # Right Arm
    draw.line([neck, r_shoulder], fill=(255, 85, 0), width=th)
    draw.line([r_shoulder, r_elbow], fill=(255, 170, 0), width=th)
    draw.line([r_elbow, r_wrist], fill=(255, 255, 0), width=th)
    
    # Left Arm
    draw.line([neck, l_shoulder], fill=(85, 255, 0), width=th)
    draw.line([l_shoulder, l_elbow], fill=(0, 255, 0), width=th)
    draw.line([l_elbow, l_wrist], fill=(0, 255, 85), width=th)
    
    # Right Leg
    draw.line([(cx, cy), r_hip], fill=(0, 255, 170), width=th)
    draw.line([r_hip, r_knee], fill=(0, 85, 255), width=th)
    draw.line([r_knee, r_ankle], fill=(0, 0, 255), width=th)
    
    # Left Leg
    draw.line([(cx, cy), l_hip], fill=(85, 0, 255), width=th)
    draw.line([l_hip, l_knee], fill=(170, 0, 255), width=th)
    draw.line([l_knee, l_ankle], fill=(255, 0, 255), width=th)
    
    # Joints
    nodes = [nose, neck, r_shoulder, r_elbow, r_wrist, l_shoulder, l_elbow, l_wrist, r_hip, r_knee, r_ankle, l_hip, l_knee, l_ankle]
    for nx, ny in nodes:
        draw.ellipse([nx-4, ny-4, nx+4, ny+4], fill=(255, 255, 255))
        
    return img

def execute_dictionary_build():
    print("=== TSFi Kinematic Ballet Dictionary (OpenPose Structs) ===")
    out_dir = "assets/ballet_dictionary"
    os.makedirs(out_dir, exist_ok=True)
    
    postures = [
        "First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position"
    ]
    
    for posture in postures:
        img = render_ballet_posture(posture)
        file_name = posture.replace(" ", "_").lower() + ".png"
        img.save(f"{out_dir}/{file_name}")
        print(f"  -> Physically locked Kinematic Vectors for: {posture}")
        
    print(f"\\n[ABSOLUTE SUCCESS] The Five Classical Ballet Matrices are compiled in {out_dir}/")

if __name__ == "__main__":
    execute_dictionary_build()
