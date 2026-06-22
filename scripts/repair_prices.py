import json
import os

PRICE_CACHE_FILE = "price_cache.json"
RESOLVED_FILE = "resolved_swaps.json"
UNRESOLVED_FILE = "unresolved_swaps.json"

USDC_ADDR = "0x15d38573d2feeb82e7ad5187ab8c1d52810b1f07"
USDC_ADDR2 = "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48"
USDT_ADDR = "0x0cb81b54a05e0547d2d08c4a9e273a7d4c72b9eb"
USDT_ADDR2 = "0xdac17f958d2ee523a2206206994597c13d831ec7"
DAI_ADDR = "0xefd766ccb8c15e5e9f813af7b2809857baa53a1f"
DAI_ADDR2 = "0x6b175474e89094c44da98b954eedeac495271d0f"

STABLECOINS = {USDC_ADDR, USDC_ADDR2, USDT_ADDR, USDT_ADDR2, DAI_ADDR, DAI_ADDR2}

def main():
    print("Starting price repair and transaction history rebuild...")
    
    # 1. Load price cache to preserve metadata
    price_cache = {}
    if os.path.exists(PRICE_CACHE_FILE):
        try:
            with open(PRICE_CACHE_FILE, "r") as f:
                price_cache = json.load(f)
        except Exception as e:
            print(f"Error loading price cache: {e}")
            
    # Reset all prices to 0.0/None, except stablecoins which are 1.0
    for k in list(price_cache.keys()):
        addr = k.lower()
        if addr in STABLECOINS:
            price_cache[addr]["price"] = 1.0
        else:
            price_cache[addr]["price"] = 0.0
            
    # Make sure all stablecoins are in the cache
    stablecoin_meta = {
        USDC_ADDR: {"symbol": "USDC", "name": "USD Coin from Ethereum"},
        USDC_ADDR2: {"symbol": "USDC", "name": "USD Coin"},
        USDT_ADDR: {"symbol": "USDT", "name": "Tether USD"},
        USDT_ADDR2: {"symbol": "USDT", "name": "Tether USD"},
        DAI_ADDR: {"symbol": "DAI", "name": "Dai Stablecoin"},
        DAI_ADDR2: {"symbol": "DAI", "name": "Dai Stablecoin"}
    }
    for addr, meta in stablecoin_meta.items():
        if addr not in price_cache:
            price_cache[addr] = {
                "price": 1.0,
                "symbol": meta["symbol"],
                "name": meta["name"],
                "is_treasury": False,
                "treasury_owner": None
            }
        else:
            price_cache[addr]["price"] = 1.0

    # 2. Load all swaps
    resolved_swaps = []
    if os.path.exists(RESOLVED_FILE):
        try:
            with open(RESOLVED_FILE, "r") as f:
                resolved_swaps = json.load(f)
        except Exception:
            pass
            
    unresolved_swaps = []
    if os.path.exists(UNRESOLVED_FILE):
        try:
            with open(UNRESOLVED_FILE, "r") as f:
                unresolved_swaps = json.load(f)
        except Exception:
            pass
            
    # Merge and deduplicate
    all_swaps = []
    seen = set()
    
    for swap in resolved_swaps + unresolved_swaps:
        if not isinstance(swap, dict):
            continue
        # Deduplication key
        tx_hash = swap.get("tx_hash", "").lower()
        t0_addr = swap.get("token0", {}).get("address", "").lower()
        t1_addr = swap.get("token1", {}).get("address", "").lower()
        amt0 = float(swap.get("amount0", 0.0))
        amt1 = float(swap.get("amount1", 0.0))
        timestamp = float(swap.get("timestamp", 0.0))
        
        dup_key = (tx_hash, t0_addr, t1_addr, amt0, amt1)
        if dup_key in seen:
            continue
        seen.add(dup_key)
        all_swaps.append(swap)
        
    # Sort chronologically by timestamp
    all_swaps.sort(key=lambda s: float(s.get("timestamp", 0.0)))
    print(f"Total unique swaps to process: {len(all_swaps)}")
    
    # 3. Replay loop
    resolved_list = []
    unresolved_list = []
    
    def get_cached_price(addr):
        addr_lower = addr.lower()
        entry = price_cache.get(addr_lower)
        if entry is None:
            return None
        price = entry.get("price", 0.0)
        if price == 0.0:
            return None
        return price

    def update_price(addr, price, symbol, name, is_treasury=False, treasury_owner=None):
        addr_lower = addr.lower()
        if addr_lower in STABLECOINS:
            # Keep stablecoins at 1.0
            price_cache[addr_lower]["price"] = 1.0
            return
        if addr_lower not in price_cache:
            price_cache[addr_lower] = {
                "price": float(price),
                "symbol": symbol,
                "name": name,
                "is_treasury": is_treasury,
                "treasury_owner": treasury_owner
            }
        else:
            price_cache[addr_lower]["price"] = float(price)

    # Process sequentially
    for swap in all_swaps:
        t0 = swap.get("token0", {})
        t1 = swap.get("token1", {})
        t0_addr = t0.get("address", "").lower()
        t1_addr = t1.get("address", "").lower()
        amt0 = float(swap.get("amount0", 0.0))
        amt1 = float(swap.get("amount1", 0.0))
        tx_hash = swap.get("tx_hash", "")
        pool_address = swap.get("pool_address", "")
        version = swap.get("version", "")
        timestamp = swap.get("timestamp", 0.0)
        
        p0 = get_cached_price(t0_addr)
        p1 = get_cached_price(t1_addr)
        
        usd_val = None
        if p0 is not None:
            usd_val = amt0 * p0
            if amt1 > 0:
                update_price(t1_addr, usd_val / amt1, t1.get("symbol"), t1.get("name"), t1.get("is_treasury", False), t1.get("treasury_owner"))
        elif p1 is not None:
            usd_val = amt1 * p1
            if amt0 > 0:
                update_price(t0_addr, usd_val / amt0, t0.get("symbol"), t0.get("name"), t0.get("is_treasury", False), t0.get("treasury_owner"))
                
        if usd_val is not None:
            resolved_list.append({
                "tx_hash": tx_hash,
                "pool_address": pool_address,
                "version": version,
                "token0": t0,
                "token1": t1,
                "amount0": amt0,
                "amount1": amt1,
                "usd_value": usd_val,
                "timestamp": timestamp,
                **({"is_nonukes": True} if swap.get("is_nonukes") else {})
            })
        else:
            unresolved_list.append(swap)
            
    print(f"Initial pass: resolved {len(resolved_list)} swaps, unresolved {len(unresolved_list)} swaps.")
    
    # 4. Retroactive resolution loop
    resolved_any = True
    while resolved_any:
        resolved_any = False
        remaining = []
        for swap in unresolved_list:
            t0 = swap.get("token0", {})
            t1 = swap.get("token1", {})
            t0_addr = t0.get("address", "").lower()
            t1_addr = t1.get("address", "").lower()
            amt0 = float(swap.get("amount0", 0.0))
            amt1 = float(swap.get("amount1", 0.0))
            
            p0 = get_cached_price(t0_addr)
            p1 = get_cached_price(t1_addr)
            
            if p0 is not None and p1 is not None:
                usd_val = amt0 * p0
                resolved_list.append({
                    "tx_hash": swap.get("tx_hash"),
                    "pool_address": swap.get("pool_address"),
                    "version": swap.get("version"),
                    "token0": t0,
                    "token1": t1,
                    "amount0": amt0,
                    "amount1": amt1,
                    "usd_value": usd_val,
                    "timestamp": swap.get("timestamp"),
                    **({"is_nonukes": True} if swap.get("is_nonukes") else {})
                })
                resolved_any = True
            elif p0 is not None:
                if amt1 > 0:
                    p1_val = (amt0 * p0) / amt1
                    update_price(t1_addr, p1_val, t1.get("symbol"), t1.get("name"), t1.get("is_treasury", False), t1.get("treasury_owner"))
                    usd_val = amt0 * p0
                    resolved_list.append({
                        "tx_hash": swap.get("tx_hash"),
                        "pool_address": swap.get("pool_address"),
                        "version": swap.get("version"),
                        "token0": t0,
                        "token1": t1,
                        "amount0": amt0,
                        "amount1": amt1,
                        "usd_value": usd_val,
                        "timestamp": swap.get("timestamp"),
                        **({"is_nonukes": True} if swap.get("is_nonukes") else {})
                    })
                    resolved_any = True
            elif p1 is not None:
                if amt0 > 0:
                    p0_val = (amt1 * p1) / amt0
                    update_price(t0_addr, p0_val, t0.get("symbol"), t0.get("name"), t0.get("is_treasury", False), t0.get("treasury_owner"))
                    usd_val = amt1 * p1
                    resolved_list.append({
                        "tx_hash": swap.get("tx_hash"),
                        "pool_address": swap.get("pool_address"),
                        "version": swap.get("version"),
                        "token0": t0,
                        "token1": t1,
                        "amount0": amt0,
                        "amount1": amt1,
                        "usd_value": usd_val,
                        "timestamp": swap.get("timestamp"),
                        **({"is_nonukes": True} if swap.get("is_nonukes") else {})
                    })
                    resolved_any = True
            else:
                remaining.append(swap)
        unresolved_list = remaining

    print(f"After retroactive loops: resolved {len(resolved_list)} swaps, unresolved {len(unresolved_list)} swaps.")
    
    # Sort resolved swaps chronologically again to make sure they are written correctly
    resolved_list.sort(key=lambda s: float(s.get("timestamp", 0.0)))
    
    # Save files
    # Note: monitor_pulsex.py slices to last 200 swaps, let's do the same
    with open(RESOLVED_FILE, "w") as f:
        json.dump(resolved_list[-200:], f, indent=4)
        
    with open(UNRESOLVED_FILE, "w") as f:
        json.dump(unresolved_list, f, indent=4)
        
    with open(PRICE_CACHE_FILE, "w") as f:
        json.dump(price_cache, f, indent=4)
        
    print("Successfully repaired prices and updated JSON files!")

if __name__ == "__main__":
    main()
