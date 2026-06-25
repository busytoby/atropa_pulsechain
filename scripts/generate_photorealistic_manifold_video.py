import os
import sys
import math
import random
import wave
import struct
import subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

# --- Settings ---
SAMPLE_RATE = 44100
DURATION_SECS = 32 # Shortened from 74 to 32 seconds to optimize render speed and match controlnet layers
FPS = 30
WIDTH = 1280
HEIGHT = 720
TOTAL_FRAMES = DURATION_SECS * FPS  # 960 frames

# Super-sampling scale (1x for speed, can be 2x for production)
SCALE = 1
W_S = WIDTH * SCALE
H_S = HEIGHT * SCALE

EPOCHS = [
    {"num": 0, "name": "INIT", "formula": "Rod->Base, Secret, Signal = RANDOM()"},
    {"num": 1, "name": "AVAIL", "formula": "Rod->Contour = Xi ^ Rod->Secret mod Prime"},
    {"num": 2, "name": "FORM (SWAP 1)", "formula": "Rod->Base = Cone->Contour ^ Rod->Secret mod Prime"},
    {"num": 3, "name": "POLARIZE", "formula": "Rod->Pole = Rod->Base ^ Rod->Secret mod Prime"},
    {"num": 4, "name": "CONJUGATE (SWAP 2)", "formula": "Rod->Coord = Cone->Pole ^ Rod->Secret mod Prime"},
    {"num": 5, "name": "CONIFY", "formula": "Rod->Foundation = Rod->Base ^ Cone->Identity mod Prime"},
    {"num": 6, "name": "SATURATE (SWAP 3)", "formula": "Rod->Element = Beta + Limit + Charge"},
    {"num": 7, "name": "IONIZE", "formula": "Rod->Dynamo = Rod->Base ^ Rod->Signal mod Rod->Element"},
    {"num": 8, "name": "MAGNETIZE (LOCK)", "formula": "Assert: Adduct(Rod, Cone->Dyn) == Adduct(Cone, Rod->Dyn)"}
]

def load_wav_samples(filename):
    if not os.path.exists(filename):
        print(f"[ERROR] Audio file {filename} not found.")
        sys.exit(1)
    with wave.open(filename, "rb") as wf:
        params = wf.getparams()
        frames = wf.readframes(params.nframes)
        count = params.nframes * params.nchannels
        samples = list(struct.unpack(f"<{count}h", frames))
        if params.nchannels > 1:
            samples = samples[::params.nchannels]
        return [s / 32768.0 for s in samples]

GLOBAL_CAM_ROLL = 0.0

# 3D Projection functions (scaled to SSAO dimensions)
def project_3d(x, y, z, cx, cy, cz, pitch, yaw, fov, w, h, tx=0, ty=0, tz=0, roll=None):
    global GLOBAL_CAM_ROLL
    if roll is None:
        roll = GLOBAL_CAM_ROLL
    x_tr = x - (cx + tx)
    y_tr = y - (cy + ty)
    z_tr = z - (cz + tz)
    
    cos_y, sin_y = math.cos(yaw), math.sin(yaw)
    x1 = x_tr * cos_y - z_tr * sin_y
    z1 = x_tr * sin_y + z_tr * cos_y
    y1 = y_tr
    
    cos_p, sin_p = math.cos(pitch), math.sin(pitch)
    y2 = y1 * cos_p - z1 * sin_p
    z2 = y1 * sin_p + z1 * cos_p
    x2 = x1

    # Apply camera roll rotation
    cos_r, sin_r = math.cos(roll), math.sin(roll)
    x3 = x2 * cos_r - y2 * sin_r
    y3 = x2 * sin_r + y2 * cos_r
    
    if z2 <= 1.0:
        return None, None, z2
        
    scale = fov / z2
    screen_x = int(w / 2 + x3 * scale)
    screen_y = int(h / 2 + y3 * scale)
    return screen_x, screen_y, z2

