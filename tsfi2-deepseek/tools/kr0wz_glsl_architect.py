import os
import re
import subprocess
from llama_cpp import Llama

def retrieve_physical_geometry():
    print("[RAG] Retrieving Absolute Geometry from inc/tsfi_atropa_geometry.h...")
    try:
        with open("inc/tsfi_atropa_geometry.h", "r") as f:
            c_header = f.read()
        match = re.search(r'typedef struct \{[^}]*\}\s*TsfiTeddyEye;', c_header, re.DOTALL)
        struct_def = match.group(0) if match else ""
        match2 = re.search(r'static const TsfiTeddyEye TSFI_BASELINE_EYE[^;]*;', c_header, re.DOTALL)
        baseline_def = match2.group(0) if match2 else ""
        
        match3 = re.search(r'typedef struct \{[^}]*\}\s*TsfiTeddyHead;', c_header, re.DOTALL)
        head_def = match3.group(0) if match3 else ""
        match4 = re.search(r'static const TsfiTeddyHead TSFI_BASELINE_HEAD[^;]*;', c_header, re.DOTALL)
        head_base = match4.group(0) if match4 else ""
        
        match5 = re.search(r'typedef struct \{[^}]*\}\s*TsfiTeddyBody;', c_header, re.DOTALL)
        body_def = match5.group(0) if match5 else ""
        match6 = re.search(r'static const TsfiTeddyBody TSFI_BASELINE_BODY[^;]*;', c_header, re.DOTALL)
        body_base = match6.group(0) if match6 else ""
        
        return f"{struct_def}\\n{baseline_def}\\n{head_def}\\n{head_base}\\n{body_def}\\n{body_base}"
    except Exception as e:
        print(f"[FRACTURE] Retrieval Failed: {e}")
        return None

def compile_glsl_shader(shader_text):
    shader_path = "shaders/tsfi_eye.comp"
    
    if not os.path.exists("shaders"):
        os.makedirs("shaders")
        
    with open(shader_path, "w") as f:
        f.write(shader_text)
        
    print(f"\\n[INFO] Compiling Shader via Vulkan glslc...")
    
    # We compile to SPIR-V in-memory (stdout) to verify syntax without writing .spv to disk
    cmd = ["glslc", shader_path, "-o", "/dev/null"]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            print("[ABSOLUTE SUCCESS] GLSL Shader Compiled Flawlessly.")
            print("-> The mathematics are structurally sound for Vulkan hardware injection.")
        else:
            print("[FRACTURE] GLSL Compilation Failed:")
            print(result.stderr)
    except FileNotFoundError:
        print("[WARN] 'glslc' not found in PATH. Skipping validation. Shader saved to disk.")

def run_glsl_architect():
    print("=== TSFi Autonomous GLSL Architect ===", flush=True)
    
    geometry = retrieve_physical_geometry()
    if not geometry: return
    
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path): 
        print("[FRACTURE] Model not found.")
        return
        
    print("\\n-> Booting DeepSeek-Coder for GPU Shader Synthesis...", flush=True)
    llm = Llama(model_path=gguf_path, n_ctx=2048, n_threads=16, n_gpu_layers=-1, verbose=False)
    
    prompt = f"""You are the Elite TSFi Vulkan Architect.
I retrieved this geometry from the C-codebase:
```c
{geometry}
```

Your objective is to write a purely mathematical Vulkan Compute Shader (`.comp` GLSL) that procedurally renders Phase 1, Phase 2, and Phase 3 (The Eye, The Head, and The Sick Body).
- You MUST start the shader with exactly: `#version 450`
- Use `layout (local_size_x = 16, local_size_y = 16) in;`
- Output to `layout(rgba8, binding = 0) uniform image2D outImage;`
- Implement a basic hash-based noise function `float noise(vec2 p)` to generate the 'sickness'.
- Use the distance function `distance(p, center)` to draw the circles instead of complex SDFs to save space!
- The resolution is 1024x1536. 
- You MUST hardcode the integer values from the C-structs into the GLSL math (e.g. use `250.0`, DO NOT write `TSFI_BASELINE_BODY.body_radius_x` as C-structs do not exist in GLSL).
- Draw the Sick Body (Dark Brown: 0.36, 0.25, 0.20, 1.0) using the hardcoded body_radius_x. If `noise(p)` > 0.8, color it Sick Green (0.33, 0.42, 0.18, 1.0).
- Draw the Cranium, Ears, and Eyes on top. Keep the code extremely short and efficient!
Respond strictly with the raw GLSL code. Do not use markdown ticks. No explanations."""

    output = llm(f"### Instruction:\\n{prompt}\\n### Response:\\n", max_tokens=2048, temperature=0.1, stop=["### Instruction:"], echo=False)
    report = output['choices'][0]['text'].strip()
    
    # Strip markdown if hallucinated
    report = report.replace('```glsl', '')
    report = report.replace('```c', '')
    report = report.replace('```', '')
    report = report.replace('\\n', '\n')
    report = report.strip()
            
    print("\n[ACTIVE] Extracting DeepSeek GLSL Matrix...")
    compile_glsl_shader(report)

if __name__ == "__main__":
    run_glsl_architect()
