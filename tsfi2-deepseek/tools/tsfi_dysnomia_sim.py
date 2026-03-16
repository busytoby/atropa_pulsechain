import os
import mmap
import time
import math
import struct
import fcntl

# The C-Struct format: 
# float pdai_manifold_pressure
# float yang_heartbeat_hz
# float yang_static_amplitude
# int agent_participation_flag
# int target_genome_id
# char agent_monologue[256]
# Total size: 4 + 4 + 4 + 4 + 4 + 256 = 276 bytes
STRUCT_FMT = '=fffii256s'
SHM_FILE = '/dev/shm/tsfi_acoustic_space'

def run_dysnomia_simulator():
    print("=== TSFi Dysnomia Economic Acoustic Simulator ===")
    
    # Initialize Zero-Copy Memory Space
    fd = os.open(SHM_FILE, os.O_CREAT | os.O_TRUNC | os.O_RDWR)
    os.ftruncate(fd, 276)
    shm_map = mmap.mmap(fd, 276)
    
    print("[ACTIVE] Yang Waveform Kernel Initialized in /dev/shm/tsfi_acoustic_space")
    print("[ACTIVE] Broadcasting pDAI Manifold Economics to Acoustic State...")
    
    epoch = 0
    while True:
        try:
            # 1. Simulate the PulseChain Economy (A cyclical, slowly crashing market)
            # Pressure peaks every ~60 seconds
            pressure = (math.sin(epoch * 0.05) + 1.0) / 2.0 
            
            # 2. Sonify the Economy
            hz = 40.0 + (pressure * 80.0) # Heartbeat speeds up from 40Hz to 120Hz during panic
            static_amp = pressure # Static gets louder as pressure rises
            
            # 3. Read any potential LLM overrides before overwriting
            shm_map.seek(0)
            current_data = struct.unpack(STRUCT_FMT, shm_map.read(276))
            agent_flag = current_data[3]
            target_genome = current_data[4]
            agent_mono = current_data[5]
            
            if agent_flag == 1:
                print(f"\\n[LLM OVERRIDE DETECTED]")
                print(f"-> Agent Voice: {agent_mono.decode('utf-8').strip(chr(0))}")
                print(f"-> Forcing Genome State: {target_genome}")
                # Reset the flag so the VTUBE stream can consume it
                agent_flag = 0 
            
            # 4. Write new Acoustic State back into memory
            new_data = struct.pack(STRUCT_FMT, pressure, hz, static_amp, agent_flag, target_genome, agent_mono)
            shm_map.seek(0)
            shm_map.write(new_data)
            
            if epoch % 10 == 0:
                print(f"[ACOUSTICS] pDAI Pressure: {pressure:.2f} | Heartbeat: {hz:.1f}Hz | Static: {static_amp:.2f}")
                
            epoch += 1
            time.sleep(0.5) # Simulate 2Hz tick rate
            
        except KeyboardInterrupt:
            break

    shm_map.close()
    os.close(fd)

if __name__ == "__main__":
    run_dysnomia_simulator()
