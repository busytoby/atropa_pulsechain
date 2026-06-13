import os
import struct
import math

def bessel_j0(x):
    """
    Taylor series expansion of the Bessel function of the first kind of order zero, J0(x).
    Converges very rapidly for small to medium values of x.
    """
    val = 0.0
    term = 1.0
    for k in range(12):
        if k > 0:
            term *= - (x * x) / (4.0 * k * k)
        val += term
    return val

def compile_dna():
    print("=== Procedural DNA Compiler for TSFi2 (Bessel-Biotika Edition) ===")
    
    TOTAL_FRAMES = 10800
    STAND_FRAME = 9900
    
    # Baseline colors from DeepSeek/VLM validation
    fur_r = 91
    fur_g = 63
    fur_b = 51
    
    eye_r = 0
    eye_g = 255
    eye_b = 0
    
    base_sick = 0.5
    
    # Biotika Neuron State Variables
    bionika_vm = 0.0
    leak_factor = 0.95  # 5% membrane decay per frame
    
    # Decaying physical muscle twitch filters (AI smooth kinematics)
    twitch_x = 0.0
    twitch_y = 0.0
    twitch_stretch = 0.0
    decay_rate = 0.88   # Smooth muscle relaxation rate
    
    # Decaying visual spike glow envelope
    spike_glow = 0.0
    
    os.makedirs("tsfi2-deepseek/assets", exist_ok=True)
    dna_path = "tsfi2-deepseek/assets/atropa.dna"
    
    with open(dna_path, "wb") as f:
        # Write DNA Header (Magic bytes + Total Frames)
        f.write(b'TSFI')
        f.write(struct.pack('=I', TOTAL_FRAMES))
        
        g_x = 0.0
        g_y = 0.0
        body_y_stretch = 0.25
        
        for frame in range(TOTAL_FRAMES):
            macro = frame / float(TOTAL_FRAMES)
            
            # Bessel Morphing: Calculate breathing pulse using J0(x)
            bessel_arg = (macro * math.pi * 50.0) % 5.0  # Modulo to stay in highly accurate Taylor convergence domain
            pulse = bessel_j0(bessel_arg)
            
            # Biotika Spiking Simulation
            # Stimulus is mapped from absolute pulse amplitude velocity
            stimulus = abs(pulse) * 0.075
            bionika_vm = bionika_vm * leak_factor + stimulus
            
            spike = False
            if bionika_vm >= 1.0:
                spike = True
                bionika_vm = 0.0  # Reset membrane potential
            
            # Stand & walk transitions
            if frame > STAND_FRAME:
                active = frame - STAND_FRAME
                if active < 60:
                    prog = active / 60.0
                    g_y = -(prog * 0.15)
                    body_y_stretch = 0.25 + (prog * 0.15)
                else:
                    walk = active - 60
                    g_y = -0.15 + (math.sin(walk * 0.5) * 0.02)
                    g_x = -(walk * 0.003) + (math.cos(walk * 0.25) * 0.01)
                    body_y_stretch = 0.40
            
            # Apply decaying muscle twitches for biological physical reactions
            if spike:
                twitch_x += 0.04 * bessel_j0(frame % 7)
                twitch_y += 0.02 * math.sin(frame)
                twitch_stretch += 0.08
                spike_glow = 1.0
            else:
                twitch_x *= decay_rate
                twitch_y *= decay_rate
                twitch_stretch *= decay_rate
                spike_glow *= 0.90 # Visual glow decay
            
            current_g_x = g_x + twitch_x
            current_g_y = g_y + twitch_y
            current_stretch = body_y_stretch + twitch_stretch
            
            dynamic_sick = base_sick - (macro * 0.2)
            if spike:
                # Spiking neuron event intensifies sickness mutation locally
                dynamic_sick = min(1.0, dynamic_sick + 0.25)
            
            # Eye suppression logic based on sickness state
            eye_count = 2
            if dynamic_sick < 0.6: 
                eye_count = 1
                
            # Specular lighting dynamics
            light_intensity = 0.5
            if current_g_y < 0.0:
                light_intensity += min(0.5, abs(current_g_y) * 2.0)
            
            # AI Aesthetic Enhancement: Dynamic fur transitions (Warm glow shifts towards magenta on spikes)
            fur_r_dyn = min(255, max(0, int(fur_r + 8 * pulse + 20 * spike_glow)))
            fur_g_dyn = min(255, max(0, int(fur_g + 5 * pulse)))
            fur_b_dyn = min(255, max(0, int(fur_b + 4 * pulse + 30 * spike_glow)))
            
            # AI Aesthetic Enhancement: Pulsing glowing eyes
            eye_g_dyn = min(255, max(0, int(180 + 40 * pulse + 75 * spike_glow)))
            eye_r_dyn = min(255, max(0, int(120 * dynamic_sick * (1.0 + spike_glow))))
            eye_b_dyn = min(255, max(0, int(50 * spike_glow)))
                
            # Pack: 6 floats (24 bytes) + 7 bytes = 31 bytes per frame
            frame_data = struct.pack('=ffffffBBBBBBB', 
                                     current_g_x, current_g_y, current_stretch, pulse, dynamic_sick, light_intensity,
                                     fur_r_dyn, fur_g_dyn, fur_b_dyn, eye_r_dyn, eye_g_dyn, eye_b_dyn, eye_count)
            f.write(frame_data)
            
    file_size = os.path.getsize(dna_path)
    print(f"[SUCCESS] Bessel-Biotika DNA vessel sealed at: {dna_path} ({file_size / 1024:.2f} KB).")

if __name__ == "__main__":
    compile_dna()
