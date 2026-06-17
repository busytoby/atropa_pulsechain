import json
import os
from web3 import Web3
from eth_abi import abi

RPC_URL = "https://rpc.pulsechain.com"
MAP_ADDRESS = "0xD3a7A95012Edd46Ea115c693B74c5e524b3DdA75"

def main():
    print("Connecting to PulseChain RPC...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to PulseChain")
        return

    print(f"Using verified mainnet MAP contract address: {MAP_ADDRESS}")

    # Ensure topic has 0x prefix
    new_qing_topic = "0x" + Web3.keccak(text="NewQing(address,address,uint256)").hex()
    latest_block = w3.eth.block_number

    # Search block range from 15,000,000 to latest block in chunks of 500,000
    from_block = 15000000
    step = 500000
    all_logs = []

    print(f"Querying logs from block {from_block} to {latest_block} in steps of {step}...")
    for start in range(from_block, latest_block, step):
        end = min(start + step - 1, latest_block)
        try:
            logs = w3.eth.get_logs({
                "fromBlock": start,
                "toBlock": end,
                "address": Web3.to_checksum_address(MAP_ADDRESS),
                "topics": [new_qing_topic]
            })
            all_logs.extend(logs)
        except Exception as e:
            print(f"  Error querying blocks {start} to {end}: {e}")

    print(f"Found {len(all_logs)} total NewQing events.")

    qings = []
    for log in all_logs:
        try:
            data = log["data"]
            decoded = abi.decode(["address", "address", "uint256"], data)
            qing_addr = decoded[0]
            integrative_addr = decoded[1]
            waat = decoded[2]
            qings.append({
                "qing": qing_addr,
                "integrative": integrative_addr,
                "waat": waat,
                "block": log["blockNumber"]
            })
        except Exception as e:
            print(f"  Error decoding log: {e}")

    out_file = f"tmp/scanned_qings_{MAP_ADDRESS.lower()}.json"
    os.makedirs("tmp", exist_ok=True)
    with open(out_file, "w") as f:
        json.dump(qings, f, indent=4)
    print(f"Saved QINGs list to {out_file}")

    # Check if NoNukes (0x174A0ad99c60c20D9B3d94c3095BC1fb9ddEFd62) is in the list
    nonukes_addr = "0x174A0ad99c60c20D9B3d94c3095BC1fb9ddEFd62".lower()
    nonukes_matches = [q for q in qings if q["integrative"].lower() == nonukes_addr]
    if nonukes_matches:
        print(f"\nSUCCESS: Found NoNukes QING at address: {nonukes_matches[0]['qing']}")
        print(f"Created at block: {nonukes_matches[0]['block']}")
    else:
        print("\nNoNukes QING NOT found on this MAP contract.")
        print(f"Scanned {len(qings)} total QING registrations.")

if __name__ == "__main__":
    main()
