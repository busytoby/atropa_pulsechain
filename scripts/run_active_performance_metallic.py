import os
import sys
import struct
import subprocess

def main():
    print("=== INITIATING AUTOMATED METALLIC PERFORMANCE RUN ===")
    
    # 1. Temporarily modify scripts/manifold_interop_renderer.c to boost specularity exponent
    # for high-gloss metallic casing surfaces
    with open("scripts/manifold_interop_renderer.c", "r") as f:
        code = f.read()
    
    metallic_code = code.replace(
        "double spec_power = 8.0 + ((reg.frame_modulation_factor + mq_boost) * 16.0);",
        "double spec_power = 32.0 + ((reg.frame_modulation_factor + mq_boost) * 64.0);" # Deep metallic gloss
    )
    
    with open("scripts/manifold_interop_renderer.c", "w") as f:
        f.write(metallic_code)

    # Rebuild interop objects
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

    # Restore original C code to keep workspace clean
    with open("scripts/manifold_interop_renderer.c", "w") as f:
        f.write(code)

    # 2. Simulate WinchesterMQ Register state progression
    reg_log = "active_performance_registry.bin"
    with open(reg_log, "wb") as f:
        for frame in range(960):
            epoch = min(8, frame // 110)
            barn_val = 0
            if epoch == 8:
                barn_val = 800
            elif epoch > 4:
                barn_val = 100 + (epoch - 4) * 150
                
            element_val = 1000
            mod_factor = float(barn_val) / float(element_val) if element_val > 0 else 1.0
            
            vx = [0.0] * 16
            vy = [0.0] * 16
            vz = [0.0] * 16
            f.write(struct.pack('<11Q2If16f16f16fII4108s', 
                0, 0, 0, 0, 0, 0, 0, element_val, 0, 0, barn_val, 
                epoch, frame, mod_factor,
                *vx, *vy, *vz, 0, 0, b'\x00' * 4108
            ))

    # 3. Trigger composition pipeline
    output_video = "active_performance_metallic.mp4"
    audio_path = "cloudburst_ambient_resonance.wav"
    
    # We edit run_composer.sh temporarily or run the inline script to target output_video
    subprocess.run([
        "./scripts/run_composer.sh",
        "--output", output_video,
        "--audio", audio_path
    ], check=True)

    # Clean up temp log
    if os.path.exists(reg_log):
        os.remove(reg_log)

    print(f"\n=== METALLIC PERFORMANCE COMPLETED ===")
    print(f"-> Composited video saved: {output_video}")

if __name__ == "__main__":
    main()
