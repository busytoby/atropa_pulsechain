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

Write a complete, standalone Verilog testbench named `tb_HelmholtzHelmholtzFirmware.v`. 
The testbench should:
1. Instantiate the `HelmholtzHelmholtzFirmware` module.
2. Generate a clock (`clk`) and a `reset` sequence.
3. Simulate a standard epoch transition by setting `directive_strobe` and `directive_cmd = 32'h48454C4D` ('HELM').
4. Provide dummy `react_result_zmm` data when `react_strobe` is high, and set `react_ready` to simulate the sovereign core.
5. Wait for `helmholtz_done` to assert high.
6. Provide ONLY the Verilog code for the testbench. Do not include markdown blocks or any other text.
"""

    if not os.path.exists(gguf_path):
        print(f"[FRACTURE] GGUF file not found: {gguf_path}")
        sys.exit(1)

    try:
        llm = Llama(
            model_path=gguf_path,
            n_ctx=4096,
            n_threads=16,
            verbose=False
        )

        formatted_prompt = f"You are a Verilog hardware assistant.\\n### Instruction:\\n{prompt}\\n### Response:\\n"

        output = llm(
            formatted_prompt,
            max_tokens=1024,
            stop=["### Instruction:"],
            echo=False
        )

        print(output['choices'][0]['text'].strip())

    except Exception as e:
        print(f"[FRACTURE] LLM Inference failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
