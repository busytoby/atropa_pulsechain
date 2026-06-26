import os
import sys
import math
import random
import numpy as np
import wave
import struct
from PIL import Image, ImageDraw, ImageFilter

def load_wav_samples(filename):
    if not os.path.exists(filename):
        return [0.0] * (44100 * DURATION_SECS)
    with wave.open(filename, "rb") as wf:
        params = wf.getparams()
        frames = wf.readframes(params.nframes)
        count = params.nframes * params.nchannels
        samples = list(struct.unpack(f"<{count}h", frames))
        if params.nchannels > 1:
            samples = samples[::params.nchannels]
        return [s / 32768.0 for s in samples]

# --- Settings ---
DURATION_SECS = 32
FPS = 30
WIDTH = 1280
HEIGHT = 720
TOTAL_FRAMES = DURATION_SECS * FPS  # 960 frames
SCALE = 1  # Super-sampling scale (1 for high-performance direct rendering)
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

GLOBAL_CAM_ROLL = 0.0

# 3D Projection functions
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

def init_ffmpeg_pipe(filename):
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "rawvideo",
        "-pix_fmt", "rgb24",
        "-s", f"{WIDTH}x{HEIGHT}",
        "-r", str(FPS),
        "-i", "-",
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        filename
    ]
    return subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

