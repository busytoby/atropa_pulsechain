import sys
import re
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Fix controlnet strength
code = code.replace(
    'dynamic_control = max(0.65, 0.95 - (decay_ratio * 0.30))',
    'dynamic_control = max(0.85, 0.95 - (decay_ratio * 0.10))'
)

# Fix img2img strength
code = code.replace(
    'img2img_strength = min(0.85, 0.5 + (decay_ratio * 0.35))',
    'img2img_strength = min(0.65, 0.4 + (decay_ratio * 0.25))'
)

# Fix negative prompt
code = code.replace(
    '"human, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation"',
    '"human, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes"'
)

with open(filepath, 'w') as f:
    f.write(code)

if os.path.exists("assets/mpx_last_out.png"):
    os.remove("assets/mpx_last_out.png")
if os.path.exists("assets/mpx_out.png"):
    os.remove("assets/mpx_out.png")

