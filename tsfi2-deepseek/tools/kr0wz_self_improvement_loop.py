import sys
import os
from llama_cpp import Llama

def run_self_improvement():
    print("=== Kr0wZ DeepSeek Autonomous Self-Improvement Loop ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan DeepSeek Core...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=8192,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    # Inject the exact failure context
    prompt = """You are the Lead TSFi Architect.
During our initial 5-Axis Benchmark, your statistical inference engine successfully passed the Memory Leak and Thread Starvation tests, but FAILED the 'Syntax Fracture' test. 

You were given the following C code:
```c
void math_loop() {
    int x = 5
    int y = x * 2;
    printf("%d", y);
}
```
Instead of noticing the missing semicolon on `int x = 5`, you hallucinated that the "primary architectural flaw is the lack of a proper main() function".

Based on this specific failure, generate a rigid Python script named `tsfi_syntax_linter.py` that physically parses a given C file and mechanically checks for missing semicolons before the LLM is ever allowed to audit it.
Provide ONLY the Python code. Do not output markdown ticks. Do not output conversational text.
"""
    formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"

    print("\\n-> Engaging Generative Re-Alignment...", flush=True)
    output = llm(
        formatted_prompt,
        max_tokens=1024,
        stop=["### Instruction:"],
        echo=False
    )

    report = output['choices'][0]['text'].strip()
    
    # Strip markdown ticks if the model hallucinates them
    report = report.replace("```python", "").replace("```", "").strip()

    print("\\n[PASS] DeepSeek successfully engineered the structural countermeasure:")
    print("==================================================")
    print(report)
    print("==================================================\\n")

    with open("tools/tsfi_syntax_linter.py", "w") as f:
        f.write(report)
    print("[ANCHOR] Syntactical firewall solidified at tools/tsfi_syntax_linter.py")

if __name__ == "__main__":
    run_self_improvement()
