import sys
import os
import subprocess
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    target_file = "src/tsfi_soft_encode.c"
    test_file = "tests/test_soft_encode_leak.c"
    
    if not os.path.exists(gguf_path) or not os.path.exists(target_file):
        print("[FRACTURE] Model or target file not found.")
        return

    print(f"=== Booting Vulkan DeepSeek Core for Unit Test Generation ===")
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
The following file (`{target_file}`) was just rewritten to fix a memory leak by using a persistent buffer.
Write a standalone C unit test file that:
1. Includes `tsfi_soft_encode.h` and `lau_memory.h`.
2. Calls `tsfi_soft_encode_frame` multiple times in a loop.
3. Asserts that the memory footprint remains stable (does not leak 1MB per frame).
4. Prints `[PASS] Zero memory leakage verified.` if successful.
Provide the ENTIRE C source code for the test file.

File: {target_file}
```c
{code}
```
"""
    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"

    print("-> DeepSeek is synthesizing the unit test architecture...", flush=True)
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
        with open(test_file, "w") as f:
            f.write(rewritten_code)
        print(f"[ANCHOR] DeepSeek successfully generated {test_file}.")
    else:
        print("[FRACTURE] DeepSeek failed to return a proper C code block.")

if __name__ == "__main__":
    main()
