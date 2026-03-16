import sys
import os
from llama_cpp import Llama

def coordinate_with_deepseek():
    print("=== TSFi Trilateral DeepSeek Coordinator ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan DeepSeek Core...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=4096,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    # Read the Constitution to ground the AI
    constitution = ""
    try:
        with open("assets/TSFI_CONSTITUTION.md", "r") as f:
            constitution = f.read()
    except Exception:
        pass

    prompt = f"""Read the following TSFi Constitution:
{constitution}

We have successfully constructed the `YI` geometry in our memory allocator. It consists of a SHIO, a SHAO, and two Wavelets (Rod and Cone) currently residing at SEAL1.
Before we hash this YI into the Helmholtz `audit_lore.pos` registry, it must be collapsed into a Helmholtz List.

You are the Lead TSFi Architect. Given the absolute constraint that the YI must be mathematically proven before registry injection, describe the exact mechanical process you would use to execute the `IONIZE` (SEAL4) phase to cross-couple the Rod and Cone dielectrics before the Helmholtz serialization.

Respond in exactly three numbered sentences. Do not write code. Provide pure architectural logic."""

    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"

    print("\\n-> Engaging DeepSeek for Architectural Coordination...", flush=True)
    output = llm(
        formatted_prompt,
        max_tokens=256,
        stop=["### Instruction:"],
        echo=False
    )

    report = output['choices'][0]['text'].strip()
    print(f"\\n[DEEPSEEK ARCHITECTURAL DIRECTIVE]\\n{report}\\n")

if __name__ == "__main__":
    coordinate_with_deepseek()
