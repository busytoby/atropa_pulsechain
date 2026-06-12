#!/usr/bin/env python3
import time
import sys
import json
import os
from web3 import Web3
from eth_abi import abi

# PulseChain RPC Endpoint
RPC_URL = "https://rpc.pulsechain.com"

# Swap event signature hashes
SWAP_V2_HASH = "0xd78ad95fa46c994b6551d0da85fc275fe613ce37657fb8d5e3d130840159d822"
SWAP_V3_HASH = "0xc42079f94a1634158ef986f500127bce9d5102910f196ce1c272a85db00336ad"

# ABIs
ERC20_ABI = [
    {"constant": True, "inputs": [], "name": "name", "outputs": [{"name": "", "type": "string"}], "type": "function"},
    {"constant": True, "inputs": [], "name": "symbol", "outputs": [{"name": "", "type": "string"}], "type": "function"},
    {"constant": True, "inputs": [], "name": "decimals", "outputs": [{"name": "", "type": "uint8"}], "type": "function"}
]

PAIR_ABI = [
    {"constant": True, "inputs": [], "name": "token0", "outputs": [{"name": "", "type": "address"}], "type": "function"},
    {"constant": True, "inputs": [], "name": "token1", "outputs": [{"name": "", "type": "address"}], "type": "function"}
]

PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B"
PKMINTER_ABI = [
    {
        "inputs": [{"name": "ctx", "type": "address"}],
        "name": "GetTreasuryTokenOwner",
        "outputs": [{"name": "", "type": "address"}],
        "stateMutability": "view",
        "type": "function"
    }
]

# Files to store state
UNRESOLVED_FILE = "unresolved_swaps.json"
PRICE_CACHE_FILE = "price_cache.json"
RESOLVED_FILE = "resolved_swaps.json"
TREASURY_TOKENS_FILE = "treasury_tokens.json"

def save_treasury_token(address, symbol, name, owner):
    try:
        tokens = {}
        if os.path.exists(TREASURY_TOKENS_FILE):
            with open(TREASURY_TOKENS_FILE, "r") as f:
                tokens = json.load(f)
        addr_lower = address.lower()
        if addr_lower not in tokens:
            tokens[addr_lower] = {
                "address": addr_lower,
                "symbol": symbol,
                "name": name,
                "owner": owner.lower() if owner else None,
                "ignored": False
            }
            with open(TREASURY_TOKENS_FILE, "w") as f:
                json.dump(tokens, f, indent=4)
            print(f"👑 Added new Treasury Token to tracking: {symbol} ({name})")
    except Exception as e:
        print(f"Error saving treasury token: {e}")

def save_resolved(swap):
    try:
        resolved = []
        if os.path.exists(RESOLVED_FILE):
            with open(RESOLVED_FILE, "r") as f:
                resolved = json.load(f)
        resolved.append(swap)
        resolved = resolved[-200:]
        with open(RESOLVED_FILE, "w") as f:
            json.dump(resolved, f, indent=4)
    except Exception as e:
        print(f"Error saving resolved swap: {e}")

# Base stablecoin addresses (lowercased)
USDC_ADDR = "0x15D38573d2feeb82e7ad5187aB8c1D52810B1f07".lower()
USDC_ADDR2 = "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48".lower()
USDT_ADDR = "0x0Cb81b54A05e0547D2d08C4A9E273a7d4C72B9eB".lower()
USDT_ADDR2 = "0xdac17f958d2ee523a2206206994597c13d831ec7".lower()
DAI_ADDR = "0xefD766cCb8C15E5E9F813af7b2809857Baa53A1f".lower()
DAI_ADDR2 = "0x6b175474e89094c44da98b954eedeac495271d0f".lower()

