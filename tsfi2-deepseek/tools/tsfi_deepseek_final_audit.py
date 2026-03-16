import os
from llama_cpp import Llama

def run_final_deepseek_audit():
    print("=== TSFi DeepSeek-Coder Final Architectural Audit ===", flush=True)
    
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model missing.")
        return
        
    print("[ACTIVE] Ingesting Full VLM/Painter Dataset and Reference DNA...")
    try:
        with open("teddy_bear_visualization.md", "r") as f:
            vlm_dataset = f.read()
        with open("inc/tsfi_atropa_geometry.h", "r") as f:
            reference_dna = f.read()
    except Exception as e:
        print(f"[FRACTURE] Failed to load documentation: {e}")
        return
        
    print("\\n-> Booting DeepSeek-Coder 6.7B Neural Matrix...", flush=True)
    # High context window required to ingest the entire documentation set
    llm = Llama(model_path=gguf_path, n_ctx=8192, n_threads=16, n_gpu_layers=-1, verbose=False)
    
    prompt = f"""You are the TSFi Lead Architect. 
I have compiled the final structural dataset from the Vision-Language Model (Moondream) and the Vulkan Painter (Stable Diffusion). 
Here is the complete architectural mapping:

DATASET:
{vlm_dataset}

REFERENCE C-STRUCT DNA:
```c
{reference_dna}
```

TASK:
Review the complete dataset and the reference DNA. 
1. Do you agree with this mathematical architecture? 
2. Are there any physical contradictions between the VLM's findings and the C-structs?
3. Provide exactly ONE specific, highly technical C-code or mathematical improvement to the `tsfi_atropa_geometry.h` file to better align it with the deep volumetric constraints (e.g. the Gaussian falloff for the stomach or the bifurcated shadow).

Respond clearly and architecturally."""

    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
    
    print("\\n[ACTIVE] DeepSeek evaluating Trilateral coherence...", flush=True)
    output = llm(formatted_prompt, max_tokens=512, temperature=0.2, stop=["### Instruction:"], echo=False)
    response = output['choices'][0]['text'].strip()
    
    print("\\n=== DEEPSEEK ARCHITECTURAL RULING ===")
    print(response)
    print("=======================================")

if __name__ == "__main__":
    run_final_deepseek_audit()
