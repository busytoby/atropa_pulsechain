#!/usr/bin/env python3
import json
import os
from web3 import Web3
from eth_abi import abi

RPC_URL = "https://rpc.pulsechain.com"
CHO_ADDRESS = "0xB6be11F0A788014C1F68C92F8D6CcC1AbF78F2aB"
MULTICALL3_ADDRESS = "0xcA11bde05977b3631167028862bE2a173976CA11"

MULTICALL3_ABI = [
    {
        "inputs": [
            {
                "components": [
                    {"name": "target", "type": "address"},
                    {"name": "allowFailure", "type": "bool"},
                    {"name": "callData", "type": "bytes"}
                ],
                "name": "calls",
                "type": "tuple[]"
            }
        ],
        "name": "aggregate3",
        "outputs": [
            {
                "components": [
                    {"name": "success", "type": "bool"},
                    {"name": "returnData", "type": "bytes"}
                ],
                "name": "returnValues",
                "type": "tuple[]"
            }
        ],
        "stateMutability": "payable",
        "type": "function"
    }
]

CHO_ABI_DUMMY = [
    {"inputs": [{"name": "wallet", "type": "address"}], "name": "GetUserTokenAddress", "outputs": [{"name": "UserToken", "type": "address"}], "stateMutability": "view", "type": "function"}
]

ERC20_ABI_DUMMY = [
    {"inputs": [], "name": "name", "outputs": [{"name": "", "type": "string"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "symbol", "outputs": [{"name": "", "type": "string"}], "stateMutability": "view", "type": "function"}
]

def main():
    print("Connecting to PulseChain RPC...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to PulseChain")
        return

    print("Scanning CHO Transfer logs to discover wallets...")
    transfer_topic = "0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef"
    
    wallets = set()
    
    # Scan logs in block chunks from CHO deployment block (around block 17200000 on PulseChain)
    start_block = 17200000
    latest_block = w3.eth.block_number
    step = 100000
    
    print(f"Scanning blocks {start_block} to {latest_block}...")
    for from_block in range(start_block, latest_block + 1, step):
        to_block = min(from_block + step - 1, latest_block)
        try:
            logs = w3.eth.get_logs({
                "address": w3.to_checksum_address(CHO_ADDRESS),
                "topics": [transfer_topic],
                "fromBlock": from_block,
                "toBlock": to_block
            })
            for log in logs:
                if len(log["topics"]) >= 3:
                    from_addr = "0x" + log["topics"][1].hex()[-40:]
                    to_addr = "0x" + log["topics"][2].hex()[-40:]
                    if from_addr != "0x0000000000000000000000000000000000000000":
                        wallets.add(from_addr.lower())
                    if to_addr != "0x0000000000000000000000000000000000000000":
                        wallets.add(to_addr.lower())
        except Exception as e:
            pass

    # Merge with published addresses
    published_path = "published_addresses.json"
    if os.path.exists(published_path):
        try:
            with open(published_path, "r") as f:
                for addr in json.load(f):
                    wallets.add(addr.lower())
        except Exception:
            pass

    addresses = sorted(list(wallets))
    print(f"Scanning CHO registry for {len(addresses)} candidate addresses using Multicall3...")

    cho_dummy = w3.eth.contract(abi=CHO_ABI_DUMMY)
    erc20_dummy = w3.eth.contract(abi=ERC20_ABI_DUMMY)
    multicall_contract = w3.eth.contract(address=Web3.to_checksum_address(MULTICALL3_ADDRESS), abi=MULTICALL3_ABI)

    calls = []
    for addr in addresses:
        addr_checksum = w3.to_checksum_address(addr)
        calls.append({
            "target": Web3.to_checksum_address(CHO_ADDRESS),
            "allowFailure": True,
            "callData": cho_dummy.encode_abi("GetUserTokenAddress", args=[addr_checksum])
        })

    # Execute in chunks
    results = []
    chunk_size = 500
    for i in range(0, len(calls), chunk_size):
        chunk = calls[i:i+chunk_size]
        print(f"Batch {i//chunk_size + 1}/{(len(calls)-1)//chunk_size + 1}...")
        try:
            res = multicall_contract.functions.aggregate3(chunk).call()
            results.extend(res)
        except Exception as e:
            print(f"Batch failed: {e}")
            return

    # Parse discovered LAUs
    delegates = {}
    tokens_to_fetch = []
    for idx, (success, data) in enumerate(results):
        if success and len(data) >= 32:
            lau_addr = Web3.to_hex(data[-20:])
            if lau_addr != "0x0000000000000000000000000000000000000000":
                wallet_addr = addresses[idx]
                delegates[lau_addr.lower()] = {
                    "address": lau_addr.lower(),
                    "symbol": "LAU",
                    "name": "Discovered Delegate",
                    "owner": wallet_addr,
                    "minter_address": CHO_ADDRESS.lower(),
                    "minter_name": "CHO Registry",
                    "parent_address": None,
                    "parent_symbol": None,
                    "ignored": False
                }
                tokens_to_fetch.append(lau_addr)

    print(f"Discovered {len(delegates)} registered delegate LAU addresses!")

    # Fetch names/symbols for discovered LAU tokens
    if tokens_to_fetch:
        print("Querying token metadata (symbol, name) via Multicall...")
        meta_calls = []
        for t_addr in tokens_to_fetch:
            t_checksum = w3.to_checksum_address(t_addr)
            meta_calls.append({
                "target": t_checksum,
                "allowFailure": True,
                "callData": erc20_dummy.encode_abi("symbol")
            })
            meta_calls.append({
                "target": t_checksum,
                "allowFailure": True,
                "callData": erc20_dummy.encode_abi("name")
            })

        try:
            meta_results = multicall_contract.functions.aggregate3(meta_calls).call()
            for idx, t_addr in enumerate(tokens_to_fetch):
                symbol_success, symbol_data = meta_results[idx*2]
                name_success, name_data = meta_results[idx*2 + 1]

                sym = "LAU"
                if symbol_success and len(symbol_data) > 32:
                    try:
                        sym = abi.decode(["string"], symbol_data)[0]
                    except:
                        pass

                name = "Discovered Delegate"
                if name_success and len(name_data) > 32:
                    try:
                        name = abi.decode(["string"], name_data)[0]
                    except:
                        pass

                delegates[t_addr.lower()]["symbol"] = sym
                delegates[t_addr.lower()]["name"] = name
                print(f" -> Active Delegate: {sym} ({name}) at {t_addr}")
        except Exception as e:
            print(f"Metadata fetch failed: {e}")

    # Save to dynamic roster files (both cho_delegates.json and the treasury registry file)
    roster_path = "cho_delegates.json"
    with open(roster_path, "w") as f:
        json.dump(delegates, f, indent=4)
        
    registry_path = "assets/treasury_tokens_cho_registry.json"
    with open(registry_path, "w") as f:
        json.dump(delegates, f, indent=4)

    print(f"CHO Delegate Roster successfully compiled and written to {roster_path} and {registry_path}")

if __name__ == "__main__":
    main()
