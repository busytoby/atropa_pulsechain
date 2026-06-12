#!/usr/bin/env python3
import re
import os
import json
from concurrent.futures import ThreadPoolExecutor, as_completed
from web3 import Web3

RPC_URL = "https://rpc.pulsechain.com"
PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B"

PKMINTER_ABI_FULL = [
    {"inputs": [], "name": "TreasuryMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "FederalMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "BureauMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "IndexMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "PersonalMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "Cho", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "MRPK", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"}
]

MINTER_ABI = [
    {
        "inputs": [{"name": "ctx", "type": "address"}],
        "name": "GetTreasuryTokenOwner",
        "outputs": [{"name": "", "type": "address"}],
        "stateMutability": "view",
        "type": "function"
    }
]

ERC20_ABI = [
    {"constant": True, "inputs": [], "name": "name", "outputs": [{"name": "", "type": "string"}], "type": "function"},
    {"constant": True, "inputs": [], "name": "symbol", "outputs": [{"name": "", "type": "string"}], "type": "function"}
]

def check_address(addr, minter_contracts, w3):
    addr_checksum = Web3.to_checksum_address(addr)
    for minter_addr, contract in minter_contracts:
        owner = "0x0000000000000000000000000000000000000000"
        
        # 1. Try GetTreasuryTokenOwner
        try:
            owner = contract.functions.GetTreasuryTokenOwner(addr_checksum).call()
        except Exception:
            pass
            
        # 2. Try TreasuryTokens mapping if GetTreasuryTokenOwner returned null or failed
        if owner == "0x0000000000000000000000000000000000000000":
            try:
                temp_contract = w3.eth.contract(
                    address=Web3.to_checksum_address(minter_addr),
                    abi=[{"inputs": [{"name": "", "type": "address"}], "name": "TreasuryTokens", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"}]
                )
                owner = temp_contract.functions.TreasuryTokens(addr_checksum).call()
            except Exception:
                pass
                
        if owner != "0x0000000000000000000000000000000000000000":
            # Fetch token name/symbol
            token_contract = w3.eth.contract(address=addr_checksum, abi=ERC20_ABI)
            try:
                name = token_contract.functions.name().call()
            except Exception:
                name = "Unknown Treasury Token"
            try:
                symbol = token_contract.functions.symbol().call()
            except Exception:
                symbol = "UNKNOWN"
            return {
                "address": addr,
                "symbol": symbol,
                "name": name,
                "owner": owner.lower(),
                "ignored": False
            }
    return None

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

    # Load existing registry
    registry_file = "treasury_tokens.json"
    registry = {}
    if os.path.exists(registry_file):
        try:
            with open(registry_file, "r") as f:
                registry = json.load(f)
        except Exception:
            pass

    # Pre-build minter contract instances
    minter_contracts = []
    for minter_addr in minter_addresses:
        minter_contracts.append((minter_addr, w3.eth.contract(address=Web3.to_checksum_address(minter_addr), abi=MINTER_ABI)))

    # Filter out addresses to check
    addresses_to_check = [
        addr for addr in unique_addresses 
        if addr not in ["0x000000000000000000000000000000000000dead", "0x0000000000000000000000000000000000000000"]
    ]

    print(f"Scanning {len(addresses_to_check)} addresses using multithreading (GetTreasuryTokenOwner + TreasuryTokens)...")
    
    # Run check in thread pool
    results = []
    with ThreadPoolExecutor(max_workers=25) as executor:
        futures = {executor.submit(check_address, addr, minter_contracts, w3): addr for addr in addresses_to_check}
        for future in as_completed(futures):
            res = future.result()
            if res:
                results.append(res)
                print(f"👑 Found Treasury Token: {res['symbol']} ({res['name']}) at {res['address']}")

    # Save results to registry
    for res in results:
        addr = res["address"]
        if addr not in registry:
            registry[addr] = res
        else:
            registry[addr]["owner"] = res["owner"]
            if registry[addr]["symbol"] == "UNKNOWN" and res["symbol"] != "UNKNOWN":
                registry[addr]["symbol"] = res["symbol"]
            if registry[addr]["name"] == "Unknown Treasury Token" and res["name"] != "Unknown Treasury Token":
                registry[addr]["name"] = res["name"]

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

    print(f"Scanning complete. Total treasury tokens in registry: {len(registry)}")

if __name__ == "__main__":
    main()
