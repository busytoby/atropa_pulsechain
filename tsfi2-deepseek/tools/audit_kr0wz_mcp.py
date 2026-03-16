import sys
import os
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    files_to_audit = [
        "src/tsfi_mcp_server.c",
        "src/tsfi_zmm_rpc.c",
        "src/tsfi_helmholtz.c",
        "tools/kr0wz_bridge_daemon.py"
    ]

    code_context = ""
    for f_path in files_to_audit:
        if os.path.exists(f_path):
            with open(f_path, "r") as f:
                # Read ~300 lines to fit in strict 16K GPU context window
                lines = f.readlines()[:300]
                code_context += f"--- {f_path} ---\\n{''.join(lines)}\\n\\n"

    prompt = f"""You are the Kr0wZ Architectural Auditor. 
Analyze the following source code topology representing the 'Helmholtz MCP Kr0wZ' integration.
The system uses `tsfi_mcp_server.c` and `tsfi_zmm_rpc.c` to dispatch RPC commands over port 10042.
It uses `tsfi_helmholtz.c` to manage a 522-bit prime acoustic state tree.
It uses `kr0wz_bridge_daemon.py` to stream real-time lore updates back to the user.

Your task:
1. Identify any bottlenecks in the data path between the Helmholtz manifold and the MCP server.
2. Suggest a specific architectural method to eliminate polling (time.sleep) in the Kr0wZ bridge daemon using zero-copy memory triggers or Vulkan ReBAR interrupt events.
3. Provide a strictly analytical output. 
CRITICAL RULE: The TSFi architecture STRICTLY uses `select()`. Do not suggest `poll()` or `epoll_wait()`.
DO NOT write code. DO NOT output markdown blocks.

```c
{code_context}
```
"""

    if not os.path.exists(gguf_path):
        print(f"[FRACTURE] GGUF file not found: {gguf_path}")
        sys.exit(1)

    try:
        llm = Llama(
            model_path=gguf_path,
            n_ctx=16384,
            n_threads=16,
            n_gpu_layers=-1,
            verbose=True
        )

        formatted_prompt = f"You are a strict TSFi systems architect. Provide ONLY your analysis.\\n### Instruction:\\n{prompt}\\n### Response:\\n"

        print("Querying DeepSeek for Helmholtz MCP Kr0wZ Topology Analysis...")
        output = llm(
            formatted_prompt,
            max_tokens=1024,
            stop=["### Instruction:", "```", "/*", "//", "========================================"],
            echo=False
        )

        report = output['choices'][0]['text'].strip()
        print("\\n=== DeepSeek Kr0wZ Audit ===")
        print(report)
        print("============================\\n")

        # Automatically anchor this into the Manifold so the user sees it via the real-time bridge
        import subprocess
        import time
        safe_text = report.replace('"', "'").replace('\\n', ' ')[:512]
        offset = int(time.time()) % 1024
        cmd = f"./tsfi_helmholtz_cli post_lore audit_lore.pos {offset} \"KR0WZ_AUDIT: {safe_text}\""
        subprocess.run(cmd, shell=True, capture_output=True)

    except Exception as e:
        print(f"[FRACTURE] LLM Inference failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
