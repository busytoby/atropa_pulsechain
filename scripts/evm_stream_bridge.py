#!/usr/bin/env python3
import json
import time
import sys
from web3 import Web3

MotzkinPrime = 953467954114363

# Sworn-in LAU Nodes and their modular exponents
BEARS = [
    {"name": "Judge Bear (Alpha)", "address": "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B", "exp": 440},
    {"name": "Witness A (Beta)", "address": "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce", "exp": 12000},
    {"name": "Witness B (Gamma)", "address": "0xd32c39fee49391c7952d1b30b15921b0d3b42e69", "exp": 8877},
    {"name": "Juror 1 (Delta)", "address": "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b", "exp": 4433},
    {"name": "Juror 2 (Epsilon)", "address": "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e", "exp": 5522}
]

EMOTIONS = {
    "JOY": 1009,
    "SORROW": 1543,
    "ANGER": 2237,
    "FEAR": 3109,
    "MELANCHOLY": 4423,
    "EUPHORIA": 5519,
    "SUSPENSE": 6701,
    "TRANQUILITY": 7883,
    "CONFUSION": 8923
}

def big_mod_exp(base, exp, mod):
    return pow(base, exp, mod)

# Precompute target ratios for all 9 emotions on each bear's database
BEAR_DATABASES = []
for b in BEARS:
    db = {}
    for emo, val in EMOTIONS.items():
        res = big_mod_exp(val, b["exp"], MotzkinPrime)
        R = (res % 50) + 15.0
        r = (res % 25) + 8.0
        db[emo] = R / r
    BEAR_DATABASES.append(db)

def classify_ratio(active_ratio, db):
    best_emo = "CONFUSION"
    min_diff = float("inf")
    for emo, target_ratio in db.items():
        diff = abs(target_ratio - active_ratio)
        if diff < min_diff:
            min_diff = diff
            best_emo = emo
    return best_emo

def process_tx_hash(tx_hash_hex):
    # Convert hex string to integer and modulo MotzkinPrime to derive Pi
    tx_int = int(tx_hash_hex, 16)
    pi = tx_int % MotzkinPrime
    
    # Speaker is arbitrarily Judge Bear (Alpha) driving the stream
    speaker = BEARS[0]
    res = big_mod_exp(pi, speaker["exp"], MotzkinPrime)
    R_spk = (res % 50) + 15.0
    r_spk = (res % 25) + 8.0
    active_ratio = R_spk / r_spk

    print(f"\n[EVM Block Event] Hash: {tx_hash_hex[:18]}... -> Pi: {pi}")
    print(f"  Speaker: {speaker['name']} | Ratio: {active_ratio:.3f}")
    
    # Each observer decodes the stream and outputs its classification
    matrix = {}
    for i, observer in enumerate(BEARS):
        db = BEAR_DATABASES[i]
        classified = classify_ratio(active_ratio, db)
        matrix[observer["name"]] = classified
        print(f"    ├─ {observer['name']:<22} classifies: {classified}")
        
    return matrix

if __name__ == "__main__":
    print("[BRIDGE] Initializing EVM Blockchain to DAC Pi Stream Bridge...")
    rpc_url = "https://rpc.pulsechain.com"
    w3 = Web3(Web3.HTTPProvider(rpc_url))
    
    if not w3.is_connected():
        print(f"[ERROR] Could not connect to PulseChain RPC at {rpc_url}. Exiting.")
        sys.exit(1)
        
    print(f"[SUCCESS] Connected to PulseChain RPC. Listening for new blocks...")
    
    # Poll for the latest block transactions
    latest_block = w3.eth.get_block('latest', full_transactions=True)
    txs = latest_block.get('transactions', [])
    
    print(f"[INFO] Latest Block: #{latest_block['number']} containing {len(txs)} transactions.")
    
    # Process up to 5 transactions from the latest block to demonstrate real-time classification
    count = 0
    for tx in txs:
        if count >= 5:
            break
        tx_hash_hex = tx['hash'].hex()
        process_tx_hash(tx_hash_hex)
        count += 1
        time.sleep(0.5)
        
    print("[SUCCESS] EVM Stream Bridge run complete.")
