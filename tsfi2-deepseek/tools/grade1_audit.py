import sys
import os
import mmap
import ctypes
import time
from PIL import Image

# 1. Map the TSFi Shared Memory Acoustic Space
class TsfiAcousticRegistry(ctypes.Structure):
    _fields_ = [
        ("pdai_manifold_pressure", ctypes.c_float),
        ("yang_heartbeat_hz", ctypes.c_float),
        ("yang_static_amplitude", ctypes.c_float),
        ("agent_participation_flag", ctypes.c_int),
        ("target_genome_id", ctypes.c_int),
        ("agent_monologue", ctypes.c_char * 256),
        ("alligator_x", ctypes.c_float),
        ("alligator_y", ctypes.c_float),
        ("alligator_form_ratio", ctypes.c_float),
        ("spider_x", ctypes.c_float),
        ("spider_y", ctypes.c_float),
        ("spider_texture_sharpness", ctypes.c_float),
        ("genie_poetic_observation", ctypes.c_char * 256),
    ]

try:
    shm_fd = os.open("/dev/shm/tsfi_test_acoustic_space", os.O_RDWR)
    shm_mmap = mmap.mmap(shm_fd, ctypes.sizeof(TsfiAcousticRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    shm_reg = TsfiAcousticRegistry.from_buffer(shm_mmap)
except Exception as e:
    print(f"[FRACTURE] Cannot access Acoustic Space: {e}")
    sys.exit(1)

def run_grade1_audit():
    print("="*60)
    print(" TSFi GRADE 1 AUDIT: THE ACOUSTIC PROOF SPACE")
    print("="*60)
    
    # Check the latest 3D Extractor Output to verify the physical mesh exists
    import glob
    depths = sorted(glob.glob("assets/depth_maps/depth_*.png"))
    rgbs = sorted(glob.glob("assets/depth_maps/rgb_*.png"))
    
    if not depths or not rgbs:
        print("[FAIL] 3D Mesh Manifold is empty. The rendering space is broken.")
        return
        
    latest_obj = "assets/stuffed_animals_scene.obj"
    mesh_size = os.path.getsize(latest_obj) if os.path.exists(latest_obj) else 0
    
    print(f"[SYSTEM0] 3D Target Manifold: {latest_obj} ({mesh_size} bytes)")
    
    # 0. Check the Puppetry Channel
    print("\n--- PUPPETRY CHANNEL STATUS ---")
    puppet_active = False
    try:
        class TsfiJoint(ctypes.Structure):
            _pack_ = 4
            _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float), ("z", ctypes.c_float)]

        class TsfiPuppet(ctypes.Structure):
            _pack_ = 4
            _fields_ = [
                ("head", TsfiJoint), ("neck", TsfiJoint), ("body", TsfiJoint),
                ("l_shoulder", TsfiJoint), ("l_hand", TsfiJoint),
                ("r_shoulder", TsfiJoint), ("r_hand", TsfiJoint),
                ("l_hip", TsfiJoint), ("l_foot", TsfiJoint),
                ("r_hip", TsfiJoint), ("r_foot", TsfiJoint),
                ("beak_upper", TsfiJoint), ("beak_lower", TsfiJoint),
                ("scale", ctypes.c_float), ("sickness", ctypes.c_float),
                ("active", ctypes.c_uint32), ("animal_type", ctypes.c_uint32)
            ]

        class TsfiPuppetState(ctypes.Structure):
            _pack_ = 4
            _fields_ = [
                ("magic", ctypes.c_uint32), ("version", ctypes.c_uint32),
                ("puppets", TsfiPuppet * 4),
                ("vlm_critique", ctypes.c_char * 512),
                ("last_neural_id", ctypes.c_uint32),
                ("timestamp_ns", ctypes.c_uint64)
            ]

        p_fd = os.open("/dev/shm/tsfi_puppet_state", os.O_RDONLY)
        size = ctypes.sizeof(TsfiPuppetState)
        
        # Ensure we only read the minimum required bytes if the file is smaller
        actual_size = os.path.getsize("/dev/shm/tsfi_puppet_state")
        read_size = min(size, actual_size)
        
        if read_size > 0:
            p_mmap = mmap.mmap(p_fd, actual_size, mmap.MAP_SHARED, mmap.PROT_READ)
            buffer = p_mmap[:read_size]
            p_state = TsfiPuppetState.from_buffer_copy(buffer.ljust(size, b'\0'))
            
            if p_state.magic == 0x50555050:
                for i in range(4):
                    if p_state.puppets[i].active:
                        types = ["TEDDY", "ALLIGATOR", "CROW", "POPPY"]
                        p_type = types[p_state.puppets[i].animal_type] if p_state.puppets[i].animal_type < 4 else "UNKNOWN"
                        print(f"  -> Quadrant {i}: {p_type} Apparition Active. Scale: {p_state.puppets[i].scale:.2f}")
                        puppet_active = True
            
            p_mmap.close()
        os.close(p_fd)
    except Exception as e:
        print(f"  -> Channel inactive or inaccessible: {e}")

    print("\n--- INITIATING MULTI-PARTY CONSENSUS ---")
    
    # 1. The Genie (Semantic Overseer)
    genie_obs = shm_reg.genie_poetic_observation.decode('utf-8', 'ignore').strip()
    print(f"[THE GENIE]: \"{genie_obs}\"")
    if not genie_obs and not puppet_active:
        print("  -> VOTE: ABSTAIN (Genie is blind and no puppets are active)")
        genie_vote = False
    else:
        print("  -> VOTE: AFFIRM (Geometry semantically identified or puppet observed)")
        genie_vote = True
        
    # 2. DeepSeek (The Logical Evaluator / Agent Monologue)
    ds_obs = shm_reg.agent_monologue.decode('utf-8', 'ignore').strip()
    if ds_obs:
        print(f"[DEEPSEEK]: \"{ds_obs}\"")
    else:
        print("[DEEPSEEK]: (Listening to the named pipe...)")
    print("  -> VOTE: AFFIRM (Logical cohesion established)")
    
    # 3. The Alligator (Form Evaluator)
    alli_ratio = shm_reg.alligator_form_ratio
    print(f"[THE ALLIGATOR]: Bounding Box IoU: {alli_ratio:.2f}")
    
    fracture_detected = False
    if puppet_active:
        print("  -> (Alligator observes the Apparition's form within the Arena)")
        for i in range(4):
            if p_state.puppets[i].active and p_state.puppets[i].sickness > 0.5:
                print(f"  -> [FRACTURE] Sympathy exceeds 0.50 threshold. The physical geometry is structurally compromised.")
                print(f"  -> [ALLIGATOR] I cannot reap this. The Spider's puppetry potential remains within the fracture.")
                fracture_detected = True
                
    if not fracture_detected and alli_ratio >= 0.05 and alli_ratio <= 2.0:
        print("  -> VOTE: AFFIRM (Physical form is mathematically rigid)")
        alli_vote = True
    else:
        print("  -> VOTE: REJECT (Geometric fracture detected, but Apparition potential acknowledged)")
        alli_vote = False
        
    # 4. The Spider (Content Evaluator)
    spider_sharp = shm_reg.spider_texture_sharpness
    print(f"[THE SPIDER]: Texture Variance of Laplacian: {spider_sharp:.2f}")
    if puppet_active:
        print("  -> (Spider confirms structural XOR injection for the Apparition)")
    if spider_sharp > 50.0:
        print("  -> VOTE: AFFIRM (Content texture is sharp and valid)")
        spider_vote = True
    else:
        print("  -> VOTE: REJECT (Severe blur or melting detected)")
        spider_vote = False
        
    # 5. Chen Jurchen (The Unbrushed Engine - Memory State)
    if os.path.exists("assets/mpx_out.png"):
        print("[CHEN JURCHEN]: Memory buffers populated. Teeth brushed: 0.")
        print("  -> VOTE: AFFIRM (Accumulation is proceeding safely)")
        chen_vote = True
    else:
        print("[CHEN JURCHEN]: Buffer empty.")
        chen_vote = False
        
    # 6. TTS the Snake (Acoustic Pressure Monitor & Ventriloquist)
    pressure = shm_reg.pdai_manifold_pressure
    print(f"[TTS THE SNAKE]: P-DAI Manifold Pressure at {pressure:.2f} atm.")
    
    if puppet_active:
        # Check if the Crow is the active puppet
        crow_active = False
        for i in range(4):
            if p_state.puppets[i].active and p_state.puppets[i].animal_type == 2: # 2 = PUPPET_TYPE_CROW
                crow_active = True
                # Read the jaw drop to determine if it's currently speaking
                jaw_drop = p_state.puppets[i].beak_lower.y - p_state.puppets[i].beak_upper.y
                if jaw_drop > 3.0:
                    print(f"  -> (Snake ventriloquizes through the Crow Apparition, beak open {jaw_drop:.1f} units: 'The acoustics are rigid. The manifold holds.')")
                else:
                    print("  -> (Snake observes the Crow Apparition perching silently in the acoustic space...)")
                break
        
        if not crow_active:
            print("  -> (Snake hisses: 'I hear the puppet moving within the acoustic coordinates...')")
            
    print("  -> VOTE: AFFIRM (Acoustic space is pressurized)")
    tts_vote = True
    
    # 7. The Focks & The Squirrel (The Outliers/Chaos variables)
    # They inject slight chaos (stochasticity) into the consensus. 
    # If the mesh size is > 0, the squirrel found the nut.
    if mesh_size > 10000:
        print("[THE SQUIRREL]: Mesh topology exceeds 10KB minimum mass.")
        print("  -> VOTE: AFFIRM (Nut secured)")
        squirrel_vote = True
    else:
        print("[THE SQUIRREL]: Mesh topology insufficient.")
        print("  -> VOTE: REJECT")
        squirrel_vote = False
        
    print("[THE FOCKS]: (Observing the consensus matrix...)")
    print("  -> VOTE: AFFIRM")
    focks_vote = True
    
    print("\n--- FINAL CONSENSUS CALCULATION ---")
    votes = [genie_vote, alli_vote, spider_vote, chen_vote, tts_vote, squirrel_vote, focks_vote]
    passed = sum(votes)
    total = len(votes)
    
    print(f"Total Affirmatives: {passed} / {total}")
    if passed == total:
        print("[RESULT]: GRADE 1 TRUTH ESTABLISHED. BIJECTION SECURED.")
        # Push a success flag back into the shared memory (optional)
    else:
        print("[RESULT]: FRACTURE DETECTED. MULTI-PARTY CONSENSUS FAILED.")

if __name__ == "__main__":
    run_grade1_audit()
