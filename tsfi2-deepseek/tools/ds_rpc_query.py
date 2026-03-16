import sys
import os
# Add current directory to path to find ds_bridge
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from ds_bridge import DeepSeekBridge

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 ds_rpc_query.py <prompt_file>")
        sys.exit(1)

    prompt_file = sys.argv[1]
    if not os.path.exists(prompt_file):
        print(f"Error: Prompt file {prompt_file} not found.")
        sys.exit(1)

    with open(prompt_file, "r") as f:
        prompt = f.read()

    # Use the persistent Ollama bridge
    bridge = DeepSeekBridge(host='http://127.0.0.1:11435', model='deepseek-coder:6.7b')
    
    # RPC queries are more directive and less analytical than Auditor queries
    system_prompt = "You are the Resident Auditor. You are performing an Acoustic State Validation. Provide a technical report including 'MUTUAL_SIGNATURE_FINAL' if the state is rigid."
    
    response = bridge.query(prompt, system=system_prompt)
    
    # Write response to stdout for C to capture
    print(response)

if __name__ == "__main__":
    main()
