import sys
import os
import subprocess
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    target_file = "src/tsfi_soft_encode.c"
    
    if not os.path.exists(gguf_path) or not os.path.exists(target_file):
        print("[FRACTURE] Model or target file not found.")
        return

    print(f"=== Booting Vulkan DeepSeek Core for Architectural Rewrite of {target_file} ===")
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    with open(target_file, "r") as f:
        code = f.read()

    prompt = f"""You are an elite C Systems Architect.
The following file (`{target_file}`) has a critical memory leak: `tsfi_soft_encode_frame` allocates 1MB on every call but never frees it.
Rewrite the `tsfi_soft_encode_frame` function to use a single, static, or externally persistent zero-copy buffer allocated via `lau_malloc` (defined in `lau_memory.h`) rather than `malloc`.
Provide the ENTIRE rewritten C source code for the file. 

File: {target_file}
```c
{code}
```
"""
    formatted_prompt = f"### Instruction:\n{prompt}\n### Response:\n"

    print("-> DeepSeek is synthesizing the rewritten architecture...", flush=True)
    output = llm(
        formatted_prompt,
        max_tokens=2048,
        stop=["### Instruction:"],
        echo=False
    )

    report = output['choices'][0]['text'].strip()
    
    # Extract the code block
    import re
    code_match = re.search(r'```c(.*?)```', report, re.DOTALL)
    if code_match:
        rewritten_code = code_match.group(1).strip()
        with open(target_file, "w") as f:
            f.write(rewritten_code)
        print(f"[ANCHOR] DeepSeek successfully rewrote {target_file}.")
    else:
        print("[FRACTURE] DeepSeek failed to return a proper C code block.")

if __name__ == "__main__":
    main()
