import os
import sys
import math
import subprocess
from PIL import Image, ImageDraw, ImageFont

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

    # Initialize FFmpeg pipeline
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-f", "image2pipe",
        "-vcodec", "png",
        "-r", str(fps),
        "-i", "-",
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480",
        video_output
    ]

    process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    # 5 letter nodes representing the string "TSFi2"
    chars = ['T', 'S', 'F', 'i', '2']
    node_count = len(chars)

    # Physical spline rope nodes matching the letters
    rope_nodes = []
    for i in range(node_count):
        t = i / float(node_count - 1)
        rx = 120.0 + t * 400.0
        ry = 240.0
        rope_nodes.append([rx, ry, 0.0, rx, ry, 0.0]) # x, y, z, px, py, pz

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

            # Sub-surface scattering (translucent soft tissue outlines)
            draw_retro_char(draw, char, px, py, 42, (r // 4, g // 4, b // 4))

            # Draw block dropshadow
            draw_retro_char(draw, char, px + 4, py + 4, 36, (0, 0, 0))

            # Draw block character with environment map specular reflection highlight glints
            draw_retro_char(draw, char, px, py, 36, color)
            draw_retro_char(draw, char, px - 2, py - 2, 28, (255, 255, 255)) # Specular reflection glint

        # Pipe image frame to FFmpeg
        img.save(process.stdin, "PNG")

    process.stdin.close()
    process.wait()
    print(f"[SUCCESS] Remorse style video rendered: {video_output}")

if __name__ == "__main__":
    main()
