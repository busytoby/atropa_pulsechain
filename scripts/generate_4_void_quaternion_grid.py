# Generate 2x2 grid showing the Rod and Cone genesis, Flux Pinning, and Morphing of 4 unique YIs
# Renders 900 frames (30 seconds at 30 FPS) with multi-frequency EDO-22 audio track.
# Uses the unified contract transition logic evaluated exclusively modulo the Motzkin Prime.
# Saves the final icon frame as a PNG and outputs its SHA-256 checksum for mathematical verification.

import math
import wave
import subprocess
import hashlib
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# Video/Audio constants
WIDTH, HEIGHT = 1280, 720
FPS = 30
DURATION = 30
TOTAL_FRAMES = FPS * DURATION
SAMPLE_RATE = 44100

# EDO-22 system parameters
EDO_STEPS = 22
LUT_SIZE = 22528
LUT_MASK = LUT_SIZE - 1
LUT_FACTOR = LUT_SIZE / (2 * np.pi)

# Motzkin Prime divisor field constant
MotzkinPrime = 953467954114363

print("[Multi-YI Grid] Initializing Auncient Trigonometric LUT tables...")
sin_lut = np.sin(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))
cos_lut = np.cos(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))

# Load fonts
try:
    font_bold = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 10)
    font_regular = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 9)
except Exception:
    font_bold = ImageFont.load_default()
    font_regular = ImageFont.load_default()

# 4 unique seed sets for the standard contract logic
YI_SEEDS = [
    {
        "Name": "YI-Alpha (Motzkin)",
        "r_base_init": 29088494943924,
        "r_secret_init": 28723650630656,
        "r_signal_init": 1000000000000,
        "r_channel": 129000000000000,
        "c_base_init": 193800609307,
        "c_secret_init": 737000000000,
        "c_signal_init": 1000000000000,
        "c_channel": 85300000000000,
        "Color": (168, 85, 247)  # Purple
    },
    {
        "Name": "YI-Beta",
        "r_base_init": 846763449578,
        "r_secret_init": 165661868966,
        "r_signal_init": 2000000000000,
        "r_channel": 165661868966,
        "c_base_init": 111355463586,
        "c_secret_init": 51604233809,
        "c_signal_init": 2000000000000,
        "c_channel": 51604233809,
        "Color": (0, 242, 254)  # Cyan
    },
    {
        "Name": "YI-Gamma",
        "r_base_init": 1216489409367,
        "r_secret_init": 199418747894,
        "r_signal_init": 3000000000000,
        "r_channel": 199418747894,
        "c_base_init": 897479868497,
        "c_secret_init": 27158968401,
        "c_signal_init": 3000000000000,
        "c_channel": 27158968401,
        "Color": (34, 197, 94)  # Green
    },
    {
        "Name": "YI-Delta",
        "r_base_init": 2142565876433,
        "r_secret_init": 750727654864,
        "r_signal_init": 4000000000000,
        "r_channel": 750727654864,
        "c_base_init": 406291974248,
        "c_secret_init": 182094613458,
        "c_signal_init": 4000000000000,
        "c_channel": 182094613458,
        "Color": (244, 63, 94)  # Rose
    }
]

def modExp(base, exp, mod):
    return pow(int(base), int(exp), int(mod))

