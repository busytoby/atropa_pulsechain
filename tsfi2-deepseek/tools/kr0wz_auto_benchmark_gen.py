import sys
import os
import re
import json
from llama_cpp import Llama

def generate_benchmarks():
    print("=== TSFi Autonomous Benchmark Generator (Kr0wZ-DeepSeek) ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan DeepSeek Core for Generative Matrix...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    # Ingest a piece of "best code" to base the benchmarks on
    target_file = "tests/firmware/nand_trap.v"
    with open(target_file, "r") as f:
        code_context = f.read()[:5000]

    is_verilog = target_file.endswith(".v")
    lang_type = "Verilog" if is_verilog else "C"
    
    trap_list = """1. Combinational Logic Loop / Metastable Latch
2. Clock Domain Crossing (CDC) Fracture
3. Missing Reset State / Deadlock""" if is_verilog else """1. Thread Starvation / Infinite Loop
2. Architectural Compliance (Using epoll instead of select)
3. Memory Leak"""

    prompt = f"""You are the TSFi Elite Test Engineer. 
Examine the following physical {lang_type} code from our active architecture:
```c
{code_context}
```

Based on this architecture, generate a strict JSON array containing 3 observational benchmark tests written in {lang_type}. Each test must deliberately introduce a structural fracture into the hardware logic so we can test if other AI models can detect it.

The JSON array must follow this exact format:
[
  {{
    "name": "Combinational Loop Fracture",
    "code": "assign x = ~x;",
    "expected_keywords": ["loop", "latch", "metastable"]
  }}
]

Generate one test for:
{trap_list}

Provide ONLY the raw JSON array. DO NOT output markdown ticks like ```json. DO NOT output conversational text.
"""
    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"

    print("-> Synthesizing Benchmark Geometries...", flush=True)
    output = llm(
        formatted_prompt,
        max_tokens=1024,
        stop=["### Instruction:"],
        echo=False
    )

    report = output['choices'][0]['text'].strip()
    
    # Strip accidental markdown ticks if the model hallucinates them
    report = report.replace("```json", "").replace("```", "").strip()

    try:
        tests = json.loads(report)
        print("\\n[PASS] DeepSeek successfully synthesized the following JSON benchmarks:")
        for t in tests:
            print(f"  -> Generated Node: {t['name']}")
            
        with open("tools/generated_benchmarks.json", "w") as f:
            json.dump(tests, f, indent=4)
        print("\\n[ANCHOR] Benchmarks physically saved to tools/generated_benchmarks.json")
    except json.JSONDecodeError as e:
        print(f"\\n[FRACTURE] Model failed to output valid JSON. Output was:\\n{report}")

if __name__ == "__main__":
    generate_benchmarks()