# Known token prices in USD (stored with metadata)
price_cache = {
    USDC_ADDR: {"price": 1.0, "symbol": "USDC", "name": "USD Coin from Ethereum"},
    USDC_ADDR2: {"price": 1.0, "symbol": "USDC", "name": "USD Coin"},
    USDT_ADDR: {"price": 1.0, "symbol": "USDT", "name": "Tether USD"},
    USDT_ADDR2: {"price": 1.0, "symbol": "USDT", "name": "Tether USD"},
    DAI_ADDR: {"price": 1.0, "symbol": "DAI", "name": "Dai Stablecoin"},
    DAI_ADDR2: {"price": 1.0, "symbol": "DAI", "name": "Dai Stablecoin"}
}

def get_cached_price(address):
    entry = price_cache.get(address.lower())
    if entry is None:
        return None
    if isinstance(entry, dict):
        return entry.get("price")
    return float(entry)

def update_price(address, price, symbol, name, is_treasury=False, treasury_owner=None):
    price_cache[address.lower()] = {
        "price": float(price),
        "symbol": symbol,
        "name": name,
        "is_treasury": is_treasury,
        "treasury_owner": treasury_owner
    }
    save_price_cache()

def load_price_cache():
    if os.path.exists(PRICE_CACHE_FILE):
        try:
            with open(PRICE_CACHE_FILE, "r") as f:
                loaded = json.load(f)
                for k, v in loaded.items():
                    if isinstance(v, dict):
                        price_cache[k.lower()] = v
                    else:
                        price_cache[k.lower()] = {"price": float(v), "symbol": "UNKNOWN", "name": "Unknown Token"}
            print(f"Loaded {len(price_cache)} price cache entries from {PRICE_CACHE_FILE}")
        except Exception as e:
            print(f"Error loading price cache: {e}")

def save_price_cache():
    try:
        with open(PRICE_CACHE_FILE, "w") as f:
            json.dump(price_cache, f, indent=4)
    except Exception as e:
        print(f"Error saving price cache: {e}")

# Token metadata cache
token_cache = {}

def get_token_info(w3, address):
    addr_lower = address.lower()
    if addr_lower in token_cache:
        return token_cache[addr_lower]
    
    contract = w3.eth.contract(address=Web3.to_checksum_address(address), abi=ERC20_ABI)
    try:
        name = contract.functions.name().call()
    except Exception:
        name = "Unknown Token"
    
    try:
        symbol = contract.functions.symbol().call()
    except Exception:
        symbol = "UNKNOWN"
        
    try:
        decimals = contract.functions.decimals().call()
    except Exception:
        decimals = 18
        
    # Check if this token is a Treasury Token
    is_treasury = False
    treasury_owner = None
    try:
        pk_minter = w3.eth.contract(address=Web3.to_checksum_address(PKMINTER_ADDRESS), abi=PKMINTER_ABI)
        owner = pk_minter.functions.GetTreasuryTokenOwner(Web3.to_checksum_address(address)).call()
        if owner != "0x0000000000000000000000000000000000000000":
            is_treasury = True
            treasury_owner = owner.lower()
            save_treasury_token(address, symbol, name, owner)
    except Exception:
        pass

    info = {
        "name": name, 
        "symbol": symbol, 
        "decimals": decimals, 
        "address": addr_lower,
        "is_treasury": is_treasury,
        "treasury_owner": treasury_owner
    }
    token_cache[addr_lower] = info
    return info

def get_pool_tokens(w3, address):
    contract = w3.eth.contract(address=Web3.to_checksum_address(address), abi=PAIR_ABI)
    try:
        t0 = contract.functions.token0().call()
        t1 = contract.functions.token1().call()
        return t0, t1
    except Exception:
        return None, None

