import sys
import os
import subprocess
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    target_file = "src/tsfi_fused_operator.c"
    
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
During a mass audit, you correctly identified a memory leak in `{target_file}`.
"In the function tsfi_fused_add, memory is allocated for an array of TSFiOperator pointers (fused->ops), but no function is provided to free this memory when the fused operator object is destroyed."

Rewrite the `tsfi_fused_operator_destroy` function (or add it if it is missing) to explicitly call `lau_free(fused->ops)` and then `lau_free(fused)`. 
Provide the ENTIRE rewritten C source code for the file. 

CRITICAL RULE: The TSFi architecture STRICTLY uses `select()`. Do not ever suggest `poll()` or `epoll_wait()`.

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