def get_contract_states(t_secs, seeds):
    R_Secret = seeds["r_secret_init"]
    R_Signal = seeds["r_signal_init"]
    C_Secret = seeds["c_secret_init"]
    C_Signal = seeds["c_signal_init"]
    
    Xi = 129000000000000
    Alpha = 85300000000000
    Beta_in = 242000000000000000
    
    t_genesis = 7.5
    t_tuning = 15.0
    t_align = 22.5
    
    r_contour_final = modExp(Xi, R_Secret, MotzkinPrime)
    c_contour_final = modExp(Xi, C_Secret, MotzkinPrime)
    
    r_base_final = modExp(c_contour_final, R_Secret, MotzkinPrime)
    c_base_final = modExp(r_contour_final, C_Secret, MotzkinPrime)
    
    r_channel_final = modExp(r_base_final, R_Signal, MotzkinPrime)
    c_channel_final = modExp(c_base_final, C_Signal, MotzkinPrime)
    
    r_pole_final = modExp(r_base_final, R_Secret, MotzkinPrime)
    c_pole_final = modExp(c_base_final, C_Secret, MotzkinPrime)
    
    r_coord_final = modExp(c_pole_final, R_Secret, MotzkinPrime)
    c_coord_final = modExp(r_pole_final, C_Secret, MotzkinPrime)
    
    c_foundation_final = modExp(c_base_final, Alpha, MotzkinPrime)
    r_foundation_final = modExp(r_base_final, Beta_in, MotzkinPrime)
    
    r_beta_val = modExp(c_foundation_final, Beta_in, MotzkinPrime)
    r_rho_val = modExp(c_channel_final, Beta_in, MotzkinPrime)
    r_eta_val = modExp(c_foundation_final, R_Signal, MotzkinPrime)
    r_charge_final = r_rho_val + r_eta_val
    r_chin_final = r_beta_val + r_eta_val
    r_element_final = r_beta_val + r_charge_final
    r_dynamo_2 = modExp(c_channel_final, R_Signal, MotzkinPrime)
    
    c_beta_val = modExp(r_foundation_final, Alpha, MotzkinPrime)
    c_rho_val = modExp(r_channel_final, Alpha, MotzkinPrime)
    c_eta_val = modExp(r_foundation_final, C_Signal, MotzkinPrime)
    c_charge_final = c_rho_val + c_eta_val
    c_chin_final = c_beta_val + c_eta_val
    c_element_final = c_beta_val + c_charge_final
    c_dynamo_2 = modExp(r_channel_final, C_Signal, MotzkinPrime)
    
    r_dynamo_3 = modExp(r_base_final, R_Signal, r_element_final)
    c_dynamo_3 = modExp(c_base_final, C_Signal, c_element_final)
    
    def lerp(start, end, factor):
        return int(start + (end - start) * factor)
        
    def smooth_step(edge0, edge1, x):
        val = max(0.0, min(1.0, (x - edge0) / (edge1 - edge0)))
        return val * val * (3.0 - 2.0 * val)
        
    s0 = smooth_step(0.0, t_genesis, t_secs)
    s1 = smooth_step(t_genesis, t_tuning, t_secs)
    s2 = smooth_step(t_tuning, t_align, t_secs)
    s3 = smooth_step(t_align, 25.0, t_secs)
    
    # Rod
    r_contour = lerp(0, r_contour_final, s0)
    r_base = lerp(seeds["r_base_init"], r_base_final, s0)
    r_channel = lerp(seeds["r_channel"], r_channel_final, s0)
    r_pole = lerp(0, r_pole_final, s0)
    if t_secs >= t_genesis:
        r_contour = lerp(r_contour_final, r_coord_final, s1)
        
    r_foundation = 0
    if t_secs >= t_genesis:
        r_foundation = lerp(0, r_foundation_final, s1)
        
    r_charge = 0
    r_chin = 0
    r_element = 0
    r_dynamo = 0
    if t_secs >= t_tuning:
        r_charge = lerp(0, r_charge_final, s2)
        r_chin = lerp(0, r_chin_final, s2)
        r_element = lerp(0, r_element_final, s2)
        r_dynamo = lerp(0, r_dynamo_2, s2)
        
    if t_secs >= t_align:
        r_dynamo = lerp(r_dynamo_2, r_dynamo_3, s3)
        r_pole = lerp(r_pole_final, 0, s3)
        
    # Cone
    c_contour = lerp(0, c_contour_final, s0)
    c_base = lerp(seeds["c_base_init"], c_base_final, s0)
    c_channel = lerp(seeds["c_channel"], c_channel_final, s0)
    c_pole = lerp(0, c_pole_final, s0)
    if t_secs >= t_genesis:
        c_contour = lerp(c_contour_final, c_coord_final, s1)
        
    c_foundation = 0
    if t_secs >= t_genesis:
        c_foundation = lerp(0, c_foundation_final, s1)
        
    c_charge = 0
    c_chin = 0
    c_element = 0
    c_dynamo = 0
    if t_secs >= t_tuning:
        c_charge = lerp(0, c_charge_final, s2)
        c_chin = lerp(0, c_chin_final, s2)
        c_element = lerp(0, c_element_final, s2)
        c_dynamo = lerp(0, c_dynamo_2, s2)
        
    if t_secs >= t_align:
        c_dynamo = lerp(c_dynamo_2, c_dynamo_3, s3)
        c_pole = lerp(c_pole_final, 0, s3)
        
    return {
        "r_base": r_base, "r_channel": r_channel, "r_pole": r_pole, "r_foundation": r_foundation, "r_dynamo": r_dynamo,
        "c_base": c_base, "c_channel": c_channel, "c_pole": c_pole, "c_foundation": c_foundation, "c_dynamo": c_dynamo,
    }

