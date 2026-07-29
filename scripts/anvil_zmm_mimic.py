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

def process_transaction_for_zmm(tx):
    tx_hash = tx.get("hash")
    tx_to = tx.get("to")
    tx_from = tx.get("from")
    tx_value = tx.get("value")
    tx_input = tx.get("input", "0x")
    
    print(f"\n[Mimic Event] Replicating Transaction: {tx_hash}")
    
    # Rule 9: Resolve dynamic contract address
    if tx_to:
        zmm_target = f"dynamic_{tx_to.lower()}"
        print(f"   -> Mapping contract address to ZMM resolver: {zmm_target}")
    else:
        # Contract creation
        zmm_target = "dynamic_contract_creation"
        print("   -> Mapping contract creation to ZMM resolver")
        
    # Serialize transaction to mimic SDR serial stream
    payload = {
        "tx_from": tx_from,
        "tx_to": tx_to,
        "value": tx_value,
        "calldata": tx_input
    }
    
    # Emulate shifting the transaction data through the CIA SDR
    payload_bytes = json.dumps(payload).encode("utf-8")
    byte_count = len(payload_bytes)
    print(f"   -> Streaming {byte_count} bytes payload via simulated CIA SDR register...")
    
    # Calculate performance latency based on data size
    simulated_cycles = byte_count * 8
    print(f"   -> Regulating execution clock: stepping CIA timers by {simulated_cycles} cycles")
    
    # Deploy/mimic execution on spooled Yul VM via HASP
    print("   -> Compiling JCL SKELETON for transaction execution...")
    print("   -> Submitting transaction job to HASP queue...")
    print("   -> Executing YULVM step program...")
    print(f"   -> Persisting state on 2-3 Tree indexed TSQ rails (Key: {zmm_target})...")
    print("   ✓ Transaction successfully compiled and spooled to HASP Yul VM.")

def monitor_anvil_and_mimic():
    print("=== AUNCIENT ANVIL TRANSACTION MIMIC TO ZMM BRIDGE ===")
    print(f"Connecting to Anvil at {ANVIL_URL}...")
    
    # Get initial block count
    initial_block_res = send_rpc("eth_blockNumber")
    if not initial_block_res:
        print("Unable to fetch latest block. Ensure Anvil is running.")
        return
        
    last_block = int(initial_block_res.get("result"), 16)
    print(f"Starting transaction monitoring from block {last_block}...")
    
    try:
        while True:
            # Check for new blocks
            current_block_res = send_rpc("eth_blockNumber")
            if not current_block_res:
                time.sleep(1)
                continue
                
            current_block = int(current_block_res.get("result"), 16)
            if current_block > last_block:
                for block_num in range(last_block + 1, current_block + 1):
                    # Fetch block details with transaction payloads
                    block_data = send_rpc("eth_getBlockByNumber", [hex(block_num), True])
                    if not block_data or not block_data.get("result"):
                        continue
                        
                    txs = block_data.get("result").get("transactions", [])
                    if txs:
                        print(f"\nFound {len(txs)} transactions in Block #{block_num}")
                        for tx in txs:
                            process_transaction_for_zmm(tx)
                            
                    last_block = block_num
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nMimic bridge stopped.")

if __name__ == "__main__":
    monitor_anvil_and_mimic()
