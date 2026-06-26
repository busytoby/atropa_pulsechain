# Generate intricate technical blueprint of Dysnomia YI epoch birth canal: Rod and Cone genesis
# Renders 1800 frames (60 seconds at 30 FPS) with dynamic 3D camera pan/orbit/zoom, audio, and visual effects.

import math
import random
import wave
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# Video/Audio constants
WIDTH, HEIGHT = 1280, 720
FPS = 30
DURATION = 60
TOTAL_FRAMES = FPS * DURATION
SAMPLE_RATE = 44100

# EDO-22 system parameters
EDO_STEPS = 22
LUT_SIZE = 22528
LUT_MASK = LUT_SIZE - 1
LUT_FACTOR = LUT_SIZE / (2 * np.pi)

print("[Void System] Initializing Auncient Trigonometric LUT tables...")
sin_lut = np.sin(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))
cos_lut = np.cos(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))

# Load DejaVuSans vector fonts for premium UI overlay
try:
    font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 15)
    font_bold = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 12)
    font_regular = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 11)
except Exception:
    font_title = ImageFont.load_default()
    font_bold = ImageFont.load_default()
    font_regular = ImageFont.load_default()

# Motzkin Prime constant
MotzkinPrime = 953467954114363

# SHA Contract states (from actual ZMM deployment)
RodState = {
    "Base_init": 29088494943924305920000,
    "Secret_init": 28723650630656000000,
    "Signal_init": 1000000000000000000,
    "Channel_init": 129000000000000,
    "Identity_mut": 242000000000000000,
}

ConeState = {
    "Base_init": 1938006093077708800000,
    "Secret_init": 737000000000000000,
    "Signal_init": 1000000000000000000,
    "Channel_init": 85300000000000,
    "Identity_mut": 386000000000000000,
}

def modExp(base, exp, mod):
    return pow(int(base), int(exp), int(mod))

