import os
import sys
import struct
import subprocess

def main():
    print("=== INITIATING AUTOMATED ACTIVE PERFORMANCE RUN ===")
    
    # 1. Compile interop components fresh to ensure absolute consistency
    print("[COMPILER] Rebuilding shared registry and compositor...")
    subprocess.run([
        "gcc", "-O3", "-shared", "-fPIC", 
        "scripts/libantigravity_interop.c", 
        "-o", "scripts/libantigravity_interop.so"
    ], check=True)
    
    subprocess.run([
        "gcc", "-O3", "scripts/manifold_interop_renderer.c", 
        "-o", "scripts/manifold_interop_renderer", 
        "-Lscripts/", "-lantigravity_interop", "-lm", 
        "-Wl,-rpath,./scripts/"
    ], check=True)

    # 2. Simulate WinchesterMQ Register state progression across 960 frames
    print("[SIMULATOR] Generating VM register telemetry log...")
    reg_log = "active_performance_registry.bin"
    with open(reg_log, "wb") as f:
        for frame in range(960):
            # Map active epoch (0 to 8)
            epoch = min(8, frame // 110)
            
            # Modulate thermodynamic Barn heat (builds up to peak in epoch 8)
            barn_val = 0
            if epoch == 8:
                # Simulates thermal stabilization lock fluctuations
                barn_val = 600 + int(200 * abs(struct.pack('f', frame / 30.0)[0] % 1.0))
            elif epoch > 4:
                # Builds heat during compression phase
                barn_val = 100 + (epoch - 4) * 100
                
            element_val = 1000
            # Calculate modulation factor dynamically
            mod_factor = float(barn_val) / float(element_val) if element_val > 0 else 1.0
            
            # Pack matching InteropRegistry layout:
            # 11 uint64_t registers (88 bytes)
            # 2 uint32_t trackers (8 bytes)
            # 1 float modulation factor (4 bytes)
            # 16 float verlet_x (64 bytes)
            # 16 float verlet_y (64 bytes)
            # 16 float verlet_z (64 bytes)
            # 1 uint32_t active_verlet_count (4 bytes)
            # 1 uint32_t boundary_collision_flags (4 bytes)
            # 4108 bytes sector queue padding
            vx = [0.0] * 16
            vy = [0.0] * 16
            vz = [0.0] * 16
            f.write(struct.pack('<11Q2If16f16f16fII4108s', 
                0, 0, 0, 0, 0, 0, 0, element_val, 0, 0, barn_val, 
                epoch, frame, mod_factor,
                *vx, *vy, *vz, 0, 0, b'\x00' * 4108
            ))

    # 3. Trigger composition pipeline using the simulated register telemetry
    print("[PIPELINE] Compositing video using simulated WinchesterMQ registry...")
    output_video = "active_performance.mp4"
    audio_path = "cloudburst_ambient_resonance.wav"
    
    subprocess.run([
        "./scripts/run_composer.sh",
        "--output", output_video,
        "--audio", audio_path
    ], check=True)

    # 4. Generate vocalized Dirac commentary narration
    print("[NARRATOR] Running vocoded Dirac commentary...")
    # Change output dir in speech script to match active performance output
    # We will trigger the speech demo script to sync commentary
    subprocess.run([
        "python3", "scripts/make_teddy_speech_demo.py"
    ], check=True)

    # Clean up temp log
    if os.path.exists(reg_log):
        os.remove(reg_log)

    print(f"\n=== PERFORMANCE COMPLETED ===")
    print(f"-> Composited video saved: {output_video}")
    print(f"-> Commentary video saved: /home/mariarahel/.gemini/antigravity-cli/brain/38436d6a-7ad9-4ea9-8cde-2800837ce953/teddy_speech_demo.mp4")

if __name__ == "__main__":
    main()