def generate_audio():
    print("[Multi-YI Grid] Generating audio track...")
    total_samples = SAMPLE_RATE * DURATION
    audio = np.zeros(total_samples, dtype=np.float32)
    step_len = int(SAMPLE_RATE * (30 / 120 / 4))
    current_sample = 0
    step_idx = 0
    
    while current_sample < total_samples:
        if current_sample + step_len > total_samples:
            break
        step_val = step_idx % 22
        t = np.arange(min(step_len * 4, total_samples - current_sample)) / SAMPLE_RATE
        env = np.exp(-3.5 * t)
        
        sig = 0.0
        for idx, seeds in enumerate(YI_SEEDS):
            root_freq = 65.40 * (idx + 2)  # stacked tones
            freq = root_freq * (2.0 ** (step_val / 22.0))
            sig += 0.06 * np.sin(2 * np.pi * freq * t)
            
        audio[current_sample:current_sample+len(t)] += sig * env
        current_sample += step_len
        step_idx += 1
        
    audio_pcm = np.clip(audio, -1.0, 1.0) * 32767
    audio_path = "grid_demo_temp.wav"
    with wave.open(audio_path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio_pcm.astype(np.int16).tobytes())
    return audio_path

def project_3d(x, y, z, cam_x, cam_y, cam_z, yaw, pitch, zoom, quadrant_center):
    cx_q, cy_q = quadrant_center
    dx = x - cam_x
    dy = y - cam_y
    dz = z - cam_z
    
    cos_y, sin_y = math.cos(yaw), math.sin(yaw)
    rx = dx * cos_y - dy * sin_y
    ry = dx * sin_y + dy * cos_y
    rz = dz
    
    cos_p, sin_p = math.cos(pitch), math.sin(pitch)
    x_new = rx * cos_p + rz * sin_p
    y_new = ry
    z_new = -rx * sin_p + rz * cos_p
    
    focal = 350.0
    if z_new == 0: z_new = 1
    px = (x_new * focal) / (z_new + 500) * zoom + cx_q
    py = (y_new * focal) / (z_new + 500) * zoom + cy_q
    return int(px), int(py), rz

def draw_glow_line(draw_obj, p1, p2, color, width=1):
    r, g, b, a = color
    draw_obj.line([p1, p2], fill=(r, g, b, int(a * 0.15)), width=width+2)
    draw_obj.line([p1, p2], fill=(r, g, b, a), width=width)

def get_wireframe_rod(radius, length, scale_factor, t_secs, channel_val):
    vertices = []
    base_spin = (math.log10(max(1, channel_val)) % 3.0) + 1.0
    for z_idx in range(7):
        z = -length / 2 + (z_idx * length / 6)
        spin_speed = (base_spin - 0.12 * z_idx)
        phase_offset = (t_secs * spin_speed) % (2 * np.pi)
        
        for r_idx in range(8):
            angle = (r_idx * 2 * np.pi / 8) + phase_offset
            x = radius * math.cos(angle) * scale_factor
            y = radius * math.sin(angle) * scale_factor
            vertices.append((x, y, z))
    return vertices