def get_contract_states(t_secs):
    R_Secret = RodState["Secret_init"]
    R_Signal = RodState["Signal_init"]
    C_Secret = ConeState["Secret_init"]
    C_Signal = ConeState["Signal_init"]
    
    Xi = 129000000000000
    Alpha = 85300000000000
    Beta_in = 242000000000000000
    
    # 1. Seeding / Genesis Seeding (0s to 15s)
    r_contour_final = modExp(Xi, R_Secret, MotzkinPrime)
    c_contour_final = modExp(Xi, C_Secret, MotzkinPrime)
    
    r_base_final = modExp(c_contour_final, R_Secret, MotzkinPrime)
    c_base_final = modExp(r_contour_final, C_Secret, MotzkinPrime)
    
    r_channel_final = modExp(r_base_final, R_Signal, MotzkinPrime)
    c_channel_final = modExp(c_base_final, C_Signal, MotzkinPrime)
    
    r_pole_final = modExp(r_base_final, R_Secret, MotzkinPrime)
    c_pole_final = modExp(c_base_final, C_Secret, MotzkinPrime)
    
    # 2. Resonance Tuning (15s to 30s)
    r_coord_final = modExp(c_pole_final, R_Secret, MotzkinPrime)
    c_coord_final = modExp(r_pole_final, C_Secret, MotzkinPrime)
    
    c_foundation_final = modExp(c_base_final, Alpha, MotzkinPrime)
    r_foundation_final = modExp(r_base_final, Beta_in, MotzkinPrime)
    
    # 3. Xi Vector Align (30s to 45s) -> Saturate
    r_beta_val = modExp(c_foundation_final, Beta_in, MotzkinPrime)
    r_rho_val = modExp(c_channel_final, Beta_in, MotzkinPrime)
    r_eta_val = modExp(c_foundation_final, R_Signal, MotzkinPrime)
    r_charge_final = r_rho_val + r_eta_val
    r_chin_final = r_beta_val + r_eta_val
    r_element_final = r_beta_val + r_charge_final
    r_dynamo_2 = modExp(c_channel_final, R_Signal, MotzkinPrime)
    r_monopole_final = modExp(r_chin_final, Beta_in, MotzkinPrime)
    
    c_beta_val = modExp(r_foundation_final, Alpha, MotzkinPrime)
    c_rho_val = modExp(r_channel_final, Alpha, MotzkinPrime)
    c_eta_val = modExp(r_foundation_final, C_Signal, MotzkinPrime)
    c_charge_final = c_rho_val + c_eta_val
    c_chin_final = c_beta_val + c_eta_val
    c_element_final = c_beta_val + c_charge_final
    c_dynamo_2 = modExp(r_channel_final, C_Signal, MotzkinPrime)
    c_monopole_final = modExp(c_chin_final, Alpha, MotzkinPrime)
    
    # 4. Ionize (45s to 60s) -> Bond()
    r_dynamo_3 = modExp(r_base_final, R_Signal, r_element_final)
    c_dynamo_3 = modExp(c_base_final, C_Signal, c_element_final)
    
    # Interpolation helper
    def lerp(start, end, factor):
        return int(start + (end - start) * factor)
        
    def smooth_step(edge0, edge1, x):
        val = max(0.0, min(1.0, (x - edge0) / (edge1 - edge0)))
        return val * val * (3.0 - 2.0 * val)
        
    s0 = smooth_step(0.0, 15.0, t_secs)
    s1 = smooth_step(15.0, 30.0, t_secs)
    s2 = smooth_step(30.0, 45.0, t_secs)
    s3 = smooth_step(45.0, 50.0, t_secs)
    
    # Rod interpolated
    r_contour = lerp(0, r_contour_final, s0)
    r_base = lerp(RodState["Base_init"], r_base_final, s0)
    r_channel = lerp(RodState["Channel_init"], r_channel_final, s0)
    r_pole = lerp(0, r_pole_final, s0)
    if t_secs >= 15.0:
        r_contour = lerp(r_contour_final, r_coord_final, s1)
        
    r_foundation = 0
    r_identity = 0
    if t_secs >= 15.0:
        r_identity = lerp(0, Beta_in, s1)
        r_foundation = lerp(0, r_foundation_final, s1)
        
    r_charge = 0
    r_chin = 0
    r_element = 0
    r_dynamo = 0
    r_monopole = 0
    if t_secs >= 30.0:
        r_charge = lerp(0, r_charge_final, s2)
        r_chin = lerp(0, r_chin_final, s2)
        r_element = lerp(0, r_element_final, s2)
        r_dynamo = lerp(0, r_dynamo_2, s2)
        r_monopole = lerp(0, r_monopole_final, s2)
        
    if t_secs >= 45.0:
        r_dynamo = lerp(r_dynamo_2, r_dynamo_3, s3)
        r_pole = lerp(r_pole_final, 0, s3)
        
    # Cone interpolated
    c_contour = lerp(0, c_contour_final, s0)
    c_base = lerp(ConeState["Base_init"], c_base_final, s0)
    c_channel = lerp(ConeState["Channel_init"], c_channel_final, s0)
    c_pole = lerp(0, c_pole_final, s0)
    if t_secs >= 15.0:
        c_contour = lerp(c_contour_final, c_coord_final, s1)
        
    c_foundation = 0
    c_identity = 0
    if t_secs >= 15.0:
        c_identity = lerp(0, Alpha, s1)
        c_foundation = lerp(0, c_foundation_final, s1)
        
    c_charge = 0
    c_chin = 0
    c_element = 0
    c_dynamo = 0
    c_monopole = 0
    if t_secs >= 30.0:
        c_charge = lerp(0, c_charge_final, s2)
        c_chin = lerp(0, c_chin_final, s2)
        c_element = lerp(0, c_element_final, s2)
        c_dynamo = lerp(0, c_dynamo_2, s2)
        c_monopole = lerp(0, c_monopole_final, s2)
        
    if t_secs >= 45.0:
        c_dynamo = lerp(c_dynamo_2, c_dynamo_3, s3)
        c_pole = lerp(c_pole_final, 0, s3)
        
    return {
        "r_contour": r_contour, "r_base": r_base, "r_channel": r_channel, "r_pole": r_pole,
        "r_identity": r_identity, "r_foundation": r_foundation, "r_charge": r_charge, "r_chin": r_chin,
        "r_element": r_element, "r_dynamo": r_dynamo, "r_monopole": r_monopole,
        "c_contour": c_contour, "c_base": c_base, "c_channel": c_channel, "c_pole": c_pole,
        "c_identity": c_identity, "c_foundation": c_foundation, "c_charge": c_charge, "c_chin": c_chin,
        "c_element": c_element, "c_dynamo": c_dynamo, "c_monopole": c_monopole,
    }

