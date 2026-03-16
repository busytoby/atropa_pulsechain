import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Add the mmap/struct imports and setup at the top
import_str = """import ctypes
import mmap
import fcntl

# TSFi Registry Definition
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

shm_fd = None
shm_reg = None
try:
    shm_fd = os.open("/dev/shm/tsfi_test_acoustic_space", os.O_RDWR)
    shm_mmap = mmap.mmap(shm_fd, ctypes.sizeof(TsfiAcousticRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    shm_reg = TsfiAcousticRegistry.from_buffer(shm_mmap)
    print("[INFO] Bound to TSFi Shared Memory Acoustic Space.")
except Exception as e:
    print(f"[WARN] Acoustic Registry not found. Running disconnected. ({e})")
"""

code = code.replace(
    'from PIL import Image, ImageDraw, ImageFont\nimport cv2\nimport numpy as np',
    'from PIL import Image, ImageDraw, ImageFont\nimport cv2\nimport numpy as np\n' + import_str
)

# Update the Genie thread to write its observation into the registry
old_genie = """        ans2 = genie_model.answer_question(enc_image, q2, genie_processor)
        
        genie_current_thought = f"DAEMON SEES: {ans1.upper()} | GRADE: {ans2.upper()}"
    except:"""

new_genie = """        ans2 = genie_model.answer_question(enc_image, q2, genie_processor)
        
        genie_current_thought = f"DAEMON SEES: {ans1.upper()} | GRADE: {ans2.upper()}"
        
        if shm_reg is not None:
            shm_reg.genie_poetic_observation = str(ans1).encode('utf-8')[:255]
    except:"""

code = code.replace(old_genie, new_genie)


# Update the Alligator Vision Critic to write its findings to the registry
old_alligator = """        render_area = max(1, (rx_max - rx_min) * (ry_max - ry_min))
        ratio = render_area / float(skel_area)"""

new_alligator = """        render_area = max(1, (rx_max - rx_min) * (ry_max - ry_min))
        ratio = render_area / float(skel_area)
        
        if shm_reg is not None:
            shm_reg.alligator_form_ratio = float(ratio)
            # The contour center represents the physical Alligator mass
            shm_reg.alligator_x = float(rx_min + (rx_max - rx_min)/2.0)
            shm_reg.alligator_y = float(ry_min + (ry_max - ry_min)/2.0)"""

code = code.replace(old_alligator, new_alligator)

# Update the Spider Vision Critic to write its findings to the registry
old_spider = """        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0"""

new_spider = """        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0
        
        if shm_reg is not None:
            shm_reg.spider_texture_sharpness = float(sharpness)"""

code = code.replace(old_spider, new_spider)

with open(filepath, 'w') as f:
    f.write(code)