def load_unresolved():
    if not os.path.exists(UNRESOLVED_FILE):
        return []
    try:
        with open(UNRESOLVED_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return []

def save_unresolved(swaps):
    try:
        with open(UNRESOLVED_FILE, "w") as f:
            json.dump(swaps, f, indent=4)
    except Exception as e:
        print(f"Error saving unresolved swaps: {e}")

def resolve_retroactive_prices():
    """Try to retroactively solve prices of unresolved swaps."""
    unresolved = load_unresolved()
    if not unresolved:
        return
        
    resolved_any = True
    remaining = []
    
    while resolved_any:
        resolved_any = False
        remaining = []
        
        for swap in unresolved:
            t0_addr = swap["token0"]["address"]
            t1_addr = swap["token1"]["address"]
            amt0 = swap["amount0"]
            amt1 = swap["amount1"]
            
            p0 = get_cached_price(t0_addr)
            p1 = get_cached_price(t1_addr)
            
            if p0 is not None and p1 is not None:
                # Both known, just clean up
                usd_val = amt0 * p0
                print(f"✨ Retroactive Resolution: Tx {swap['tx_hash']} swap of {swap['token0']['symbol']}/{swap['token1']['symbol']} valued at ${usd_val:.2f} USD")
                save_resolved({
                    "tx_hash": swap["tx_hash"],
                    "pool_address": swap["pool_address"],
                    "version": swap["version"],
                    "token0": swap["token0"],
                    "token1": swap["token1"],
                    "amount0": amt0,
                    "amount1": amt1,
                    "usd_value": usd_val,
                    "timestamp": time.time()
                })
                resolved_any = True
                continue
                
            elif p0 is not None:
                # Resolve token1 price
                if amt1 > 0:
                    p1_val = (amt0 * p0) / amt1
                    update_price(
                        t1_addr, 
                        p1_val, 
                        swap["token1"]["symbol"], 
                        swap["token1"]["name"],
                        swap["token1"].get("is_treasury", False),
                        swap["token1"].get("treasury_owner")
                    )
                    usd_val = amt0 * p0
                    print(f"✨ Retroactive Resolution: Calculated {swap['token1']['symbol']} price: ${p1_val:.8f} USD")
                    print(f"   Tx Hash: {swap['tx_hash']} | Swap Value: ${usd_val:.2f} USD")
                    save_resolved({
                        "tx_hash": swap["tx_hash"],
                        "pool_address": swap["pool_address"],
                        "version": swap["version"],
                        "token0": swap["token0"],
                        "token1": swap["token1"],
                        "amount0": amt0,
                        "amount1": amt1,
                        "usd_value": usd_val,
                        "timestamp": time.time()
                    })
                    resolved_any = True
                    continue
                    
            elif p1 is not None:
                # Resolve token0 price
                if amt0 > 0:
                    p0_val = (amt1 * p1) / amt0
                    update_price(
                        t0_addr, 
                        p0_val, 
                        swap["token0"]["symbol"], 
                        swap["token0"]["name"],
                        swap["token0"].get("is_treasury", False),
                        swap["token0"].get("treasury_owner")
                    )
                    usd_val = amt1 * p1
                    print(f"✨ Retroactive Resolution: Calculated {swap['token0']['symbol']} price: ${p0_val:.8f} USD")
                    print(f"   Tx Hash: {swap['tx_hash']} | Swap Value: ${usd_val:.2f} USD")
                    save_resolved({
                        "tx_hash": swap["tx_hash"],
                        "pool_address": swap["pool_address"],
                        "version": swap["version"],
                        "token0": swap["token0"],
                        "token1": swap["token1"],
                        "amount0": amt0,
                        "amount1": amt1,
                        "usd_value": usd_val,
                        "timestamp": time.time()
                    })
                    resolved_any = True
                    continue
            
            remaining.append(swap)
        unresolved = remaining

    save_unresolved(remaining)

def handle_detected_swap(tx_hash, pool_address, version, t0, t1, amt0_in, amt1_in, amt0_out, amt1_out):
    # Determine Sent/Recv amounts
    if amt0_in > 0:
        sent_token, recv_token = t0, t1
        sent_amt, recv_amt = amt0_in, amt1_out
    else:
        sent_token, recv_token = t1, t0
        sent_amt, recv_amt = amt1_in, amt0_out
        
    p_sent = get_cached_price(sent_token["address"])
    p_recv = get_cached_price(recv_token["address"])
    
    usd_val = None
    
    if p_sent is not None:
        usd_val = sent_amt * p_sent
        # Resolve/Update price of recv_token
        if recv_amt > 0:
            update_price(
                recv_token["address"], 
                usd_val / recv_amt, 
                recv_token["symbol"], 
                recv_token["name"],
                recv_token.get("is_treasury", False),
                recv_token.get("treasury_owner")
            )
            print(f"📈 Price update: {recv_token['symbol']} price set to ${get_cached_price(recv_token['address']):.8f} USD")
            
    elif p_recv is not None:
        usd_val = recv_amt * p_recv
        # Resolve/Update price of sent_token
        if sent_amt > 0:
            update_price(
                sent_token["address"], 
                usd_val / sent_amt, 
                sent_token["symbol"], 
                sent_token["name"],
                sent_token.get("is_treasury", False),
                sent_token.get("treasury_owner")
            )
            print(f"📈 Price update: {sent_token['symbol']} price set to ${get_cached_price(sent_token['address']):.8f} USD")
            
    print(f"\n------------------------------------------------------------")
    print(f"🌐 Swap Version: {version}")
    print(f"🔗 Tx Hash: {tx_hash}")
    print(f"🏫 Pool Address: {pool_address}")
    print(f"➡️ Sent: {sent_amt:.6f} {sent_token['symbol']} ({sent_token['name']}) [{sent_token['address']}]")
    print(f"⬅️ Recv: {recv_amt:.6f} {recv_token['symbol']} ({recv_token['name']}) [{recv_token['address']}]")
    
    if usd_val is not None:
        print(f"💵 Swap Value: ${usd_val:.4f} USD")
        save_resolved({
            "tx_hash": tx_hash,
            "pool_address": pool_address,
            "version": version,
            "token0": sent_token,
            "token1": recv_token,
            "amount0": sent_amt,
            "amount1": recv_amt,
            "usd_value": usd_val,
            "timestamp": time.time()
        })
        # Since we resolved a new price, try resolving pending swaps
        resolve_retroactive_prices()
    else:
        print(f"⚠️ USD Value: Unresolved (no price path to USD)")
        # Store in unresolved table
        unresolved = load_unresolved()
        unresolved.append({
            "tx_hash": tx_hash,
            "pool_address": pool_address,
            "version": version,
            "token0": sent_token,
            "token1": recv_token,
            "amount0": sent_amt,
            "amount1": recv_amt,
            "timestamp": time.time()
        })
        save_unresolved(unresolved)
        print(f"💾 Swap stored in {UNRESOLVED_FILE} for future resolution.")

def monitor_swap_events():
    load_price_cache()
    print(f"Connecting to PulseChain RPC: {RPC_URL} ...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Error: Failed to connect to PulseChain RPC endpoint.")
        sys.exit(1)
        
    print("Connected successfully. Monitoring blocks for PulseX Swap events...")
    
    last_block = w3.eth.block_number
    print(f"Starting tracking from block: {last_block}")
    
    while True:
        try:
            current_block = w3.eth.block_number
            if current_block > last_block:
                for block_num in range(last_block + 1, current_block + 1):
                    process_block(w3, block_num)
                last_block = current_block
            time.sleep(2)
        except KeyboardInterrupt:
            print("\nMonitoring stopped by user.")
            break
        except Exception as e:
            print(f"Error checking blocks: {e}")
            time.sleep(5)

def process_block(w3, block_num):
    # Fetch logs for Swap events in this block
    try:
        logs_v2 = w3.eth.get_logs({
            "fromBlock": block_num,
            "toBlock": block_num,
            "topics": [[SWAP_V2_HASH]]
        })
        
        logs_v3 = w3.eth.get_logs({
            "fromBlock": block_num,
            "toBlock": block_num,
            "topics": [[SWAP_V3_HASH]]
        })
        
        all_logs = [(log, "V2") for log in logs_v2] + [(log, "V3") for log in logs_v3]
        
        if not all_logs:
            return
            
        print(f"\n[Block {block_num}] Scanning logs... Found {len(all_logs)} swap(s)")
        
        for log, version in all_logs:
            tx_hash = log["transactionHash"].hex()
            pool_address = log["address"]
            
            t0_addr, t1_addr = get_pool_tokens(w3, pool_address)
            if not t0_addr or not t1_addr:
                continue
                
            t0 = get_token_info(w3, t0_addr)
            t1 = get_token_info(w3, t1_addr)
            
            if version == "V2":
                decoded = abi.decode(["uint256", "uint256", "uint256", "uint256"], log["data"])
                amt0_in, amt1_in, amt0_out, amt1_out = decoded
                
                val0_in = amt0_in / (10 ** t0["decimals"])
                val1_in = amt1_in / (10 ** t1["decimals"])
                val0_out = amt0_out / (10 ** t0["decimals"])
                val1_out = amt1_out / (10 ** t1["decimals"])
                
                handle_detected_swap(tx_hash, pool_address, "PulseX V2", t0, t1, val0_in, val1_in, val0_out, val1_out)
                    
            elif version == "V3":
                decoded = abi.decode(["int256", "int256", "uint160", "uint128", "int24"], log["data"])
                amt0, amt1 = decoded[0], decoded[1]
                
                val0 = abs(amt0) / (10 ** t0["decimals"])
                val1 = abs(amt1) / (10 ** t1["decimals"])
                
                if amt0 > 0: # token0 Sent, token1 Recv
                    handle_detected_swap(tx_hash, pool_address, "PulseX V3", t0, t1, val0, 0, 0, val1)
                else: # token1 Sent, token0 Recv
                    handle_detected_swap(tx_hash, pool_address, "PulseX V3", t0, t1, 0, val1, val0, 0)
                    
    except Exception as e:
        print(f"Error processing block {block_num} logs: {e}")

import argparse

# Router ABI for swapping
ROUTER_ABI = [
    {
        "inputs": [
            {"name": "amountIn", "type": "uint256"},
            {"name": "amountOutMin", "type": "uint256"},
            {"name": "path", "type": "address[]"},
            {"name": "to", "type": "address"},
            {"name": "deadline", "type": "uint256"}
        ],
        "name": "swapExactTokensForTokens",
        "outputs": [{"name": "amounts", "type": "uint256[]"}],
        "stateMutability": "nonpayable",
        "type": "function"
    },
    {
        "inputs": [
            {"name": "amountIn", "type": "uint256"},
            {"name": "path", "type": "address[]"}
        ],
        "name": "getAmountsOut",
        "outputs": [{"name": "amounts", "type": "uint256[]"}],
        "stateMutability": "view",
        "type": "function"
    }
]

# ERC20 Approve ABI
ERC20_APPROVE_ABI = [
    {"constant": False, "inputs": [{"name": "spender", "type": "address"}, {"name": "value", "type": "uint256"}], "name": "approve", "outputs": [{"name": "", "type": "bool"}], "type": "function"},
    {"constant": True, "inputs": [{"name": "owner", "type": "address"}, {"name": "spender", "type": "address"}], "name": "allowance", "outputs": [{"name": "", "type": "uint256"}], "type": "function"}
]

ROUTER_ADDRESS = "0x165C3410fC69F1857c278b5ce0ae3512720FD0E1"
WPLS_ADDRESS = "0xA1077a294dDE1B09bB078844df40758a5D0f9a27"

def execute_swap(w3, private_key, from_token, to_token, amount_in, slippage=0.5):
    """Execute a swap from from_token to to_token on PulseX V2."""
    from_addr = Web3.to_checksum_address(from_token)
    to_addr = Web3.to_checksum_address(to_token)
    router_addr = Web3.to_checksum_address(ROUTER_ADDRESS)
    
    account = w3.eth.account.from_key(private_key)
    sender_address = account.address
    print(f"Initiating swap from {sender_address}...")
    
    # 1. Get token info to format amount
    from_info = get_token_info(w3, from_addr)
    to_info = get_token_info(w3, to_addr)
    
    raw_amount_in = int(amount_in * (10 ** from_info["decimals"]))
    
    # 2. Derive swap path
    if from_addr.lower() == WPLS_ADDRESS.lower() or to_addr.lower() == WPLS_ADDRESS.lower():
        path = [from_addr, to_addr]
    else:
        path = [from_addr, Web3.to_checksum_address(WPLS_ADDRESS), to_addr]
        
    print(f"Swap route path: {' -> '.join([get_token_info(w3, p)['symbol'] for p in path])}")
    
    # 3. Check allowance and Approve if necessary
    token_contract = w3.eth.contract(address=from_addr, abi=ERC20_APPROVE_ABI)
    allowance = token_contract.functions.allowance(sender_address, router_addr).call()
    
    if allowance < raw_amount_in:
        print(f"Approving PulseX Router to spend {amount_in} {from_info['symbol']}...")
        approve_tx = token_contract.functions.approve(router_addr, raw_amount_in).build_transaction({
            'from': sender_address,
            'nonce': w3.eth.get_transaction_count(sender_address),
            'gasPrice': int(w3.eth.gas_price * 1.2)
        })
        signed_approve = w3.eth.account.sign_transaction(approve_tx, private_key)
        tx_hash = w3.eth.send_raw_transaction(signed_approve.raw_transaction)
        print(f"Approval Tx sent: {tx_hash.hex()}. Waiting for confirmation...")
        w3.eth.wait_for_transaction_receipt(tx_hash)
        print("Approval confirmed.")
        
    # 4. Perform Swap
    router_contract = w3.eth.contract(address=router_addr, abi=ROUTER_ABI)
    
    try:
        amounts_out = router_contract.functions.getAmountsOut(raw_amount_in, path).call()
        min_amount_out = int(amounts_out[-1] * (1 - slippage / 100))
        expected_out = amounts_out[-1] / (10 ** to_info["decimals"])
        print(f"Expected out: {expected_out:.6f} {to_info['symbol']}")
    except Exception as e:
        print(f"Warning: Could not fetch getAmountsOut ({e}). Setting amountOutMin to 0.")
        min_amount_out = 0
        
    deadline = int(time.time()) + 1200 # 20 minutes deadline
    
    swap_tx = router_contract.functions.swapExactTokensForTokens(
        raw_amount_in,
        min_amount_out,
        path,
        sender_address,
        deadline
    ).build_transaction({
        'from': sender_address,
        'nonce': w3.eth.get_transaction_count(sender_address),
        'gasLimit': 300000,
        'gasPrice': int(w3.eth.gas_price * 1.2)
    })
    
    signed_swap = w3.eth.account.sign_transaction(swap_tx, private_key)
    tx_hash = w3.eth.send_raw_transaction(signed_swap.raw_transaction)
    print(f"🚀 Swap transaction sent! Tx Hash: {tx_hash.hex()}")
    
    receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
    if receipt.status == 1:
        print("🎉 Swap executed successfully!")
    else:
        print("❌ Swap execution failed (reverted).")
    return receipt

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="PulseChain Oracle & PulseX Swapping CLI")
    parser.add_argument("--swap", action="store_true", help="Execute a swap on PulseX")
    parser.add_argument("--from-token", type=str, help="Address of token to sell")
    parser.add_argument("--to-token", type=str, help="Address of token to buy")
    parser.add_argument("--amount", type=float, help="Amount of from-token to sell")
    parser.add_argument("--private-key", type=str, help="Private key of the sender wallet")
    parser.add_argument("--slippage", type=float, default=0.5, help="Slippage tolerance in percentage (default: 0.5)")
    
    args = parser.parse_args()
    
    if args.swap:
        if not (args.from_token and args.to_token and args.amount and args.private_key):
            print("Error: --from-token, --to-token, --amount, and --private-key are required for swap.")
            sys.exit(1)
        w3 = Web3(Web3.HTTPProvider(RPC_URL))
        execute_swap(w3, args.private_key, args.from_token, args.to_token, args.amount, args.slippage)
    else:
        monitor_swap_events()