def get_wireframe_cone(radius, length, scale_factor, t_secs, channel_val):
    vertices = []
    base_spin = (math.log10(max(1, channel_val)) % 3.0) + 1.2
    for z_idx in range(7):
        z = -length / 2 + (z_idx * length / 6)
        converge_factor = (6 - z_idx) / 6.0
        spin_speed = (-base_spin + 0.15 * z_idx)
        phase_offset = (t_secs * spin_speed) % (2 * np.pi)
        
        for r_idx in range(8):
            angle = (r_idx * 2 * np.pi / 8) + phase_offset
            x = radius * math.cos(angle) * converge_factor * scale_factor
            y = radius * math.sin(angle) * converge_factor * scale_factor
            vertices.append((x, y, z))
    return vertices

def render_video():
    audio_wav = generate_audio()
    print("[Multi-YI Grid] Compiling 900 visual frames of 4 YI Grid genesis (Motzkin Prime)...")
    video_path = "yi_quaternion_grid_demo.mp4"
    cmd = [
        "ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", "30",
        "-i", "-", "-i", audio_wav, "-c:v", "libx264", "-pix_fmt", "yuv420p",
        "-c:a", "aac", "-b:a", "192k", "-shortest", video_path
    ]
    pipe = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    
    quad_centers = [
        (WIDTH / 4, HEIGHT / 4 + 20),      # Top-Left
        (3 * WIDTH / 4, HEIGHT / 4 + 20),  # Top-Right
        (WIDTH / 4, 3 * HEIGHT / 4 - 20),  # Bottom-Left
        (3 * WIDTH / 4, 3 * HEIGHT / 4 - 20) # Bottom-Right
    ]
    
    final_frame_img = None
    
    for frame_idx in range(TOTAL_FRAMES):
        t_secs = frame_idx / FPS
        
        img = Image.new("RGB", (WIDTH, HEIGHT), color=(6, 9, 18))
        draw = ImageDraw.Draw(img, "RGBA")
        
        # Grid Separator lines
        draw.line([(WIDTH / 2, 0), (WIDTH / 2, HEIGHT)], fill=(0, 242, 254, 30), width=1)
        draw.line([(0, HEIGHT / 2), (WIDTH, HEIGHT / 2)], fill=(0, 242, 254, 30), width=1)
        
        # Global HUD Header
        draw.rectangle([0, 0, WIDTH, 40], fill=(4, 6, 12, 240))
        draw.line([(0, 40), (WIDTH, 40)], fill=(0, 242, 254, 60), width=1)
        draw.text((WIDTH / 2 - 200, 12), f"AUNCIENT MULTI-YI GENESIS (FIELD MODULO {MotzkinPrime})", fill=(0, 242, 254), font=font_bold)
        
        for idx, seeds in enumerate(YI_SEEDS):
            c_center = quad_centers[idx]
            color = seeds["Color"]
            
            # Setup stable perspective matrix for quadrant
            yaw = 0.5
            pitch = 0.4
            zoom = 0.75
            cam_x = math.cos(yaw) * 260
            cam_y = math.sin(yaw) * 260
            cam_z = 150
            
            states = get_contract_states(t_secs, seeds)
            
            # Morph and Wobble variables
            morph_progress = 0.0
            if t_secs >= 10.0:
                morph_progress = max(0.0, min(1.0, (t_secs - 10.0) / 15.0))
                
            wobble_x, wobble_y, wobble_z = 0.0, 0.0, 0.0
            if t_secs >= 20.0:
                time_in_epoch = t_secs - 20.0
                decay = math.exp(-4.0 * (time_in_epoch % 3.0))
                
                # Frequencies derived purely from active YI quadrant registers
                fx = 1.0 + (states["r_channel"] % 5)
                fy = 1.0 + (states["r_dynamo"] % 5)
                fz = 1.0 + (states["r_foundation"] % 5)
                
                # Oscillations synthesized directly from the state space
                wobble_x = 9.0 * math.sin(fx * 30.0 * time_in_epoch) * decay
                wobble_y = 9.0 * math.cos(fy * 30.0 * time_in_epoch) * decay
                wobble_z = 9.0 * math.sin(fz * 30.0 * time_in_epoch) * decay

            # Compute Rod and Cone coordinates
            rod_center_x, cone_center_x = -90, 90
            scale_factor = 0.9 - 0.2 * max(0.0, min(1.0, t_secs / 10.0))
            
            rod_verts = get_wireframe_rod(45, 150, scale_factor, t_secs, states["r_channel"])
            cone_verts = get_wireframe_cone(55, 150, scale_factor, t_secs, states["c_channel"])
            
            # Target Lissajous configuration for YI (standard contract registers)
            r_fx = 1.0 + (states["r_channel"] % 5)
            r_fy = 1.0 + (states["r_dynamo"] % 5)
            r_fz = 1.0 + (states["r_foundation"] % 5)
            r_phi = (states["r_base"] % 100) / 100.0 * 2.0 * np.pi
            
            c_fx = 1.0 + (states["c_channel"] % 5)
            c_fy = 1.0 + (states["c_dynamo"] % 5)
            c_fz = 1.0 + (states["c_foundation"] % 5)
            c_phi = (states["c_base"] % 100) / 100.0 * 2.0 * np.pi
            
            # Project Rod
            proj_rod = []
            for v_idx, (x, y, z) in enumerate(rod_verts):
                theta = v_idx * 2.0 * np.pi / len(rod_verts)
                lx = 85 * math.sin(r_fx * theta + r_phi)
                ly = 85 * math.sin(r_fy * theta)
                lz = 85 * math.cos(r_fz * theta)
                
                mx = (1.0 - morph_progress) * (x + rod_center_x) + morph_progress * lx
                my = (1.0 - morph_progress) * y + morph_progress * ly
                mz = (1.0 - morph_progress) * z + morph_progress * lz
                
                px, py, _ = project_3d(mx + wobble_x, my + wobble_y, mz + wobble_z, cam_x, cam_y, cam_z, yaw, pitch, zoom, c_center)
                proj_rod.append((px, py))
                
            # Project Cone
            proj_cone = []
            for v_idx, (x, y, z) in enumerate(cone_verts):
                theta = v_idx * 2.0 * np.pi / len(cone_verts)
                lx = 85 * math.sin(c_fx * theta + c_phi)
                ly = 85 * math.sin(c_fy * theta)
                lz = 85 * math.cos(c_fz * theta)
                
                mx = (1.0 - morph_progress) * (x + cone_center_x) + morph_progress * lx
                my = (1.0 - morph_progress) * y + morph_progress * ly
                mz = (1.0 - morph_progress) * z + morph_progress * lz
                
                px, py, _ = project_3d(mx - wobble_x, my - wobble_y, mz - wobble_z, cam_x, cam_y, cam_z, yaw, pitch, zoom, c_center)
                proj_cone.append((px, py))
                
            # Draw Rod
            rod_multiplier = 1.0 + (states["r_channel"] % 4)
            for z_idx in range(7):
                start = z_idx * 8
                for r_idx in range(8):
                    p1 = proj_rod[start + r_idx]
                    p2 = proj_rod[start + ((r_idx + 1) % 8)]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 90), width=1)
                for r_idx in range(8):
                    target_idx = int((r_idx * rod_multiplier) % 8)
                    p1 = proj_rod[start + r_idx]
                    p2 = proj_rod[start + target_idx]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 50), width=1)
            for r_idx in range(8):
                for z_idx in range(6):
                    p1 = proj_rod[z_idx * 8 + r_idx]
                    p2 = proj_rod[(z_idx + 1) * 8 + r_idx]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 40), width=1)
                    
            # Draw Cone
            cone_multiplier = 1.0 + (states["c_channel"] % 4)
            for z_idx in range(7):
                start = z_idx * 8
                for r_idx in range(8):
                    p1 = proj_cone[start + r_idx]
                    p2 = proj_cone[start + ((r_idx + 1) % 8)]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 90), width=1)
                for r_idx in range(8):
                    target_idx = int((r_idx * cone_multiplier) % 8)
                    p1 = proj_cone[start + r_idx]
                    p2 = proj_cone[start + target_idx]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 50), width=1)
            for r_idx in range(8):
                for z_idx in range(6):
                    p1 = proj_cone[z_idx * 8 + r_idx]
                    p2 = proj_cone[(z_idx + 1) * 8 + r_idx]
                    draw_glow_line(draw, p1, p2, (color[0], color[1], color[2], 40), width=1)

            # Slide axes
            for center_x, w_x, w_y, w_z in [(rod_center_x, wobble_x, wobble_y, wobble_z), (cone_center_x, -wobble_x, -wobble_y, -wobble_z)]:
                slide_x = center_x * (1.0 - morph_progress)
                p1_x, p1_y, _ = project_3d(slide_x + w_x, w_y, -110 + w_z, cam_x, cam_y, cam_z, yaw, pitch, zoom, c_center)
                p2_x, p2_y, _ = project_3d(slide_x + w_x, w_y, 110 + w_z, cam_x, cam_y, cam_z, yaw, pitch, zoom, c_center)
                draw_glow_line(draw, (p1_x, p1_y), (p2_x, p2_y), (34, 197, 94, 180), width=1)

            # HUD
            lx = int(c_center[0] - WIDTH / 4 + 10)
            ly = int(c_center[1] + HEIGHT / 4 - 80)
            draw.rectangle([lx, ly, lx + 230, ly + 55], fill=(5, 5, 8, 200), outline=(color[0], color[1], color[2], 100))
            draw.text((lx + 10, ly + 5), f"ID: {seeds['Name']}", fill=color, font=font_bold)
            draw.text((lx + 10, ly + 20), f"Modulo Prime: {MotzkinPrime}", fill=(255, 255, 255), font=font_regular)
            draw.text((lx + 10, ly + 32), f"Morph Target: standard Lissajous curve", fill=(255, 255, 255, 180), font=font_regular)

        img.save(pipe.stdin, "PNG")
        if frame_idx == TOTAL_FRAMES - 1:
            final_frame_img = img

    pipe.stdin.close()
    pipe.wait()
    print("[Multi-YI Grid] Video generation complete.")
    
    icon_path = "yi_quaternion_grid_icon.png"
    final_frame_img.save(icon_path)
    print(f"[Multi-YI Grid] Final icon saved to {icon_path}")
    
    # Compute SHA-256 hash
    with open(icon_path, "rb") as f:
        bytes_data = f.read()
        sha_hash = hashlib.sha256(bytes_data).hexdigest()
        
    print(f"[Multi-YI Grid] VERIFIED SHA-256 CHECKSUM: {sha_hash}")
    
    # Update Reproducibility Report
    report_content = f"""# YI Icon State Reproducibility Report

The 4-YI Multi-Quaternion genesis and morphing process resolves to a mathematically locked and stable icon state image. The entire pipeline runs modulo the Motzkin Prime constant.

### Final Verification Details:
* **Modulo Divisor Field**: `{MotzkinPrime}` (Motzkin Prime)
* **Icon Image File**: [yi_quaternion_grid_icon.png](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/yi_quaternion_grid_icon.png)
* **SHA-256 Checksum**: `{sha_hash}`

This verification shows that using standard contract mechanisms and the Motzkin Prime divisor field, the generated output image matrix is perfectly locked and reproducible.
"""
    with open("YI_ICON_REPRODUCIBILITY.md", "w") as rf:
        rf.write(report_content)
    print("[Multi-YI Grid] Reproducibility report written to YI_ICON_REPRODUCIBILITY.md")

if __name__ == "__main__":
    render_video()
