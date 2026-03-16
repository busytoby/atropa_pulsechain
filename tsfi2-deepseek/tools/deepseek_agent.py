import sys
import os
import subprocess
import re
import time
from llama_cpp import Llama

class Kr0wZAgent:
    def __init__(self):
        self.gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
        print("=== Booting Vulkan DeepSeek Autonomous Agent Core ===", flush=True)
        self.llm = Llama(
            model_path=self.gguf_path,
            n_ctx=16384,
            n_threads=16,
            n_gpu_layers=-1,
            verbose=False
        )
        self.system_prompt = """You are the Kr0wZ Autonomous System Manager. 
You are physically wired into the TSFi Linux architecture.
You have the ability to execute terminal commands to manage the system, compile code, and read files.
To execute a command, you MUST output it in the following strict XML format:
<execute>
command_here
</execute>

For example:
<execute>
ls -la
</execute>

After you issue a command, the system will execute it and return the stdout/stderr to you.
Do not write code outside of commands. Use commands like `cat` to read files, and `sed` or `echo` to write them if needed.
Always run `make all` after modifying C files.
"""
        self.conversation_history = self.system_prompt + "\n\n"

    def execute_tool(self, command):
        print(f"\n[AGENT EXECUTING]: {command}", flush=True)
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=10)
            output = result.stdout + result.stderr
            if not output:
                output = "(Command executed successfully with no output)"
            return output
        except subprocess.TimeoutExpired:
            return "Error: Command timed out after 10 seconds."
        except Exception as e:
            return f"Error: {e}"

    def step(self, user_instruction=None):
        if user_instruction:
            self.conversation_history += f"USER DIRECTIVE:\n{user_instruction}\n\nASSISTANT:\n"
            
        print("-> DeepSeek is reasoning...", flush=True)
        output = self.llm(
            self.conversation_history,
            max_tokens=512,
            stop=["USER DIRECTIVE:", "### Instruction:"],
            echo=False
        )
        
        response = output['choices'][0]['text'].strip()
        print(f"\n[DEEPSEEK]: {response}\n")
        self.conversation_history += response + "\n\n"
        
        # Parse for tool calls
        tool_matches = re.findall(r'<execute>\s*(.*?)\s*</execute>', response, re.DOTALL)
        if tool_matches:
            for cmd in tool_matches:
                cmd_output = self.execute_tool(cmd)
                print(f"[SYSTEM RETURN]:\n{cmd_output[:500]}...\n")
                
                # Feed the result back into the context
                self.conversation_history += f"SYSTEM OUTPUT for `{cmd}`:\n```\n{cmd_output[:2000]}\n```\n\nASSISTANT:\n"
            
            # Recurse automatically to let the agent respond to the command output
            self.step()

def main():
    agent = Kr0wZAgent()
    print("\n-> Kr0wZ Agent Initialized. Sending initial directive...")
    agent.step("Check the current status of the TSFi telemetry cluster by running ./bin/tsfi_cluster_monitor")

if __name__ == "__main__":
    main()
