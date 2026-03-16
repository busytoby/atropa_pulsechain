import os
import re
import json
from llama_cpp import Llama

def extract_c_struct(filepath, struct_name):
    print(f"[RAG] Scanning {filepath} for {struct_name}...", flush=True)
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
        # Regex to find typedef struct { ... } StructName;
        pattern = r'typedef\s+struct\s*\{[^}]*\}\s*' + struct_name + r'\s*;'
        match = re.search(pattern, content, re.DOTALL)
        if match:
            print(f"  -> [PASS] Physical geometry located.")
            return match.group(0)
        else:
            print(f"  -> [FAIL] Geometry missing from file.")
            return None
    except Exception as e:
        print(f"  -> [FRACTURE] File I/O failed: {e}")
        return None

def run_rag_pipeline():
    print("=== TSFi Trilateral RAG Engine ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.", flush=True)
        return

    # 1. RAG Extraction
    print("\\n[RAG] Scanning inc/tsfi_atropa_geometry.h for physical geometry bounds...", flush=True)
    try:
        with open("inc/tsfi_atropa_geometry.h", 'r') as f:
            struct_code = f.read()
    except Exception as e:
        print(f"[FRACTURE] Failed to retrieve geometry: {e}")
        return
        
    print("\\n-> Booting Vulkan DeepSeek Core for Semantic Translation...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=4096,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    # 2. Neural Blueprint Generation based on Perceived Reality
    prompt = f"""You are the autonomous Art Director for the Atropa/999 Sovereign Choreography.
We are physically assembling the geometry one part at a time. Phase 1 is the Eye.
I have extracted the following physical geometric constants from our codebase:
```c
{struct_code}
```

Your task is to design a strict geometric JSON blueprint to visualize this structurally retrieved "Eye" on a 1024x1536 resolution screen.
- You must perfectly scale the retrievable C-constants (`origin_x`, `origin_y`, `pupil_radius`, `iris_radius`) into the JSON dimensions.
- Create two overlapping ovals: one for the Iris (using the `default_color` 0x00FF00) and one for the Pupil (Black or dark color).
Generate a JSON array of PM4 geometry. 
Example format:
[
  {{"object": "iris", "shape": "oval", "x": 467, "y": 355, "w": 90, "h": 90, "color": "0xFF00FF00"}},
  {{"object": "pupil", "shape": "oval", "x": 492, "y": 380, "w": 40, "h": 40, "color": "0xFF000000"}}
]
Respond ONLY with the raw JSON array. Do not use markdown ticks. No conversational text."""

    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
    
    print("\\n[RAG] Injecting Extracted Code into Neural Context...", flush=True)
    output = llm(formatted_prompt, max_tokens=256, temperature=0.2, stop=["### Instruction:"], echo=False)
    report = output['choices'][0]['text'].strip()
    
    # Extract JSON
    code_match = re.search(r'\[(.*?)\]', report.replace('\\n', ' '), re.DOTALL)
    if not code_match:
        # Fallback if the AI hallucinated brackets differently or got cut off
        code_match = re.search(r'\[(.*)', report.replace('\\n', ' '), re.DOTALL)
        if not code_match:
            print("[FRACTURE] DeepSeek failed to compile JSON blueprint.")
            print("Raw output:", report)
            return
            
    json_str = "[" + code_match.group(1)
    if not json_str.endswith(']'):
        json_str += "]"
        
    try:
        # Validate the JSON before saving
        json.loads(json_str)
    except:
        # If it cut off mid-string, try to aggressively cap it
        last_brace = json_str.rfind('}')
        if last_brace != -1:
            json_str = json_str[:last_brace+1] + "]"
        else:
            print("[FRACTURE] Corrupted JSON matrix.")
            return
    
    # 3. Store the Blueprint
    blueprint_path = "assets/rag_fa_blueprint.json"
    if not os.path.exists("assets"):
        os.makedirs("assets")
        
    with open(blueprint_path, "w") as f:
        f.write(json_str)
        
    print(f"\\n[ABSOLUTE SUCCESS] DeepSeek successfully mapped the C-code into a Visual Blueprint.")
    print(f"-> Blueprint anchored at {blueprint_path}")

if __name__ == "__main__":
    run_rag_pipeline()
