import sys
import os
from llama_cpp import Llama

def main():
    if len(sys.argv) < 3:
        print("Usage: query_local_deepseek.py <gguf_path> <prompt>")
        sys.exit(1)

    gguf_path = sys.argv[1]
    prompt = sys.argv[2]

    if not os.path.exists(gguf_path):
        print(f"[FRACTURE] GGUF file not found: {gguf_path}")
        sys.exit(1)

    try:
        # Load the model with minimal configuration to stay strictly within CPU/RAM constraints
        llm = Llama(
            model_path=gguf_path,
            n_ctx=4096,
            n_threads=16, # Saturate the physical CPU bounds
            verbose=False
        )

        # DeepSeek format requires true line breaks, not escaped string literals
        formatted_prompt = f"You are a C programming assistant.\n### Instruction:\n{prompt}\n### Response:\n"

        # Generate response
        output = llm(
            formatted_prompt,
            max_tokens=1024,
            stop=["### Instruction:"],
            echo=False
        )

        response_text = output['choices'][0]['text'].strip()
        print(response_text)

    except Exception as e:
        print(f"[FRACTURE] LLM Inference failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
