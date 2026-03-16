import sys
import os
import subprocess
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.")
        return

    print("=== Booting Vulkan DeepSeek Core for Managerial Architecture ===")
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    with open("src/tsfi_cluster_monitor.c", "r") as f:
        code = f.read()

    prompt = f"""You are an elite C Systems Architect.
The following C file (`tsfi_cluster_monitor.c`) successfully lists all active TSFi Telemetry IDs.
The user wants to upgrade this passive monitor into an active Managerial System that can send commands (like `PAUSE_VTUBE`, `RESUME_VTUBE`, `SHUTDOWN`) directly to the running `vtube` processes using the `telem->request_cmd` string buffer located in the `LauTelemetryState` shared memory.

Write a new C function named `cluster_send_command` that takes two arguments: `const char *instance_id` and `const char *command`. It should map the shared memory for that ID, safely `strncpy` the command into `telem->request_cmd`, and then `munmap`.
Provide ONLY the C code for the function.

File: tsfi_cluster_monitor.c
```c
{code}
```
"""
    formatted_prompt = f"### Instruction:\n{prompt}\n### Response:\n"

    print("-> DeepSeek is synthesizing the managerial architecture...", flush=True)
    output = llm(
        formatted_prompt,
        max_tokens=1024,
        stop=["### Instruction:"],
        echo=False
    )

    report = output['choices'][0]['text'].strip()
    
    import re
    code_match = re.search(r'```c(.*?)```', report, re.DOTALL)
    if code_match:
        rewritten_code = code_match.group(1).strip()
        print(f"\n[DEEPSEEK MANAGERIAL ARCHITECTURE]\n{rewritten_code}\n")
    else:
        print("[FRACTURE] DeepSeek failed to return a proper C code block.")
        print(report)

if __name__ == "__main__":
    main()
