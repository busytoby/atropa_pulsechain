import socket
import threading
import queue
import time
import os
from llama_cpp import Llama

# Thread-safe prompt queue
prompt_queue = queue.Queue()

def llm_worker(llm):
    while True:
        try:
            # Block until a prompt arrives
            prompt_text = prompt_queue.get()
            
            print(f"\\n[Kr0wZ INGEST] Processing: {prompt_text[:50]}...", flush=True)

            formatted_prompt = f"You are the TSFi Kr0wZ Broadcaster. Respond to the following real-time data stream with an architectural judgment for the YouTube audience. Limit to 3 sentences.\\n### Context:\\n{prompt_text}\\n### Response:\\n"

            output = llm(
                formatted_prompt,
                max_tokens=256,
                stop=["### Instruction:", "```", "/*", "//"],
                echo=False
            )

            report = output['choices'][0]['text'].strip()
            print(f"-> Generated Broadcast: {report[:50]}...", flush=True)
            
            # Pipe to VTUBE physical feed
            with open("tools/AUDIT_FEED.md", "w") as af:
                af.write(f"=== DEEPSEEK LIVE STREAM ===\\n{report}\\n")
                
            prompt_queue.task_done()
        except Exception as e:
            print(f"[FRACTURE] LLM Worker Error: {e}", flush=True)

def socket_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # Bind to physical ingestion port
    server.bind(('127.0.0.1', 10043))
    server.listen(5)
    print("-> Kr0wZ Real-Time Ingestion Socket Active on TCP 10043.", flush=True)

    while True:
        client, addr = server.accept()
        data = client.recv(4096)
        if data:
            decoded = data.decode('utf-8').strip()
            if decoded:
                prompt_queue.put(decoded)
        client.close()

def main():
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.")
        return

    print("=== Booting Vulkan DeepSeek VTUBE Ingestion Engine ===", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=8192,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    # Start the LLM processing thread
    worker_thread = threading.Thread(target=llm_worker, args=(llm,), daemon=True)
    worker_thread.start()

    # Run the TCP ingestion server on the main thread
    socket_server()

if __name__ == "__main__":
    main()
