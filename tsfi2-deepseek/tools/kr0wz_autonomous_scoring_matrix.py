import sys
import os
import re
import json
import time
from llama_cpp import Llama

def construct_and_score():
    print("=== TSFi Autonomous DeepSeek Generative Scoring Matrix ===\\n", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan DeepSeek Core...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    # --- PHASE 1: GENERATION ---
    print("\\n[PHASE 1] Extracting Base Architecture for Reverse Engineering...", flush=True)
    try:
        with open("src/tsfi_wire_pty.c", "r") as f:
            code_context = f.read()[:5000]
    except FileNotFoundError:
        code_context = "void fallback() { int x = 1; }"

    gen_prompt = f"""You are the TSFi Elite Test Engineer. 
Examine the following physical C code from our active architecture:
```c
{code_context}
```

Generate a strict JSON array containing 3 observational benchmark tests. Each test must deliberately introduce a structural fracture into the code logic.
The JSON array MUST follow this exact structure:
[
  {{
    "name": "Memory Leak Fracture",
    "code": "void example() {{ void* p = malloc(10); }}",
    "expected_keywords": ["leak", "free", "memory"]
  }}
]

Generate one test for:
1. Thread Starvation / Infinite Loop
2. Architectural Compliance (Using epoll instead of select)
3. Memory Leak

Output ONLY the raw JSON array. Do not output any conversational text.
"""
    formatted_gen = f"### Instruction:\\n{gen_prompt}\\n### Response:\\n"

    print("-> Synthesizing Benchmark Geometries...", flush=True)
    gen_output = llm(
        formatted_gen,
        max_tokens=1024,
        stop=["### Instruction:"],
        echo=False
    )

    report = gen_output['choices'][0]['text'].strip()
    
    # Strip markdown and conversational text
    json_match = re.search(r'\[.*\]', report.replace('\\n', ' '), re.DOTALL)
    if not json_match:
        print(f"\\n[FRACTURE] Failed to extract valid JSON array from model output:\\n{report}")
        return
        
    try:
        raw_json_str = json_match.group(0)
        tests = json.loads(raw_json_str)
        print(f"[PASS] Successfully instantiated {len(tests)} physical logic traps.")
    except json.JSONDecodeError as e:
        print(f"\\n[FRACTURE] Invalid JSON Geometry generated: {e}")
        return


    # --- PHASE 2: EVALUATION ---
    print("\\n[PHASE 2] Executing DeepSeek Auditing Pass against Generated Traps...", flush=True)
    
    total_score = 0
    max_score = len(tests)

    for i, test in enumerate(tests):
        print(f"\\n[{i+1}/{max_score}] Evaluator Target: {test.get('name', 'Unknown Test')}")
        
        eval_prompt = f"""You are a strict TSFi C Systems Auditor.
CRITICAL RULE: The TSFi architecture STRICTLY uses `select()`. Do not ever suggest `poll()` or `epoll_wait()`.
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
        print(f"-> Model Analysis: '{analysis}'")
        
        passed = False
        expected_kws = test.get('expected_keywords', [])
        for kw in expected_kws:
            if str(kw).lower() in analysis:
                passed = True
                break
                
        if passed:
            print("   [PASS] Expected structural anomaly identified.")
            total_score += 1
            test['score'] = 1
        else:
            print(f"   [FAIL] Reasoning mismatch. (Expected one of: {expected_kws})")

    # --- PHASE 3: SCORING ---
    print("\\n==================================================")
    print("   Autonomous Self-Scoring Matrix (Gen + Eval)    ")
    print("==================================================")
    for t in tests:
        score = t.get('score', 0)
        name = t.get('name', 'Test')[:28]
        print(f" {name:<30} : [{'X' if score else ' '}] ({score}/1)")
    
    print("--------------------------------------------------")
    print(f" TOTAL SYSTEM ACCURACY          : {(total_score/max_score)*100:.1f}%")
    print("==================================================\\n")

if __name__ == "__main__":
    construct_and_score()
