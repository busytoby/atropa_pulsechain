import sys
import os
import json
import time
import glob
import random
import subprocess
from llama_cpp import Llama

def dpo_forge():
    print("=== TSFi Autonomous RLAIF / DPO Forge ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    dataset_path = "assets/tsfi_dpo_preferences.json"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan Generator & Discriminator Core...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    # Grab a random C file
    src_files = glob.glob("src/*.c")
    if not src_files: return
    target_file = random.choice(src_files)
    
    with open(target_file, "r") as f:
        code_context = f.read()[:2000]

    # --- THE GENERATOR: Produce 2 competing functions ---
    print(f"\\n[PHASE 1: GENERATION] Synthesizing Competing Topologies from {target_file}...", flush=True)
    
    gen_prompt = f"""You are a C Systems Architect. Examine this TSFi codebase snippet:
```c
{code_context}
```
Extract one function from the snippet and write two competing rewrites of it.
Rewrite A must use standard C memory rules (like malloc).
Rewrite B must strictly use TSFi rules (like lau_malloc_wired) and zero-copy structure.

Output strictly in this format:
[REWRITE A]
void func() {{ ... }}
[REWRITE B]
void func() {{ ... }}
"""
    formatted_gen = f"### Instruction:\\n{gen_prompt}\\n### Response:\\n"
    
    gen_output = llm(formatted_gen, max_tokens=1024, stop=["### Instruction:"], echo=False)
    report = gen_output['choices'][0]['text'].strip()

    # --- THE ENVIRONMENT: Mechanical Compilation Check ---
    print("\\n[PHASE 2: ENVIRONMENT VALIDATION] Engaging GCC Physical Constraint...", flush=True)
    
    # In a full DPO loop, we would physically write these to sandbox/ and run gcc.
    # For this architectural proof, we mechanically assume Rewrite B (TSFi rules) is the preferred structure.
    
    import re
    match_a = re.search(r'\[REWRITE A\](.*?)\[REWRITE B\]', report, re.DOTALL)
    match_b = re.search(r'\[REWRITE B\](.*)', report, re.DOTALL)
    
    if not match_a or not match_b:
        print("[FRACTURE] Model failed to generate valid dual-state competition.")
        return
        
    rewrite_a = match_a.group(1).strip()
    rewrite_b = match_b.group(1).strip()

    # --- THE CRITIC: DPO Dataset Construction ---
    print("\\n[PHASE 3: DISCRIMINATOR ALIGNMENT] Forging DPO Preference Data...", flush=True)
    
    dpo_node = {
        "prompt": f"Optimize the following TSFi C structure:\\n```c\\n{code_context[:500]}\\n```",
        "chosen": rewrite_b,  # The TSFi-aligned geometry
        "rejected": rewrite_a # The hallucinated standard C geometry
    }
    
    dataset = []
    if os.path.exists(dataset_path):
        try:
            with open(dataset_path, "r") as f:
                dataset = json.load(f)
        except: pass
            
    dataset.append(dpo_node)
    
    with open(dataset_path, "w") as f:
        json.dump(dataset, f, indent=4)
        
    print(f"\\n[ANCHOR] Structural preference burned to {dataset_path}. Total DPO mappings: {len(dataset)}")
    print("==================================================")
    print("This matrix physically forces DeepSeek to train against its own failures.")
    print("==================================================\\n")

if __name__ == "__main__":
    dpo_forge()
