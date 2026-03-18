import os
import math
from PIL import Image, ImageDraw

def render_crow_ballet_posture(position_name, W=1280, H=720):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Base Anchors
    cx = W // 2
    cy = int(H * 0.6)
    
    s = 6.0 # Scale
    
    # Defaults (Idle perching)
    neck_pos = (cx, cy - 20*s)
    head_top = (cx, cy - 35*s)
    body_mid = (cx, cy + 10*s)
    beak_base = (cx + 10*s, cy - 25*s)
    beak_tip = (cx + 25*s, cy - 25*s)
    r_eye = (cx + 5*s, cy - 30*s)
    
    l_shoulder = (cx - 10*s, cy - 5*s)
    r_shoulder = (cx + 10*s, cy - 5*s)
    l_wing_tip = (cx - 40*s, cy + 10*s)
    r_wing_tip = (cx + 40*s, cy + 10*s)
    
    l_hip = (cx - 8*s, cy + 25*s)
    r_hip = (cx + 8*s, cy + 25*s)
    l_foot = (cx - 12*s, cy + 60*s)
    r_foot = (cx + 12*s, cy + 60*s)
    
    tail_base = (cx - 15*s, cy + 30*s)
    tail_tip = (cx - 40*s, cy + 45*s)

    if position_name == "First Position":
        # Heels touching, wings curved in front
        l_foot = (cx - 5*s, cy + 60*s)
        r_foot = (cx + 5*s, cy + 60*s)
        l_wing_tip = (cx - 15*s, cy + 20*s)
        r_wing_tip = (cx + 15*s, cy + 20*s)
        
    elif position_name == "Second Position":
        # Wide stance, wings out
        l_foot = (cx - 40*s, cy + 60*s)
        r_foot = (cx + 40*s, cy + 60*s)
        l_wing_tip = (cx - 80*s, cy - 5*s)
        r_wing_tip = (cx + 80*s, cy - 5*s)
        
    elif position_name == "Third Position":
        # One foot in front, one wing out, one in front
        l_foot = (cx - 5*s, cy + 55*s)
        r_foot = (cx + 5*s, cy + 65*s)
        l_wing_tip = (cx - 15*s, cy + 20*s)
        r_wing_tip = (cx + 80*s, cy - 5*s)
        
    elif position_name == "Fourth Position":
        # One foot far in front, one wing up, one in front
        l_foot = (cx - 10*s, cy + 45*s)
        r_foot = (cx + 10*s, cy + 75*s)
        l_wing_tip = (cx - 15*s, cy + 20*s)
        r_wing_tip = (cx + 20*s, cy - 80*s) # Raised above head
        
    elif position_name == "Fifth Position":
        # Feet crossed, both wings up
        l_foot = (cx + 5*s, cy + 60*s)
        r_foot = (cx - 5*s, cy + 60*s)
        l_wing_tip = (cx - 20*s, cy - 80*s)
        r_wing_tip = (cx + 20*s, cy - 80*s)
        
    elif position_name == "Plier":
        # Deep squat, wings tucked
        cy += 20*s
        neck_pos = (cx, cy - 20*s)
        head_top = (cx, cy - 35*s)
        body_mid = (cx, cy + 10*s)
        l_foot = (cx - 30*s, cy + 40*s)
        r_foot = (cx + 30*s, cy + 40*s)
        l_wing_tip = (cx - 10*s, cy + 5*s)
        r_wing_tip = (cx + 10*s, cy + 5*s)
        
    elif position_name == "Etendre":
        # Stretch, wings wide and high
        cy -= 20*s
        neck_pos = (cx, cy - 20*s)
        head_top = (cx, cy - 45*s)
        l_wing_tip = (cx - 90*s, cy - 40*s)
        r_wing_tip = (cx + 90*s, cy - 40*s)
        
    elif position_name == "Relever":
        # On tips of claws
        l_foot = (cx - 12*s, cy + 75*s)
        r_foot = (cx + 12*s, cy + 75*s)
        l_wing_tip = (cx - 60*s, cy + 0*s)
        r_wing_tip = (cx + 60*s, cy + 0*s)
        
    elif position_name == "Reverence":
        # Deep bow
        head_top = (cx + 20*s, cy + 10*s)
        neck_pos = (cx + 10*s, cy + 0*s)
        beak_base = (cx + 30*s, cy + 20*s)
        beak_tip = (cx + 45*s, cy + 30*s)
        l_wing_tip = (cx - 50*s, cy - 30*s) # Swept back
        r_wing_tip = (cx + 10*s, cy + 40*s) # Tucked forward

    th = 8
    def draw_bone_local(p1, p2, color, thickness=th):
        draw.line([p1, p2], fill=tuple(color), width=thickness)

    # Draw OpenPose skeleton for Crow
    # Head & Face (Red)
    draw_bone_local(head_top, neck_pos, [255, 0, 0])
    draw_bone_local(head_top, beak_base, [255, 0, 0])
    # Beak (Magenta)
    draw_bone_local(beak_base, beak_tip, [255, 0, 255], 4)
    
    # Spine (Orange)
    draw_bone_local(neck_pos, body_mid, [255, 85, 0], th+2)
    
    # Left Wing (Green shades)
    draw_bone_local(neck_pos, l_shoulder, [170, 255, 0])
    draw_bone_local(l_shoulder, l_wing_tip, [0, 255, 0], th+1)
    
    # Right Wing (Yellow shades)
    draw_bone_local(neck_pos, r_shoulder, [85, 255, 0])
    draw_bone_local(r_shoulder, r_wing_tip, [0, 255, 85], th+1)
    
    # Left Leg (Purple shades)
    draw_bone_local(body_mid, l_hip, [0, 255, 170])
    draw_bone_local(l_hip, l_foot, [0, 170, 255], th-2)
    
    # Right Leg (Blue shades)
    draw_bone_local(body_mid, r_hip, [0, 255, 255])
    draw_bone_local(r_hip, r_foot, [0, 85, 255], th-2)
    
    # Tail (Yellowish)
    draw_bone_local(body_mid, tail_base, [255, 170, 0], th-1)
    draw_bone_local(tail_base, tail_tip, [255, 255, 0], th-1)

    # Joint nodes
    nodes = [head_top, neck_pos, body_mid, l_shoulder, r_shoulder, l_wing_tip, r_wing_tip, l_hip, r_hip, l_foot, r_foot, beak_tip]
    for nx, ny in nodes:
        draw.ellipse([nx-4, ny-4, nx+4, ny+4], fill=(255, 255, 255))
        
    return img

def execute_crow_dictionary_build():
    print("=== TSFi Kinematic Crow Ballet Dictionary (OpenPose) ===")
    out_dir = "assets/crow_ballet_dictionary"
    os.makedirs(out_dir, exist_ok=True)
    
    postures = [
        "First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position",
        "Plier", "Etendre", "Relever", "Reverence"
    ]
    
    for posture in postures:
        img = render_crow_ballet_posture(posture)
        file_name = posture.replace(" ", "_").lower() + ".png"
        img.save(f"{out_dir}/{file_name}")
        print(f"  -> Physical Vectors Locked for Crow: {posture}")
        
    print(f"\\n[ABSOLUTE SUCCESS] Crow Ballet Matrices compiled in {out_dir}/")

if __name__ == "__main__":
    execute_crow_dictionary_build()
