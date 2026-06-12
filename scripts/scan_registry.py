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

TT_ABI_DUMMY = [
    {"inputs": [], "name": "V2Minter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "PersonalMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "IndexMinter", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"},
    {"inputs": [], "name": "Parent", "outputs": [{"name": "", "type": "address"}], "stateMutability": "view", "type": "function"}
]

# Mapping of known minter contract addresses to friendly names
MINTER_NAMES = {
    "0x9f4e1471e614747a9a56a33eb0338671eba1de2b": "PKMinter",
    "0xc7bdac3e6bb5ec37041a11328723e9927ccf430b": "TreasuryMinter",
    "0xc15c5f699daf5e1135732139f05d2c05b3ef4354": "FederalMinter",
    "0x0b92ad7ed0da6c44bf71b3fcee668d1670000ff5": "BureauMinter",
    "0x0c4f73328dfcecfbecf235c9f78a4494a7ec5ddc": "IndexMinter",
    "0x394c3d5990cefc7be36b82fdb07a7251ace61cc7": "PersonalMinter",
    "0xb6be11f0a788014c1f68c92f8d6ccc1abf78f2ab": "ChoMinter",
    "0x3827e9035d94ad636d006288683ade2294f210a5": "MRPKMinter"
}

def main():
    print("Connecting to PulseChain...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to PulseChain")
        return

    # 1. Fetch all minters referenced from PKMinter
    minter_addresses = [PKMINTER_ADDRESS.lower()]
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
    print(f"Active Minter Lookup registry addresses (formatted): {[x.lower() for x in minter_addresses]}")

    # 2. Extract all unique addresses from solidity/ directory files recursively
    raw_addresses = []
    solidity_dir = "solidity"
    if os.path.exists(solidity_dir):
        for root, dirs, files in os.walk(solidity_dir):
            for file in files:
                if file.endswith(".sol") or file.endswith(".yul"):
                    file_path = os.path.join(root, file)
                    try:
                        with open(file_path, "r", errors="ignore") as f:
                            content = f.read()
                            raw_addresses.extend(re.findall(r"0x[0-9a-fA-F]{40}", content))
                    except Exception as e:
                        print(f"Error reading solidity file {file_path}: {e}")
    
    # Set addresses to check strictly to Published log events to achieve 100% precision and instant speed
    published_path = "published_addresses.json"
    if os.path.exists(published_path):
        try:
            with open(published_path, "r") as f:
                addresses_to_check = sorted(list(set(addr.lower() for addr in json.load(f))))
            print(f"Instantly loading {len(addresses_to_check)} published log addresses to scan.")
        except Exception as e:
            print(f"Error reading published_addresses.json: {e}")
            addresses_to_check = []
    else:
        addresses_to_check = []

    print(f"Sample addresses_to_check: {addresses_to_check[:15]}")

    # Pre-build dummy contract objects for encoding/decoding
    dummy_minter = w3.eth.contract(abi=MINTER_ABI_DUMMY)
    dummy_erc20 = w3.eth.contract(abi=ERC20_ABI_DUMMY)
    dummy_tt = w3.eth.contract(abi=TT_ABI_DUMMY)

    # 3. Pack calls for Multicall3 (First Phase: Treasury ownership checks only)
    calls = []
    call_metadata = []
    
    # Prioritize addresses by putting those in state files first (likely active tokens)
    active_priority = []
    inactive_priority = []
    
    # Read cache directories to see which tokens have resolved or unresolved swaps
    priority_addrs = set()
    for cache_path in ["resolved_swaps.json", "unresolved_swaps.json"]:
        if os.path.exists(cache_path):
            try:
                with open(cache_path, "r") as f:
                    for m in re.finditer(r"0x[0-9a-fA-F]{40}", f.read()):
                        priority_addrs.add(m.group(0).lower())
            except Exception:
                pass
                
    for addr in addresses_to_check:
        if addr in priority_addrs:
            active_priority.append(addr)
        else:
            inactive_priority.append(addr)
            
    sorted_addresses = active_priority + inactive_priority
    print(f"Prioritized search: {len(active_priority)} high-priority (active swaps) addresses first, then {len(inactive_priority)} others.")

    # Define optimized list of first-round lookup minter address contracts:
    # 1. PKMINTER (0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B) - resolves GetTreasuryTokenOwner recursively
    # 2. V1MINTER (0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B) - mapping of TreasuryTokens directly
    opt_minters = [
        PKMINTER_ADDRESS.lower(),
        "0xc7bdac3e6bb5ec37041a11328723e9927ccf430b" # V1Minter
    ]

    for addr in sorted_addresses:
        addr_checksum = Web3.to_checksum_address(addr)
        
        # Query GetTreasuryTokenOwner and TreasuryTokens only on primary resolution targets
        for minter_addr in opt_minters:
            minter_checksum = Web3.to_checksum_address(minter_addr)
            
            # Call: GetTreasuryTokenOwner
            calls.append({
                "target": minter_checksum,
                "allowFailure": True,
                "callData": dummy_minter.encode_abi("GetTreasuryTokenOwner", args=[addr_checksum])
            })
            call_metadata.append({"type": "owner_lookup", "address": addr.lower(), "minter": minter_addr.lower(), "method": "GetTreasuryTokenOwner"})

            # Call: TreasuryTokens mapping
            calls.append({
                "target": minter_checksum,
                "allowFailure": True,
                "callData": dummy_minter.encode_abi("TreasuryTokens", args=[addr_checksum])
            })
            call_metadata.append({"type": "owner_lookup", "address": addr.lower(), "minter": minter_addr.lower(), "method": "TreasuryTokens"})

    print(f"Packed {len(calls)} lookup calls. Executing in batches of 1000 to prevent rpc return data limits...")
    multicall_contract = w3.eth.contract(address=Web3.to_checksum_address(MULTICALL3_ADDRESS), abi=MULTICALL3_ABI)
    
    results = []
    chunk_size = 1000
    for i in range(0, len(calls), chunk_size):
        chunk = calls[i:i+chunk_size]
        print(f"Executing batch {i//chunk_size + 1}/{(len(calls)-1)//chunk_size + 1} ({len(chunk)} calls)...")
        try:
            res = multicall_contract.functions.aggregate3(chunk).call()
            results.extend(res)
        except Exception as e:
            print(f"Batch failed: {e}")
            # Fill failed batch with default empty responses
            results.extend([(False, b"")] * len(chunk))

    print(f"Total first-round Multicall returned {len(results)} results.")
    successes = sum(1 for success, _ in results if success)
    print(f"First-round success rate: {successes}/{len(results)}")

    # 4. Parse first-round results to identify treasury tokens
    token_data = {}
    symbol_map = {} # To resolve parent symbols

    # Fill name/symbol map first
    for idx, (success, return_data) in enumerate(results):
        meta = call_metadata[idx]
        addr = meta["address"]
        if success and return_data and meta["type"] == "symbol":
            try:
                decoded = abi.decode(["string"], return_data)[0]
                symbol_map[addr] = decoded
            except Exception:
                pass

    for idx, (success, return_data) in enumerate(results):
        meta = call_metadata[idx]
        addr = meta["address"]
        
        if addr not in token_data:
            token_data[addr] = {
                "is_treasury": False,
                "owner": None,
                "name": "Unknown Treasury Token",
                "symbol": "UNKNOWN",
                "minter_address": None,
                "minter_name": None,
                "parent_address": None,
                "parent_symbol": None
            }

        if not success or not return_data:
            continue

        if meta["type"] == "owner_lookup":
            try:
                decoded = abi.decode(["address"], return_data)[0]
                if decoded != "0x0000000000000000000000000000000000000000":
                    token_data[addr]["is_treasury"] = True
                    token_data[addr]["owner"] = decoded.lower()
                    print(f"Token {addr} owner lookup found treasury ownership: owner={decoded}")
            except Exception as e:
                print(f"Decode error for owner_lookup: {e}")
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

    # 5. For identified treasury tokens, perform a second multicall to fetch Parent(), V2Minter(), name(), symbol()
    registry_file = "treasury_tokens.json"
    cached_addrs = []
    if os.path.exists(registry_file):
        try:
            with open(registry_file, "r") as f:
                cached_addrs = [k.lower() for k in json.load(f).keys()]
        except Exception:
            pass

    treasury_addrs = sorted(list(set(
        [addr for addr, info in token_data.items() if info["is_treasury"] or info["owner"] is not None] + 
        cached_addrs + 
        ["0x38407c5a0c26675e34b6dd06bf98c571cbCdb6bf"]
    )))
    print(f"Identified treasury tokens for secondary checks: {treasury_addrs}")
    if treasury_addrs:
        second_calls = []
        second_metadata = []
        for addr in treasury_addrs:
            addr_checksum = Web3.to_checksum_address(addr)
            
            # Call: name()
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_erc20.encode_abi("name")
            })
            second_metadata.append({"type": "name", "address": addr})

            # Call: symbol()
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_erc20.encode_abi("symbol")
            })
            second_metadata.append({"type": "symbol", "address": addr})

            # Call: V2Minter() view
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_tt.encode_abi("V2Minter")
            })
            second_metadata.append({"type": "minter_view", "address": addr, "var_name": "V2Minter"})

            # Call: PersonalMinter() view
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_tt.encode_abi("PersonalMinter")
            })
            second_metadata.append({"type": "minter_view", "address": addr, "var_name": "PersonalMinter"})

            # Call: IndexMinter() view
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_tt.encode_abi("IndexMinter")
            })
            second_metadata.append({"type": "minter_view", "address": addr, "var_name": "IndexMinter"})

            # Call: Parent() view
            second_calls.append({
                "target": addr_checksum,
                "allowFailure": True,
                "callData": dummy_tt.encode_abi("Parent")
            })
            second_metadata.append({"type": "parent_view", "address": addr})
            
            # Ensure key exists in token_data if it only came from cached registry
            if addr not in token_data:
                token_data[addr] = {
                    "is_treasury": True,
                    "owner": None,
                    "name": "Cached Treasury Token",
                    "symbol": "UNKNOWN",
                    "minter_address": None,
                    "minter_name": None,
                    "parent_address": None,
                    "parent_symbol": None
                }

        print(f"Executing secondary Multicall to fetch V2Minter, PersonalMinter, IndexMinter, and Parent for {len(treasury_addrs)} treasury tokens...")
        try:
            second_results = multicall_contract.functions.aggregate3(second_calls).call()
            for idx, (success, return_data) in enumerate(second_results):
                meta = second_metadata[idx]
                addr = meta["address"]
                if not success or not return_data:
                    continue
                if meta["type"] == "name":
                    try:
                        decoded = abi.decode(["string"], return_data)[0]
                        token_data[addr]["name"] = decoded
                    except Exception:
                        pass
                elif meta["type"] == "symbol":
                    try:
                        decoded = abi.decode(["string"], return_data)[0]
                        token_data[addr]["symbol"] = decoded
                        symbol_map[addr] = decoded
                    except Exception:
                        pass
                elif meta["type"] == "minter_view":
                    try:
                        decoded = abi.decode(["address"], return_data)[0]
                        if decoded != "0x0000000000000000000000000000000000000000":
                            token_data[addr]["minter_address"] = decoded.lower()
                            token_data[addr]["minter_name"] = MINTER_NAMES.get(decoded.lower(), "Unknown Minter")
                    except Exception as e:
                        print(f"Error decoding {meta.get('var_name', 'Minter')} for {addr}: {e}")
                elif meta["type"] == "parent_view":
                    try:
                        decoded = abi.decode(["address"], return_data)[0]
                        if decoded != "0x0000000000000000000000000000000000000000":
                            token_data[addr]["parent_address"] = decoded.lower()
                            token_data[addr]["parent_symbol"] = symbol_map.get(decoded.lower(), "UNKNOWN")
                    except Exception as e:
                        print(f"Error decoding Parent for {addr}: {e}")

            # 5b. Resolve symbols for parent addresses that are still UNKNOWN by running a quick multicall for their name/symbol
            unresolved_parents = []
            for addr in treasury_addrs:
                p_addr = token_data.get(addr, {}).get("parent_address")
                if p_addr and p_addr != "0x0000000000000000000000000000000000000000" and p_addr not in symbol_map:
                    unresolved_parents.append(p_addr)
            
            if unresolved_parents:
                unresolved_parents = list(set(unresolved_parents))
                print(f"Resolving names/symbols for {len(unresolved_parents)} unresolved parent tokens...")
                parent_calls = []
                parent_meta = []
                for p_addr in unresolved_parents:
                    p_checksum = Web3.to_checksum_address(p_addr)
                    parent_calls.append({
                        "target": p_checksum,
                        "allowFailure": True,
                        "callData": dummy_erc20.encode_abi("symbol")
                    })
                    parent_meta.append({"type": "symbol", "address": p_addr})
                
                try:
                    parent_results = multicall_contract.functions.aggregate3(parent_calls).call()
                    for idx, (success, return_data) in enumerate(parent_results):
                        meta = parent_meta[idx]
                        p_addr = meta["address"]
                        if success and return_data:
                            try:
                                decoded = abi.decode(["string"], return_data)[0]
                                symbol_map[p_addr] = decoded
                                print(f"Resolved parent symbol for {p_addr}: {decoded}")
                            except Exception:
                                pass
                    
                    # Update token_data entries with newly resolved parent symbols
                    for addr in treasury_addrs:
                        p_addr = token_data.get(addr, {}).get("parent_address")
                        if p_addr in symbol_map:
                            token_data[addr]["parent_symbol"] = symbol_map[p_addr]
                except Exception as e:
                    print(f"Parent symbol resolution Multicall failed: {e}")
        except Exception as e:
            print(f"Secondary Multicall failed: {e}")

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
        addr_lower = addr.lower()
        if info["is_treasury"]:
            # Fallbacks for minter/parent names if not queryable but resolved by owner
            minter_name = info["minter_name"]
            if not minter_name and info["owner"]:
                # The owner returned is the address that minted it, which could match a minter address
                minter_name = MINTER_NAMES.get(info["owner"], "Unknown Minter")

            if addr_lower not in registry:
                registry[addr_lower] = {
                    "address": addr_lower,
                    "symbol": info["symbol"],
                    "name": info["name"],
                    "owner": info["owner"],
                    "minter_address": info["minter_address"],
                    "minter_name": minter_name,
                    "parent_address": info["parent_address"],
                    "parent_symbol": info["parent_symbol"],
                    "ignored": False
                }
                added += 1
                print(f"👑 Found Treasury Token: {info['symbol']} ({info['name']}) at {addr_lower}")
            else:
                registry[addr_lower]["owner"] = info["owner"] or registry[addr_lower].get("owner")
                registry[addr_lower]["minter_address"] = info["minter_address"] or registry[addr_lower].get("minter_address")
                registry[addr_lower]["minter_name"] = minter_name or registry[addr_lower].get("minter_name")
                registry[addr_lower]["parent_address"] = info["parent_address"] or registry[addr_lower].get("parent_address")
                registry[addr_lower]["parent_symbol"] = info["parent_symbol"] or registry[addr_lower].get("parent_symbol")
                if (registry[addr_lower]["symbol"] == "UNKNOWN" or registry[addr_lower]["symbol"] == "Cached Treasury Token") and info["symbol"] != "UNKNOWN":
                    registry[addr_lower]["symbol"] = info["symbol"]
                if (registry[addr_lower]["name"] == "Unknown Treasury Token" or registry[addr_lower]["name"] == "Cached Treasury Token") and info["name"] != "Unknown Treasury Token":
                    registry[addr_lower]["name"] = info["name"]

    # Ensure FINVESTIBLE stays in the registry
    finvestible_addr = "0x38407c5a0c26675e34b6dd06bf98c571cbcdb6bf"
    
    # Remove any uppercase variant from registry to ensure complete lowercase clean
    if "0x38407c5a0c26675e34b6dd06bf98c571cbCdb6bf" in registry:
        del registry["0x38407c5a0c26675e34b6dd06bf98c571cbCdb6bf"]
        
    if finvestible_addr not in registry:
        registry[finvestible_addr] = {
            "address": finvestible_addr,
            "symbol": "FINVESTIBLE",
            "name": "Proof Of Finvestment",
            "owner": None,
            "minter_address": None,
            "minter_name": None,
            "parent_address": None,
            "parent_symbol": None,
            "ignored": False
        }
    else:
        # Update symbol/name if it was set to placeholder values
        if registry[finvestible_addr]["symbol"] == "UNKNOWN" or registry[finvestible_addr]["symbol"] == "Cached Treasury Token":
            registry[finvestible_addr]["symbol"] = "FINVESTIBLE"
        if registry[finvestible_addr]["name"] == "Unknown Treasury Token" or registry[finvestible_addr]["name"] == "Cached Treasury Token":
            registry[finvestible_addr]["name"] = "Proof Of Finvestment"

    # Also clean up any other uppercase keys if present in registry
    for k in list(registry.keys()):
        if k != k.lower():
            val = registry[k]
            val["address"] = k.lower()
            registry[k.lower()] = val
            del registry[k]

    # Save registry
    with open(registry_file, "w") as f:
        json.dump(registry, f, indent=4)

    print(f"Scanning complete. Reduced RPC queries to 1 Multicall request. Total treasury tokens in registry: {len(registry)}")

if __name__ == "__main__":
    main()
