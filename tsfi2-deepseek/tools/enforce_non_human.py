import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Update positive prompt to be even more aggressively non-human, specifying NO anthropomorphism.
old_pos = 'prompt = f"A photorealistic macro photography shot of three stuffed plush toys interacting, NO HUMANS, strictly non-human stuffed animals, {current_genome} {action_text}, {defect_string}, professional softbox studio lighting, bright clean photography, well-lit environment, neutral background, highly detailed stuffed animals, masterpiece, 8k resolution, cute, friendly <lora:lcm_lora_sd15:1.0>"'
new_pos = 'prompt = f"A photorealistic macro photography shot of strictly non-human stuffed animals, NO HUMANS, NO ANTHROPOMORPHISM, just three plush toys interacting, {current_genome} {action_text}, {defect_string}, professional softbox studio lighting, bright clean photography, well-lit environment, neutral background, highly detailed plush fabric textures, masterpiece, 8k resolution, cute, friendly <lora:lcm_lora_sd15:1.0>"'

code = code.replace(old_pos, new_pos)

# Update negative prompt to be an absolute wall against any human interpretation.
old_neg = '"-n", "human, person, man, woman, child, people, human face, human hands, human body, realistic animals, flesh, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",'
new_neg = '"-n", "human, humanoid, person, man, woman, child, people, boy, girl, human face, human hands, human fingers, human limbs, human skin, human body, realistic animals, flesh, standing upright like a human, wearing human clothes, abstract, blurry, ugly, deformed anatomy, bad proportions, missing limbs, disconnected limbs, mutation, balls, spheres, blobs, melting, loose parts, simple geometric shapes, text, font, unicode, watermark, signature",'

code = code.replace(old_neg, new_neg)

with open(filepath, 'w') as f:
    f.write(code)

