import sys
import os
from llama_cpp import Llama

def main():
    if len(sys.argv) < 2:
        print("Usage: daemon_llm.py <gguf_path>")
        sys.exit(1)

    gguf_path = sys.argv[1]
    
    # Increase context window to 8192 for document reviews
    print(f"[DAEMON] Loading physical matrix from {gguf_path} (n_ctx=8192)...")
    try:
        llm = Llama(
            model_path=gguf_path,
            n_ctx=8192,
            n_threads=16,
            verbose=False
        )
    except Exception as e:
        print(f"[FRACTURE] Matrix load failed: {e}")
        sys.exit(1)

    print("[DAEMON] Matrix active and waiting for instructions in /tmp/tsfi_llm_in", flush=True)

    # Use a consistent output path
    output_path = "/tmp/tsfi_llm_out.txt"

    # Create the physical memory boundaries
    os.system("rm -f /tmp/tsfi_llm_in " + output_path)
    os.mkfifo("/tmp/tsfi_llm_in")

    while True:
        try:
            with open("/tmp/tsfi_llm_in", "r") as f_in:
                prompt = f_in.read().strip()
                if not prompt: continue
                print(f"[DAEMON] Received payload (len: {len(prompt)})", flush=True)
                
            if prompt == "EXIT_DAEMON":
                break
                
            # Construct the formatted prompt
            formatted_prompt = f"<|begin_of_sentence|>User: {prompt}\n\nAssistant: "
            
            output = llm(
                formatted_prompt,
                max_tokens=4096,
                stop=["<|end_of_sentence|>", "User:", "<|EOT|>"],
                echo=False
            )
            
            response_text = output['choices'][0]['text'].strip()
            print(f"[DAEMON] Generated response (len: {len(response_text)})", flush=True)
            
            with open(output_path, "w") as f_out:
                f_out.write(response_text)
                f_out.flush()
                    
        except Exception as e:
            print(f"[FRACTURE] Daemon iteration failure: {e}")
            
    os.system("rm -f /tmp/tsfi_llm_in")

if __name__ == "__main__":
    main()