# 3D Projection Math
def project_3d(x, y, z, cam_x, cam_y, cam_z, yaw, pitch, zoom):
    def get_raw_proj(px_val, py_val, pz_val):
        dx = px_val - cam_x
        dy = py_val - cam_y
        dz = pz_val - cam_z
        
        cos_y, sin_y = math.cos(yaw), math.sin(yaw)
        rx = dx * cos_y - dy * sin_y
        ry = dx * sin_y + dy * cos_y
        rz = dz
        
        cos_p, sin_p = math.cos(pitch), math.sin(pitch)
        x_new = rx * cos_p + rz * sin_p
        y_new = ry
        z_new = -rx * sin_p + rz * cos_p
        
        focal = 750.0
        if z_new == 0: z_new = 1
        px = (x_new * focal) / (z_new + 900) * zoom
        py = (y_new * focal) / (z_new + 900) * zoom
        return px, py, z_new

    px_raw, py_raw, z_new = get_raw_proj(x, y, z)
    px_orig, py_orig, _ = get_raw_proj(0, 0, 0)
    
    px_final = px_raw - px_orig + WIDTH / 2
    py_final = py_raw - py_orig + HEIGHT / 2
    
    return int(px_final), int(py_final), z_new

# Generate EDO-22 soundtrack matching the epoch stages
def generate_audio():
    print("[Void System] Generating EDO-22 microtonal audio soundtrack...")
    total_samples = SAMPLE_RATE * DURATION
    audio = np.zeros(total_samples, dtype=np.float32)
    step_len = int(SAMPLE_RATE * (60 / 120 / 4)) # 120 BPM base
    
    current_sample = 0
    step_idx = 0
    root_freq = 130.81
    edo_freqs = [root_freq * (2 ** (i / 22)) for i in range(22)]
    
    while current_sample < total_samples:
        if current_sample + step_len > total_samples:
            break
            
        step_val = step_idx % 22
        freq = edo_freqs[step_val]
        
        epoch_idx = current_sample / total_samples * 4.0
        t = np.arange(min(step_len * 4, total_samples - current_sample)) / SAMPLE_RATE
        env = np.exp(-3.5 * t)
        
        sig = 0.22 * np.sin(2 * np.pi * freq * t)
        if epoch_idx > 1.0:
            sig += 0.12 * np.sin(2 * np.pi * (freq * 1.5) * t)
        if epoch_idx > 2.0:
            sig += 0.05 * np.random.normal(0, 1, len(t))
        if epoch_idx > 3.0:
            sig = np.tanh(sig * 1.5) / 1.5
            
        audio[current_sample:current_sample+len(t)] += sig * env
        current_sample += step_len
        step_idx += 1
        
    audio_pcm = np.clip(audio, -1.0, 1.0) * 32767
    audio_path = "void_demo_temp.wav"
    with wave.open(audio_path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio_pcm.astype(np.int16).tobytes())
    return audio_path

def create_radial_gradient(width, height):
    bg = Image.new("RGB", (width, height))
    pixels = bg.load()
    cx, cy = width // 2, height // 2
    max_d = math.sqrt(cx**2 + cy**2)
    for y in range(height):
        for x in range(width):
            dx = x - cx
            dy = y - cy
            d = math.sqrt(dx**2 + dy**2)
            f = d / max_d
            r = int(12 * (1.0 - f) + 4 * f)
            g = int(22 * (1.0 - f) + 6 * f)
            b = int(45 * (1.0 - f) + 12 * f)
            pixels[x, y] = (r, g, b)
    return bg

bg_template = create_radial_gradient(WIDTH, HEIGHT)

def draw_glow_line(draw_obj, p1, p2, color, width=1):
    r, g, b, a = color
    draw_obj.line([p1, p2], fill=(r, g, b, int(a * 0.15)), width=width+4)
    draw_obj.line([p1, p2], fill=(r, g, b, int(a * 0.40)), width=width+2)
    draw_obj.line([p1, p2], fill=(r, g, b, a), width=width)


