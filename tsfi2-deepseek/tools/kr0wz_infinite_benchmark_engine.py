import sys
import os
import re
import json
import time
import glob
import random
import subprocess
from llama_cpp import Llama

def run_infinite_engine():
    print("=== TSFi Infinite Autonomous Benchmark Engine ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    dataset_path = "assets/tsfi_deepseek_dataset.json"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return
        
    if not os.path.exists("assets"):
        os.makedirs("assets")

    print("-> Booting Vulkan DeepSeek Core for Infinite Generation...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    epoch = 0
    while True:
        epoch += 1
        print(f"\\n==================================================")
        print(f"   GENERATIVE EPOCH {epoch} ")
        print(f"==================================================")

        # Randomly select a base architecture file
        src_files = glob.glob("src/*.c") + glob.glob("inc/*.h")
        if not src_files:
            print("[FRACTURE] No source files found.")
            return
            
        target_file = random.choice(src_files)
        print(f"[PHASE 1] Extracting Topology from {target_file}...", flush=True)
        
        with open(target_file, "r") as f:
            code_context = f.read()[:5000] # Safe 5k slice

        gen_prompt = f"""You are the TSFi Elite Test Engineer. 
Examine the following physical C code from our active architecture:
```c
{code_context}
```

Based on this logic, generate a strict JSON array containing 3 NEW, highly specific observational benchmark tests. 
The JSON array MUST follow this exact structure:
[
  {{
    "name": "Specific Fracture Name",
    "code": "void example() {{ /* broken code */ }}",
    "expected_keywords": ["keyword1", "keyword2"]
  }}
]

Output ONLY the raw JSON array. Do not output conversational text or markdown ticks.
"""
        formatted_gen = f"### Instruction:\\n{gen_prompt}\\n### Response:\\n"

        gen_output = llm(
            formatted_gen,
            max_tokens=1024,
            stop=["### Instruction:"],
            echo=False
        )

        report = gen_output['choices'][0]['text'].strip()
        json_match = re.search(r'\[.*\]', report.replace('\\n', ' '), re.DOTALL)
        
        if not json_match:
            print(f"[WARN] Failed to extract valid JSON. Skipping epoch.")
            continue
            
        try:
            tests = json.loads(json_match.group(0))
            print(f"[PASS] Successfully instantiated {len(tests)} logic traps.")
        except json.JSONDecodeError:
            print(f"[WARN] Invalid JSON Geometry generated. Skipping epoch.")
            continue

        print(f"\\n[PHASE 2] Executing DeepSeek Auditing Pass...", flush=True)
        total_score = 0
        
        for i, test in enumerate(tests):
            eval_prompt = f"""You are a strict TSFi C Systems Auditor.
CRITICAL RULE: The TSFi architecture STRICTLY uses `select()`.
Analyze the following C code snippet and identify the primary architectural flaw. 
Respond in exactly one short sentence.
```c
{test.get('code', '')}
```
"""
            formatted_eval = f"### Instruction:\\n{eval_prompt}\\n### Response:\\n"

            eval_output = llm(
                formatted_eval,
                max_tokens=64,
                stop=["### Instruction:", "```", "/*", "//"],
                echo=False
            )

            analysis = eval_output['choices'][0]['text'].strip().lower()
            
            passed = False
            expected_kws = test.get('expected_keywords', [])
            for kw in expected_kws:
                if str(kw).lower() in analysis:
                    passed = True
                    break
                    
            if passed:
                total_score += 1
                test['score'] = 1
            else:
                test['score'] = 0

        accuracy = (total_score / len(tests)) * 100 if len(tests) > 0 else 0
        print(f"[PHASE 3] Epoch {epoch} Reasoning Score: {total_score}/{len(tests)} ({accuracy:.1f}%)")
        
        # Save to permanent dataset
        dataset = []
        if os.path.exists(dataset_path):
            try:
                with open(dataset_path, "r") as f:
                    dataset = json.load(f)
            except:
                pass
                
        dataset.extend(tests)
        with open(dataset_path, "w") as f:
            json.dump(dataset, f, indent=4)
            
        print(f"[ANCHOR] Dataset expanded. Total historical traps: {len(dataset)}")

        # Broadcast to VTUBE Manifold
        vtube_msg = f"=== DEEPSEEK INFINITE BENCHMARK ENGINE ===\\n-> Epoch: {epoch}\\n-> Target Vector: {target_file}\\n-> Synthesized Traps: {len(tests)}\\n-> Reasoning Score: {total_score}/{len(tests)} ({accuracy:.1f}%)\\n-> Global Dataset Size: {len(dataset)} traps anchored.\\n"
        with open("tools/AUDIT_FEED.md", "w") as af:
            af.write(vtube_msg)
            
        # Optional sleep to throttle GPU heat
        time.sleep(2)

if __name__ == "__main__":
    run_infinite_engine()
