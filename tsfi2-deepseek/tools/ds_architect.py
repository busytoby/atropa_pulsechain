import sys
import os
from llama_cpp import Llama

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 ds_architect.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    if not os.path.exists(file_path):
        print(f"Error: File {file_path} not found.")
        sys.exit(1)

    with open(file_path, "r") as f:
        code = f.read()

    model_path = "assets/DeepSeek-Coder-V2-Lite-Instruct.gguf"
    print(f"-> Loading Resident Architect from {model_path}...")
    
    try:
        llm = Llama(
            model_path=model_path,
            n_ctx=16384,
            n_threads=16,
            verbose=False
        )
    except Exception as e:
        print(f"[FRACTURE] Architect load failed: {e}")
        sys.exit(1)

    prompt = f"""User: Analyze the following C source code. 
Identify any 'trigonometric measurement fallacies' where external observation is used instead of internal code rigidity. 
Suggest refactors to use 'internal development' via AVX-512, Trilateral Wavefield Accumulation, or direct ZMM manipulation.

Source Code:
```c
{code}
```
Assistant: """

    print(f"-> Analyzing {file_path}...")
    try:
        output = llm(
            prompt,
            max_tokens=2048,
            stop=["User:", "<|EOT|>"],
            echo=False
        )
        print("\n=== Architectural Analysis ===\n")
        print(output['choices'][0]['text'])
    except Exception as e:
        print(f"[FRACTURE] Analysis failed: {e}")

if __name__ == "__main__":
    main()
