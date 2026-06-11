import os
import struct
import math

def bessel_j0(x):
    """Taylor series expansion of the Bessel function of order zero, J0(x)"""
    val = 0.0
    term = 1.0
    for k in range(12):
        if k > 0:
            term *= - (x * x) / (4.0 * k * k)
        val += term
    return val

def compile_dragon_dna():
    print("=== Procedural Dragon's Lair Style DNA Compiler ===")
    
    TOTAL_FRAMES = 1200
    
    # Red Dragon baseline colors (Don Bluth Cel Red: RGB 200, 30, 45)
    dragon_r = 200
    dragon_g = 30
    dragon_b = 45
    
    # Glowing yellow eyes (RGB 255, 215, 0)
    eye_r = 255
    eye_g = 215
    eye_b = 0
    
    os.makedirs("tsfi2-deepseek/assets", exist_ok=True)
    dna_path = "tsfi2-deepseek/assets/dragon.dna"
    
    with open(dna_path, "wb") as f:
        # Write DNA Header (Magic bytes + Total Frames)
        f.write(b'TSFI')
        f.write(struct.pack('=I', TOTAL_FRAMES))
        
        for frame in range(TOTAL_FRAMES):
            # Phase mapping
            # Phase 0: 0-300 (Breathing / Idle)
            # Phase 1: 300-600 (Crouch & Build up)
            # Phase 2: 600-900 (Fire blast attack)
            # Phase 3: 900-1200 (Roar / Victory)
            
            g_x = 0.0
            g_y = 0.0
            stretch = 1.0
            fire_intensity = 0.0
            light_intensity = 0.5
            
            # Breathing pulse using J0
            bessel_arg = (frame * math.pi * 0.1) % 10.0
            pulse = bessel_j0(bessel_arg)
            
            if frame < 300: # Idle
                # Gentle vertical breathing bob
                g_y = pulse * 0.05
                stretch = 1.0 + pulse * 0.02
                fire_intensity = 0.1
                light_intensity = 0.4 + pulse * 0.05
                
            elif frame < 600: # Crouch
                # Squash down preparing to strike
                t = (frame - 300) / 300.0
                g_y = -t * 0.15
                stretch = 1.0 - t * 0.25 # squash
                fire_intensity = 0.1 + t * 0.3 # fire building up in throat
                light_intensity = 0.3 + t * 0.2
                
            elif frame < 900: # Fire blast
                # Stretch forward and fire eruption
                t = (frame - 600) / 300.0
                g_x = t * 0.25
                g_y = 0.1 * math.sin(t * math.pi * 8) # shake
                stretch = 1.0 + t * 0.3 # stretch
                fire_intensity = 1.0 - (t * 0.8) # extreme fire blast
                light_intensity = 1.0 - (t * 0.5) # blinding flash of light
                
            else: # Roar / Victory
                t = (frame - 900) / 300.0
                g_y = 0.1 - (t * 0.1) + pulse * 0.08
                stretch = 1.0 + pulse * 0.05
                fire_intensity = 0.2
                light_intensity = 0.5 + pulse * 0.1
                
            eye_count = 2
            
            # Pack matching the 31-byte struct specification:
            # 6 floats (24 bytes) + 7 bytes = 31 bytes
            frame_data = struct.pack('=ffffffBBBBBBB', 
                                     g_x, g_y, stretch, pulse, fire_intensity, light_intensity,
                                     dragon_r, dragon_g, dragon_b, eye_r, eye_g, eye_b, eye_count)
            f.write(frame_data)
            
    file_size = os.path.getsize(dna_path)
    print(f"[SUCCESS] Dragon's Lair DNA sequence compiled at: {dna_path} ({file_size / 1024:.2f} KB).")

if __name__ == "__main__":
    compile_dragon_dna()
