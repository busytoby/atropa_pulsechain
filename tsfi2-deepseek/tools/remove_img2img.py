import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# 1. Lock the seed. Remove divergent seed logic.
code = code.replace(
    '''                    # Progressive Seed Logic:
                    # Lock the seed tightly for the first 10 frames to establish a perfect physical plush form.
                    # Then introduce a controlled, accelerating chaotic drift to simulate the decay mathematically.
                    base_epoch_seed = 42 + hash(current_genome + current_posture) % 10000
                    if global_frame_idx < 10:
                        divergent_seed = str(base_epoch_seed)
                    else:
                        divergent_seed = str(base_epoch_seed + int(global_frame_idx * 1.5))''',
    '''                    # Lock seed per epoch so the plushies remain coherent
                    base_epoch_seed = 42 + hash(current_genome + current_posture) % 10000
                    divergent_seed = str(base_epoch_seed)'''
)

# 2. Increase steps to 4 for better quality without img2img 
code = code.replace(
    '''                        "--steps", "2", # Ultra-fast 2-step generation to prevent RTMP buffering''',
    '''                        "--steps", "4", # Solid 4-step LCM generation'''
)

# 3. Completely remove the img2img pipelining which causes the distortion
code = code.replace(
    '''                    # Image-to-Image Pipelining: Carry the rotting pixels forward
                    if local_f_idx > 0 and os.path.exists(init_img_path):
                        # Force img2img strength to perfectly track the decay ratio, allowing the rot to compound
                        img2img_strength = min(0.65, 0.4 + (decay_ratio * 0.25))
                        cmd.extend(["--init-img", init_img_path, "--strength", f"{img2img_strength:.2f}"])''',
    '''                    # Removed img2img pipelining to stop unicode/noise distortion. 
                    # Each frame is generated purely from the locked seed + ControlNet skeleton.'''
)

with open(filepath, 'w') as f:
    f.write(code)

if os.path.exists("assets/mpx_last_out.png"):
    os.remove("assets/mpx_last_out.png")
if os.path.exists("assets/mpx_out.png"):
    os.remove("assets/mpx_out.png")
