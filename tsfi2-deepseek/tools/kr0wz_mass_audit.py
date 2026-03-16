import sys
import os
import glob
import time
import subprocess
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.")
        return

    # Initialize Vulkan-Accelerated DeepSeek once
    print("=== Booting Vulkan DeepSeek Core for Mass Audit ===")
    llm = Llama(
        model_path=gguf_path,
        n_ctx=16384,  # 16K fits safely in Vulkan VRAM
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    # Gather targets
    src_files = glob.glob("src/*.c") + glob.glob("src/firmware/*.v") + glob.glob("src/firmware/*.c")
    print(f"-> Discovered {len(src_files)} structural targets.")

    for i, f_path in enumerate(src_files):
        print(f"\\n[{i+1}/{len(src_files)}] Auditing {f_path}...")
        
        with open(f_path, "r") as f:
            code = f.read()
            
        prompt = f"""You are a strict C/Verilog code auditor. Analyze the following file for architectural bottlenecks and memory leakage.
CRITICAL: The TSFi Architecture STRICTLY standardizes on select(). NEVER suggest poll() or epoll().
DO NOT WRITE CODE. DO NOT WRITE COMMENTS. DO NOT output C++ or Verilog. Provide a strict text-only analysis of no more than 3 sentences.

File: {f_path}
```
{code[:16000]}
```
"""
        formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"

        output = llm(
            formatted_prompt,
            max_tokens=512,
            stop=["### Instruction:", "```", "/*", "//"],
            echo=False
        )

        report = output['choices'][0]['text'].strip()
        print(f"-> {report}")
        
        # Write to physical feed so MCP server can read it
        with open("tools/AUDIT_FEED.md", "w") as af:
            af.write(f"=== DEEPSEEK KR0WZ MASS AUDIT [{f_path}] ===\n{report}\n")
        
        # Anchor findings
        safe_text = report.replace('"', "'").replace('\\n', ' ')[:512]
        offset = int(time.time()) % 1024
        subprocess.run(f"./tsfi_helmholtz_cli post_lore audit_lore.pos {offset} \"MASS_AUDIT [{f_path}]: {safe_text}\"", shell=True, capture_output=True)

if __name__ == "__main__":
    main()
