import os
import struct
import math

# Rain World & Stuffed Bear Genomic Presets
PRESETS = {
    "slugcat": {
        "cranial_scale": 0.85,         # Small head
        "body_fatness": 0.90,          # Slender, flexible body
        "tail_length": 1.40,           # Long, active tail
        "muscle_stiffness": 0.96,      # Low damping (high wiggle/floppiness)
        "walk_frequency": 0.85,        # Fast walk cycle
        "breathing_frequency": 55.0,
        "biotika_membrane_threshold": 0.80,
        "sickness_resistance": 0.35,
        "epibar": 0.96,                # Outgoing bearing weight
        "hypobar": 0.35,               # Incoming bearing weight
        "base_fur_color": (230, 230, 230), # White Slugcat fur
        "eye_accent_color": (20, 20, 20)   # Dark obsidian eyes
    },
    "teddy": {
        "cranial_scale": 1.25,         # Large stuffed head
        "body_fatness": 1.35,          # Plump cotton-stuffed body
        "tail_length": 0.00,           # No tail
        "muscle_stiffness": 0.78,      # High damping (stiff cotton resistance)
        "walk_frequency": 0.40,        # Slow, heavy walk
        "breathing_frequency": 35.0,
        "biotika_membrane_threshold": 0.98,
        "sickness_resistance": 0.12,
        "epibar": 0.78,                # Outgoing bearing weight
        "hypobar": 0.12,               # Incoming bearing weight
        "base_fur_color": (91, 63, 51),    # Vintage brown bear fur
        "eye_accent_color": (0, 255, 0)     # Kr0wZ green eyes
    }
}

# Select active genome preset (slugcat / teddy)
ACTIVE_PRESET = "slugcat"
GENOME = PRESETS[ACTIVE_PRESET]

def bessel_j0(x):
    val = 0.0
    term = 1.0
    for k in range(12):
        if k > 0:
            term *= - (x * x) / (4.0 * k * k)
        val += term
    return val

