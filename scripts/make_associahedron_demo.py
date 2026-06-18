import os
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30

def generate_local_elevenlabs_speech():
    print("[ElevenLabs ZMM] Triggering local speech synthesizer...")
    subprocess.run("node scripts/make_elevenlabs_speech.js", shell=True)
    return "temp_narration_raw.wav"

def get_audio_duration(file_path):
    cmd = f"ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 {file_path}"
    out = subprocess.check_output(cmd, shell=True)
    return float(out.strip())

def generate_background_synth(duration):
    print(f"[DSP] Generating backing track for {duration:.2f} seconds...")
    num_samples = int(SAMPLE_RATE * duration)
    t = np.linspace(0, duration, num_samples, endpoint=False)
    
    # 808-style kick pattern (at 120 BPM = 0.5s intervals)
    kick = np.zeros_like(t)
    beat_len = int(SAMPLE_RATE * 0.5)
    num_beats = int(duration / 0.5)
    for i in range(num_beats + 1):
        onset = i * beat_len
        age = (np.arange(num_samples) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < 0.4)
        if np.any(mask):
            k_age = age[mask]
            pitch_env = np.exp(-k_age / 0.04)
            freq = 48.0 + 102.0 * pitch_env
            phase_slice = 2.0 * np.pi * freq * k_age
            kick_val = np.sin(phase_slice) * np.exp(-k_age / 0.35)
            kick[mask] += np.tanh(kick_val * 1.5)

    # 303 arpeggiator (8th notes = 0.25s)
    bass = np.zeros_like(t)
    notes = [65.41, 130.81, 77.78, 98.00, 116.54, 130.81]
    num_steps = int(duration / 0.25)
    for i in range(num_steps + 1):
        step_duration = int(SAMPLE_RATE * 0.25)
        onset = i * step_duration
        note_freq = notes[i % len(notes)]
        age = (np.arange(num_samples) - onset) / SAMPLE_RATE
        mask = (age >= 0) & (age < 0.2)
        if np.any(mask):
            b_age = age[mask]
            phase = (note_freq * b_age) % 1.0
            saw = 2.0 * phase - 1.0
            env = np.exp(-b_age / 0.08)
            cutoff = 200.0 + 1200.0 * env
            coeff = (2.0 * np.pi * cutoff) / SAMPLE_RATE
            p = 0.0
            for j in range(len(saw)):
                p += coeff[j] * (saw[j] - p)
                saw[j] = p
            bass[mask] += np.tanh(saw * 2.0) * 0.18

    # Blend Kick & Bass
    backing = (kick * 0.35) + (bass * 0.22)
    backing_path = "temp_backing.wav"
    
    # Save backing track to file
    byte_data = bytearray(44 + len(backing) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(backing) * 2).to_bytes(4, "little")
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
    byte_data[40:44] = int(len(backing) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in backing:
        s = max(-1.0, min(1.0, val))
        v = int(s * 32767) if s >= 0 else int(s * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(backing_path, "wb") as f:
        f.write(byte_data)
    return backing_path

def merge_audio(narration_path, backing_path):
    print("[FFMPEG] Merging narration and backing track with child pitch-shift...")
    out_audio = "temp_audio_final.wav"
    cmd = (
        f"ffmpeg -y -i {backing_path} -i {narration_path} "
        f'-filter_complex "[1:a]rubberband=pitch=1.35,adelay=1000|1000[delayed];[0:a][delayed]amix=inputs=2:duration=first:dropout_transition=2" '
        f"-t 10 {out_audio}"
    )
    subprocess.run(cmd, shell=True)
    return out_audio

def render_loop_frames(total_frames):
    print(f"[GFX] Rendering {total_frames} animated frames with Stasheff Geometry & Sprite/Projectile Hit Tracker...")
    os.makedirs("temp_frames", exist_ok=True)
    
    # Left Panel: Associahedron vertices centered at (200, 280)
    pts_l = [
        (200, 160),
        (314, 243),
        (270, 377),
        (130, 377),
        (86, 243)
    ]

    for frame in range(total_frames):
        img = Image.new("RGB", (800, 600), "#060812")
        draw = ImageDraw.Draw(img)
        
        # Grid lines
        for y in range(0, 600, 40):
            draw.line([(0, y), (800, y)], fill="#0e1224", width=1)
        for x in range(0, 800, 40):
            draw.line([(x, 0), (x, 600)], fill="#0e1224", width=1)

        # Panel Divider
        draw.line([(390, 80), (390, 470)], fill="#1e293b", width=2)

        # Title Block
        draw.rectangle([(20, 15), (780, 75)], fill="#0f172a", outline="#334155", width=1)
        draw.text((35, 25), "GAUNTLET OF THE ASSOCIAHEDRON", fill="#ff007f")
        draw.text((35, 45), "🧬 INTEGRATED NEURAL LATTICE EMULATOR & ATARI PMG SPRITE ENGINE", fill="#00f3ff")
        
        # LEFT PANEL: Stasheff Geometry
        draw.text((40, 95), "I. ASSOCIAHEDRON KN (STASHEFF GEOMETRY)", fill="#ffd700")
        
        # Draw edges of pentagon
        for i in range(5):
            draw.line([pts_l[i], pts_l[(i + 1) % 5]], fill="#ffd700", width=2)
            
        # Draw sweeping active diagonal flips
        chord_state = (frame // 40) % 2
        if chord_state == 0:
            draw.line([pts_l[0], pts_l[2]], fill="#ff007f", width=4)
            draw.text((120, 410), "Active Chord: [A] (Thor/Warrior)", fill="#ff007f")
        else:
            draw.line([pts_l[4], pts_l[2]], fill="#00f3ff", width=4)
            draw.text((120, 410), "Active Chord: [B] (Thyra/Valkyrie)", fill="#00f3ff")
            
        # Vertices
        for idx, pt in enumerate(pts_l):
            draw.ellipse([pt[0]-6, pt[1]-6, pt[0]+6, pt[1]+6], fill="#00f3ff")
            draw.text((pt[0]+10, pt[1]-5), f"K4:{idx}", fill="#94a3b8")

        # RIGHT PANEL: Sprite PMG Gameplay & Projectile Hit Tracker
        draw.text((410, 95), "II. ATARI PMG SPRITE ENGINE & PROJECTILE HIT TRACKER", fill="#38bdf8")
        
        # Draw simulated retro playfield walls
        # Outer border
        draw.rectangle([(410, 125), (770, 450)], outline="#1e293b", width=2)
        
        # Draw Teddy Bear Army formation (3x3 grid)
        bears = []
        for r_idx in range(3):
            for c_idx in range(3):
                bx = 430 + c_idx * 30
                by = 240 + r_idx * 35
                bears.append((bx, by))
                # Draw small bear sprite (representing a Teddy Soldier)
                draw.rectangle([(bx-6, by-8), (bx+6, by+8)], fill="#78350f", outline="#b45309", width=1)
                # Draw tiny green eyes to signify DNA matches
                draw.rectangle([(bx-3, by-4), (bx-1, by-2)], fill="#00ff00")
                draw.rectangle([(bx+1, by-4), (bx+3, by-2)], fill="#00ff00")
        
        # Spawner (Ghost Generator) simulated at (710, 280)
        sx, sy = 710, 280
        draw.rectangle([(sx-20, sy-20), (sx+20, sy+20)], fill="#7f1d1d", outline="#f87171", width=2)
        draw.text((sx-15, sy-5), "SPAWN", fill="#f87171")
        
        # Projectile simulation loop (resets every 60 frames)
        cycle_len = 60
        progress = (frame % cycle_len) / float(cycle_len)
        is_hit = (frame % cycle_len) >= 55
        
        # Draw missiles from each bear towards Spawner (sx, sy)
        for idx, (bx, by) in enumerate(bears):
            proj_x = bx + (sx - bx) * progress
            proj_y = by + (sy - by) * progress
            
            # Draw Projectile Trail (Atari PMG Missile 0)
            trail_steps = 6
            for t_step in range(1, trail_steps):
                t_ratio = (frame % cycle_len - t_step) / float(cycle_len)
                if t_ratio >= 0:
                    tx = bx + (sx - bx) * t_ratio
                    ty = by + (sy - by) * t_ratio
                    draw.rectangle([(tx-1, ty-1), (tx+1, ty+1)], fill="#fbbf24")
            
            # Draw active Projectile
            if not is_hit:
                draw.rectangle([(proj_x-3, proj_y-3), (proj_x+3, proj_y+3)], fill="#fbbf24", outline="#ffffff", width=1)
        
        if is_hit:
            # Hit Explosion!
            draw.ellipse([sx-35, sy-35, sx+35, sy+35], outline="#f59e0b", width=3)
            draw.ellipse([sx-18, sy-18, sx+18, sy+18], fill="#ef4444")
            draw.text((sx-30, sy-42), "[ARMY COLLISION]", fill="#ef4444")
            
        # HUD / Hit Tracker Telemetry panel in right panel
        draw.rectangle([(420, 380), (760, 440)], fill="#020617", outline="#0f172a", width=1)
        draw.text((430, 385), f"PMG REGISTERS: HPOSM0-8 | HPOSP0-8 (Army Linked)", fill="#38bdf8")
        coll_status = "1 (HIT!)" if is_hit else "0"
        draw.text((430, 400), f"COLLISION REG: M0-8_PF={coll_status}  | P0-8_PF=0", fill="#ef4444" if is_hit else "#10b981")
        draw.text((430, 415), f"SCORE: {2400 + (frame // cycle_len) * 100}  | TEDDY ARMY: ACTIVE", fill="#ffd700")

        # BOTTOM TELEMETRY BOARD
        draw.rectangle([(20, 480), (780, 580)], fill="#090d16", outline="#1e293b", width=1)
        draw.text((35, 490), f"SYSTEM STATUS: COMPILING TO BIJECTIVE HELMHOLTZ RDNA4 EMULATION", fill="#e2e8f0")
        draw.text((35, 510), f"AUDIO: 16KHz Neural Voice (elevenlabs-zmm) mixed with 120BPM 303/808 backing track", fill="#a7f3d0")
        draw.text((35, 530), f"FRAME INDEX: {frame:04d}/{total_frames:04d}  |  SASI BUS STATUS: CD=1 REQ=1 ACK={(frame//15)%2}", fill="#f472b6")
        draw.text((35, 550), "TELEMETRY: Real-time Player-Missile collision mappings resolving on-chain.", fill="#94a3b8")

        img.save(f"temp_frames/frame_{frame:04d}.png")

def compile_final_video(audio_path):
    print("[FFMPEG] Generating final video file...")
    cmd = (
        f"ffmpeg -y -r 30 -i temp_frames/frame_%04d.png "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p "
        f"-c:a aac -b:a 192k -t 10 temple_associahedron_demo.mp4"
    )
    subprocess.run(cmd, shell=True)
    print("[SUCCESS] temple_associahedron_demo.mp4 successfully created!")

def cleanup(total_frames):
    print("[CLEANUP] Purging intermediate file layers...")
    for frame in range(total_frames):
        try:
            os.remove(f"temp_frames/frame_{frame:04d}.png")
        except FileNotFoundError:
            pass
    try:
        os.rmdir("temp_frames")
    except Exception:
        pass
    
    files = ["temp_backing.wav", "temp_narration_raw.wav", "temp_audio_final.wav"]
    for file in files:
        try:
            os.remove(file)
        except Exception:
            pass

if __name__ == "__main__":
    narration = generate_local_elevenlabs_speech()
    duration = 10.0
    backing = generate_background_synth(duration)
    final_audio = merge_audio(narration, backing)
    
    total_frames = int(duration * FPS)
    render_loop_frames(total_frames)
    compile_final_video(final_audio)
    cleanup(total_frames)