# Main generation loop
def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument("--end-frame", type=int, default=960)
    parser.add_argument("--output-prefix", type=str, default="")
    args = parser.parse_args()

    start_frame = args.start_frame
    end_frame = args.end_frame
    prefix = args.output_prefix

    print("=== STARTING CONTROLNET LAYER VIDEO COMPILER ===")
    
    global subprocess
    import subprocess

    # Initialize layer video pipes
    pipes = {
        "lineart": init_ffmpeg_pipe(prefix + "manifold_layer_lineart.mp4"),
        "depth": init_ffmpeg_pipe(prefix + "manifold_layer_depth.mp4"),
        "normal": init_ffmpeg_pipe(prefix + "manifold_layer_normal.mp4"),
        "segmentation": init_ffmpeg_pipe(prefix + "manifold_layer_segmentation.mp4")
    }

    # Initialize 3D particles matching main video physics
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

    prev_canvases = {"lineart": None, "depth": None, "normal": None, "segmentation": None}

    audio_sample_rate = 44100
    samples_per_frame = int(audio_sample_rate / FPS)
    audio_data = load_wav_samples("cloudburst_ambient_resonance.wav")

    # Initialize cinematic camera tracking states for smooth gliding transitions
    cam_x, cam_y, cam_z = 0.0, -150.0, -550.0
    cam_yaw, cam_pitch, GLOBAL_CAM_ROLL = -0.25, 0.35, 0.0
    fov = float(680 * SCALE)

    print("[VIDEO] Compiling 960 frames per layer...")
    for frame_idx in range(start_frame, end_frame):
        time_secs = frame_idx / FPS
        
        # Audio amplitude mapping for beat-reactive camera matching
        start_s = frame_idx * samples_per_frame
        end_s = start_s + samples_per_frame
        chunk = audio_data[start_s:end_s]
        rms = np.sqrt(np.mean(np.array(chunk)**2)) if len(chunk) > 0 else 0
        amp = max(0.0, min(1.0, rms * 4.0))
        # Continuous epoch progression to render smooth transitions
        epoch_progression = (time_secs / DURATION_SECS) * (len(EPOCHS) - 1)
        epoch_idx = int(epoch_progression)
        next_epoch_idx = min(len(EPOCHS) - 1, epoch_idx + 1)
        epoch_t = epoch_progression - epoch_idx

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

        # --- Layer Canvases Setup ---
        canvases = {
            "lineart": Image.new("RGB", (W_S, H_S), (255, 255, 255)),
            "depth": Image.new("RGB", (W_S, H_S), (0, 0, 0)),
            "normal": Image.new("RGB", (W_S, H_S), (128, 128, 255)), # Facing camera is blue (0,0,1)
            "segmentation": Image.new("RGB", (W_S, H_S), (0, 0, 0)) # Grid background flat color
        }
        draws = {k: ImageDraw.Draw(canvases[k]) for k in canvases}

        # 1. Render Substrate Grid (depth / segmentation / lineart / normal)
        grid_extent = 500
        grid_step = 50
        
        # Grid lines parallel to Z-axis
        for gx in range(-grid_extent, grid_extent + 1, grid_step):
            pts = []
            sz_vals = []
            for gz in range(-grid_extent, grid_extent + 1, 50):
                gy = 100
                if epoch_idx >= 5 or next_epoch_idx >= 5:
                    t_val = epoch_t if epoch_idx == 5 else (1.0 if epoch_idx > 5 else 0.0)
                    dist_to_center = math.sqrt(gx**2 + gz**2)
                    gy += int((dist_to_center * 0.3) * t_val)
                sx, sy, sz = project_3d(gx, gy, gz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                    pts.append((sx, sy))
                    sz_vals.append(sz)
            if len(pts) > 1:
                avg_sz = sum(sz_vals) / len(sz_vals)
                # Volumetric LineArt attenuation: distant lines are thinner and lighter gray
                la_opacity = min(255, max(40, int(255 * (1.0 - avg_sz / 900.0))))
                la_width = max(1, int(1.5 * SCALE * (1.0 - avg_sz / 900.0)))
                draws["lineart"].line(pts, fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=la_width)
                draws["depth"].line(pts, fill=(15, 15, 15), width=1 * SCALE)
                draws["normal"].line(pts, fill=(128, 255, 128), width=1 * SCALE)
                draws["segmentation"].line(pts, fill=(0, 128, 0), width=1 * SCALE)

        # Grid lines parallel to X-axis
        for gz in range(-grid_extent, grid_extent + 1, grid_step):
            pts = []
            sz_vals = []
            for gx in range(-grid_extent, grid_extent + 1, 50):
                gy = 100
                if epoch_idx >= 5 or next_epoch_idx >= 5:
                    t_val = epoch_t if epoch_idx == 5 else (1.0 if epoch_idx > 5 else 0.0)
                    dist_to_center = math.sqrt(gx**2 + gz**2)
                    gy += int((dist_to_center * 0.3) * t_val)
                sx, sy, sz = project_3d(gx, gy, gz, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                    pts.append((sx, sy))
                    sz_vals.append(sz)
            if len(pts) > 1:
                avg_sz = sum(sz_vals) / len(sz_vals)
                la_opacity = min(255, max(40, int(255 * (1.0 - avg_sz / 900.0))))
                la_width = max(1, int(1.5 * SCALE * (1.0 - avg_sz / 900.0)))
                draws["lineart"].line(pts, fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=la_width)
                draws["depth"].line(pts, fill=(15, 15, 15), width=1 * SCALE)
                draws["normal"].line(pts, fill=(128, 255, 128), width=1 * SCALE)
                draws["segmentation"].line(pts, fill=(0, 128, 0), width=1 * SCALE)

        # Projected Node Centers
        rx, ry, rz = project_3d(rod_3d[0], rod_3d[1], rod_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
        cx, cy, cz = project_3d(cone_3d[0], cone_3d[1], cone_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)

        def draw_3d_transfer_line_layers(x1, y1, z1, x2, y2, z2, t_pulse, weight=1.0):
            sx1, sy1, sz1 = project_3d(x1, y1, z1, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            sx2, sy2, sz2 = project_3d(x2, y2, z2, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if sx1 is not None and sx2 is not None:
                p_depth = min(255, max(0, int(255 * (1.0 - ((sz1 + sz2)/2.0) / 800.0))))
                draws["lineart"].line([(sx1, sy1), (sx2, sy2)], fill=(0, 0, 0), width=int(2.5 * SCALE * weight) or 1)
                draws["depth"].line([(sx1, sy1), (sx2, sy2)], fill=(p_depth, p_depth, p_depth), width=int(2 * SCALE))
                draws["normal"].line([(sx1, sy1), (sx2, sy2)], fill=(128, 128, 255), width=int(2 * SCALE))
                draws["segmentation"].line([(sx1, sy1), (sx2, sy2)], fill=(255, 0, 0), width=int(2 * SCALE))
                
                # moving packet L -> R / R -> L
                p_t = (t_pulse * 1.8) % 1.0
                px = sx1 + (sx2 - sx1) * p_t
                py = sy1 + (sy2 - sy1) * p_t
                p_size = 4 * SCALE
                p_depth_pkt = min(255, max(0, int(255 * (1.0 - (sz1 + (sz2 - sz1)*p_t) / 800.0))))
                
                draws["lineart"].ellipse([px - p_size, py - p_size, px + p_size, py + p_size], fill=(0, 0, 0))
                draws["depth"].ellipse([px - p_size, py - p_size, px + p_size, py + p_size], fill=(p_depth_pkt, p_depth_pkt, p_depth_pkt))
                draws["normal"].ellipse([px - p_size, py - p_size, px + p_size, py + p_size], fill=(128, 128, 255))
                draws["segmentation"].ellipse([px - p_size, py - p_size, px + p_size, py + p_size], fill=(255, 0, 0))

        # 2. Render Node structures into the layers
        def draw_layer_node_guts(nx, ny, nz, is_rod, rz_val):
            size = 45 * SCALE
            
            # LineArt: thick black circle boundaries and inner shading rings
            draws["lineart"].ellipse([nx - size, ny - size, nx + size, ny + size], outline=(0, 0, 0), width=3 * SCALE)
            for r_inner in range(int(size * 0.8), int(size * 0.2), -int(10 * SCALE)):
                draws["lineart"].ellipse([nx - r_inner, ny - r_inner, nx + r_inner, ny + r_inner], outline=(60, 60, 60), width=1 * SCALE)
            
            # Depth: brighter gray circles based on Z depth
            depth_val = min(255, max(0, int(255 * (1.0 - rz_val / 800.0))))
            draws["depth"].ellipse([nx - size, ny - size, nx + size, ny + size], fill=(depth_val, depth_val, depth_val))

            # Segmentation: Node A = Solid Blue, Node B = Solid Yellow
            seg_color = (0, 0, 255) if is_rod else (255, 255, 0)
            draws["segmentation"].ellipse([nx - size, ny - size, nx + size, ny + size], fill=seg_color)

            # Normal Map: Spherical normal projection map
            for ry_step in range(-size, size, 2 * SCALE):
                for rx_step in range(-size, size, 2 * SCALE):
                    if rx_step**2 + ry_step**2 <= size**2:
                        vx = rx_step / size
                        vy = ry_step / size
                        vz = math.sqrt(max(0.0, 1.0 - vx**2 - vy**2))
                        r_col = int((vx + 1.0) * 127.5)
                        g_col = int((vy + 1.0) * 127.5)
                        b_col = int((vz + 1.0) * 127.5)
                        draws["normal"].rectangle(
                            [nx + rx_step, ny + ry_step, nx + rx_step + 2*SCALE, ny + ry_step + 2*SCALE],
                            fill=(r_col, g_col, b_col)
                        )

        if rx is not None:
            draw_layer_node_guts(rx, ry, rod_3d[2], True, rz)
        if cx is not None:
            draw_layer_node_guts(cx, cy, cone_3d[2], False, cz)

        # 3. Connections Pass (For Segmentation, Depth, LineArt, and Normal)
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
                        msx, msy, msz = project_3d(0, 0, 0, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                        la_opacity = min(255, max(40, int(255 * (1.0 - msz / 900.0) * weight)))
                        draws["lineart"].polygon(pts_ring, outline=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(2 * SCALE))
                        # depth of origin
                        d_val = min(255, max(0, int(255 * (1.0 - msz / 800.0))))
                        draws["depth"].polygon(pts_ring, outline=(d_val, d_val, d_val), width=int(2 * SCALE))
                        draws["normal"].polygon(pts_ring, outline=(128, 128, 255), width=int(2 * SCALE))
                        draws["segmentation"].polygon(pts_ring, outline=(255, 0, 0), width=int(2 * SCALE))

        # Epoch 2: FORM / barrier transition and contour transfer line
        if epoch_idx == 2 or (epoch_idx == 1 and epoch_t > 0.1) or (epoch_idx == 2 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 1 else (1.0 - epoch_t if epoch_idx == 2 else 1.0)
            sx1, sy1, sz1 = project_3d(0, -180 * weight, -100, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            sx2, sy2, sz2 = project_3d(0, 180 * weight, 100, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if sx1 is not None and sx2 is not None:
                p_depth = min(255, max(0, int(255 * (1.0 - ((sz1 + sz2)/2.0) / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - ((sz1 + sz2)/2.0) / 900.0) * weight)))
                draws["segmentation"].line([(sx1, sy1), (sx2, sy2)], fill=(255, 0, 0), width=int(4 * SCALE * weight) or 1)
                draws["lineart"].line([(sx1, sy1), (sx2, sy2)], fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(4 * SCALE * weight) or 1)
                draws["depth"].line([(sx1, sy1), (sx2, sy2)], fill=(p_depth, p_depth, p_depth), width=int(4 * SCALE * weight) or 1)
                draws["normal"].line([(sx1, sy1), (sx2, sy2)], fill=(128, 128, 255), width=int(4 * SCALE * weight) or 1)
            draw_3d_transfer_line_layers(rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, weight)

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
                p_depth = min(255, max(0, int(255 * (1.0 - rod_3d[2] / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - rod_3d[2] / 900.0) * weight)))
                draws["lineart"].line(pts_vert, fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(3 * SCALE))
                draws["depth"].line(pts_vert, fill=(p_depth, p_depth, p_depth), width=int(3 * SCALE))
                draws["normal"].line(pts_vert, fill=(128, 128, 255), width=int(3 * SCALE))
                draws["segmentation"].line(pts_vert, fill=(255, 0, 0), width=int(3 * SCALE))
                
            # Horizontal line crossing Cone node
            pts_horiz = []
            for gx in range(-300, 301, 50):
                sx, sy, sz = project_3d(gx, cone_3d[1], cone_3d[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                if sx is not None:
                    pts_horiz.append((sx, sy))
            if len(pts_horiz) > 1:
                p_depth = min(255, max(0, int(255 * (1.0 - cone_3d[2] / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - cone_3d[2] / 900.0) * weight)))
                draws["lineart"].line(pts_horiz, fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(3 * SCALE))
                draws["depth"].line(pts_horiz, fill=(p_depth, p_depth, p_depth), width=int(3 * SCALE))
                draws["normal"].line(pts_horiz, fill=(128, 128, 255), width=int(3 * SCALE))
                draws["segmentation"].line(pts_horiz, fill=(255, 0, 0), width=int(3 * SCALE))

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
                p_depth = min(255, max(0, int(255 * (1.0 - ((rz + cz)/2.0) / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - ((rz + cz)/2.0) / 900.0) * weight)))
                for k in range(len(l_points) - 1):
                    draws["segmentation"].line([l_points[k], l_points[k+1]], fill=(255, 0, 0), width=int(3 * SCALE * weight) or 1)
                    draws["lineart"].line([l_points[k], l_points[k+1]], fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(3 * SCALE * weight) or 1)
                    draws["depth"].line([l_points[k], l_points[k+1]], fill=(p_depth, p_depth, p_depth), width=int(3 * SCALE * weight) or 1)
                    draws["normal"].line([l_points[k], l_points[k+1]], fill=(128, 128, 255), width=int(3 * SCALE * weight) or 1)
            draw_3d_transfer_line_layers(rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, weight)

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
                    p_depth = min(255, max(0, int(255 * (1.0 - ((psz + msz)/2.0) / 800.0))))
                    la_opacity = min(255, max(40, int(255 * (1.0 - ((psz + msz)/2.0) / 900.0) * weight)))
                    draws["lineart"].line([(psx, psy), (msx, msy)], fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(1.5 * SCALE))
                    draws["depth"].line([(psx, psy), (msx, msy)], fill=(p_depth, p_depth, p_depth), width=int(1 * SCALE))
                    draws["normal"].line([(psx, psy), (msx, msy)], fill=(128, 128, 255), width=int(1 * SCALE))
                    draws["segmentation"].line([(psx, psy), (msx, msy)], fill=(255, 0, 0), width=int(1 * SCALE))

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
                # depth at center
                msx, msy, msz = project_3d(0, 0, 0, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                d_val = min(255, max(0, int(255 * (1.0 - msz / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - msz / 900.0) * weight)))
                draws["lineart"].polygon(pts_blast, outline=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(3 * SCALE))
                draws["depth"].polygon(pts_blast, outline=(d_val, d_val, d_val), width=int(3 * SCALE))
                draws["normal"].polygon(pts_blast, outline=(128, 128, 255), width=int(3 * SCALE))
                draws["segmentation"].polygon(pts_blast, outline=(255, 0, 0), width=int(3 * SCALE))
            draw_3d_transfer_line_layers(rod_3d[0], rod_3d[1], rod_3d[2], cone_3d[0], cone_3d[1], cone_3d[2], time_secs, weight)

        # Epoch 7: IONIZE / plasma bridge
        if epoch_idx == 7 or (epoch_idx == 6 and epoch_t > 0.1) or (epoch_idx == 7 and epoch_t < 0.9):
            weight = epoch_t if epoch_idx == 6 else (1.0 - epoch_t if epoch_idx == 7 else 1.0)
            if rx is not None and cx is not None:
                p_depth = min(255, max(0, int(255 * (1.0 - ((rz + cz)/2.0) / 800.0))))
                la_opacity = min(255, max(40, int(255 * (1.0 - ((rz + cz)/2.0) / 900.0) * weight)))
                draws["segmentation"].line([(rx, ry), (cx, cy)], fill=(255, 0, 0), width=int(10 * SCALE * weight) or 1)
                draws["lineart"].line([(rx, ry), (cx, cy)], fill=(255 - la_opacity, 255 - la_opacity, 255 - la_opacity), width=int(10 * SCALE * weight) or 1)
                draws["depth"].line([(rx, ry), (cx, cy)], fill=(p_depth, p_depth, p_depth), width=int(10 * SCALE * weight) or 1)
                draws["normal"].line([(rx, ry), (cx, cy)], fill=(128, 128, 255), width=int(10 * SCALE * weight) or 1)
                for offset in range(-int(6*SCALE*weight), int(7*SCALE*weight), 3*SCALE or 1):
                    l_points = [(rx, ry)]
                    for step in range(1, 10):
                        tx = rx + (cx - rx) * (step / 10.0)
                        ty = ry + (cy - ry) * (step / 10.0) + math.sin(time_secs * 25 + step) * 12 * SCALE * weight + offset
                        l_points.append((tx, ty))
                    l_points.append((cx, cy))
                    for k in range(len(l_points) - 1):
                        draws["segmentation"].line([l_points[k], l_points[k+1]], fill=(255, 0, 0), width=int(2 * SCALE * weight) or 1)
                        draws["lineart"].line([l_points[k], l_points[k+1]], fill=(0, 0, 0), width=int(2 * SCALE * weight) or 1)
                        draws["depth"].line([l_points[k], l_points[k+1]], fill=(p_depth, p_depth, p_depth), width=int(2 * SCALE * weight) or 1)
                        draws["normal"].line([l_points[k], l_points[k+1]], fill=(128, 128, 255), width=int(2 * SCALE * weight) or 1)

        # Epoch 8: MAGNETIZE / locked mandala lattice
        if epoch_idx == 8 or (epoch_idx == 7 and epoch_t > 0.1):
            weight = epoch_t if epoch_idx == 7 else 1.0
            if rx is not None and cx is not None:
                mid_pt_x, mid_pt_y = (rx + cx) // 2, (ry + cy) // 2
                
                # concentric circles
                for m_r in range(int(140*SCALE*weight), int(20*SCALE*weight), -30*SCALE or 1):
                    draws["segmentation"].ellipse([mid_pt_x - m_r, mid_pt_y - m_r, mid_pt_x + m_r, mid_pt_y + m_r], outline=(255, 0, 0), width=int(2 * SCALE * weight) or 1)
                    draws["lineart"].ellipse([mid_pt_x - m_r, mid_pt_y - m_r, mid_pt_x + m_r, mid_pt_y + m_r], outline=(0, 0, 0), width=int(2 * SCALE * weight) or 1)
                    draws["depth"].ellipse([mid_pt_x - m_r, mid_pt_y - m_r, mid_pt_x + m_r, mid_pt_y + m_r], outline=(230, 230, 230), width=int(2 * SCALE * weight) or 1)
                    draws["normal"].ellipse([mid_pt_x - m_r, mid_pt_y - m_r, mid_pt_x + m_r, mid_pt_y + m_r], outline=(128, 128, 255), width=int(2 * SCALE * weight) or 1)
                
                # outer rectangle
                rect_coords = [mid_pt_x - int(180*SCALE*weight), mid_pt_y - int(120*SCALE*weight), mid_pt_x + int(180*SCALE*weight), mid_pt_y + int(120*SCALE*weight)]
                draws["segmentation"].rectangle(rect_coords, outline=(255, 0, 0), width=int(2 * SCALE * weight) or 1)
                draws["lineart"].rectangle(rect_coords, outline=(0, 0, 0), width=int(2 * SCALE * weight) or 1)
                draws["depth"].rectangle(rect_coords, outline=(230, 230, 230), width=int(2 * SCALE * weight) or 1)
                draws["normal"].rectangle(rect_coords, outline=(128, 128, 255), width=int(2 * SCALE * weight) or 1)

        # Reset seed for particles random simulation consistency
        random.seed(42)

        # 4. Draw particles in layers with correct physics
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

            res_p = project_3d(p["x"], p["y"], p["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, roll=GLOBAL_CAM_ROLL)
            if res_p is not None:
                sx, sy, sz = res_p
                if sx is not None and 0 <= sx < W_S and 0 <= sy < H_S:
                    # Depth of Field (DoF) / Bokeh CoC mapping
                    coc = abs(sz - focal_dist) * 0.02 * SCALE
                    p_size = max(1, int(5 * (400 / sz) + coc)) * SCALE
                    p_depth = min(255, max(0, int(255 * (1.0 - sz / 800.0) / (1.0 + coc * 0.4))))
                    
                    # LineArt: particle dots are black
                    draws["lineart"].ellipse([sx - p_size, sy - p_size, sx + p_size, sy + p_size], fill=(0, 0, 0))
                    # Depth: particle grayscale depth
                    draws["depth"].ellipse([sx - p_size, sy - p_size, sx + p_size, sy + p_size], fill=(p_depth, p_depth, p_depth))
                    # Normal: particle spheres have flat normal facing camera (128, 128, 255)
                    draws["normal"].ellipse([sx - p_size, sy - p_size, sx + p_size, sy + p_size], fill=(128, 128, 255))
                    # Segmentation: Particles = Solid Red (255, 0, 0)
                    draws["segmentation"].ellipse([sx - p_size, sy - p_size, sx + p_size, sy + p_size], fill=(255, 0, 0))

        # Draw the 64 chattering LAU nodes in controlnet layers
        active_lau_idx = min(63, int(frame_idx // (TOTAL_FRAMES / 64.0)))
        for idx, lau in enumerate(laus):
            lsx, lsy, lsz = project_3d(lau["x"], lau["y"], lau["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if lsx is not None and 0 <= lsx < W_S and 0 <= lsy < H_S:
                if idx == active_lau_idx:
                    p_size = 9 * SCALE
                    p_depth = min(255, max(0, int(255 * (1.0 - lsz / 800.0))))
                    
                    # LineArt: Active LAU circle boundary
                    draws["lineart"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], outline=(0, 0, 0), width=2 * SCALE)
                    # Depth: Active LAU grayscale depth
                    draws["depth"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(p_depth, p_depth, p_depth))
                    # Normal: Camera-facing flat normal
                    draws["normal"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(128, 128, 255))
                    # Segmentation: Active LAU and its connection are solid Red (255, 0, 0)
                    draws["segmentation"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(255, 0, 0))
                    
                    # Connection line
                    target_node = rod_3d if idx % 2 == 0 else cone_3d
                    tx, ty, tz = project_3d(target_node[0], target_node[1], target_node[2], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
                    if tx is not None:
                        draws["lineart"].line([(lsx, lsy), (tx, ty)], fill=(0, 0, 0), width=int(2 * SCALE))
                        draws["depth"].line([(lsx, lsy), (tx, ty)], fill=(p_depth, p_depth, p_depth), width=int(2 * SCALE))
                        draws["normal"].line([(lsx, lsy), (tx, ty)], fill=(128, 128, 255), width=int(2 * SCALE))
                        draws["segmentation"].line([(lsx, lsy), (tx, ty)], fill=(255, 0, 0), width=int(2 * SCALE))
                else:
                    p_size = 2 * SCALE
                    p_depth = min(255, max(0, int(255 * (1.0 - lsz / 800.0))))
                    draws["lineart"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(0, 0, 0))
                    draws["depth"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(p_depth, p_depth, p_depth))
                    draws["normal"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(128, 128, 255))
                    # Segmentation: idle LAUs are colored solid Blue (0, 0, 255)
                    draws["segmentation"].ellipse([lsx - p_size, lsy - p_size, lsx + p_size, lsy + p_size], fill=(0, 0, 255))

        # Draw 3D Solidity contract topologies and their relations to ERIS center (0, 0, 0) in controlnet layers
        esx, esy, esz = project_3d(0, 0, 0, cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
        for c in contracts:
            csx, csy, csz = project_3d(c["x"], c["y"], c["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if csx is not None and 0 <= csx < W_S and 0 <= csy < H_S:
                c_size = 3 * SCALE
                p_depth = min(255, max(0, int(255 * (1.0 - csz / 800.0))))
                
                # LineArt: thin black circle
                draws["lineart"].ellipse([csx - c_size, csy - c_size, csx + c_size, csy + c_size], outline=(0, 0, 0), width=1 * SCALE)
                # Depth: grayscale depth
                draws["depth"].ellipse([csx - c_size, csy - c_size, csx + c_size, csy + c_size], fill=(p_depth, p_depth, p_depth))
                # Normal: flat normal
                draws["normal"].ellipse([csx - c_size, csy - c_size, csx + c_size, csy + c_size], fill=(128, 128, 255))
                # Segmentation: Contracts are colored solid Yellow (255, 255, 0)
                draws["segmentation"].ellipse([csx - c_size, csy - c_size, csx + c_size, csy + c_size], fill=(255, 255, 0))
                
                # Connection to ERIS center (0, 0, 0)
                if esx is not None:
                    draws["lineart"].line([(csx, csy), (esx, esy)], fill=(0, 0, 0), width=int(1 * SCALE))
                    draws["depth"].line([(csx, csy), (esx, esy)], fill=(p_depth, p_depth, p_depth), width=int(1 * SCALE))
                    draws["normal"].line([(csx, csy), (esx, esy)], fill=(128, 128, 255), width=int(1 * SCALE))
                    draws["segmentation"].line([(csx, csy), (esx, esy)], fill=(255, 255, 0), width=int(1 * SCALE))

        # Draw topology connection links between contracts in controlnet layers
        topology_links = [(0, 1), (2, 3), (4, 5), (6, 7), (8, 9)]
        for c1_idx, c2_idx in topology_links:
            c1 = contracts[c1_idx]
            c2 = contracts[c2_idx]
            csx1, csy1, csz1 = project_3d(c1["x"], c1["y"], c1["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            csx2, csy2, csz2 = project_3d(c2["x"], c2["y"], c2["z"], cam_x, cam_y, cam_z, cam_pitch, cam_yaw, fov, W_S, H_S, look_x, look_y, look_z)
            if csx1 is not None and csx2 is not None:
                p_depth = min(255, max(0, int(255 * (1.0 - ((csz1 + csz2)/2.0) / 800.0))))
                draws["lineart"].line([(csx1, csy1), (csx2, csy2)], fill=(0, 0, 0), width=int(1 * SCALE))
                draws["depth"].line([(csx1, csy1), (csx2, csy2)], fill=(p_depth, p_depth, p_depth), width=int(1 * SCALE))
                draws["normal"].line([(csx1, csy1), (csx2, csy2)], fill=(128, 128, 255), width=int(1 * SCALE))
                draws["segmentation"].line([(csx1, csy1), (csx2, csy2)], fill=(255, 255, 0), width=int(1 * SCALE))

        # Shutter angle emulation / temporal Motion Blur for each ControlNet layer
        for key in pipes:
            if prev_canvases[key] is not None:
                canvases[key] = Image.blend(prev_canvases[key], canvases[key], 0.65)
            prev_canvases[key] = canvases[key].copy()

        # Downsample and send raw RGB24 bytes to FFmpeg pipes
        for key in pipes:
            if SCALE != 1:
                downsampled = canvases[key].resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS)
            else:
                downsampled = canvases[key]
            # Write raw RGB24 bytes directly to the FFmpeg pipe
            pipes[key].stdin.write(downsampled.tobytes())

        if frame_idx % 120 == 0:
            print(f"[VIDEO] Rendered {frame_idx}/{TOTAL_FRAMES} frames...")

    # Close and finalize all pipes
    for key in pipes:
        pipes[key].stdin.close()
        pipes[key].wait()
    print("=== CONTROLNET GUIDANCE LAYER VIDEOS COMPILED SUCCESSFULLY ===")

if __name__ == "__main__":
    main()