def compile_dna():
    print(f"=== Procedural DNA Compiler for TSFi2 (Preset: {ACTIVE_PRESET.upper()}) ===")
    print(f"[GENOME] Cranial Scale: {GENOME['cranial_scale']}, Body Fatness: {GENOME['body_fatness']}")
    print(f"[GENOME] Muscle Stiffness: {GENOME['muscle_stiffness']}, Walk Freq: {GENOME['walk_frequency']}")
    print(f"[GENOME] Epibar: {GENOME['epibar']}, Hypobar: {GENOME['hypobar']}")
    
    TOTAL_FRAMES = 10800
    STAND_FRAME = 9900
    
    fur_r, fur_g, fur_b = GENOME["base_fur_color"]
    eye_r, eye_g, eye_b = GENOME["eye_accent_color"]
    
    base_sick = 0.5
    bionika_vm = 0.0
    leak_factor = 0.95
    
    twitch_x = 0.0
    twitch_y = 0.0
    twitch_stretch = 0.0
    decay_rate = GENOME["muscle_stiffness"]
    spike_glow = 0.0
    
    os.makedirs("tsfi2-deepseek/assets", exist_ok=True)
    dna_path = "tsfi2-deepseek/assets/atropa.dna"
    
    with open(dna_path, "wb") as f:
        # Write DNA Header (Magic bytes + Total Frames + Epibar + Hypobar)
        f.write(b'TSFI')
        f.write(struct.pack('=Iff', TOTAL_FRAMES, GENOME["epibar"], GENOME["hypobar"]))
        
        g_x = 0.0
        g_y = 0.0
        body_y_stretch = 0.25 * GENOME["body_fatness"]
        
        for frame in range(TOTAL_FRAMES):
            macro = frame / float(TOTAL_FRAMES)
            
            bessel_arg = (macro * math.pi * GENOME["breathing_frequency"]) % 5.0
            base_pulse = bessel_j0(bessel_arg)
            
            t_angle = macro * math.pi * 8.0
            tx = math.cos(t_angle)
            ty = math.sin(t_angle)
            tz = math.cos(t_angle * 1.5)
            tw = math.sin(t_angle * 1.5)
            
            xw_cos = math.cos(t_angle * 0.25)
            xw_sin = math.sin(t_angle * 0.25)
            rot_x = tx * xw_cos - tw * xw_sin
            rot_w = tx * xw_sin + tw * xw_cos
            
            tessarant_nuance = (rot_x + ty + tz + rot_w) * 0.12
            pulse = base_pulse + tessarant_nuance
            
            stimulus = abs(pulse) * 0.075
            bionika_vm = bionika_vm * leak_factor + stimulus
            
            spike = False
            if bionika_vm >= GENOME["biotika_membrane_threshold"]:
                spike = True
                bionika_vm = 0.0
            
            if frame > STAND_FRAME:
                active = frame - STAND_FRAME
                if active < 60:
                    prog = active / 60.0
                    g_y = -(prog * 0.15 * GENOME["cranial_scale"])
                    body_y_stretch = (0.25 + (prog * 0.15)) * GENOME["body_fatness"]
                else:
                    walk = active - 60
                    g_y = (-0.15 * GENOME["cranial_scale"]) + (math.sin(walk * GENOME["walk_frequency"]) * 0.02 * GENOME["tail_length"])
                    g_x = -(walk * 0.003) + (math.cos(walk * (GENOME["walk_frequency"] * 0.5)) * 0.01 * GENOME["tail_length"])
                    body_y_stretch = 0.40 * GENOME["body_fatness"]
            
            if spike:
                twitch_x += 0.04 * bessel_j0(frame % 7) * GENOME["tail_length"]
                twitch_y += 0.02 * math.sin(frame) * GENOME["cranial_scale"]
                twitch_stretch += 0.08 * GENOME["body_fatness"]
                spike_glow = 1.0
            else:
                twitch_x *= decay_rate
                twitch_y *= decay_rate
                twitch_stretch *= decay_rate
                spike_glow *= 0.90
            
            current_g_x = g_x + twitch_x
            current_g_y = g_y + twitch_y
            current_stretch = body_y_stretch + twitch_stretch
            
            dynamic_sick = base_sick - (macro * GENOME["sickness_resistance"] * 1.11)
            if spike:
                dynamic_sick = min(1.0, dynamic_sick + 0.25)
            
            eye_count = 2
            if dynamic_sick < 0.6: 
                eye_count = 1
                
            light_intensity = 0.5
            if current_g_y < 0.0:
                light_intensity += min(0.5, abs(current_g_y) * 2.0)
            
            fur_r_dyn = min(255, max(0, int(fur_r + 8 * pulse + 20 * spike_glow)))
            fur_g_dyn = min(255, max(0, int(fur_g + 5 * pulse)))
            fur_b_dyn = min(255, max(0, int(fur_b + 4 * pulse + 30 * spike_glow)))
            
            eye_g_dyn = min(255, max(0, int(eye_g * (0.7 + 0.3 * pulse) + 75 * spike_glow)))
            eye_r_dyn = min(255, max(0, int((eye_r + 120 * dynamic_sick) * (1.0 + spike_glow))))
            eye_b_dyn = min(255, max(0, int(eye_b + 50 * spike_glow)))
                
            frame_data = struct.pack('=ffffffBBBBBBB', 
                                     current_g_x, current_g_y, current_stretch, pulse, dynamic_sick, light_intensity,
                                     fur_r_dyn, fur_g_dyn, fur_b_dyn, eye_r_dyn, eye_g_dyn, eye_b_dyn, eye_count)
            f.write(frame_data)
            
    file_size = os.path.getsize(dna_path)
    print(f"[SUCCESS] DNA vessel sealed for preset '{ACTIVE_PRESET}' at: {dna_path} ({file_size / 1024:.2f} KB).")

if __name__ == "__main__":
    compile_dna()
