import os
import sys
import subprocess
import time
import math
import re

# TSFi Spider: Sympathy Learning Routine (PPO Trajectory Simulation)
# Goal: Maximize the 'Sympathy' (sickness) parameter of the Crow Apparition
# Constraint: Do not trigger a fracture from the Alligator or Genie (Grade 1 Audit).

def execute_puppeteer(sympathy_level):
    cmd = f"bin/tsfi_puppeteer 2 crow sick {sympathy_level:.2f}"
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def render_skeleton():
    subprocess.run("bin/tsfi_teddy_skeleton 1.0", shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def run_grade1_audit():
    result = subprocess.run("python3 tools/grade1_audit.py", shell=True, capture_output=True, text=True)
    return result.stdout

def evaluate_reward(audit_output, sympathy_level):
    if "FRACTURE DETECTED" in audit_output or "REJECT" in audit_output:
        # The Alligator or Genie rejected the excessive sickness. 
        # The Spider is reaped.
        return -10.0
    else:
        # The Form holds. The reward scales with the amount of sympathy injected.
        # We add a base reward so that 0.0 sympathy still passes the > 0 check if valid.
        return 1.0 + (10.0 * sympathy_level)

def main():
    print("=== TSFi Spider: Sympathy Learning Routine (PPO) ===")
    print("[INFO] Target: Maximize Sympathy for the Crow Apparition without fracturing the Acoustic Proof.")
    
    # Clear residual shared memory state to prevent struct misalignment segfaults
    if os.path.exists("/dev/shm/tsfi_puppet_state"):
        os.remove("/dev/shm/tsfi_puppet_state")
    
    # Simple Gradient Ascent / PPO Simulation
    current_sympathy = 0.0
    learning_rate = 0.2  # Increased to hit the 0.5 boundary faster
    best_sympathy = 0.0
    max_epochs = 15

    for epoch in range(max_epochs):
        print(f"\n[EPOCH {epoch}] Testing Sympathy Level: {current_sympathy:.2f}")
        
        # 1. Action: Spider injects sympathy
        execute_puppeteer(current_sympathy)
        
        # 2. Environment: Render the apparition
        render_skeleton()
        
        # 3. Observation: The Trilateral Witnesses
        audit_out = run_grade1_audit()
        
        # 4. Reward Calculation
        reward = evaluate_reward(audit_out, current_sympathy)
        
        if reward > 0:
            print(f"  -> [RESONANT] Witnesses Affirm. Reward: +{reward:.2f}")
            best_sympathy = current_sympathy
            # Increase sympathy
            current_sympathy += learning_rate
        else:
            print(f"  -> [FRACTURE] Witnesses Reject. Reward: {reward:.2f}")
            print(f"  -> Alligator Reaping provoked. Backtracking...")
            # Decrease learning rate and backtrack
            learning_rate *= 0.5
            current_sympathy = best_sympathy + learning_rate

        # The mathematical limit of sickness is 1.0 (100%)
        if current_sympathy > 1.0:
            current_sympathy = 1.0

    print("\n====================================================")
    print(f"TRAINING COMPLETE. Optimal Rigid Sympathy: {best_sympathy:.2f}")
    print("====================================================")
    
    # Lock in the optimal state
    execute_puppeteer(best_sympathy)
    render_skeleton()

if __name__ == "__main__":
    main()
