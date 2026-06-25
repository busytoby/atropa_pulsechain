import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
DURATION = 8.0  # 8 seconds demo
num_samples = int(SAMPLE_RATE * DURATION)
total_frames = int(DURATION * FPS)
dt = 1.0 / SAMPLE_RATE

# Center coordinates for rendering
CX = 360
CY = 640

def quaternion_multiply(q1, q2):
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    return (
        w1*w2 - x1*x2 - y1*y2 - z1*z2,
        w1*x2 + x1*w2 + y1*z2 - z1*y2,
        w1*y2 - x1*z2 + y1*w2 + z1*x2,
        w1*z2 + x1*y2 - y1*x2 + z1*w2
    )

def rotate_vector_by_quaternion(v, q):
    # v is (x, y, z)
    q_vec = (0, v[0], v[1], v[2])
    q_conj = (q[0], -q[1], -q[2], -q[3])
    temp = quaternion_multiply(q, q_vec)
    result = quaternion_multiply(temp, q_conj)
    return (result[1], result[2], result[3])

def generate_vocalized_speech():
    print("[DSP] Generating Verlet vocal fold speech audio...")
    t = np.linspace(0, DURATION, num_samples, endpoint=False)
    audio = np.zeros(num_samples)
    
    # Simulate dynamic vowels (EE -> AH -> OO -> EE)
    # Target formants F1, F2:
    # 0s - 2s (EE): F1=270, F2=2290
    # 2s - 4s (AH): F1=730, F2=1090
    # 4s - 6s (OO): F1=300, F2=870
    # 6s - 8s (EE): F1=270, F2=2290
    
    # 3-Mass Ishizaka-Flanagan style Verlet physical model simulation variables
    # Mass 1 (Lower Fold)
    x1, x1_prev = 0.05, 0.05
    m1 = 0.15
    k1_open = 1000.0
    k1_closed = 3500.0
    c1 = 1.4
    Ps1 = 0.55
    
    # Mass 2 (Upper Fold)
    x2, x2_prev = 0.04, 0.04
    m2 = 0.12
    k2_open = 1380.0
    k2_closed = 4000.0
    c2 = 1.1
    Ps2 = 0.65
    
    # Mass 3 (Resonant Load Piston representing vocal tract impedance)
    x3, x3_prev = 0.0, 0.0
    m3 = 0.08
    k3 = 800.0
    c3 = 0.5
    
    # Coupling parameters
    Kc = 180.0
    A_fold = 0.2
    
    glottal_flow = np.zeros(num_samples)
    for s in range(1, num_samples - 1):
        # Asymmetric stiffness based on displacement
        stiffness1 = k1_open if x1 > 0.0 else k1_closed
        stiffness2 = k2_open if x2 > 0.0 else k2_closed
        
        # Aerodynamic driving forces
        f_p1 = Ps1 * A_fold if x1 > 0.0 else 0.0
        f_p2 = Ps2 * A_fold if x2 > 0.0 else 0.0
        
        # Velocities
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        v3 = (x3 - x3_prev) / dt
        
        # Resonant load piston driven by glottal flow
        current_flow = (max(x1, 0.0) ** 2) + (max(x2, 0.0) ** 2)
        f_p3 = current_flow * 2.5
        
        # Feedback force from resonant piston onto upper glottal mass
        F_fb = -k3 * x3 - c3 * v3
        
        # Coupled acceleration calculations
        acc1 = (f_p1 - stiffness1 * x1 - c1 * v1 + Kc * (x2 - x1)) / m1
        acc2 = (f_p2 - stiffness2 * x2 - c2 * v2 + Kc * (x1 - x2) + F_fb) / m2
        acc3 = (f_p3 - k3 * x3 - c3 * v3) / m3
        
        # Verlet integration step
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        x3_next = 2.0 * x3 - x3_prev + acc3 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.2, min(1.0, x1_next))
        x2_prev, x2 = x2, max(-0.2, min(1.0, x2_next))
        x3_prev, x3 = x3, max(-0.5, min(1.0, x3_next))
        
        # Save output flow (modulated by acoustic load piston)
        glottal_flow[s] = current_flow
        
    glottal_flow -= np.mean(glottal_flow)
    if np.max(np.abs(glottal_flow)) > 0:
        glottal_flow /= np.max(np.abs(glottal_flow))
        
    # Generate noise sources for consonants
    noise = np.random.uniform(-1.0, 1.0, num_samples)
    
    # 1. "S" noise (high-pass filter)
    s_noise = noise.copy()
    for _ in range(3):
        s_noise = np.concatenate([[0.0], s_noise[1:] - 0.95 * s_noise[:-1]])
    s_noise /= np.maximum(np.max(np.abs(s_noise)), 1e-5)
    
    # 2. "SH" noise (band-pass filter: high-pass then moving average low-pass)
    sh_noise = np.concatenate([[0.0], noise[1:] - 0.82 * noise[:-1]])
    sh_noise = np.convolve(sh_noise, np.ones(6)/6.0, mode='same')
    sh_noise /= np.maximum(np.max(np.abs(sh_noise)), 1e-5)
    
    # Synthesize phoneme sequence: S -> EE -> SH -> AH -> T -> OO -> S
    for s in range(num_samples):
        time_sec = s / SAMPLE_RATE
        
        # Voicing or Aspiration selection
        if time_sec < 0.8:
            # "S" phoneme
            audio[s] = s_noise[s] * 0.28
        elif time_sec < 2.5:
            # "EE" vowel
            f1, f2 = 270.0, 2290.0
            formant_excitation = glottal_flow[s] * (
                0.6 * math.sin(2.0 * math.pi * f1 * time_sec) +
                0.4 * math.sin(2.0 * math.pi * f2 * time_sec)
            )
            audio[s] = formant_excitation * 0.45
        elif time_sec < 3.3:
            # "SH" phoneme
            audio[s] = sh_noise[s] * 0.26
        elif time_sec < 5.0:
            # "AH" vowel
            ratio = (time_sec - 3.3) / 1.7
            f1 = 270.0 * (1.0 - ratio) + 730.0 * ratio
            f2 = 2290.0 * (1.0 - ratio) + 1090.0 * ratio
            formant_excitation = glottal_flow[s] * (
                0.6 * math.sin(2.0 * math.pi * f1 * time_sec) +
                0.4 * math.sin(2.0 * math.pi * f2 * time_sec)
            )
            audio[s] = formant_excitation * 0.45
        elif time_sec < 5.1:
            # Silence/closure before "T" plosive
            audio[s] = 0.0
        elif time_sec < 5.25:
            # "T" release burst (short high-pass noise spike)
            decay = math.exp(-(time_sec - 5.1) / 0.02)
            audio[s] = s_noise[s] * 0.35 * decay
        elif time_sec < 7.0:
            # "OO" vowel
            ratio = (time_sec - 5.25) / 1.75
            f1 = 730.0 * (1.0 - ratio) + 300.0 * ratio
            f2 = 1090.0 * (1.0 - ratio) + 870.0 * ratio
            formant_excitation = glottal_flow[s] * (
                0.6 * math.sin(2.0 * math.pi * f1 * time_sec) +
                0.4 * math.sin(2.0 * math.pi * f2 * time_sec)
            )
            audio[s] = formant_excitation * 0.45
        else:
            # "S" trailing phoneme
            audio[s] = s_noise[s] * 0.25
        
    audio -= np.mean(audio)
    if np.max(np.abs(audio)) > 0:
        audio /= np.max(np.abs(audio))
        
    audio_path = "temp_teddy_speech.wav"
    # Save mono 16-bit PCM WAV
    byte_data = bytearray(44 + len(audio) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(audio) * 2).to_bytes(4, "little")
    byte_data[8:12] = b"WAVE"
    byte_data[12:16] = b"fmt "
    byte_data[16:20] = int(16).to_bytes(4, "little")
    byte_data[20:22] = int(1).to_bytes(2, "little")
    byte_data[22:24] = int(1).to_bytes(2, "little")
    byte_data[24:28] = int(SAMPLE_RATE).to_bytes(4, "little")
    byte_data[28:32] = int(SAMPLE_RATE * 2).to_bytes(4, "little")
    byte_data[32:34] = int(2).to_bytes(2, "little")
    byte_data[34:36] = int(16).to_bytes(2, "little")
    byte_data[36:40] = b"data"
    byte_data[40:44] = int(len(audio) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in audio:
        v = int(val * 32767) if val >= 0 else int(val * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(audio_path, "wb") as f:
        f.write(byte_data)
        
    return audio_path, glottal_flow

def generate_3d_teddy_wireframe():
    # Build a 3D wireframe teddy bear model
    # Vertices are tuples: (x, y, z, joint_type)
    # joint_type: 0 = static body, 1 = lower jaw (mouth), 2 = left arm, 3 = right arm
    vertices = []
    edges = []
    
    # 1. Head (Sphere)
    head_center = (0.0, 1.4, 0.0)
    head_rad = 0.5
    head_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat = math.cos(lat_angle)
        sin_lat = math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = head_center[0] + head_rad * cos_lat * math.cos(lon_angle)
            vy = head_center[1] + head_rad * sin_lat
            vz = head_center[2] + head_rad * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    # Connect head sphere longitude and latitude
    for lat in range(9):
        for lon in range(8):
            curr = head_v_start + lat * 8 + lon
            nxt_lon = head_v_start + lat * 8 + ((lon + 1) % 8)
            edges.append((curr, nxt_lon))
            if lat < 8:
                nxt_lat = head_v_start + (lat + 1) * 8 + lon
                edges.append((curr, nxt_lat))
                
    # 2. Lower Jaw / Mouth (moving vertices)
    jaw_v_start = len(vertices)
    jaw_center = (0.0, 1.1, 0.25)
    jaw_rad = 0.2
    for lon in range(6):
        lon_angle = (lon / 6.0) * math.pi + math.pi # lower half semicircle
        vx = jaw_center[0] + jaw_rad * math.cos(lon_angle)
        vy = jaw_center[1] + jaw_rad * math.sin(lon_angle)
        vz = jaw_center[2]
        vertices.append((vx, vy, vz, 1)) # joint type 1: moving jaw
        
    for lon in range(5):
        edges.append((jaw_v_start + lon, jaw_v_start + lon + 1))
        
    # 3. Body (Ellipsoid)
    body_center = (0.0, 0.3, 0.0)
    body_rad_x, body_rad_y, body_rad_z = 0.7, 0.8, 0.5
    body_v_start = len(vertices)
    for latitude in range(-4, 5):
        lat_angle = (latitude / 5.0) * (math.pi / 2.0)
        cos_lat = math.cos(lat_angle)
        sin_lat = math.sin(lat_angle)
        for longitude in range(8):
            lon_angle = (longitude / 8.0) * 2.0 * math.pi
            vx = body_center[0] + body_rad_x * cos_lat * math.cos(lon_angle)
            vy = body_center[1] + body_rad_y * sin_lat
            vz = body_center[2] + body_rad_z * cos_lat * math.sin(lon_angle)
            vertices.append((vx, vy, vz, 0))
            
    for lat in range(9):
        for lon in range(8):
            curr = body_v_start + lat * 8 + lon
            nxt_lon = body_v_start + lat * 8 + ((lon + 1) % 8)
            edges.append((curr, nxt_lon))
            if lat < 8:
                nxt_lat = body_v_start + (lat + 1) * 8 + lon
                edges.append((curr, nxt_lat))
                
    # 4. Ears (Left and Right Spheres)
    for sign in [-1, 1]:
        ear_center = (sign * 0.45, 1.8, 0.0)
        ear_rad = 0.2
        ear_v_start = len(vertices)
        for lat in range(-2, 3):
            lat_angle = (lat / 3.0) * (math.pi / 2.0)
            cos_lat = math.cos(lat_angle)
            sin_lat = math.sin(lat_angle)
            for lon in range(6):
                lon_angle = (lon / 6.0) * 2.0 * math.pi
                vx = ear_center[0] + ear_rad * cos_lat * math.cos(lon_angle)
                vy = ear_center[1] + ear_rad * sin_lat
                vz = ear_center[2] + ear_rad * cos_lat * math.sin(lon_angle)
                vertices.append((vx, vy, vz, 0))
                
        for lat in range(5):
            for lon in range(6):
                curr = ear_v_start + lat * 6 + lon
                nxt_lon = ear_v_start + lat * 6 + ((lon + 1) % 6)
                edges.append((curr, nxt_lon))
                if lat < 4:
                    nxt_lat = ear_v_start + (lat + 1) * 6 + lon
                    edges.append((curr, nxt_lat))
                    
    return vertices, edges

def render_demo_video(audio_path, glottal_flow, output_mp4):
    vertices, edges = generate_3d_teddy_wireframe()
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    # 3D Starfield background
    np.random.seed(420)
    stars = [{'x': np.random.uniform(-300, 300), 'y': np.random.uniform(-400, 400), 'z': np.random.uniform(20, 600)} for _ in range(50)]
    
    for frame in range(total_frames):
        time_sec = frame / float(FPS)
        progress = time_sec / DURATION
        
        # Draw background plasma
        bg = np.zeros((1280, 720, 3), dtype=np.uint8)
        img = Image.fromarray(bg)
        draw = ImageDraw.Draw(img)
        
        # Fetch vocal fold displacement amplitude at this frame onset
        sample_idx = min(num_samples - 1, int(time_sec * SAMPLE_RATE))
        amp_flow = glottal_flow[sample_idx]
        
        # 1. Starfield
        for star in stars:
            z_pos = (star['z'] - frame * 3.5) % 580 + 20
            factor = 280.0 / z_pos
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            if 0 <= sx < 720 and 0 <= sy < 1280:
                draw.ellipse([sx, sy, sx + 2, sy + 2], fill=(130, 130, 255))
                
        # 2. Draw 3D Perspective Grid
        draw.line([(0, 950), (720, 950)], fill="#003366", width=2)
        for i in range(12):
            x_bottom = (i / 11.0) * 1120.0 - 200.0
            draw.line([(CX, 950), (x_bottom, 1200)], fill=(0, 30, 90), width=1)
            
        # 3. Rotate Teddy Bear vertices using Quaternions
        # Rotate around Y axis (turning) and X axis (tilting) over time
        angle_y = progress * 2.0 * math.pi
        angle_x = 0.1 * math.sin(progress * 2.0 * math.pi * 2.0)
        
        # Quaternion components
        qy = (math.cos(angle_y / 2.0), 0.0, math.sin(angle_y / 2.0), 0.0)
        qx = (math.cos(angle_x / 2.0), math.sin(angle_x / 2.0), 0.0, 0.0)
        q_rot = quaternion_multiply(qy, qx)
        
        projected = []
        for vx, vy, vz, joint in vertices:
            # Sync jaw displacement to simulated glottal flow
            if joint == 1:
                vy = vy - 0.25 * amp_flow
                
            rx, ry, rz = rotate_vector_by_quaternion((vx, vy, vz), q_rot)
            
            # Translate coordinates back in depth
            rz += 3.8
            factor = 450.0 / rz
            
            proj_x = CX + int(rx * factor * 220.0)
            proj_y = CY - int(ry * factor * 220.0)  # invert Y for screen
            projected.append((proj_x, proj_y))
            
        # Draw Teddy bear wireframe edges
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill="#ffd700", width=1)
            
        # 4. Telemetry overlay
        draw.text((40, 60), "TSFi/2: QUATERNION TEDDY BEAR SPEECH", fill="#ff007f")
        draw.text((40, 80), f"GLOTTAL FLOW AMP: {amp_flow:.4f}", fill="#00f2fe")
        draw.text((40, 100), "VERLET STATE: PERIODIC_CYCLE", fill="#ffd700")
        
        pipe.stdin.write(img.tobytes())
        
        if frame % 30 == 0:
            print(f"  -> Rendering Teddy: {frame}/{total_frames} frames...")
            
    pipe.stdin.close()
    pipe.wait()
    print("[SUCCESS] Teddy speech presentation successfully saved!")

if __name__ == "__main__":
    audio_path, glottal_flow = generate_vocalized_speech()
    
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    output_mp4 = os.path.join(output_dir, "teddy_speech_demo.mp4")
    
    render_demo_video(audio_path, glottal_flow, output_mp4)
    
    # Cleanup temp audio
    try:
        os.remove(audio_path)
    except Exception:
        pass
