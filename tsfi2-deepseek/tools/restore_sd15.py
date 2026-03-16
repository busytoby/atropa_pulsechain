import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Restore SD1.5 + LoRA setup instead of LCM native which fails on SD.cpp version parsing
code = code.replace(
    '''                    cmd = [
                        "assets/sd_cpp/build/bin/sd-cli",
                        "-m", "assets/models/LCM_Dreamshaper_v7.safetensors",
                        "--vae", "assets/models/taesd.safetensors",''',
    '''                    # Append LoRA explicitly to prompt
                    prompt += " <lora:lcm_lora_sd15:1.0>"
                    
                    cmd = [
                        "assets/sd_cpp/build/bin/sd-cli",
                        "-m", "assets/models/sd15.safetensors",
                        "--vae", "assets/models/taesd.safetensors",
                        "--lora-model-dir", "assets/models",'''
)

with open(filepath, 'w') as f:
    f.write(code)

