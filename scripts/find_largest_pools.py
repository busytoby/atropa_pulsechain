#!/usr/bin/env python3
import sys
import json
import os
from web3 import Web3

RPC_URL = "https://rpc.pulsechain.com"
PULSEX_V2_FACTORY = "0x1715a3E4A142d8b698131108995174F37aEBA10D"

# Base asset definitions (addresses, names, symbols, decimals)
BASE_ASSETS = {
    "0xA1077a294dDE1B09bB078844df40758a5D0f9a27".lower(): {"symbol": "WPLS", "decimals": 18},
    "0x959C5Ad5C5Ad5c5ad5C5AD5C5ad5c5AD5c5AD5CD".lower(): {"symbol": "PLSX", "decimals": 18},
    "0x15D38573d2feeb82e7ad5187aB8c1D52810B1f07".lower(): {"symbol": "USDC (from Ethereum)", "decimals": 6},
    "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48".lower(): {"symbol": "USDC", "decimals": 6},
    "0xefD766cCb8C15E5E9F813af7b2809857Baa53A1f".lower(): {"symbol": "DAI (from Ethereum)", "decimals": 18},
    "0x6b175474e89094c44da98b954eedeac495271d0f".lower(): {"symbol": "DAI", "decimals": 18},
    "0x0Cb81b54A05e0547D2d08C4A9E273a7d4C72B9eB".lower(): {"symbol": "USDT (from Ethereum)", "decimals": 6},
    "0xdac17f958d2ee523a2206206994597c13d831ec7".lower(): {"symbol": "USDT", "decimals": 6},
    "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6".lower(): {"symbol": "Atropa", "decimals": 18},
    "0xd6c31bA0754C4383A41c0e9DF042C62b5e918f6d".lower(): {"symbol": "TeddyBear", "decimals": 18}
}

FACTORY_ABI = [
    {
        "constant": True,
        "inputs": [{"name": "tokenA", "type": "address"}, {"name": "tokenB", "type": "address"}],
        "name": "getPair",
        "outputs": [{"name": "pair", "type": "address"}],
        "payable": False,
        "stateMutability": "view",
        "type": "function"
    }
]

PAIR_ABI = [
    {"constant": True, "inputs": [], "name": "token0", "outputs": [{"name": "", "type": "address"}], "payable": False, "stateMutability": "view", "type": "function"},
    {"constant": True, "inputs": [], "name": "token1", "outputs": [{"name": "", "type": "address"}], "payable": False, "stateMutability": "view", "type": "function"},
    {"constant": True, "inputs": [], "name": "getReserves", "outputs": [{"name": "_reserve0", "type": "uint112"}, {"name": "_reserve1", "type": "uint112"}, {"name": "_blockTimestampLast", "type": "uint32"}], "payable": False, "stateMutability": "view", "type": "function"}
]

ERC20_ABI = [
    {"constant": True, "inputs": [], "name": "symbol", "outputs": [{"name": "", "type": "string"}], "payable": False, "stateMutability": "view", "type": "function"},
    {"constant": True, "inputs": [], "name": "name", "outputs": [{"name": "", "type": "string"}], "payable": False, "stateMutability": "view", "type": "function"},
    {"constant": True, "inputs": [], "name": "decimals", "outputs": [{"name": "", "type": "uint8"}], "payable": False, "stateMutability": "view", "type": "function"}
]

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/find_largest_pools.py <token_address>")
        sys.exit(1)
        
    token_addr = sys.argv[1].lower()
    
    print("Connecting to PulseChain...")
    w3 = Web3(Web3.HTTPProvider(RPC_URL))
    if not w3.is_connected():
        print("Failed to connect to PulseChain")
        sys.exit(1)
        
    # Get target token metadata
    token_checksum = Web3.to_checksum_address(token_addr)
    token_contract = w3.eth.contract(address=token_checksum, abi=ERC20_ABI)
    
    try:
        symbol = token_contract.functions.symbol().call()
        name = token_contract.functions.name().call()
        decimals = token_contract.functions.decimals().call()
    except Exception as e:
        print(f"Error fetching token info for {token_addr}: {e}")
        sys.exit(1)
        
    print(f"\nAnalyzing Liquidity Pools for: {symbol} ({name})")
    print(f"Token Address: {token_addr}")
    print(f"Decimals: {decimals}")
    print("=" * 60)
    
    factory_contract = w3.eth.contract(address=Web3.to_checksum_address(PULSEX_V2_FACTORY), abi=FACTORY_ABI)
    
    pools_found = []
    
    for base_addr, base_info in BASE_ASSETS.items():
        if base_addr == token_addr:
            continue
            
        try:
            pair_addr = factory_contract.functions.getPair(
                token_checksum, 
                Web3.to_checksum_address(base_addr)
            ).call()
            
            if pair_addr == "0x0000000000000000000000000000000000000000":
                continue
                
            pair_contract = w3.eth.contract(address=Web3.to_checksum_address(pair_addr), abi=PAIR_ABI)
            token0 = pair_contract.functions.token0().call().lower()
            token1 = pair_contract.functions.token1().call().lower()
            reserves = pair_contract.functions.getReserves().call()
            
            if token0 == token_addr:
                token_reserve = reserves[0]
                base_reserve = reserves[1]
            else:
                token_reserve = reserves[1]
                base_reserve = reserves[0]
                
            human_token_reserve = token_reserve / (10 ** decimals)
            human_base_reserve = base_reserve / (10 ** base_info["decimals"])
            
            pools_found.append({
                "pair_address": pair_addr.lower(),
                "base_symbol": base_info["symbol"],
                "base_address": base_addr,
                "token_reserve": human_token_reserve,
                "base_reserve": human_base_reserve
            })
            
        except Exception as e:
            # Squelch and continue
            pass
            
    # Sort pools by the size of the token reserve (liquidity depth)
    pools_found.sort(key=lambda x: x["token_reserve"], reverse=True)
    
    if not pools_found:
        print("No PulseX V2 liquidity pools found matching the standard base assets.")
    else:
        for idx, pool in enumerate(pools_found):
            print(f"Pool #{idx+1}: {symbol} / {pool['base_symbol']}")
            print(f"  Pool Contract: {pool['pair_address']}")
            print(f"  {symbol} Reserves: {pool['token_reserve']:,.4f}")
            print(f"  {pool['base_symbol']} Reserves: {pool['base_reserve']:,.4f}")
            print("-" * 60)

if __name__ == "__main__":
    main()
