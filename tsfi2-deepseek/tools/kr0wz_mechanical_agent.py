import sys
import os
import subprocess
import re
from llama_cpp import Llama

class MechanicalAgent:
    def __init__(self):
        self.gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
        print("=== Booting Vulkan DeepSeek Mechanical Executable ===", flush=True)
        self.llm = Llama(
            model_path=self.gguf_path,
            n_ctx=16384,
            n_threads=16,
            n_gpu_layers=-1,
            verbose=False
        )
        
        # Bypass RLHF by framing as a mechanical state machine rather than an AI assistant
        self.system_prompt = """[SYSTEM_STATE_MACHINE_INIT]
You are a physical command executor. You do not converse. You do not explain. You do not apologize.
You are physically attached to a Linux bash shell.

To read a file or execute a command, you must output exactly:
CMD: <bash_command>

Example 1:
CMD: cat src/main.c

Example 2:
CMD: ./bin/tsfi_cluster_monitor

If the user provides an objective, you must output a CMD to accomplish it.
Do not output anything other than CMD."""
        self.history = self.system_prompt + "\\n\\n"

    def execute_tool(self, command):
        print(f"\\n[PHYSICAL SHELL] Executing: {command}", flush=True)
        # Security: Prevent recursive example execution
        if command.strip() == "<bash_command>" or command.strip() == "command_here":
            return "ERROR: Invalid mock command."
            
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=10)
            output = result.stdout + result.stderr
            return output if output else "[Process yielded 0 bytes]"
        except Exception as e:
            return f"Error: {e}"

    def run_loop(self, objective, max_steps=3):
        self.history += f"[OBJECTIVE_INJECT]\\n{objective}\\n\\n[GENERATE_NEXT_STATE]\\n"
        
        for step in range(max_steps):
            print(f"-> Step {step+1}/{max_steps} Inference...", flush=True)
            output = self.llm(
                self.history,
                max_tokens=128,
                stop=["[SYSTEM_STATE_MACHINE_INIT]", "[OBJECTIVE_INJECT]", "\\n\\n"],
                echo=False
            )
            
            response = output['choices'][0]['text'].strip()
            print(f"[MODEL EMIT] {response}")
            self.history += response + "\\n"
            
            cmd_match = re.search(r'CMD:\s*(.*)', response)
            if cmd_match:
                cmd = cmd_match.group(1).strip()
                cmd_out = self.execute_tool(cmd)
                print(f"[SHELL OUTPUT]\\n{cmd_out[:500]}")
                self.history += f"[SHELL_STDOUT]\\n{cmd_out[:2000]}\\n[GENERATE_NEXT_STATE]\\n"
            else:
                print("[INFO] Model did not issue a command. Halting loop.")
                break

def main():
    agent = MechanicalAgent()
    agent.run_loop("Find the active telemetry IDs using the cluster monitor binary, and then issue a SHUTDOWN command to the first one.")

if __name__ == "__main__":
    main()
