import os
import sys
import math
import subprocess
import wave
import struct
from PIL import Image, ImageDraw, ImageFont

def generate_soundtrack(wav_path, duration=22, sample_rate=44100):
    bpm = 160
    total_samples = int(sample_rate * duration)
    step_samples = int(sample_rate * (60.0 / bpm) / 4) # 16th step
    
    with wave.open(wav_path, 'wb') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        
        audio_data = []
        bass_phase = 0.0
        noise_state = 12345
        
        for i in range(total_samples):
            step = i // step_samples
            beat = step // 4
            step_in_beat = step % 4
            sample_in_step = i % step_samples
            
            # 1. Synthesizer bass growl pattern
            bass_chords = [77.78, 65.41, 87.31, 58.27]
            freq = bass_chords[(beat // 4) % len(bass_chords)]
            bass_out = 0.0
            if step_in_beat in [0, 2, 3]:
                bass_phase += 2.0 * math.pi * freq / sample_rate
                bass_out = 0.35 * math.sin(bass_phase) + 0.12 * math.sin(bass_phase * 2.0)
                t_step = sample_in_step / float(sample_rate)
                bass_out *= math.exp(-t_step * 14.0)
            
            # 2. Kick drum
            kick_out = 0.0
            if step_in_beat == 0:
                t_kick = sample_in_step / float(sample_rate)
                if t_kick < 0.15:
                    k_freq = 110.0 * math.exp(-t_kick * 30.0) + 40.0
                    kick_out = 0.45 * math.sin(2.0 * math.pi * k_freq * t_kick) * math.exp(-t_kick * 9.0)
            
            # 3. Snare drum (white noise)
            snare_out = 0.0
            if step_in_beat == 2:
                t_snare = sample_in_step / float(sample_rate)
                if t_snare < 0.2:
                    noise_state = (noise_state * 1103515245 + 12345) & 0x7fffffff
                    noise_val = ((noise_state / 0x7fffffff) * 2.0) - 1.0
                    snare_out = 0.28 * noise_val * math.exp(-t_snare * 14.0)
            
            # 4. Hi-Hat
            hat_out = 0.0
            if step_in_beat in [1, 3]:
                t_hat = sample_in_step / float(sample_rate)
                if t_hat < 0.05:
                    noise_state = (noise_state * 1103515245 + 12345) & 0x7fffffff
                    noise_val = ((noise_state / 0x7fffffff) * 2.0) - 1.0
                    hat_out = 0.18 * noise_val * math.exp(-t_hat * 48.0)
            
            mixed = kick_out + snare_out + hat_out + bass_out
            if mixed > 1.0: mixed = 1.0
            elif mixed < -1.0: mixed = -1.0
            
            sample_val = int(mixed * 32767.0)
            audio_data.append(struct.pack('<h', sample_val))
            
        wav_file.writeframes(b''.join(audio_data))

def draw_cash_cow(draw, width, time):
    font_map = {
        ' ': [0x00, 0x00, 0x00, 0x00, 0x00],
        '$': [0x24, 0x2a, 0x7f, 0x2a, 0x12],
        'C': [0x3e, 0x41, 0x41, 0x41, 0x22],
        'A': [0x7e, 0x11, 0x11, 0x11, 0x7e],
        'S': [0x46, 0x49, 0x49, 0x49, 0x31],
        'H': [0x7f, 0x08, 0x08, 0x08, 0x7f],
        'O': [0x3e, 0x41, 0x41, 0x41, 0x3e],
        'W': [0x3f, 0x40, 0x38, 0x40, 0x3f]
    }
    text = "$$ CASH COW $$"
    char_count = len(text)
    frame = int(time * 30.0)

    # Staggered bounce physics simulated per-frame up to the current frame:
    sim_y = [0.0] * char_count
    sim_vy = [0.0] * char_count
    sim_inflation = [1.0] * char_count
    sim_scale_x = [3.0] * char_count
    sim_scale_y = [3.0] * char_count
    
    gravity = 0.14
    ground = 25.0
    
    for f in range(frame + 1):
        for i in range(char_count):
            sim_vy[i] += gravity
            sim_y[i] += sim_vy[i]
            
            sim_inflation[i] += (1.0 - sim_inflation[i]) * 0.02
            sim_scale_x[i] += (3.0 - sim_scale_x[i]) * 0.1
            sim_scale_y[i] += (3.0 - sim_scale_y[i]) * 0.1
            
            # Ground boundary collision (bounce)
            if sim_y[i] >= ground:
                sim_y[i] = ground
                sim_vy[i] = -sim_vy[i] * 0.65
                sim_inflation[i] = min(3.0, sim_inflation[i] + 0.45)
                sim_scale_x[i] = 3.9
                sim_scale_y[i] = 2.1
                
            # Staggered bounce kicks (pattern period = 200 frames)
            kick_tick = 30 + i * 12 + (f // 200) * 200
            if f == kick_tick:
                sim_vy[i] = -5.0
                sim_inflation[i] = min(3.0, sim_inflation[i] + 0.6)
                sim_scale_x[i] = 2.1
                sim_scale_y[i] = 3.9

    start_x = (width - char_count * 20) // 2
    
    # Painter's order: Render drop shadow pass, then body pass
    for pass_idx in range(2):
        for i, char in enumerate(text):
            if char not in font_map:
                continue
            cols = font_map[char]
            
            # Base position
            cx = start_x + i * 20
            cy = 20 + sim_y[i]
            
            elastic_scale_x = sim_scale_x[i]
            elastic_scale_y = sim_scale_y[i]
            current_inflation = sim_inflation[i]
            
            # Temporary glyph image to compute exact distance field/edge glints
            glyph_w = int(5 * elastic_scale_x + 8)
            glyph_h = int(7 * elastic_scale_y + 8)
            glyph_img = Image.new("L", (glyph_w, glyph_h), 0)
            
            # Draw raw 5x7 glyph onto the temporary mask
            for c in range(5):
                col_val = cols[c]
                for r in range(7):
                    if (col_val >> r) & 1:
                        # Draw scaled pixel block on glyph mask
                        for sy in range(int(elastic_scale_y)):
                            for sx in range(int(elastic_scale_x)):
                                gx = int(c * elastic_scale_x + sx + 4)
                                gy = int(r * elastic_scale_y + sy + 4)
                                if 0 <= gx < glyph_w and 0 <= gy < glyph_h:
                                    glyph_img.putpixel((gx, gy), 255)
            
            # Render pixels from the temporary mask
            for y in range(glyph_h):
                for x in range(glyph_w):
                    val = glyph_img.getpixel((x, y))
                    if val > 128:
                        px = int(cx + x - 4)
                        py = int(cy + y - 4)
                        
                        if pass_idx == 0:
                            # Drop shadow
                            draw.rectangle([px + 8, py + 8, px + 10, py + 10], fill=(20, 5, 0))
                        else:
                            # Foreground body
                            # Compute distance to empty pixels within mask
                            dist_sq = 16
                            for dy in range(-2, 3):
                                for dx in range(-2, 3):
                                    nx, ny_val = x + dx, y + dy
                                    is_inside = (0 <= nx < glyph_w) and (0 <= ny_val < glyph_h)
                                    is_filled = is_inside and (glyph_img.getpixel((nx, ny_val)) > 128)
                                    if not is_filled:
                                        d = dx*dx + dy*dy
                                        if d < dist_sq: dist_sq = d
                            
                            is_top_edge = (y == 0) or (glyph_img.getpixel((x, y - 1)) <= 128)
                            is_left_edge = (x == 0) or (glyph_img.getpixel((x - 1, y)) <= 128)
                            is_glossy = is_top_edge and is_left_edge
                            
                            inflation = current_inflation * 0.8
                            color = (0, 0, 0)
                            
                            is_dollar = (char == '$')
                            if is_dollar:
                                # Gold inflated body
                                if dist_sq > (3.0 * inflation):
                                    color = (255, 204, 0)
                                elif dist_sq > (2.0 * inflation):
                                    color = (80, 20, 0)
                                elif dist_sq > (1.0 * inflation):
                                    col_step = int(time * 15.0 + x) % 8
                                    color = (180 + col_step * 8, 40 + col_step * 10, 0)
                                else:
                                    color = (0, 0, 0)
                            else:
                                # CASH COW cow spots body
                                if dist_sq > (1.0 * inflation):
                                    spot = math.sin(px * 0.25) * math.cos(py * 0.25) + math.sin(px * 0.1 + py * 0.15)
                                    if spot > 0.1:
                                        color = (240, 240, 240)
                                    else:
                                        color = (15, 15, 15)
                                else:
                                    color = (0, 0, 0)
                                    
                            # Specular sheen sweep animation
                            sheen_pos = int(time * 200.0) % 900 - 200
                            dist_to_sheen = abs(px - sheen_pos)
                            if dist_to_sheen < 12 and dist_sq > 1.0:
                                color = (255, 255, 255)
                                
                            draw.rectangle([px, py, px + 2, py + 2], fill=color)
                            
                            if is_glossy and dist_sq >= 2:
                                draw.rectangle([px, py, px + 1, py + 1], fill=(255, 255, 255))

def draw_retro_char(draw, char, x, y, size, color):
    # Draw simple retro styled grid-like characters
    # to emulate the classic remorse ANSI block style
    half = size / 2.0
    if char == 'T':
        draw.rectangle([x - half, y - half, x + half, y - half + 8], fill=color)
        draw.rectangle([x - 4, y - half, x + 4, y + half], fill=color)
    elif char == 'S':
        draw.rectangle([x - half, y - half, x + half, y - half + 8], fill=color)
        draw.rectangle([x - half, y - half, x - half + 8, y], fill=color)
        draw.rectangle([x - half, y - 4, x + half, y + 4], fill=color)
        draw.rectangle([x + half - 8, y, x + half, y + half], fill=color)
        draw.rectangle([x - half, y + half - 8, x + half, y + half], fill=color)
    elif char == 'F':
        draw.rectangle([x - half, y - half, x + half, y - half + 8], fill=color)
        draw.rectangle([x - half, y - half, x - half + 8, y + half], fill=color)
        draw.rectangle([x - half, y - 4, x + 4, y + 4], fill=color)
    elif char == 'i':
        draw.ellipse([x - 4, y - half, x + 4, y - half + 8], fill=color)
        draw.rectangle([x - 4, y - half + 12, x + 4, y + half], fill=color)
    elif char == '2':
        draw.rectangle([x - half, y - half, x + half, y - half + 8], fill=color)
        draw.rectangle([x + half - 8, y - half, x + half, y], fill=color)
        draw.rectangle([x - half, y - 4, x + half, y + 4], fill=color)
        draw.rectangle([x - half, y, x - half + 8, y + half], fill=color)
        draw.rectangle([x - half, y + half - 8, x + half, y + half], fill=color)

def main():
    width, height = 640, 480
    fps = 30
    duration = 22
    total_frames = fps * duration

    video_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2_remorse_demo.mp4"
    wav_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2_remorse_soundtrack.wav"

    # Pre-generate cute drum and bass audio soundtrack
    generate_soundtrack(wav_output, duration=duration)

    # Initialize FFmpeg pipeline with video input and audio track
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(fps),
        "-i", "-",
        "-i", wav_output,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480",
        "-c:a", "aac",
        "-shortest",
        video_output
    ]

    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    # 5 letter nodes representing the string "TSFi2"
    chars = ['T', 'S', 'F', 'i', '2']
    node_count = len(chars)

    # Dictionaries to track Pixar USD animation timesamples
    points_samples = {}
    translation_samples = {i: {} for i in range(node_count)}

    # Physical spline rope nodes matching the letters
    rope_nodes = []
    for i in range(node_count):
        t = i / float(node_count - 1)
        rx = 120.0 + t * 400.0
        ry = 240.0
        rope_nodes.append([rx, ry, 0.0, rx, ry, 0.0]) # x, y, z, px, py, pz

    # Initialize Verlet hair/fiber strands for each character node
    # Each strand has 3 segments: [x, y, px, py]
    hair_strands = []
    for i in range(node_count):
        node_hairs = []
        for h in range(3):
            strand = []
            for s in range(3):
                strand.append([0.0, 0.0, 0.0, 0.0])
            node_hairs.append(strand)
        hair_strands.append(node_hairs)

    # Simulation loop
    for frame in range(total_frames):
        time = frame / float(fps)

        # Sinusoidal swing forces acting on the character nodes
        wind_x = math.sin(time * 2.0) * 15.0
        wind_y = 20.0 + math.cos(time * 3.0) * 10.0

        # Dynamic anchors (rope swings from both ends)
        rope_nodes[0][0] = 120.0 + math.sin(time * 1.5) * 50.0
        rope_nodes[0][1] = 180.0 + math.cos(time * 1.0) * 30.0

        rope_nodes[-1][0] = 520.0 + math.sin(time * 1.2 + math.pi) * 50.0
        rope_nodes[-1][1] = 300.0 + math.cos(time * 1.6) * 30.0

        # Step middle nodes using Verlet integration equations
        for i in range(1, node_count - 1):
            tx, ty = rope_nodes[i][0], rope_nodes[i][1]
            
            # Verlet step
            dx = (rope_nodes[i][0] - rope_nodes[i][2]) * 0.96 + wind_x * 0.02
            dy = (rope_nodes[i][1] - rope_nodes[i][3]) * 0.96 + wind_y * 0.02
            
            rope_nodes[i][0] += dx
            rope_nodes[i][1] += dy
            
            rope_nodes[i][2], rope_nodes[i][3] = tx, ty

        # Distance constraint relaxation (keep segment lengths stable)
        target_dist = 400.0 / float(node_count - 1)
        for _ in range(4):
            for i in range(node_count - 1):
                dx = rope_nodes[i+1][0] - rope_nodes[i][0]
                dy = rope_nodes[i+1][1] - rope_nodes[i][1]
                dist = math.sqrt(dx*dx + dy*dy)
                if dist > 0.001:
                    diff = (target_dist - dist) / dist * 0.5
                    if i > 0:
                        rope_nodes[i][0] -= dx * diff
                        rope_nodes[i][1] -= dy * diff
                    if i + 1 < node_count - 1:
                        rope_nodes[i+1][0] += dx * diff
                        rope_nodes[i+1][1] += dy * diff

        # Update Verlet hair strands anchored to each character node
        for i in range(node_count):
            px = rope_nodes[i][0]
            py = rope_nodes[i][1]
            for h in range(3):
                # Anchor node 0
                hair_strands[i][h][0][0] = px + (h - 1) * 12.0
                hair_strands[i][h][0][1] = py + 18.0
                
                # Step segment nodes 1 and 2
                for s in range(1, 3):
                    curr = hair_strands[i][h][s]
                    prev = hair_strands[i][h][s-1]
                    
                    tx, ty = curr[0], curr[1]
                    h_dx = (curr[0] - curr[2]) * 0.9 + wind_x * 0.01
                    h_dy = (curr[1] - curr[3]) * 0.9 + wind_y * 0.01 + 2.0 # gravity
                    
                    curr[0] += h_dx
                    curr[1] += h_dy
                    curr[2], curr[3] = tx, ty
                    
                    # Constrain segment length
                    hx = curr[0] - prev[0]
                    hy = curr[1] - prev[1]
                    h_dist = math.sqrt(hx*hx + hy*hy)
                    if h_dist > 15.0:
                        curr[0] = prev[0] + (hx / h_dist) * 15.0
                        curr[1] = prev[1] + (hy / h_dist) * 15.0

        # Collect USD animation timesamples
        points_samples[frame] = [(n[0], n[1], 0.0) for n in rope_nodes]
        for i in range(node_count):
            translation_samples[i][frame] = (rope_nodes[i][0], rope_nodes[i][1], 0.0)

        # Render frame
        img = Image.new("RGB", (width, height), (5, 5, 10)) # Very dark background
        draw = ImageDraw.Draw(img)

        # Draw fBm fractal landscape background simulation
        for gy in range(0, height, 40):
            for gx in range(0, width, 40):
                # Calculate simple multi-octave fBm value
                n_val = math.sin(gx * 0.01 + time) * math.cos(gy * 0.01 + time)
                n_val += 0.5 * math.sin(gx * 0.02)
                fbm_color = int((n_val + 1.5) * 10)
                draw.rectangle([gx, gy, gx + 40, gy + 40], fill=(fbm_color, fbm_color // 2, fbm_color + 10))

        # Draw diagonal grid lines for retro remorse scene backdrop
        for grid_idx in range(-height, width, 40):
            draw.line([(grid_idx, 0), (grid_idx + height, height)], fill=(20, 15, 35), width=1)

        # Draw "$$ CASH COW $$" logo header at the top
        draw_cash_cow(draw, width, time)

        # Interpolate a smooth curve connecting the letters
        curve_points = []
        for i in range(node_count - 1):
            p0 = rope_nodes[max(0, i-1)]
            p1 = rope_nodes[i]
            p2 = rope_nodes[i+1]
            p3 = rope_nodes[min(node_count - 1, i+2)]

            for step in range(12):
                t = step / 12.0
                t2 = t * t
                t3 = t2 * t
                cx = 0.5 * ((2.0 * p1[0]) + (-p0[0] + p2[0]) * t + (2.0 * p0[0] - 5.0 * p1[0] + 4.0 * p2[0] - p3[0]) * t2 + (-p0[0] + 3.0 * p1[0] - 3.0 * p2[0] + p3[0]) * t3)
                cy = 0.5 * ((2.0 * p1[1]) + (-p0[1] + p2[1]) * t + (2.0 * p0[1] - 5.0 * p1[1] + 4.0 * p2[1] - p3[1]) * t2 + (-p0[1] + 3.0 * p1[1] - 3.0 * p2[1] + p3[1]) * t3)
                curve_points.append((cx, cy))
        
        curve_points.append((rope_nodes[-1][0], rope_nodes[-1][1]))

        # Draw a cute, 3D-shaded red heart balloon in the center background
        heart_poly = []
        for s_h in range(36):
            theta = (s_h / 36.0) * 2.0 * math.pi
            hx = 16.0 * (math.sin(theta) ** 3)
            hy = 13.0 * math.cos(theta) - 5.0 * math.cos(2.0*theta) - 2.0 * math.cos(3.0*theta) - math.cos(4.0*theta)
            px_h = int(hx * 7.0 + 320.0)
            py_h = int(hy * -7.0 + 240.0)
            heart_poly.append((px_h, py_h))
        
        # Red balloon body and pinkish highlight gloss layer
        draw.polygon(heart_poly, fill=(230, 25, 65))
        highlight_poly = []
        for s_h in range(36):
            theta = (s_h / 36.0) * 2.0 * math.pi
            hx = 16.0 * (math.sin(theta) ** 3)
            hy = 13.0 * math.cos(theta) - 5.0 * math.cos(2.0*theta) - 2.0 * math.cos(3.0*theta) - math.cos(4.0*theta)
            px_h = int(hx * 3.0 + 300.0)
            py_h = int(hy * -3.0 + 210.0)
            highlight_poly.append((px_h, py_h))
        draw.polygon(highlight_poly, fill=(255, 120, 140))

        draw.line(curve_points, fill=(40, 20, 60), width=2) # Dark connector thread

        # Atmospheric Volumetric light raymarching rays radiating from the center anchor
        cx_ray = width / 2.0 + math.sin(time) * 100.0
        cy_ray = height / 2.0
        for ray_step in range(16):
            angle = (ray_step / 16.0) * 2.0 * math.pi
            rx = cx_ray + math.cos(angle) * 300.0
            ry = cy_ray + math.sin(angle) * 300.0
            draw.line([(cx_ray, cy_ray), (rx, ry)], fill=(12, 10, 25), width=2)

        # Draw letters along the spline with green and purple gradient colors
        for i, char in enumerate(chars):
            t = i / float(node_count - 1)
            
            # Green (0, 255, 0) to Purple (180, 0, 220) gradient transition
            r = int(180.0 * t)
            g = int(255.0 * (1.0 - t))
            b = int(220.0 * t)
            color = (r, g, b)

            px = rope_nodes[i][0]
            py = rope_nodes[i][1]

            # Proximity contact shadowing (contact shadow occlusion) - rendered as a glossy black bubble balloon
            draw.ellipse([px - 22, py - 22, px + 22, py + 22], fill=(5, 5, 10))
            draw.ellipse([px - 20, py - 20, px + 20, py + 20], fill=(25, 20, 30))
            draw.ellipse([px - 14, py - 14, px - 6, py - 6], fill=(255, 255, 255))

            # Sub-surface scattering (translucent soft tissue outlines)
            draw_retro_char(draw, char, px, py, 42, (r // 4, g // 4, b // 4))

            # Draw block dropshadow
            draw_retro_char(draw, char, px + 4, py + 4, 36, (0, 0, 0))

            # Draw block character with environment map specular reflection highlight glints
            draw_retro_char(draw, char, px, py, 36, color)
            draw_retro_char(draw, char, px - 2, py - 2, 28, (255, 255, 255)) # Specular reflection glint

            # Draw Verlet hair strands with Kajiya-Kay specular highlights
            for h in range(3):
                points = []
                for s in range(3):
                    points.append((hair_strands[i][h][s][0], hair_strands[i][h][s][1]))
                
                tx_seg = points[2][0] - points[0][0]
                ty_seg = points[2][1] - points[0][1]
                t_len = math.sqrt(tx_seg*tx_seg + ty_seg*ty_seg)
                spec = 1.0
                if t_len > 0.1:
                    dot_tl = -ty_seg / t_len
                    spec = math.pow(math.sqrt(1.0 - dot_tl*dot_tl), 6.0)
                
                hair_r = int(r * (0.3 + 0.7 * spec))
                hair_g = int(g * (0.3 + 0.7 * spec))
                hair_b = int(b * (0.3 + 0.7 * spec))
                draw.line(points, fill=(hair_r, hair_g, hair_b), width=2)

        # Pipe image frame to FFmpeg
        img.save(process.stdin, "PNG")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Remorse style video rendered: {video_output}")

    # Write Pixar USD ASCII file (USDA) detailing dynamic nodes and curves
    usda_output = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2_remorse_scene.usda"
    with open(usda_output, "w") as f:
        f.write("#usda 1.0\n")
        f.write("(\n")
        f.write("    startTimeCode = 0\n")
        f.write(f"    endTimeCode = {total_frames - 1}\n")
        f.write("    upAxis = \"Y\"\n")
        f.write(")\n\n")
        f.write("def Xform \"RemorseScene\"\n")
        f.write("{\n")
        f.write("    def BasisCurves \"RopeSpline\"\n")
        f.write("    {\n")
        f.write("        uniform token basis = \"bezier\"\n")
        f.write("        uniform token type = \"cubic\"\n")
        f.write("        int[] curveVertexCounts = [5]\n")
        f.write("        point3f[] points.timeSamples = {\n")
        for f_idx in sorted(points_samples.keys()):
            pts_str = ", ".join([f"({x:.3f}, {y:.3f}, {z:.3f})" for x, y, z in points_samples[f_idx]])
            f.write(f"            {f_idx}: [{pts_str}],\n")
        f.write("        }\n")
        f.write("    }\n\n")
        for i, char in enumerate(chars):
            f.write(f"    def Xform \"Letter_{char}_{i}\"\n")
            f.write("    {\n")
            f.write("        double3 xformOp:translate.timeSamples = {\n")
            for f_idx in sorted(translation_samples[i].keys()):
                tx, ty, tz = translation_samples[i][f_idx]
                f.write(f"            {f_idx}: ({tx:.3f}, {ty:.3f}, {tz:.3f}),\n")
            f.write("        }\n")
            f.write("        uniform token[] xformOpOrder = [\"xformOp:translate\"]\n")
            f.write("    }\n")

        # Generate 3D Heart Balloon Mesh points
        heart_points = []
        steps_h = 24
        heart_points.append((0.0, 0.0, 15.0)) # Index 0
        heart_points.append((0.0, 0.0, -15.0)) # Index 1
        for s_h in range(steps_h):
            theta = (s_h / float(steps_h)) * 2.0 * math.pi
            hx = 16.0 * (math.sin(theta) ** 3)
            hy = 13.0 * math.cos(theta) - 5.0 * math.cos(2.0*theta) - 2.0 * math.cos(3.0*theta) - math.cos(4.0*theta)
            heart_points.append((hx * 6.0 + 320.0, hy * -6.0 + 240.0, 0.0))

        face_counts = []
        face_indices = []
        for s_h in range(steps_h):
            next_s = (s_h + 1) % steps_h
            face_counts.append(3)
            face_indices.extend([0, 2 + s_h, 2 + next_s])
        for s_h in range(steps_h):
            next_s = (s_h + 1) % steps_h
            face_counts.append(3)
            face_indices.extend([1, 2 + next_s, 2 + s_h])

        f.write("    def Mesh \"HeartBalloon\"\n")
        f.write("    {\n")
        f.write("        int[] faceVertexCounts = [" + ", ".join(map(str, face_counts)) + "]\n")
        f.write("        int[] faceVertexIndices = [" + ", ".join(map(str, face_indices)) + "]\n")
        pts_str = ", ".join([f"({x:.3f}, {y:.3f}, {z:.3f})" for x, y, z in heart_points])
        f.write(f"        point3f[] points = [{pts_str}]\n")
        f.write("    }\n")
        f.write("}\n")
    print(f"[SUCCESS] Pixar USDA scene description exported: {usda_output}")

if __name__ == "__main__":
    main()
