import sys
import os
import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from llama_cpp import Llama

class VTubeDeepSeekWorker(FileSystemEventHandler):
    def __init__(self):
        gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
        print("=== Booting Vulkan DeepSeek VTUBE Core ===", flush=True)
        self.llm = Llama(
            model_path=gguf_path,
            n_ctx=8192,
            n_threads=16,
            n_gpu_layers=-1,
            verbose=False
        )
        print("-> VTUBE DeepSeek worker active. Awaiting Gemini CLI trigger...", flush=True)

    def on_modified(self, event):
        if "VTUBE_TRIGGER.md" in event.src_path:
            # Short sleep to ensure the MCP file write completes
            time.sleep(0.1)
            
            with open("tools/VTUBE_TRIGGER.md", "r") as f:
                prompt_text = f.read().strip()
                
            if not prompt_text: return
                
            print(f"-> Received Gemini Trigger: {prompt_text[:50]}...", flush=True)

            formatted_prompt = f"You are the TSFi Kr0wZ Broadcaster. The following context was just passed to you from the Gemini CLI operator. Respond with an architectural judgment or technical commentary for the YouTube audience. Limit to 3 sentences.\\n### Context:\\n{prompt_text}\\n### Response:\\n"

            output = self.llm(
                formatted_prompt,
                max_tokens=256,
                stop=["### Instruction:", "```", "/*", "//"],
                echo=False
            )

            report = output['choices'][0]['text'].strip()
            print(f"-> Generated Broadcast: {report[:50]}...", flush=True)
            
            # Pipe output to the physical feed that the C Broadcaster is tailing
            with open("tools/AUDIT_FEED.md", "w") as af:
                af.write(f"=== DEEPSEEK LIVE COMMENTARY ===\\n{report}\\n")
                
            # Clear trigger to prevent loops
            open("tools/VTUBE_TRIGGER.md", "w").close()

def main():
    # Ensure empty trigger file exists
    open("tools/VTUBE_TRIGGER.md", "w").close()
    
    event_handler = VTubeDeepSeekWorker()
    observer = Observer()
    observer.schedule(event_handler, path='tools/', recursive=False)
    observer.start()
    
    try:
        while True:
            time.sleep(86400)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

if __name__ == "__main__":
    main()
