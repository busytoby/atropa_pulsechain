import sys
import os
from llama_cpp import Llama

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    with open("src/firmware/HelmholtzHelmholtzFirmware.v", "r") as f:
        code = f.read()

    prompt = f"""You are an expert Verilog engineer. Analyze the following Verilog code for the `HelmholtzHelmholtzFirmware` module.

```verilog
{code}
```

The user wants to optimize the Helmholtz tree reduction and inclusion proof calculations. The current implementation uses simple register loops. Can you provide a conceptual Verilog snippet demonstrating how one might implement a fully pipelined or parallel tree reduction (even at a smaller scale, e.g., an 8-leaf reduction) that would better utilize the 512-bit datapath constraints? Focus ONLY on the architectural concept in Verilog, do not write C++ or gtest.
"""

    if not os.path.exists(gguf_path):
        print(f"[FRACTURE] GGUF file not found: {gguf_path}")
        sys.exit(1)

    try:
        # Load the model with context to handle the file
        llm = Llama(
            model_path=gguf_path,
            n_ctx=4096,
            n_threads=16,
            verbose=False
        )

        formatted_prompt = f"You are a Verilog hardware assistant.\n### Instruction:\n{prompt}\n### Response:\n"

        print("Querying DeepSeek...")
        output = llm(
            formatted_prompt,
            max_tokens=1024,
            stop=["### Instruction:"],
            echo=False
        )

        print("\n=== DeepSeek Optimization Strategy ===")
        print(output['choices'][0]['text'].strip())

    except Exception as e:
        print(f"[FRACTURE] LLM Inference failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
