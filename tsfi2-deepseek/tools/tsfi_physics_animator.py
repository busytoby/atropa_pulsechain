import os
import mmap
import struct
import time
import math

def build_pm4_rect(x, y, w, h, color):
    # PKT3_DRAW_RECT = 0x2A
    header = (3 << 30) | (5 << 16) | (0x2A << 8)
    return struct.pack('=IIIIII', header, int(x), int(y), int(w), int(h), int(color))

def run_physics_animator():
    print("=== TSFi Trilateral Physics Animator ===")
    
    shm_fd = os.open("/dev/shm/tsfi_pm4_stream", os.O_CREAT | os.O_RDWR, 0o666)
    os.ftruncate(shm_fd, 8192)
    pm4_map = mmap.mmap(shm_fd, 8192, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    
    W = 800
    H = 600
    
    frame = 0
    epoch = 0
    epoch_duration = 180 # frames per epoch (3 seconds at 60fps)
    
    print("[ACTIVE] Commencing 9-Epoch Trilateral Simulation...", flush=True)
    
    while True:
        cycle_frame = frame % epoch_duration
        progress = cycle_frame / epoch_duration
        
        pm4_buffer = bytearray()
        packet_count = 0
        
        # --- EPOCH GEOMETRY ---
        if epoch == 0:
            # SEAL0 (SHOOT): Rod and Cone are born at the edges
            rod_x = 100
            cone_x = 600
            color = 0xFF00AAFF # Trilateral Blue
            
        elif epoch == 1:
            # SEAL1 to SEAL3: Moving towards convergence
            rod_x = 100 + (progress * 150)
            cone_x = 600 - (progress * 150)
            color = 0xFF00AAFF
            
        elif epoch == 2:
            # SEAL4 (CONJUGATE): Ionization. The dielectrics cross-couple
            rod_x = 250 + (progress * 100)
            cone_x = 450 - (progress * 100)
            color = 0xFFFFAA00 if frame % 10 < 5 else 0xFF00AAFF # High Energy Pulse
            
        elif epoch == 3:
            # SEAL5 to SEAL7: Locked together, saturated
            rod_x = 350
            cone_x = 350
            color = 0xFF00FF00 # Kr0wZ Green (Locked)
            
        else:
            # SEAL8 (MAGNETIZE): The Dai is completely fused.
            rod_x = 350
            cone_x = 350
            color = 0xFFFFFFFF # Absolute Rigidity
            # Draw the Reciprocity Shadow (Daiichi)
            shadow = build_pm4_rect(350, 420, 100, 30, 0xFF555555)
            pm4_buffer.extend(shadow)
            packet_count += 6

        # Oscillating Y for biological breath
        base_y = 250 + math.sin(frame * 0.05) * 20
        
        # Assemble Rod
        pm4_buffer.extend(build_pm4_rect(rod_x, base_y, 100, 100, color))
        packet_count += 6
        
        # Assemble Cone
        # If they haven't fused, draw the cone. (At epoch 3/4 they are directly overlapping)
        pm4_buffer.extend(build_pm4_rect(cone_x, base_y, 100, 100, color))
        packet_count += 6
        
        # The Helmholtz Root Bar (audit_lore.pos hash simulation)
        root_w = int(progress * 800) if epoch < 4 else 800
        root_color = 0xFF00FF00 if epoch >= 3 else 0xFF555555
        pm4_buffer.extend(build_pm4_rect(0, 580, root_w, 20, root_color))
        packet_count += 6
        
        # Inject to hardware
        pm4_map.seek(0)
        pm4_map.write(struct.pack('=I', packet_count))
        pm4_map.write(pm4_buffer)
        
        frame += 1
        if frame % epoch_duration == 0:
            epoch = (epoch + 1) % 5
            
        time.sleep(1/60.0) # 60 FPS

if __name__ == "__main__":
    run_physics_animator()
