#!/usr/bin/env python3
import re
import os
import json
from eth_abi import abi
from web3 import Web3

RPC_URL = "https://rpc.pulsechain.com"
PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B"
MULTICALL3_ADDRESS = "0xcA11bde05977b3631167028862bE2a173976CA11"

PKMINTER_ABI_FULL = [
    {"inputs": [], "name": "TreasuryMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "FederalMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "BureauMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "IndexMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "PersonalMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "Cho", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "MRPK", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"}
]

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

MINTER_ABI_DUMMY = [
    {"inputs": [{"name": "ctx", "type": "address"}], "name": "GetTreasuryTokenOwner", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [{"name": "", "type": "address"}], "name": "TreasuryTokens", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"}
]

ERC20_ABI_DUMMY = [
    {"inputs": [], "name": "name", "outputs": [{"name": "", "type": "string"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "symbol", "outputs": [{"name": "", "type": "string"}], "stateMutability": "view", "type": "function"}
]

def main():
    print("Connecting to PulseChain...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to PulseChain")
        return

    # 1. Fetch all minters referenced from PKMinter
    minter_addresses = [PKMINTER_ADDRESS]
    try:
        pk_minter_contract = w3.eth.contract(address=Web3.to_checksum_address(PKMINTER_ADDRESS), abi=PKMINTER_ABI_FULL)
        for method in ["TreasuryMinter", "FederalMinter", "BureauMinter", "IndexMinter", "PersonalMinter", "Cho", "MRPK"]:
            try:
                addr = getattr(pk_minter_contract.functions, method)().call()
                if addr != "0x0000000000000000000000000000000000000000":
                    minter_addresses.append(addr.lower())
            except Exception as e:
                print(f"Minter lookup method {method} failed: {e}")
    except Exception as e:
        print(f"Failed to query PKMinter for child minters: {e}")

    # Remove duplicates but preserve order
    seen = set()
    minter_addresses = [x for x in minter_addresses if not (x in seen or seen.add(x))]
    print(f"Active Minter Lookup registry addresses: {minter_addresses}")

    # 2. Extract all unique addresses from addresses.sol
    address_path = "solidity/addresses.sol"
    if not os.path.exists(address_path):
        print(f"Error: {address_path} not found")
        return

    with open(address_path, "r") as f:
        content = f.read()

    raw_addresses = re.findall(r"0x[0-9a-fA-F]{40}", content)
    unique_addresses = sorted(list(set(addr.lower() for addr in raw_addresses)))
    print(f"Found {len(unique_addresses)} unique addresses in {address_path}")

    # Filter out addresses to check
    addresses_to_check = [
        addr for addr in unique_addresses 
        if addr not in ["0x000000000000000000000000000000000000dead", "0x0000000000000000000000000000000000000000"]
    ]

    # Pre-build dummy contract objects for encoding/decoding
    dummy_minter = w3.eth.contract(abi=MINTER_ABI_DUMMY)
    dummy_erc20 = w3.eth.contract(abi=ERC20_ABI_DUMMY)

    # 3. Pack calls for Multicall3
    calls = []
    call_metadata = []

    for addr in addresses_to_check:
        addr_checksum = Web3.to_checksum_address(addr)
        
        # Query GetTreasuryTokenOwner and TreasuryTokens on all minters
        for minter_addr in minter_addresses:
            minter_checksum = Web3.to_checksum_address(minter_addr)
            
            # Call: GetTreasuryTokenOwner
            calls.append({
                "target": minter_checksum,
                "allowFailure": True,
                "callData": dummy_minter.encode_abi("GetTreasuryTokenOwner", args=[addr_checksum])
            })
            call_metadata.append({"type": "owner_lookup", "address": addr, "minter": minter_addr, "method": "GetTreasuryTokenOwner"})

            # Call: TreasuryTokens mapping
            calls.append({
                "target": minter_checksum,
                "allowFailure": True,
                "callData": dummy_minter.encode_abi("TreasuryTokens", args=[addr_checksum])
            })
            call_metadata.append({"type": "owner_lookup", "address": addr, "minter": minter_addr, "method": "TreasuryTokens"})

        # Call: ERC20 name()
        calls.append({
            "target": addr_checksum,
            "allowFailure": True,
            "callData": dummy_erc20.encode_abi("name")
        })
        call_metadata.append({"type": "name", "address": addr})

        # Call: ERC20 symbol()
        calls.append({
            "target": addr_checksum,
            "allowFailure": True,
            "callData": dummy_erc20.encode_abi("symbol")
        })
        call_metadata.append({"type": "symbol", "address": addr})

    print(f"Packed {len(calls)} calls into 1 Multicall transaction.")
    print("Executing Multicall on PulseChain...")

    multicall_contract = w3.eth.contract(address=Web3.to_checksum_address(MULTICALL3_ADDRESS), abi=MULTICALL3_ABI)
    
    try:
        results = multicall_contract.functions.aggregate3(calls).call()
    except Exception as e:
        print(f"Multicall failed: {e}")
        return

    # 4. Parse results
    token_data = {}
    for idx, (success, return_data) in enumerate(results):
        meta = call_metadata[idx]
        addr = meta["address"]
        
        if addr not in token_data:
            token_data[addr] = {
                "is_treasury": False,
                "owner": None,
                "name": "Unknown Treasury Token",
                "symbol": "UNKNOWN"
            }

        if not success or not return_data:
            continue

        if meta["type"] == "owner_lookup":
            try:
                decoded = abi.decode(["address"], return_data)[0]
                if decoded != "0x0000000000000000000000000000000000000000":
                    token_data[addr]["is_treasury"] = True
                    token_data[addr]["owner"] = decoded.lower()
            except Exception:
                pass
        elif meta["type"] == "name":
            try:
                decoded = abi.decode(["string"], return_data)[0]
                token_data[addr]["name"] = decoded
            except Exception:
                pass
        elif meta["type"] == "symbol":
            try:
                decoded = abi.decode(["string"], return_data)[0]
                token_data[addr]["symbol"] = decoded
            except Exception:
                pass

    # Load existing registry
    registry_file = "treasury_tokens.json"
    registry = {}
    if os.path.exists(registry_file):
        try:
            with open(registry_file, "r") as f:
                registry = json.load(f)
        except Exception:
            pass

    added = 0
    # Save newly found tokens to registry
    for addr, info in token_data.items():
        if info["is_treasury"]:
            if addr not in registry:
                registry[addr] = {
                    "address": addr,
                    "symbol": info["symbol"],
                    "name": info["name"],
                    "owner": info["owner"],
                    "ignored": False
                }
                added += 1
                print(f"👑 Found Treasury Token: {info['symbol']} ({info['name']}) at {addr}")
            else:
                registry[addr]["owner"] = info["owner"]
                if registry[addr]["symbol"] == "UNKNOWN" and info["symbol"] != "UNKNOWN":
                    registry[addr]["symbol"] = info["symbol"]
                if registry[addr]["name"] == "Unknown Treasury Token" and info["name"] != "Unknown Treasury Token":
                    registry[addr]["name"] = info["name"]

    # Ensure FINVESTIBLE stays in the registry
    finvestible_addr = "0x38407c5a0c26675e34b6dd06bf98c571cbCdb6bf"
    if finvestible_addr not in registry:
        registry[finvestible_addr] = {
            "address": finvestible_addr,
            "symbol": "FINVESTIBLE",
            "name": "Proof Of Finvestment",
            "owner": None,
            "ignored": False
        }

    # Save registry
    with open(registry_file, "w") as f:
        json.dump(registry, f, indent=4)

    print(f"Scanning complete. Reduced RPC queries to 1 Multicall request. Total treasury tokens in registry: {len(registry)}")

if __name__ == "__main__":
    main()
