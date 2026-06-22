import json
import os
from web3 import Web3
from concurrent.futures import ThreadPoolExecutor

RPC_URL = "https://rpc.pulsechain.com"
MAP_ADDRESS = "0xD3a7A95012Edd46Ea115c693B74c5e524b3DdA75"

def process_single(w3, erc20_abi, integrative, qing, existing):
    integrative_lower = integrative.lower()
    if integrative_lower in existing:
        entry = existing[integrative_lower]
        if not entry.get("exists") or entry.get("qing") != qing:
            entry["exists"] = True
            entry["qing"] = qing
            return "updated", entry
        return "noop", entry

    symbol = f"QING_{qing[2:6].upper()}"
    name = f"Mysterious Qing {qing[2:8].upper()}"
    
    try:
        code = w3.eth.get_code(Web3.to_checksum_address(integrative))
        if len(code) > 0:
            contract = w3.eth.contract(address=Web3.to_checksum_address(integrative), abi=erc20_abi)
            try:
                symbol = contract.functions.symbol().call()
            except Exception:
                pass
            try:
                name = contract.functions.name().call()
            except Exception:
                pass
    except Exception:
        pass
        
    new_entry = {
        "address": integrative_lower,
        "symbol": symbol,
        "name": name,
        "exists": True,
        "qing": qing
    }
    return "added", new_entry

def main():
    scanned_path = f"tmp/scanned_qings_{MAP_ADDRESS.lower()}.json"
    status_path = "config/nonukes_qings_status.json"

    if not os.path.exists(scanned_path):
        print(f"Scanned QINGs file not found: {scanned_path}")
        return

    print("Connecting to PulseChain RPC...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to RPC")
        return

    with open(scanned_path, "r") as f:
        scanned_data = json.load(f)

    if os.path.exists(status_path):
        with open(status_path, "r") as f:
            status_data = json.load(f)
    else:
        status_data = []

    existing = {item["address"].lower(): item for item in status_data}

    erc20_abi = [
        {"inputs": [], "name": "symbol", "outputs": [{"type": "string"}], "stateMutability": "view", "type": "function"},
        {"inputs": [], "name": "name", "outputs": [{"type": "string"}], "stateMutability": "view", "type": "function"}
    ]

    print(f"Processing {len(scanned_data)} scanned QINGs concurrently...")
    
    added_count = 0
    updated_count = 0
    new_entries = []

    # Query with ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=30) as executor:
        futures = []
        for item in scanned_data:
            integrative = item["integrative"]
            qing = item["qing"]
            futures.append(executor.submit(process_single, w3, erc20_abi, integrative, qing, existing))
            
        for future in futures:
            res_type, entry = future.result()
            if res_type == "added":
                status_data.append(entry)
                added_count += 1
            elif res_type == "updated":
                updated_count += 1

    with open(status_path, "w") as f:
        json.dump(status_data, f, indent=2)

    print(f"Complete: Added {added_count} new entries, updated {updated_count} existing entries. Total QINGs established in config: {len(status_data)}.")

if __name__ == "__main__":
    main()