def get_wireframe_rod(radius, length, scale_factor, t_secs, channel_val):
    vertices = []
    base_spin = (math.log10(max(1, channel_val)) % 5.0) + 1.0
    for z_idx in range(9):
        z = -length / 2 + (z_idx * length / 8)
        spin_speed = (base_spin - 0.15 * z_idx)
        phase_offset = (t_secs * spin_speed) % (2 * np.pi)
        
        for r_idx in range(12):
            angle = (r_idx * 2 * np.pi / 12) + phase_offset
            x = radius * math.cos(angle) * scale_factor
            y = radius * math.sin(angle) * scale_factor
            vertices.append((x, y, z))
    return vertices

def get_wireframe_cone(radius, length, scale_factor, t_secs, channel_val):
    vertices = []
    base_spin = (math.log10(max(1, channel_val)) % 5.0) + 1.2
    for z_idx in range(9):
        z = -length / 2 + (z_idx * length / 8)
        converge_factor = (8 - z_idx) / 8.0
        
        spin_speed = (-base_spin + 0.18 * z_idx)
        phase_offset = (t_secs * spin_speed) % (2 * np.pi)
        
        for r_idx in range(12):
            angle = (r_idx * 2 * np.pi / 12) + phase_offset
            x = radius * math.cos(angle) * converge_factor * scale_factor
            y = radius * math.sin(angle) * converge_factor * scale_factor
            vertices.append((x, y, z))
    return vertices