def main():
    print("=== STARTING MULTI-PASS SHADER VIDEO GENERATOR ===")
    audio_path = "cloudburst_ambient_resonance.wav"
    audio_data = load_wav_samples(audio_path)
    print(f"[AUDIO] Loaded {len(audio_data)} samples.")

    # FFmpeg command to merge piped raw RGB24 frames with looped WAV audio
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "rawvideo",
        "-pix_fmt", "rgb24",
        "-s", f"{WIDTH}x{HEIGHT}",
        "-r", str(FPS),
        "-i", "-",
        "-stream_loop", "-1",
        "-i", audio_path,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        "ambient_manifold_photorealistic.mp4"
    ]
    
    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
    samples_per_frame = len(audio_data) // TOTAL_FRAMES

    # Initialize 3D particles
    particles = []
    for _ in range(300):
        particles.append({
            "x": random.uniform(-450, 450),
            "y": random.uniform(-150, 150),
            "z": random.uniform(-450, 450),
            "speed": random.uniform(2.0, 5.0),
            "phase": random.uniform(0, math.pi * 2)
        })

    # Initialize 64 chattering LAUs
    laus = []
    random.seed(42) # Auncient seed
    for i in range(64):
        laus.append({
            "id": f"LAU_0x{random.randint(0x1000, 0xFFFF):04X}",
            "x": random.uniform(-400, 400),
            "y": random.uniform(-100, 50),
            "z": random.uniform(-400, 400),
            "ip": f"10.0.{random.randint(1, 254)}.{random.randint(1, 254)}",
            "active_frame_start": int(i * (TOTAL_FRAMES / 64.0)),
        })

    # Initialize 3D Solidity contract topology
    contracts = [
        {"name": "ATROPA", "x": -180, "y": -50, "z": 120, "addr": "0x7a20...3414"},
        {"name": "TREBIZOND", "x": 180, "y": -50, "z": -120, "addr": "0x9030...0F78"},
        {"name": "USDC", "x": -280, "y": 70, "z": 220, "addr": "0xA0b8...eB48"},
        {"name": "METIS", "x": 280, "y": 70, "z": -220, "addr": "0x36d4...b4e1"},
        {"name": "TEDDYBEAR", "x": -120, "y": 110, "z": 320, "addr": "0xd6c3...8f6d"},
        {"name": "NONUKES", "x": 120, "y": 110, "z": -320, "addr": "0x174A...Fd62"},
        {"name": "CHEROKEE", "x": -320, "y": -90, "z": -170, "addr": "0xb4C1...2b10"},
        {"name": "BUCKINGHAM", "x": 320, "y": -90, "z": 170, "addr": "0xe5d3...5f11"},
        {"name": "LIBERTY", "x": 0, "y": 130, "z": -70, "addr": "0xFE9b...Cb61"},
        {"name": "CROWS", "x": 0, "y": -130, "z": 70, "addr": "0x203e...4fa1"}
    ]

    rod_3d = (-220, 0, 0)
    cone_3d = (220, 0, 0)

    prev_img = None

    # Initialize cinematic camera tracking states for smooth gliding transitions
    cam_x, cam_y, cam_z = 0.0, -150.0, -550.0
    cam_yaw, cam_pitch, GLOBAL_CAM_ROLL = -0.25, 0.35, 0.0
    fov = float(680 * SCALE)

    print("[VIDEO] Executing multi-pass render loops...")
    for frame_idx in range(TOTAL_FRAMES):
        time_secs = frame_idx / FPS
        # Continuous epoch progression to render smooth transitions
        epoch_progression = (time_secs / DURATION_SECS) * (len(EPOCHS) - 1)
        epoch_idx = int(epoch_progression)
        next_epoch_idx = min(len(EPOCHS) - 1, epoch_idx + 1)
        epoch_t = epoch_progression - epoch_idx  # transition weight between 0.0 and 1.0
        
        epoch = EPOCHS[epoch_idx]

        # Audio amplitude mapping
        start_s = frame_idx * samples_per_frame
        end_s = start_s + samples_per_frame
        chunk = audio_data[start_s:end_s]
        rms = np.sqrt(np.mean(np.array(chunk)**2)) if len(chunk) > 0 else 0
        amp = max(0.0, min(1.0, rms * 4.0))

        # Create base background canvas in SSAA size
        img = Image.new("RGBA", (W_S, H_S), color=(2, 3, 5, 255))
        draw = ImageDraw.Draw(img)

        # Determine active LAU node target
        active_lau_idx = min(63, int(frame_idx // (TOTAL_FRAMES / 64.0)))
        active_lau = laus[active_lau_idx]

        # Maintain a completely steady, normal perspective viewport.
        # Remove all dynamic panning, rolling (roll is fixed to 0.0), and zooming/cuts.
        cam_x, cam_y, cam_z = 0.0, -100.0, -580.0
        cam_yaw, cam_pitch, GLOBAL_CAM_ROLL = -0.15, 0.25, 0.0
        fov = float(720 * SCALE)
        look_x, look_y, look_z = 0.0, 0.0, 0.0
        focal_dist = abs(cam_z)
 
        # Target look-at coordinates remain completely centered and steady at the origin
        look_x, look_y, look_z = 0.0, 0.0, 0.0

        # Interpolated colors/properties based on epoch transition weight epoch_t
        def interpolate_color(c1, c2, t):
            return (
                int(c1[0] + (c2[0] - c1[0]) * t),
                int(c1[1] + (c2[1] - c1[1]) * t),
                int(c1[2] + (c2[2] - c1[2]) * t)
            )

        def draw_3d_transfer_line(draw, x1, y1, z1, x2, y2, z2, t_pulse, label_lr, label_rl, weight=1.0):
            sx1, sy1, sz1 = project_3d(x1, y1, z1, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            sx2, sy2, sz2 = project_3d(x2, y2, z2, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if sx1 is not None and sx2 is not None:
                draw.line([(sx1, sy1), (sx2, sy2)], fill=(40, 40, 60, int(180 * weight)), width=int(2 * SCALE))
                # moving packet L -> R
                p_t = (t_pulse * 1.8) % 1.0
                px = sx1 + (sx2 - sx1) * p_t
                py = sy1 + (sy2 - sy1) * p_t
                p_size = 4 * SCALE
                draw.ellipse([px - p_size, py - p_size, px + p_size, py + p_size], fill=(57, 255, 20, int(255 * weight)))
                
                # moving packet R -> L
                p_t2 = (1.0 - p_t)
                px2 = sx1 + (sx2 - sx1) * p_t2
                py2 = sy1 + (sy2 - sy1) * p_t2
                draw.ellipse([px2 - p_size, py2 - p_size, px2 + p_size, py2 + p_size], fill=(0, 240, 255, int(255 * weight)))
                
                # Labels
                draw.text((sx1 + 10*SCALE, sy1 - 25*SCALE), f"> {label_lr}", fill=(57, 255, 20, int(220 * weight)))
                draw.text((sx2 - 190*SCALE, sy2 + 18*SCALE), f"{label_rl} <", fill=(0, 240, 255, int(220 * weight)))

        # Base node colors for the epoch states
        color_map = {
            0: ((100, 100, 100), (80, 80, 80)), # INIT
            1: ((57, 255, 20), (0, 240, 255)),  # AVAIL
            2: ((255, 49, 49), (255, 49, 49)),  # FORM
            3: ((57, 255, 20), (0, 240, 255)),  # POLARIZE
            4: ((255, 49, 49), (255, 49, 49)),  # CONJUGATE
            5: ((128, 0, 128), (0, 128, 128)),  # CONIFY
            6: ((255, 105, 180), (255, 20, 147)), # SATURATE
            7: ((0, 240, 255), (57, 255, 20)),  # IONIZE
            8: ((255, 215, 0), (255, 215, 0))   # MAGNETIZE
        }

        rod_c_curr, cone_c_curr = color_map[epoch_idx]
        rod_c_next, cone_c_next = color_map[next_epoch_idx]
        rod_glow_color = interpolate_color(rod_c_curr, rod_c_next, epoch_t)
        cone_glow_color = interpolate_color(cone_c_curr, cone_c_next, epoch_t)

        # 1. Geometry Pass: 3D Banach-Hilbert substrate grid with interpolated warp
        grid_color_curr = (25, 30, 45, 255) if epoch_idx < 8 else (42, 35, 22, 255)
        grid_color_next = (25, 30, 45, 255) if next_epoch_idx < 8 else (42, 35, 22, 255)
        grid_color = interpolate_color(grid_color_curr[:3], grid_color_next[:3], epoch_t) + (255,)
        grid_extent = 500
        grid_step = 50
        
        # Grid lines parallel to Z-axis
        for gx in range(-grid_extent, grid_extent + 1, grid_step):
            pts = []
            for gz in range(-grid_extent, grid_extent + 1, 50):
                # Interpolate grid warping dynamically across transitions (vortex funnel morph)
                gy = 100
                if epoch_idx >= 5 or next_epoch_idx >= 5:
                    t_val = epoch_t if epoch_idx == 5 else (1.0 if epoch_idx > 5 else 0.0)
                    dist_to_center = math.sqrt(gx**2 + gz**2)
                    gy += int((dist_to_center * 0.3) * t_val)
                sx, sy, sz = project_3d(gx, gy, gz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                    pts.append((sx, sy))
            if len(pts) > 1:
                draw.line(pts, fill=grid_color, width=1 * SCALE)
                
        # Grid lines parallel to X-axis
        for gz in range(-grid_extent, grid_extent + 1, grid_step):
            pts = []
            for gx in range(-grid_extent, grid_extent + 1, 50):
                gy = 100
                if epoch_idx >= 5 or next_epoch_idx >= 5:
                    t_val = epoch_t if epoch_idx == 5 else (1.0 if epoch_idx > 5 else 0.0)
                    dist_to_center = math.sqrt(gx**2 + gz**2)
                    gy += int((dist_to_center * 0.3) * t_val)
                sx, sy, sz = project_3d(gx, gy, gz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                    pts.append((sx, sy))
            if len(pts) > 1:
                draw.line(pts, fill=grid_color, width=1 * SCALE)

        # Projected Node Centers
        rx, ry, rz = project_3d(rod_3d[0], rod_3d[1], rod_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
        cx, cy, cz = project_3d(cone_3d[0], cone_3d[1], cone_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)

        # 2. Bloom & Light Emission Pass
        bloom_layer = Image.new("RGBA", (W_S, H_S), (0, 0, 0, 0))
        bloom_draw = ImageDraw.Draw(bloom_layer)

        # Draw glowing spots for bloom
        if rx is not None:
            bloom_draw.ellipse([rx - 140*SCALE, ry - 140*SCALE, rx + 140*SCALE, ry + 140*SCALE], fill=(rod_glow_color[0], rod_glow_color[1], rod_glow_color[2], 130))
        if cx is not None:
            bloom_draw.ellipse([cx - 140*SCALE, cy - 140*SCALE, cx + 140*SCALE, cy + 140*SCALE], fill=(cone_glow_color[0], cone_glow_color[1], cone_glow_color[2], 130))

        # Perform Gaussian Blur on Bloom Layer
        bloom_blurred = bloom_layer.filter(ImageFilter.GaussianBlur(radius=30*SCALE))
        img = Image.alpha_composite(img, bloom_blurred)
        draw = ImageDraw.Draw(img)

        # 3. Shader & Texture Pass: Render 3D nodes with Phong shading
        def draw_shaded_node(draw, nx, ny, nz, namp, color, is_rod):
            nonlocal img
            size = (45 + namp * 5) * SCALE
            
            # Draw Core Rotating 3D Triangle Prism
            angle = time_secs * 4.0 * (1 if is_rod else -1)
            core_size = (28 + namp * 12) * SCALE
            
            # Form 3D vertices of a prism/tetrahedron
            nodes = [
                (0, -core_size, 0),
                (core_size * math.cos(angle), core_size * 0.5, core_size * math.sin(angle)),
                (core_size * math.cos(angle + 2*math.pi/3), core_size * 0.5, core_size * math.sin(angle + 2*math.pi/3)),
                (core_size * math.cos(angle + 4*math.pi/3), core_size * 0.5, core_size * math.sin(angle + 4*math.pi/3)),
            ]
            
            proj_pts = []
            for (vx, vy, vz) in nodes:
                sx, sy, sz = project_3d(nx + vx, ny + vy, nz + vz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None:
                    proj_pts.append((sx, sy))
            
            if len(proj_pts) == 4:
                # Translucent faces representing volumetric plasma cores
                draw.polygon([proj_pts[0], proj_pts[1], proj_pts[2]], outline=color + (255,), fill=(color[0], color[1], color[2], 40))
                draw.polygon([proj_pts[0], proj_pts[2], proj_pts[3]], outline=color + (255,), fill=(color[0], color[1], color[2], 40))
                draw.polygon([proj_pts[0], proj_pts[3], proj_pts[1]], outline=color + (255,), fill=(color[0], color[1], color[2], 40))

            # Fast Rendering technique (>10fps): draw layered concentric gradient circles to achieve shading casing
            casing_layer = Image.new("RGBA", (W_S, H_S), (0, 0, 0, 0))
            c_draw = ImageDraw.Draw(casing_layer)
            # Specular light highlight offset offset (shifts light reflection center to top-left)
            light_offset = int(12 * SCALE)
            
            # Base casing color (dark body)
            c_draw.ellipse([nx - size, ny - size, nx + size, ny + size], fill=(int(color[0]*0.4), int(color[1]*0.4), int(color[2]*0.4), 255))
            
            # Layered diffuse highlight sphere levels
            for i in range(1, 6):
                ratio = 1.0 - (i / 6.0)
                r_size = size * ratio
                # Shift centers to simulate light projection
                cx_shifted = nx - int(light_offset * (1.0 - ratio))
                cy_shifted = ny - int(light_offset * (1.0 - ratio))
                diff_gain = 0.4 + 0.6 * (i / 6.0)
                c_draw.ellipse(
                    [cx_shifted - r_size, cy_shifted - r_size, cx_shifted + r_size, cy_shifted + r_size], 
                    fill=(int(color[0] * diff_gain), int(color[1] * diff_gain), int(color[2] * diff_gain), 200)
                )
                
            # Specular hot-spot overlay (Phong replacement)
            spec_r = size * 0.18
            cx_spec = nx - int(light_offset * 0.7)
            cy_spec = ny - int(light_offset * 0.7)
            c_draw.ellipse(
                [cx_spec - spec_r, cy_spec - spec_r, cx_spec + spec_r, cy_spec + spec_r],
                fill=(255, 255, 255, 220)
            )
            
            img = Image.alpha_composite(img, casing_layer)
            draw = ImageDraw.Draw(img)

        # Drawing the shaded nodes
        if rx is not None:
            draw_shaded_node(draw, rx, ry, rz, amp, rod_glow_color, True)
        if cx is not None:
            draw_shaded_node(draw, cx, cy, cz, amp, cone_glow_color, False)

        # 4. Volumetric Plasma Connections Pass with interpolated transitions
        # Epoch 1: AVAIL wavefront ripples from center
        if epoch_idx == 1 or (epoch_idx == 0 and epoch_t > 0.1) or (epoch_idx == 1 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 0 else (1.0 - epoch_t if epoch_idx == 1 else 1.0)
            wave_rad = int((time_secs * 200) % 250)
            for r_offset in [0, -80, -160]:
                curr_r = (wave_rad + r_offset) % 250
                if curr_r > 0:
                    pts_ring = []
                    for deg in range(0, 360, 15):
                        rad_a = math.radians(deg)
                        wx = curr_r * math.cos(rad_a)
                        wz = curr_r * math.sin(rad_a)
                        wsx, wsy, wsz = project_3d(wx, 0, wz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                        if wsx is not None:
                            pts_ring.append((wsx, wsy))
                    if len(pts_ring) > 2:
                        draw.polygon(pts_ring, outline=(0, 240, 255, int(130 * weight)), width=int(2 * SCALE))

        # Epoch 2: FORM / barrier transition and contour transfer line
        if epoch_idx == 2 or (epoch_idx == 1 and epoch_t > 0.1) or (epoch_idx == 2 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 1 else (1.0 - epoch_t if epoch_idx == 2 else 1.0)
            sx1, sy1, sz1 = project_3d(0, -180 * weight, -100, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            sx2, sy2, sz2 = project_3d(0, 180 * weight, 100, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if sx1 is not None and sx2 is not None:
                draw.line([(sx1, sy1), (sx2, sy2)], fill=(255, 49, 49, int(200 * weight)), width=int(4 * SCALE * weight) or 1)
            draw_3d_transfer_line(draw, rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, "ROD CONTOUR -> CONE BASE", "CONE CONTOUR -> ROD BASE", weight)

        # Epoch 3: POLARIZE / orthogonal grids
        if epoch_idx == 3 or (epoch_idx == 2 and epoch_t > 0.1) or (epoch_idx == 3 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 2 else (1.0 - epoch_t if epoch_idx == 3 else 1.0)
            # Vertical line crossing Rod node
            pts_vert = []
            for gy in range(-200, 201, 50):
                sx, sy, sz = project_3d(rod_3d[0], gy, rod_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None:
                    pts_vert.append((sx, sy))
            if len(pts_vert) > 1:
                draw.line(pts_vert, fill=(57, 255, 20, int(180 * weight)), width=int(3 * SCALE))
                
            # Horizontal line crossing Cone node
            pts_horiz = []
            for gx in range(-300, 301, 50):
                sx, sy, sz = project_3d(gx, cone_3d[1], cone_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None:
                    pts_horiz.append((sx, sy))
            if len(pts_horiz) > 1:
                draw.line(pts_horiz, fill=(0, 240, 255, int(180 * weight)), width=int(3 * SCALE))

        # Epoch 4: CONJUGATE / lightning arcs transition and pole transfer line
        if epoch_idx == 4 or (epoch_idx == 3 and epoch_t > 0.1) or (epoch_idx == 4 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 3 else (1.0 - epoch_t if epoch_idx == 4 else 1.0)
            if rx is not None and cx is not None:
                random.seed(frame_idx)
                l_points = [(rx, ry)]
                for step in range(1, 8):
                    tx = rx + (cx - rx) * (step / 8.0)
                    ty = ry + (cy - ry) * (step / 8.0) + random.randint(-int(35*SCALE*weight), int(35*SCALE*weight))
                    l_points.append((tx, ty))
                l_points.append((cx, cy))
                for k in range(len(l_points) - 1):
                    draw.line([l_points[k], l_points[k+1]], fill=(255, 49, 49, int(255 * weight)), width=int(3 * SCALE * weight) or 1)
            draw_3d_transfer_line(draw, rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, "ROD POLE -> CONE COORD", "CONE POLE -> ROD COORD", weight)

        # Epoch 5: CONIFY / compression vortex funnel lines
        if epoch_idx == 5 or (epoch_idx == 4 and epoch_t > 0.1) or (epoch_idx == 5 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 4 else (1.0 - epoch_t if epoch_idx == 5 else 1.0)
            for angle in range(0, 360, 45):
                rad_a = math.radians(angle + time_secs * 60)
                px = math.cos(rad_a) * 150
                pz = math.sin(rad_a) * 150
                psx, psy, psz = project_3d(px, 0, pz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                msx, msy, msz = project_3d(0, 0, 0, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if psx is not None and msx is not None:
                    draw.line([(psx, psy), (msx, msy)], fill=(255, 215, 0, int(150 * weight)), width=int(1 * SCALE))

        # Epoch 6: SATURATE / expanding blast wave and foundation transfer line
        if epoch_idx == 6 or (epoch_idx == 5 and epoch_t > 0.1) or (epoch_idx == 6 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 5 else (1.0 - epoch_t if epoch_idx == 6 else 1.0)
            blast_rad = int(30 + (time_secs * 250) % 250)
            pts_blast = []
            for deg in range(0, 360, 15):
                rad_a = math.radians(deg)
                bx = blast_rad * math.cos(rad_a)
                bz = blast_rad * math.sin(rad_a)
                bsx, bsy, bsz = project_3d(bx, 0, bz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if bsx is not None:
                    pts_blast.append((bsx, bsy))
            if len(pts_blast) > 2:
                draw.polygon(pts_blast, outline=(255, 49, 49, int(160 * weight)), width=int(3 * SCALE))
            draw_3d_transfer_line(draw, rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, "ROD FOUNDATION -> CONE ELEMENT", "CONE FOUNDATION -> ROD ELEMENT", weight)

        # Epoch 7: IONIZE / plasma bridge transition
        if epoch_idx == 7 or (epoch_idx == 6 and epoch_t > 0.1) or (epoch_idx == 7 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 6 else (1.0 - epoch_t if epoch_idx == 7 else 1.0)
            if rx is not None and cx is not None:
                draw.line([(rx, ry), (cx, cy)], fill=(0, 240, 255, int(120 * weight)), width=int(10 * SCALE * weight) or 1)
                for offset in range(-int(6*SCALE*weight), int(7*SCALE*weight), 3*SCALE or 1):
                    l_points = [(rx, ry)]
                    for step in range(1, 10):
                        tx = rx + (cx - rx) * (step / 10.0)
                        ty = ry + (cy - ry) * (step / 10.0) + math.sin(time_secs * 25 + step) * 12 * SCALE * weight + offset
                        l_points.append((tx, ty))
                    l_points.append((cx, cy))
                    for k in range(len(l_points) - 1):
                        draw.line([l_points[k], l_points[k+1]], fill=(255, 255, 255, int(200 * weight)), width=int(2 * SCALE * weight) or 1)
                        
        # Epoch 8: MAGNETIZE / locked mandala lattice transition
        if epoch_idx == 8 or (epoch_idx == 7 and epoch_t > 0.1):
            weight = epoch_t if epoch_idx == 7 else 1.0
            if rx is not None and cx is not None:
                mid_pt_x, mid_pt_y = (rx + cx) // 2, (ry + cy) // 2
                
                # Draw Barn thermodynamic "heat" glow
                barn_layer = Image.new("RGBA", (W_S, H_S), (0, 0, 0, 0))
                b_draw = ImageDraw.Draw(barn_layer)
                for h_r in range(int(240*SCALE*weight), 0, -6*SCALE or 1):
                    alpha = int(50 * (1.0 - (h_r / (240.0*SCALE*weight))) * weight)
                    b_draw.ellipse([mid_pt_x - h_r, mid_pt_y - h_r, mid_pt_x + h_r, mid_pt_y + h_r], fill=(255, 69, 0, alpha))
                img = Image.alpha_composite(img, barn_layer)
                draw = ImageDraw.Draw(img)

                # Golden background concentric mandala discs
                for m_r in range(int(140*SCALE*weight), int(20*SCALE*weight), -30*SCALE or 1):
                    draw.ellipse([mid_pt_x - m_r, mid_pt_y - m_r, mid_pt_x + m_r, mid_pt_y + m_r], outline=(255, 215, 0, int(160 * weight)), width=int(2 * SCALE * weight) or 1)
                draw.rectangle([mid_pt_x - int(180*SCALE*weight), mid_pt_y - int(120*SCALE*weight), mid_pt_x + int(180*SCALE*weight), mid_pt_y + int(120*SCALE*weight)], outline=(255, 215, 0, int(100 * weight)), width=int(2 * SCALE * weight) or 1)

        # 3D Verlet Particles Flowing (continuous orbit transitions)
        for p in particles:
            if epoch_idx == 1 or (epoch_idx == 0 and epoch_t > 0.5):
                t_val = epoch_t if epoch_idx == 0 else (1.0 - epoch_t if epoch_idx == 1 else 1.0)
                p["x"] += p["speed"] * 1.5 * t_val
                if p["x"] > 350:
                    p["x"] = -350
                    p["y"] = random.uniform(-20, 20)
                    p["z"] = random.uniform(-20, 20)
            elif epoch_idx == 5 or (epoch_idx == 4 and epoch_t > 0.5):
                t_val = epoch_t if epoch_idx == 4 else (1.0 - epoch_t if epoch_idx == 5 else 1.0)
                rad = math.sqrt(p["x"]**2 + p["z"]**2) or 1
                theta = math.atan2(p["z"], p["x"]) + 0.05
                new_rad = rad - p["speed"] * 0.8 * t_val
                if new_rad < 10:
                    new_rad = 400
                p["x"] = new_rad * math.cos(theta)
                p["z"] = new_rad * math.sin(theta)
                p["y"] += (0 - p["y"]) * 0.05 * t_val
            else:
                theta = time_secs * 0.2 + p["phase"]
                p["x"] += math.sin(theta) * p["speed"] * 0.5
                p["z"] += math.cos(theta) * p["speed"] * 0.5
                p["y"] += math.sin(time_secs + p["phase"]) * 0.3

            sx, sy, sz = project_3d(p["x"], p["y"], p["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                p_size = max(1, int(5 * (400 / sz))) * SCALE
                p_alpha = min(255, max(30, int(255 * (1.0 - sz / 800))))
                p_color = (255, 215, 0, p_alpha) if epoch_idx == 8 else ((57, 255, 20, p_alpha) if p["x"] < 0 else (0, 240, 255, p_alpha))
                draw.ellipse([sx - p_size, sy - p_size, sx + p_size, sy + p_size], fill=p_color)

        # Draw the 64 chattering LAU nodes in 3D VOID space
        active_lau_idx = min(63, int(frame_idx // (TOTAL_FRAMES / 64.0)))
        for idx, lau in enumerate(laus):
            lsx, lsy, lsz = project_3d(lau["x"], lau["y"], lau["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if lsx is not None and 0 <= lsx < W_S and 0 <= lsy < H_S:
                if idx == active_lau_idx:
                    # Animate active LAU with flash pulse and volumetric line connection to nearest dynamo node
                    p_size = 9 * SCALE
                    # Pulsing orange/red color
                    pulse_val = int(127 + 128 * math.sin(time_secs * 20))
                    draw.ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(255, pulse_val, 50, 255), outline=(255, 255, 255, 255), width=2*SCALE)
                    
                    target_node = rod_3d if idx % 2 == 0 else cone_3d
                    tx, ty, tz = project_3d(target_node[0], target_node[1], target_node[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                    if tx is not None:
                        draw.line([(lsx, lsy), (tx, ty)], fill=(255, 180, 50, 180), width=int(2 * SCALE))
                else:
                    # Faint idle blue-cyan LAU chattering node
                    p_size = 2 * SCALE
                    draw.ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(0, 240, 255, 70))

        # Draw 3D Solidity contract topologies and their relations to ERIS center (0, 0, 0)
        esx, esy, esz = project_3d(0, 0, 0, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
        for c in contracts:
            csx, csy, csz = project_3d(c["x"], c["y"], c["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if csx is not None and 0 <= csx < W_S and 0 <= csy < H_S:
                c_size = 3 * SCALE
                # Bronze/copper tone nodes for contracts
                draw.ellipse([csx - c_size, csy - c_size, csx + c_size, csy + c_size], fill=(205, 127, 50, 160))
                draw.text((csx + 6 * SCALE, csy - 6 * SCALE), c["name"], fill=(205, 127, 50, 140))
                
                # Draw relative topology connector line to ERIS center (0, 0, 0)
                if esx is not None:
                    draw.line([(csx, csy), (esx, esy)], fill=(205, 150, 100, 35), width=int(1 * SCALE))

        # Draw topology connection links between related contracts
        topology_links = [(0, 1), (2, 3), (4, 5), (6, 7), (8, 9)]
        for c1_idx, c2_idx in topology_links:
            c1 = contracts[c1_idx]
            c2 = contracts[c2_idx]
            csx1, csy1, csz1 = project_3d(c1["x"], c1["y"], c1["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            csx2, csy2, csz2 = project_3d(c2["x"], c2["y"], c2["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if csx1 is not None and csx2 is not None:
                draw.line([(csx1, csy1), (csx2, csy2)], fill=(205, 127, 50, 60), width=int(1 * SCALE))

        # Dynamic State HUD Text
        draw.text((40*SCALE, 40*SCALE), "DYSNOMIA LABYRINTH MANIFOLD ENGINE", fill=(200, 200, 255, 255))
        draw.text((40*SCALE, 60*SCALE), f"ACTIVE PIPELINE: {epoch['name']}", fill=(255, 215, 0, 255) if epoch_idx == 8 else (0, 240, 255, 255))
        draw.text((40*SCALE, 80*SCALE), f"FORMULA: {epoch['formula']}", fill=(180, 180, 200, 255))
        draw.text((40*SCALE, 100*SCALE), f"AMPLITUDE RESONANCE: {int(amp * 100)}%", fill=(180, 180, 200, 255))

        # YI mechanical transducer and foundational random seed telemetry readout
        draw.text((40*SCALE, 130*SCALE), f"RANDOM SEED ROOT: 0x{42:016X}", fill=(100, 200, 150, 220))
        draw.text((40*SCALE, 150*SCALE), f"YI TRANSDUCER STAGE: {'BARRIER ACTIVE' if epoch_idx < 3 else 'LOCKED COMPLIANCE'}", fill=(255, 180, 50, 220))
        draw.text((40*SCALE, 170*SCALE), f"INPUT P_i: {1000 + int(amp * 500)} | BIAS V_b: {0.1 + amp * 0.9:.4f}V | GAIN: {150}", fill=(0, 240, 255, 200))

        draw.text((W_S - 280*SCALE, 40*SCALE), "STATE: RIGID LOCK" if epoch_idx == 8 else "STATE: RESOLVING CODES", fill=(57, 255, 20, 255) if epoch_idx == 8 else (255, 49, 49, 255))
        draw.text((W_S - 280*SCALE, 60*SCALE), f"FRAME INDEX: {frame_idx}/{TOTAL_FRAMES}", fill=(180, 180, 200, 255))
        draw.text((W_S - 280*SCALE, 80*SCALE), f"TEMPORAL STABILIZER: {time_secs:.2f}s", fill=(180, 180, 200, 255))

        # Scrolling LAU VOID Chatter Terminal (Bottom Left) with mock chat packets sent across the void
        draw.text((40*SCALE, H_S - 240*SCALE), "--- LAU VOID CHATTER & CHAT PACKETS ---", fill=(100, 180, 255, 180))
        chat_msgs = [
            "HELLO WORLD: SHA-Rod initialization complete",
            "Avail wavefront propagation active: Xi computed",
            "Form base established: Base reciprocity verified",
            "Polarize node swap triggered",
            "Conify compression phase initialized: V_out locked",
            "Saturate: Volumetric logic gates evolving",
            "Ionize: Transduction product cleared to state root"
        ]
        for log_offset, i in enumerate(range(max(0, active_lau_idx - 4), active_lau_idx + 1)):
            lau_log = laus[i]
            log_str = f"[{lau_log['active_frame_start']:03d}F] {lau_log['id']} ({lau_log['ip']}): {chat_msgs[i % len(chat_msgs)]}"
            draw.text((40*SCALE, H_S - 210*SCALE + log_offset * 22 * SCALE), log_str, fill=(100, 255, 150, 255) if i == active_lau_idx else (100, 180, 150, 150))

        # Shutter angle emulation / temporal Motion Blur
        if prev_img is not None:
            img = Image.blend(prev_img, img, 0.65)
        prev_img = img.copy()

        # Downsample SSAA image using Lanczos to match original resolution
        final_img = img.resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS)
        
        # --- 5. Post-Processing Pass ---
        arr = np.array(final_img, dtype=np.float32)
        
        # Auto-Exposure (modulates highlight gain dynamically to prevent sensor clipping during loud synth spikes)
        exposure = 1.0 - 0.20 * amp
        arr[:, :, :3] *= exposure
        
        # Fast Post-Processing technique (>12fps): native PIL color filters
        # Cinematic Film Grain & Vignette emulated via fast PIL image blending
        vignette_mask = Image.new("L", (WIDTH, HEIGHT), 255)
        v_draw = ImageDraw.Draw(vignette_mask)
        # Draw soft gradient vignette using concentric ellipses
        for r_step in range(HEIGHT, int(HEIGHT * 0.4), -8):
            opacity = int(120 * (1.0 - r_step / float(HEIGHT)))
            v_draw.ellipse([WIDTH//2 - r_step, HEIGHT//2 - r_step, WIDTH//2 + r_step, HEIGHT//2 + r_step], outline=0, width=8)
        
        # Vignette merge
        black_bg = Image.new("RGB", (WIDTH, HEIGHT), (0, 0, 0))
        rgb_img = Image.composite(final_img.convert("RGB"), black_bg, vignette_mask)
        
        # Super fast PIL chromatic aberration (shift offset by 2px)
        r, g, b = rgb_img.split()
        r_shifted = r.transform(r.size, Image.Transform.AFFINE, (1, 0, -2, 0, 1, 0))
        b_shifted = b.transform(b.size, Image.Transform.AFFINE, (1, 0, 2, 0, 1, 0))
        rgb_final = Image.merge("RGB", (r_shifted, g, b_shifted))

        # Write raw RGB24 bytes directly to the FFmpeg pipe
        process.stdin.write(rgb_final.tobytes())

        if frame_idx % 120 == 0:
            print(f"[VIDEO] Rendered {frame_idx}/{TOTAL_FRAMES} frames ({int(time_secs)}s elapsed)...")

    process.stdin.close()
    process.wait()
    print("=== VIDEO COMPILATION SUCCESSFULLY FINISHED: ambient_manifold_photorealistic.mp4 ===")

if __name__ == "__main__":
    main()
