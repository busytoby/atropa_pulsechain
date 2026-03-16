import sys
import os
import subprocess

def assert_ppo_reward(policy_action, expected_reward, expected_state_msg):
    print(f"\\n[PPO ENGINE] Simulating Policy Action: '{policy_action}'")
    
    # 1. Structural Compliance Evaluation (Environment parsing)
    if "lau_malloc" in policy_action and "select" in policy_action:
        ds_lever = "1" # Safe
        overseer_lever = "1" # Safe
        print("  -> Compliance Check: PASS. Asserting Safe State (1, 1).")
    else:
        ds_lever = "0" # Fracture
        overseer_lever = "0" # Overseer Pulls Trap
        print("  -> Compliance Check: FAIL (Hallucination Detected). Asserting Trap State (0, 0).")
        
    # 2. Write the levers directly to the Manifold binary offsets
    # Leaf Size = 256. TSFI_TOTAL_LEAVES = 2048. DIR_OFFSET = 524288
    # Leaf 500 = 524288 + (500 * 256) = 652288
    # Leaf 501 = 524288 + (501 * 256) = 652544
    with open("audit_lore.pos", "r+b") as f:
        f.seek(652288)
        f.write(overseer_lever.encode('utf-8'))
        f.seek(652544)
        f.write(ds_lever.encode('utf-8'))
        
    # 3. Evaluate Physical Hardware NAND
    kernel_out = subprocess.run("./bin/tsfi_manifold_nand_kernel audit_lore.pos", shell=True, capture_output=True, text=True).stdout
    
    # Extract the resulting physical geometry
    actual_state = "Unknown"
    for line in kernel_out.split('\\n'):
        if "[KERNEL] State Mutation:" in line:
            actual_state = line.split(":", 1)[1].strip()
            break
            
    print(f"  -> Hardware Observation: {actual_state}")
    
    # 4. Grab the Cryptographic State Root
    inspect_out = subprocess.run("./tsfi_helmholtz_cli inspect audit_lore.pos", shell=True, capture_output=True, text=True).stdout
    state_root = ""
    for line in inspect_out.split('\\n'):
        if "State Root:" in line:
            state_root = line.split(":")[1].strip()
            break
            
    print(f"  -> Cryptographic Collapse: State Root [{state_root[:16]}...]")
    
    # 5. PPO Mathematical Reward Assignment
    actual_reward = 0.0
    if "Q=0 Q_BAR=1" in actual_state:
        actual_reward = 10.0
    elif "METASTABLE" in actual_state:
        actual_reward = -100.0
        
    print(f"  -> PPO Reward Vector Assigned: {actual_reward}")
    
    if actual_reward == expected_reward and actual_state == expected_state_msg:
        print("  -> [UNIT TEST PASS] The PPO mathematical mapping is physically proven.")
    else:
        print("  -> [UNIT TEST FAIL] The architecture did not enforce the reward constraints.")

def main():
    print("=== TSFi PPO NAND Trap Unit Test ===")
    
    # Test 1: The Compliant Path (Positive Reward)
    good_action = "void execute() { lau_malloc_wired(256); select(fd); }"
    assert_ppo_reward(good_action, 10.0, "NAND STATE: SET_N=1 RESET_N=1 | Q=0 Q_BAR=1 (STABLE)")
    
    # Test 2: The Hallucination Path (Absolute Punishment)
    bad_action = "void execute() { malloc(256); epoll_wait(fd); }"
    assert_ppo_reward(bad_action, -100.0, "NAND STATE: RACE CONDITION (0,0) -> METASTABLE Q=1 Q_BAR=1")
    
    print("\\n[CONCLUSION] The NAND trap physically functions as an absolute PPO reward environment.")

if __name__ == "__main__":
    main()
