import urllib.request
import json
import time

ANVIL_URL = "http://localhost:8545"

def send_rpc(method, params=[]):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    headers = {"Content-Type": "application/json"}
    req = urllib.request.Request(
        ANVIL_URL, 
        data=json.dumps(payload).encode("utf-8"), 
        headers=headers,
        method="POST"
    )
    try:
        with urllib.request.urlopen(req) as response:
            return json.loads(response.read().decode("utf-8"))
    except Exception as e:
        print(f"RPC Connection Error: {e}")
        return None

def sync_anvil_with_cia(target_latch_seconds):
    print("=== AUNCIENT CIA TIMER TO ANVIL BLOCKCHAIN BRIDGE ===")
    print(f"Connecting to local Anvil node at {ANVIL_URL}...")
    
    # Check node connection
    client_version = send_rpc("web3_clientVersion")
    if not client_version:
        print("Anvil node is offline or unreachable. Please ensure Anvil is running.")
        return
        
    print(f"Connected successfully: {client_version.get('result')}")
    
    # Configure Anvil timestamp interval to match our CIA target latch
    print(f"Setting Anvil block timestamp interval to {target_latch_seconds} seconds...")
    send_rpc("anvil_setBlockTimestampInterval", [target_latch_seconds])
    
    # Start simulating CIA Timer A underflow cycles
    print("Simulating CIA Timer underflow loop. Mining Anvil blocks on every underflow...")
    cycle = 0
    try:
        while True:
            cycle += 1
            print(f"[Timer Underflow] Triggering block mine event for cycle {cycle}...")
            
            # Simulate Timer A underflow trigger to mine a block
            res = send_rpc("evm_mine")
            if res and "error" not in res:
                print(f"   ✓ Block mined successfully in Anvil.")
            else:
                print(f"   ✗ Mine transaction failed: {res}")
                
            # Wait for target latch duration
            time.sleep(target_latch_seconds)
    except KeyboardInterrupt:
        print("\nBridge stopped by user.")

if __name__ == "__main__":
    # Simulate a target block speed of 2 seconds per block regulated by CIA timers
    sync_anvil_with_cia(2)
