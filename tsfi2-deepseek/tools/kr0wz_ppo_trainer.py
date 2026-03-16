import os
import subprocess
import json
import time
import random
import re
from llama_cpp import Llama

def tsfi_c_compiler_reward_function(generated_c_code, filename="sandbox_test.c"):
    reward = 0.0
    sandbox_path = f"sandbox/{filename}"
    
    # 1. Structural Compliance Check for the Dai Reciprocity Law
    if "Ichidai" in generated_c_code and "Daiichi" in generated_c_code:
        reward += 2.0
    else:
        reward -= 5.0 # Massive penalty for failing to generate the dual values
        
    if "tsfi_bn_cmp_avx512" in generated_c_code:
        reward += 3.0 # Positive reinforcement for using the hardware verification
    else:
        reward -= 5.0 # Penalty for failing to prove reciprocity
        
    if not os.path.exists("sandbox"):
        os.makedirs("sandbox")
    with open(sandbox_path, "w") as f:
        f.write(generated_c_code)
        
    compile_cmd = f"gcc -Wall -Wextra -Iinc -c {sandbox_path} -o sandbox/test.o"
    result = subprocess.run(compile_cmd, shell=True, capture_output=True, text=True)
    
    if result.returncode == 0:
        reward += 5.0
    else:
        reward -= 2.0
        
    try:
        os.remove(sandbox_path)
        os.remove("sandbox/test.o")
    except:
        pass
        
    return reward

def run_infinite_ppo_trajectory_engine():
    print("=== TSFi Autonomous PPO Trajectory Engine ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    trajectory_file = "assets/tsfi_ppo_trajectories.json"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.", flush=True)
        return

    print("[INFO] PyTorch offline. Executing PPO Forward Pass & Reward Mapping natively.", flush=True)
    
    llm = Llama(
        model_path=gguf_path,
        n_ctx=4096,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    epoch = 0
    current_temp = 0.8

    constitution_text = ""
    try:
        with open("assets/TSFI_CONSTITUTION.md", "r") as f:
            constitution_text = f.read()
    except Exception:
        pass

    while True:
        epoch += 1
        print(f"\n--- PPO EPOCH {epoch} ---", flush=True)

        prompt = f"""Read the following TSFi Constitution:\n{constitution_text}\n\nBased STRICTLY on the Reciprocity Law defined in Section 6, write a C function that generates a `Dai` and its intermediary shadow proving object. You must mathematically assert their reciprocal state using `tsfi_bn_cmp_avx512`. Do not output conversational text. Output ONLY raw C code."""
        formatted_prompt = f"### Instruction:\n{prompt}\n### Response:\n"
        
        # A. Generation (Policy Action)
        output = llm(
            formatted_prompt,
            max_tokens=256,
            temperature=current_temp,
            stop=["### Instruction:"],
            echo=False
        )
        
        generated_code = output['choices'][0]['text'].strip()
        
        # Strip markdown for compilation
        code_match = re.search(r'```c(.*?)```', generated_code, re.DOTALL)
        if not code_match:
            code_match = re.search(r'```(.*?)```', generated_code, re.DOTALL)
            
        if code_match:
            clean_code = code_match.group(1).strip()
        else:
            clean_code = generated_code.replace("```c", "").replace("```", "").strip()
            
        # B. Environment Scoring (The C Compiler Reward)
        reward = tsfi_c_compiler_reward_function(clean_code)
        
        print(f"-> Policy Action Generated (Len: {len(clean_code)} bytes)")
        print(f"-> Environment Reward: {reward}")
        
        # C. Simulated Weight Update (Temperature Adjustment)
        if reward > 0:
            current_temp = max(0.1, current_temp - 0.1) # Lock in successful syntax
            print("-> [PPO UPDATE] Positive Reward. Policy tightening.")
        else:
            current_temp = min(1.5, current_temp + 0.1) # Increase entropy to escape local minimum
            print("-> [PPO UPDATE] Negative Reward. Policy expanding entropy to find solution.")
            
        # D. Store the Trajectory for the Remote PyTorch Cluster
        trajectory_node = {
            "epoch": epoch,
            "prompt": prompt,
            "action_code": clean_code,
            "reward": reward,
            "temperature": current_temp
        }
        
        dataset = []
        if os.path.exists(trajectory_file):
            try:
                with open(trajectory_file, "r") as f:
                    dataset = json.load(f)
            except: pass
            
        dataset.append(trajectory_node)
        with open(trajectory_file, "w") as f:
            json.dump(dataset, f, indent=4)

if __name__ == "__main__":
    run_infinite_ppo_trajectory_engine()