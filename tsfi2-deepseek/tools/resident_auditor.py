import sys
import os
import time
import re
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from llama_cpp import Llama

class DeepSeekAuditor(FileSystemEventHandler):
    def __init__(self, model_path):
        print(f"-> Initializing Resident Auditor with {model_path}...", flush=True)
        self.llm = Llama(
            model_path=model_path,
            n_ctx=32768,
            n_threads=16,
            n_gpu_layers=-1,
            verbose=False
        )
        print("-> Auditor is active. Monitoring src/, tests/, *.c, and *.v for changes.", flush=True)

    def on_modified(self, event):
        if event.is_directory:
            return
        
        file_path = event.src_path
        
        # Check extensions
        if not (file_path.endswith(".c") or file_path.endswith(".v")):
            return
            
        # Check directories
        if "src/" not in file_path and "tests/" not in file_path:
            return
        
        print(f"\n[AUDIT] Detected change in {file_path}. Processing...", flush=True)
        time.sleep(1)
        
        try:
            with open(file_path, "r") as f:
                code = f.read()
        except Exception as e:
            print(f"[FRACTURE] Failed to read {file_path}: {e}", flush=True)
            return

        prompt = f"""User: You are the Resident Auditor. 
Analyze the following source code for 'internal development' and 'bijective rigidity'.
Identify 'trigonometric measurement fallacies' or external dependencies that should be internalized.
CRITICAL RULE: The TSFi architecture STRICTLY uses `select()`. Do not ever suggest `poll()` or `epoll_wait()`.
Provide ONLY a concise list of issues and observations.
CRITICAL: DO NOT WRITE CODE. DO NOT WRITE COMMENTS. DO NOT output C++ or Verilog. Provide a strict text-only analysis and then immediately terminate your output.

Source Code from {file_path}:
```
{code}
```
Assistant: """

        print(f"-> Querying DeepSeek for {file_path}...", flush=True)
        try:
            output = self.llm(
                prompt,
                max_tokens=2048,
                stop=["User:", "<|EOT|>"],
                echo=False
            )
            print(f"\n=== Audit Report: {file_path} ===\n", flush=True)
            report = output['choices'][0]['text']
            print(report, flush=True)
            with open("tools/AUDIT_FEED.md", "w") as af:
                af.write(f"=== Audit Report: {file_path} ===\n{report}\n")
            
            # Anchor the audit into the Manifold so the TSFi Manifold physically retains the DeepSeek assertion.
            import subprocess, time
            safe_text = report.replace('"', "'").replace('\n', ' ')[:512] # Truncate to first 512 bytes for structural fit
            offset = int(time.time()) % 1024
            cmd = f"./tsfi_helmholtz_cli post_lore audit_lore.pos {offset} \"AUDIT [{file_path}]: {safe_text}\""
            subprocess.run(cmd, shell=True, capture_output=True)
            print("\n" + "="*40 + "\n", flush=True)
            
            # --- Neural Persistence: Anchor report in Manifold ---
            summary = report[:512].replace('"', "'").replace('\n', ' ')
            manifold_path = "audit_lore.pos"
            # We use a leaf offset derived from the filename to prevent collision
            leaf_offset = sum(ord(c) for c in file_path) % 1024
            cmd = f"./tsfi_helmholtz_cli post_lore {manifold_path} {leaf_offset} \"AUDIT: {file_path} - {summary}\""
            os.system(cmd)
            print(f"[ANCHOR] Audit results solidified in {manifold_path} (Leaf {leaf_offset}).", flush=True)
            
        except Exception as e:
            print(f"[FRACTURE] Audit failed: {e}", flush=True)

def main():
    model_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(model_path):
        print(f"Error: Model {model_path} not found.")
        sys.exit(1)

    path = "."
    event_handler = DeepSeekAuditor(model_path)
    observer = Observer()
    observer.schedule(event_handler, path, recursive=True)
    observer.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

if __name__ == "__main__":
    main()