def render_video():
    import subprocess
    audio_wav = generate_audio()
    
    print("[Void System] Compiling 1800 visual frames of Rod and Cone genesis...")
    video_path = "void_quaternion_demo.mp4"
    cmd = [
        "ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", "30",
        "-i", "-", "-i", audio_wav, "-c:v", "libx264", "-pix_fmt", "yuv420p",
        "-c:a", "aac", "-b:a", "192k", "-shortest", video_path
    ]
    pipe = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    
    epochs = ["GENESIS SEEDING", "RESONANCE TUNING", "XI VECTOR ALIGN", "SATURATION & SPIN"]
    
    for frame_idx in range(TOTAL_FRAMES):
        t_secs = frame_idx / FPS
        
        if t_secs < 15.0:
            epoch_idx = 0
            epoch_progress = t_secs / 15.0
        elif t_secs < 30.0:
            epoch_idx = 1
            epoch_progress = (t_secs - 15.0) / 15.0
        elif t_secs < 45.0:
            epoch_idx = 2
            epoch_progress = (t_secs - 30.0) / 15.0
        else:
            epoch_idx = 3
            epoch_progress = (t_secs - 45.0) / 15.0
            
        current_epoch_name = epochs[epoch_idx]
        
        # Camera pinned stably for a clean technical view
        yaw = 0.5
        pitch = 0.4
        zoom = 0.62
        cam_x = math.cos(yaw) * 400
        cam_y = math.sin(yaw) * 400
        cam_z = 250
        
        img = bg_template.copy()
        draw = ImageDraw.Draw(img, "RGBA")
        
        # 1. Blueprint Grid Lines (fading out near screen edges for aesthetic depth)
        cx, cy = WIDTH // 2, HEIGHT // 2
        max_dist = math.sqrt(cx**2 + cy**2)
        for x in range(0, WIDTH, 40):
            dist = abs(x - cx)
            alpha = int(max(2, 14 * (1.0 - dist / max_dist)))
            draw.line([(x, 0), (x, HEIGHT)], fill=(0, 242, 254, alpha), width=1)
        for y in range(0, HEIGHT, 40):
            dist = abs(y - cy)
            alpha = int(max(2, 14 * (1.0 - dist / max_dist)))
            draw.line([(0, y), (WIDTH, y)], fill=(0, 242, 254, alpha), width=1)
            
        # 2. Status Header timeline
        draw.rectangle([0, 0, WIDTH, 50], fill=(6, 9, 18, 220))
        draw.line([(0, 50), (WIDTH, 50)], fill=(0, 242, 254, 80), width=1)
        
        timeline_width = WIDTH - 300
        start_x = 150
        for idx, name in enumerate(epochs):
            node_x = start_x + int(idx * (timeline_width / 3.0))
            is_active = (idx == epoch_idx)
            is_passed = (idx < epoch_idx)
            
            color = (0, 242, 254, 255) if is_active else ((0, 242, 254, 150) if is_passed else (100, 116, 139, 100))
            if idx < 3:
                next_x = start_x + int((idx + 1) * (timeline_width / 3.0))
                line_color = (0, 242, 254, 180) if is_passed else (100, 116, 139, 50)
                draw.line([(node_x, 25), (next_x, 25)], fill=line_color, width=2)
                
            draw.ellipse([node_x - 6, 25 - 6, node_x + 6, 25 + 6], fill=color)
            if is_active:
                draw.text((node_x - 50, 32), name, fill=(0, 242, 254, 255), font=font_bold)
            else:
                draw.text((node_x - 50, 32), name, fill=(100, 116, 139, 150), font=font_regular)
                
        # Helper for smooth transitions
        def smooth_step(edge0, edge1, x):
            val = max(0.0, min(1.0, (x - edge0) / (edge1 - edge0)))
            return val * val * (3.0 - 2.0 * val)

        # 3. Model calculations and contract states
        states = get_contract_states(t_secs)
        scale_factor = 1.0 - 0.40 * smooth_step(15.0, 30.0, t_secs)
        lock_progress = smooth_step(45.0, 50.0, t_secs)
            
        # Flux Pinning Wobble Physics (Idealized Register Synthesis)
        wobble_x, wobble_y, wobble_z = 0.0, 0.0, 0.0
        if t_secs >= 45.0:
            time_in_epoch = t_secs - 45.0
            decay = math.exp(-4.0 * (time_in_epoch % 3.0))
            
            # Frequencies derived purely from active contract registers
            fx = 1.0 + (states["r_channel"] % 5)
            fy = 1.0 + (states["r_dynamo"] % 5)
            fz = 1.0 + (states["r_foundation"] % 5)
            
            # Oscillations synthesized directly from the state space
            wobble_x = 12.0 * math.sin(fx * 30.0 * time_in_epoch) * decay
            wobble_y = 12.0 * math.cos(fy * 30.0 * time_in_epoch) * decay
            wobble_z = 12.0 * math.sin(fz * 30.0 * time_in_epoch) * decay

        # Draw the Rod (cylinder) on the left side, Cone on the right side
        rod_center_x, cone_center_x = -200, 200
        
        rod_verts = get_wireframe_rod(80, 250, scale_factor, t_secs, states["r_channel"])
        cone_verts = get_wireframe_cone(100, 250, scale_factor, t_secs, states["c_channel"])
        
        # Project and Draw Rod
        proj_rod = []
        morph_progress = smooth_step(45.0, 58.0, t_secs)
        
        # Target Lissajous frequencies and phase offsets from hardware registers
        r_fx = 1.0 + (states["r_channel"] % 5)
        r_fy = 1.0 + (states["r_dynamo"] % 5)
        r_fz = 1.0 + (states["r_foundation"] % 5)
        r_phi = (states["r_base"] % 100) / 100.0 * 2.0 * np.pi
        
        for idx, (x, y, z) in enumerate(rod_verts):
            theta = idx * 2.0 * np.pi / len(rod_verts)
            # Center coordinates morph from rod_center_x to 0
            curr_center_x = rod_center_x * (1.0 - morph_progress)
            
            # Lissajous coordinates
            lx = 150 * math.sin(r_fx * theta + r_phi)
            ly = 150 * math.sin(r_fy * theta)
            lz = 150 * math.cos(r_fz * theta)
            
            # Morph from original cylindrical vertices to the Lissajous vertices
            mx = (1.0 - morph_progress) * (x + rod_center_x) + morph_progress * lx
            my = (1.0 - morph_progress) * y + morph_progress * ly
            mz = (1.0 - morph_progress) * z + morph_progress * lz
            
            px, py, _ = project_3d(mx + wobble_x, my + wobble_y, mz + wobble_z, cam_x, cam_y, cam_z, yaw, pitch, zoom)
            proj_rod.append((px, py))
            
        # Draw Rod radial rings and internal reflection chords (Sub-micro modExp mapping)
        rod_multiplier = 1.0 + (states["r_channel"] % 5)
        for z_idx in range(9):
            start = z_idx * 12
            # Base rings
            for r_idx in range(12):
                p1 = proj_rod[start + r_idx]
                p2 = proj_rod[start + ((r_idx + 1) % 12)]
                draw_glow_line(draw, p1, p2, (168, 85, 247, 100), width=1)
            # Modulo chords reflecting the internal arithmetic
            for r_idx in range(12):
                target_idx = int((r_idx * rod_multiplier) % 12)
                p1 = proj_rod[start + r_idx]
                p2 = proj_rod[start + target_idx]
                draw_glow_line(draw, p1, p2, (168, 85, 247, 80), width=1)
                
        # Draw Rod longitudinal lines
        for r_idx in range(12):
            for z_idx in range(8):
                p1 = proj_rod[z_idx * 12 + r_idx]
                p2 = proj_rod[(z_idx + 1) * 12 + r_idx]
                draw_glow_line(draw, p1, p2, (168, 85, 247, 60), width=1)
                
        # Project and Draw Cone
        proj_cone = []
        c_fx = 1.0 + (states["c_channel"] % 5)
        c_fy = 1.0 + (states["c_dynamo"] % 5)
        c_fz = 1.0 + (states["c_foundation"] % 5)
        c_phi = (states["c_base"] % 100) / 100.0 * 2.0 * np.pi
        
        for idx, (x, y, z) in enumerate(cone_verts):
            theta = idx * 2.0 * np.pi / len(cone_verts)
            # Center coordinates morph from cone_center_x to 0
            curr_center_x = cone_center_x * (1.0 - morph_progress)
            
            # Lissajous coordinates
            lx = 150 * math.sin(c_fx * theta + c_phi)
            ly = 150 * math.sin(c_fy * theta)
            lz = 150 * math.cos(c_fz * theta)
            
            # Morph from original cone vertices to the Lissajous vertices
            mx = (1.0 - morph_progress) * (x + cone_center_x) + morph_progress * lx
            my = (1.0 - morph_progress) * y + morph_progress * ly
            mz = (1.0 - morph_progress) * z + morph_progress * lz
            
            px, py, _ = project_3d(mx - wobble_x, my - wobble_y, mz - wobble_z, cam_x, cam_y, cam_z, yaw, pitch, zoom)
            proj_cone.append((px, py))
            
        # Draw Cone radial rings and internal reflection chords
        cone_multiplier = 1.0 + (states["c_channel"] % 5)
        for z_idx in range(9):
            start = z_idx * 12
            # Base rings
            for r_idx in range(12):
                p1 = proj_cone[start + r_idx]
                p2 = proj_cone[start + ((r_idx + 1) % 12)]
                draw_glow_line(draw, p1, p2, (0, 242, 254, 100), width=1)
            # Modulo chords
            for r_idx in range(12):
                target_idx = int((r_idx * cone_multiplier) % 12)
                p1 = proj_cone[start + r_idx]
                p2 = proj_cone[start + target_idx]
                draw_glow_line(draw, p1, p2, (0, 242, 254, 80), width=1)
                
        # Draw Cone longitudinal lines
        for r_idx in range(12):
            for z_idx in range(8):
                p1 = proj_cone[z_idx * 12 + r_idx]
                p2 = proj_cone[(z_idx + 1) * 12 + r_idx]
                draw_glow_line(draw, p1, p2, (0, 242, 254, 60), width=1)
                
        # 4. Axial Unchanged Signal Vector
        for center_x, w_x, w_y, w_z, color in [
            (rod_center_x, wobble_x, wobble_y, wobble_z, (34, 197, 94, 255)),
            (cone_center_x, -wobble_x, -wobble_y, -wobble_z, (34, 197, 94, 255))
        ]:
            slide_x = center_x * (1.0 - morph_progress)
            p1_x, p1_y, _ = project_3d(slide_x + w_x, w_y, -180 + w_z, cam_x, cam_y, cam_z, yaw, pitch, zoom)
            p2_x, p2_y, _ = project_3d(slide_x + w_x, w_y, 180 + w_z, cam_x, cam_y, cam_z, yaw, pitch, zoom)
            draw_glow_line(draw, (p1_x, p1_y), (p2_x, p2_y), color, width=3)
            
        # Draw dynamic particle streams flowing from center (Xi core) to outer structures during calculations
        for sign in [-1, 1]:
                for step in range(24):
                    frac = step / 23.0
                    dist = frac * 200 * sign
                    p_offset = (t_secs * 2.0) % 1.0
                    frac_anim = (frac + p_offset) % 1.0
                    anim_dist = frac_anim * 200 * sign
                    angle = frac_anim * 5 * np.pi + t_secs * 6.0
                    sx = anim_dist
                    sy = 22 * math.sin(angle)
                    sz = 22 * math.cos(angle)
                    px, py, _ = project_3d(sx, sy, sz, cam_x, cam_y, cam_z, yaw, pitch, zoom)
                    
                    alpha = int(140 * (1.0 - frac_anim))
                    if alpha > 5:
                        draw.ellipse([px-4, py-4, px+4, py+4], fill=(255, 255, 255, int(alpha * 0.3)))
                        draw.ellipse([px-1.5, py-1.5, px+1.5, py+1.5], fill=(255, 255, 255, alpha))



        # 5. Live State / Data Registers Overlay Panel (with glassmorphic glowing border)
        panel_y = 440
        
        # Left Panel (Rod) Glow Border & Fill
        r_color = (168, 85, 247, 120)
        draw_glow_line(draw, (30, panel_y), (590, panel_y), r_color, width=1)
        draw_glow_line(draw, (590, panel_y), (590, panel_y + 260), r_color, width=1)
        draw_glow_line(draw, (590, panel_y + 260), (30, panel_y + 260), r_color, width=1)
        draw_glow_line(draw, (30, panel_y + 260), (30, panel_y), r_color, width=1)
        draw.rectangle([30, panel_y, 590, panel_y + 260], fill=(6, 9, 18, 200))
        
        draw.text((50, panel_y + 10), "SHA ROD REGISTER STATE (SROD):", fill=(168, 85, 247), font=font_bold)
        
        r_ident_str = f"{states['r_identity']}" if t_secs >= 15.0 else "INITIALIZING..."
        
        draw.text((50, panel_y + 35), f"Base:       {states['r_base']}", fill=(255, 255, 255), font=font_regular)
        draw.text((50, panel_y + 55), f"Secret:     {RodState['Secret_init']}", fill=(255, 255, 255), font=font_regular)
        draw.text((50, panel_y + 75), f"Signal:     {RodState['Signal_init']} [UNCHANGED]", fill=(34, 197, 94), font=font_regular)
        draw.text((50, panel_y + 95), f"Channel:    {states['r_channel']}", fill=(0, 242, 254), font=font_regular)
        draw.text((50, panel_y + 115), f"Coord:      {states['r_contour']}", fill=(168, 85, 247), font=font_regular)
        draw.text((50, panel_y + 135), f"Pole:       {states['r_pole']}", fill=(244, 63, 94), font=font_regular)
        draw.text((50, panel_y + 155), f"Identity:   {r_ident_str}", fill=(255, 213, 79), font=font_regular)
        draw.text((50, panel_y + 175), f"Foundation: {states['r_foundation']}", fill=(255, 255, 255, 180), font=font_regular)
        draw.text((50, panel_y + 195), f"Charge/Chin: {states['r_charge']} / {states['r_chin']}", fill=(255, 255, 255, 140), font=font_regular)
        draw.text((50, panel_y + 215), f"Element:    {states['r_element']}", fill=(255, 213, 79), font=font_regular)
        draw.text((50, panel_y + 235), f"Dynamo:     {states['r_dynamo']}", fill=(57, 255, 20), font=font_regular)
        
        # Right Panel (Cone) Glow Border & Fill
        c_color = (0, 242, 254, 120)
        draw_glow_line(draw, (WIDTH - 590, panel_y), (WIDTH - 30, panel_y), c_color, width=1)
        draw_glow_line(draw, (WIDTH - 30, panel_y), (WIDTH - 30, panel_y + 260), c_color, width=1)
        draw_glow_line(draw, (WIDTH - 30, panel_y + 260), (WIDTH - 590, panel_y + 260), c_color, width=1)
        draw_glow_line(draw, (WIDTH - 590, panel_y + 260), (WIDTH - 590, panel_y), c_color, width=1)
        draw.rectangle([WIDTH - 590, panel_y, WIDTH - 30, panel_y + 260], fill=(6, 9, 18, 200))
        
        draw.text((WIDTH - 570, panel_y + 10), "SHA CONE REGISTER STATE (SCONE):", fill=(0, 242, 254), font=font_bold)
        
        c_ident_str = f"{states['c_identity']}" if t_secs >= 15.0 else "INITIALIZING..."
        
        draw.text((WIDTH - 570, panel_y + 35), f"Base:       {states['c_base']}", fill=(255, 255, 255), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 55), f"Secret:     {ConeState['Secret_init']}", fill=(255, 255, 255), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 75), f"Signal:     {ConeState['Signal_init']} [UNCHANGED]", fill=(34, 197, 94), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 95), f"Channel:    {states['c_channel']}", fill=(0, 242, 254), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 115), f"Coord:      {states['c_contour']}", fill=(168, 85, 247), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 135), f"Pole:       {states['c_pole']}", fill=(244, 63, 94), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 155), f"Identity:   {c_ident_str}", fill=(255, 213, 79), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 175), f"Foundation: {states['c_foundation']}", fill=(255, 255, 255, 180), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 195), f"Charge/Chin: {states['c_charge']} / {states['c_chin']}", fill=(255, 255, 255, 140), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 215), f"Element:    {states['c_element']}", fill=(255, 213, 79), font=font_regular)
        draw.text((WIDTH - 570, panel_y + 235), f"Dynamo:     {states['c_dynamo']}", fill=(57, 255, 20), font=font_regular)
        
        # 6. Global Math info overlay
        draw.text((WIDTH/2 - 180, 70), "MODULAR HARDWARE REGISTER LORE", fill=(255, 255, 255, 180), font=font_bold)
        draw.text((WIDTH/2 - 180, 90), f"MotzkinPrime = {MotzkinPrime}", fill=(255, 213, 79), font=font_regular)
        draw.text((WIDTH/2 - 180, 110), "Equation: Channel = modExp(Base, Signal, Prime)", fill=(0, 242, 254), font=font_regular)
        draw.text((WIDTH/2 - 180, 130), "YI Birthing Anatomy: Bao (Nu Envelope)", fill=(244, 63, 94), font=font_regular)
        
        # Project and Draw the Bao Envelope (An outer protective 3D capsule mapping around the birth canal Rod & Cone)
        # Pulsing and opacity animate smoothly over time
        bao_pulse = 1.0 + 0.03 * math.sin(t_secs * 2 * math.pi / 4.0)
        bao_opacity = int(12 + 18 * smooth_step(0.0, 45.0, t_secs) + 4 * math.sin(t_secs * 2 * math.pi / 2.0))
        
        for i in range(8):
            # Each longitudinal loop has its own offset that is continuous
            loop_spin = (1.5 - 0.2 * i)
            phi_offset = (t_secs * loop_spin) % (2 * np.pi)
            
            angle = i * np.pi / 8
            # Render longitudinal loops of the capsule
            points = []
            for j in range(24):
                phi = (j * 2 * np.pi / 24) + phi_offset
                # Ellipsoidal radius enclosing both centers, with smooth breathing pulse
                bx = 350 * math.cos(phi) * math.sin(angle) * bao_pulse
                by = 180 * math.sin(phi) * math.sin(angle) * bao_pulse
                bz = 180 * math.cos(angle) * bao_pulse
                
                px, py, depth = project_3d(bx, by, bz, cam_x, cam_y, cam_z, yaw, pitch, zoom)
                points.append((px, py))
            
            # Draw Bao envelope contour lines
            for j in range(24):
                draw.line([points[j], points[(j+1)%24]], fill=(244, 63, 94, max(0, min(255, bao_opacity))), width=1)
        
        # Feed frame into pipeline
        img.save(pipe.stdin, "PNG")
        
    pipe.stdin.close()
    pipe.wait()
    print("[Void System] Video generation complete.")
 
if __name__ == "__main__":
    render_video()
