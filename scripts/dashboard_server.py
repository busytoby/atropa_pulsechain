#!/usr/bin/env python3
import os
import json
import http.server
import socketserver
import urllib.parse

PORT = 8080
PRICE_CACHE_FILE = "price_cache.json"
UNRESOLVED_FILE = "unresolved_swaps.json"
RESOLVED_FILE = "resolved_swaps.json"

class DashboardHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # Suppress request logging to keep stderr clean
        pass

    def do_GET(self):
        if self.path == '/api/data':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            prices = {}
            if os.path.exists(PRICE_CACHE_FILE):
                try:
                    with open(PRICE_CACHE_FILE, "r") as f:
                        prices = json.load(f)
                except Exception:
                    pass
                    
            unresolved = []
            if os.path.exists(UNRESOLVED_FILE):
                try:
                    with open(UNRESOLVED_FILE, "r") as f:
                        unresolved = json.load(f)
                except Exception:
                    pass
                    
            resolved = []
            if os.path.exists(RESOLVED_FILE):
                try:
                    with open(RESOLVED_FILE, "r") as f:
                        resolved = json.load(f)
                except Exception:
                    pass
            
            treasury_tokens = {}
            import glob
            for fpath in glob.glob("treasury_tokens_*.json"):
                try:
                    with open(fpath, "r") as f:
                        treasury_tokens.update(json.load(f))
                except Exception:
                    pass
            
            response = {
                "prices": prices,
                "unresolved": unresolved,
                "resolved": resolved,
                "treasury_tokens": treasury_tokens
            }
            self.wfile.write(json.dumps(response, indent=2).encode('utf-8'))
        elif self.path == '/api/nonukes':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            pools = {}
            reserves = {}
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools = json.load(f)
                except Exception:
                    pass
            # Load reserves from scratch directory
            res_path = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json"
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves = json.load(f)
                except Exception:
                    pass
            self.wfile.write(json.dumps({"success": True, "pools": pools, "reserves": reserves}, indent=2).encode('utf-8'))
        elif self.path.startswith('/api/pools'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            
            if address:
                address = address.lower()
                from web3 import Web3
                w3 = Web3(Web3.HTTPProvider("https://rpc.pulsechain.com"))
                
                # Setup contracts
                token_checksum = Web3.to_checksum_address(address)
                factory_checksum = Web3.to_checksum_address("0x1715a3E4A142d8b698131108995174F37aEBA10D")
                
                # ABIs
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
                
                BASE_ASSETS = {
                    "0xA1077a294dDE1B09bB078844df40758a5D0f9a27".lower(): {"symbol": "WPLS", "decimals": 18},
                    "0x959C5Ad5C5Ad5c5ad5C5AD5C5AD5C5ad5c5AD5CD".lower(): {"symbol": "PLSX", "decimals": 18},
                    "0x15D38573d2feeb82e7ad5187aB8c1D52810B1f07".lower(): {"symbol": "USDC (from Ethereum)", "decimals": 6},
                    "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48".lower(): {"symbol": "USDC", "decimals": 6},
                    "0xefD766cCb8C15E5E9F813af7b2809857Baa53A1f".lower(): {"symbol": "DAI (from Ethereum)", "decimals": 18},
                    "0x6b175474e89094c44da98b954eedeac495271d0f".lower(): {"symbol": "DAI", "decimals": 18},
                    "0x0Cb81b54A05e0547D2d08C4A9E273a7d4C72B9eB".lower(): {"symbol": "USDT (from Ethereum)", "decimals": 6},
                    "0xdac17f958d2ee523a2206206994597c13d831ec7".lower(): {"symbol": "USDT", "decimals": 6},
                    "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6".lower(): {"symbol": "Atropa", "decimals": 18},
                    "0xd6c31bA0754C4383A41c0e9DF042C62b5e918f6d".lower(): {"symbol": "TeddyBear", "decimals": 18}
                }
                
                pools_found = []
                try:
                    token_contract = w3.eth.contract(address=token_checksum, abi=ERC20_ABI)
                    decimals = token_contract.functions.decimals().call()
                    symbol = token_contract.functions.symbol().call()
                    name = token_contract.functions.name().call()
                    
                    # PulseX V1 Factory: 0x29ea7545def87022badc76323f373ea1e707c523
                    factories = {
                        "PulseX V2": "0x1715a3E4A142d8b698131108995174F37aEBA10D",
                        "PulseX V1": "0x29ea7545def87022badc76323f373ea1e707c523"
                    }
                    
                    for version_name, factory_addr in factories.items():
                        factory_contract = w3.eth.contract(address=Web3.to_checksum_address(factory_addr), abi=FACTORY_ABI)
                        for base_addr, base_info in BASE_ASSETS.items():
                            if base_addr == address:
                                continue
                            try:
                                pair_addr = factory_contract.functions.getPair(
                                    token_checksum,
                                    Web3.to_checksum_address(base_addr)
                                ).call()
                                
                                if pair_addr != "0x0000000000000000000000000000000000000000":
                                    pair_contract = w3.eth.contract(address=Web3.to_checksum_address(pair_addr), abi=PAIR_ABI)
                                    token0 = pair_contract.functions.token0().call().lower()
                                    reserves = pair_contract.functions.getReserves().call()
                                    
                                    if token0 == address:
                                        token_reserve = reserves[0]
                                        base_reserve = reserves[1]
                                    else:
                                        token_reserve = reserves[1]
                                        base_reserve = reserves[0]
                                        
                                    pools_found.append({
                                        "pair_address": pair_addr.lower(),
                                        "base_symbol": base_info["symbol"],
                                        "base_address": base_addr,
                                        "version": version_name,
                                        "token_reserve": token_reserve / (10 ** decimals),
                                        "base_reserve": base_reserve / (10 ** base_info["decimals"])
                                    })
                            except Exception:
                                pass
                    
                    # Sort pools by token reserve
                    pools_found.sort(key=lambda x: x["token_reserve"], reverse=True)
                    
                    response = {
                        "success": True,
                        "token": {
                            "address": address,
                            "symbol": symbol,
                            "name": name,
                            "decimals": decimals
                        },
                        "pools": pools_found
                    }
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    self.wfile.write(json.dumps(response, indent=2).encode('utf-8'))
                    return
                except Exception as e:
                    self.send_response(500)
                    self.end_headers()
                    self.wfile.write(json.dumps({"success": False, "error": str(e)}).encode('utf-8'))
                    return
            
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b"Invalid request")
        elif self.path.startswith('/api/ignore'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            ignored_val = params.get('ignored', ['true'])[0].lower() == 'true'
            
            if address:
                address = address.lower()
                import glob
                target_fpath = None
                tokens = {}
                
                # Search all files to find which files have the token and update them
                found_any = False
                for fpath in glob.glob("treasury_tokens_*.json"):
                    try:
                        with open(fpath, "r") as f:
                            data = json.load(f)
                        if address in data:
                            data[address]["ignored"] = ignored_val
                            with open(fpath, "w") as f:
                                json.dump(data, f, indent=4)
                            found_any = True
                    except Exception:
                        pass
                
                if not found_any:
                    # Default to unknown_minter if not found anywhere
                    target_fpath = "treasury_tokens_unknown_minter.json"
                    tokens = {}
                    if os.path.exists(target_fpath):
                        try:
                            with open(target_fpath, "r") as f:
                                tokens = json.load(f)
                        except Exception:
                            pass
                    tokens[address] = {
                        "address": address,
                        "symbol": "UNKNOWN",
                        "name": "Unknown Token",
                        "owner": None,
                        "minter_name": "Unknown Minter",
                        "ignored": ignored_val
                    }
                    try:
                        with open(target_fpath, "w") as f:
                            json.dump(tokens, f, indent=4)
                    except Exception:
                        pass
                
                try:
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    self.wfile.write(json.dumps({"success": True, "address": address, "ignored": ignored_val}).encode('utf-8'))
                    return
                except Exception as e:
                    print(f"Error sending ignore response: {e}")
            
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b"Invalid request")
        elif self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_CONTENT.encode('utf-8'))
        else:
            self.send_error(404, "File not found")

HTML_CONTENT = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PulseChain Price Oracle Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #0b0f19;
            --panel-bg: rgba(17, 25, 40, 0.75);
            --panel-border: rgba(255, 255, 255, 0.08);
            --text-color: #f3f4f6;
            --text-muted: #9ca3af;
            --primary: #8b5cf6;
            --primary-glow: rgba(139, 92, 246, 0.4);
            --success: #10b981;
            --success-glow: rgba(16, 185, 129, 0.3);
            --warning: #f59e0b;
            --danger: #ef4444;
            --glow-color: rgba(99, 102, 241, 0.15);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Outfit', sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            background-image: 
                radial-gradient(at 0% 0%, rgba(99, 102, 241, 0.15) 0px, transparent 50%),
                radial-gradient(at 100% 100%, rgba(139, 92, 246, 0.12) 0px, transparent 50%),
                radial-gradient(at 50% 50%, rgba(16, 185, 129, 0.05) 0px, transparent 50%);
            background-attachment: fixed;
            min-height: 100vh;
            padding: 2rem;
            overflow-x: hidden;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2.5rem;
            padding: 1.5rem;
            background: var(--panel-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--panel-border);
            border-radius: 20px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.3);
        }

        .logo-section {
            display: flex;
            align-items: center;
            gap: 1rem;
        }

        .pulse-indicator {
            width: 14px;
            height: 14px;
            background-color: var(--success);
            border-radius: 50%;
            position: relative;
            box-shadow: 0 0 10px var(--success);
            animation: pulse-ring 2s infinite;
        }

        @keyframes pulse-ring {
            0% {
                transform: scale(0.95);
                box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.7);
            }
            70% {
                transform: scale(1);
                box-shadow: 0 0 0 10px rgba(16, 185, 129, 0);
            }
            100% {
                transform: scale(0.95);
                box-shadow: 0 0 0 0 rgba(16, 185, 129, 0);
            }
        }

        .title {
            font-size: 1.8rem;
            font-weight: 800;
            background: linear-gradient(135deg, #a78bfa, #8b5cf6, #34d399);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .subtitle {
            font-size: 0.9rem;
            color: var(--text-muted);
            margin-top: 0.2rem;
        }

        .status-badge {
            background: rgba(16, 185, 129, 0.1);
            color: var(--success);
            border: 1px solid rgba(16, 185, 129, 0.2);
            padding: 0.4rem 1rem;
            border-radius: 30px;
            font-size: 0.85rem;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 2rem;
            margin-bottom: 2rem;
        }

        .card {
            background: var(--panel-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--panel-border);
            border-radius: 20px;
            padding: 1.5rem;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.2);
            transition: transform 0.3s ease, border-color 0.3s ease;
        }

        .card:hover {
            transform: translateY(-4px);
            border-color: rgba(139, 92, 246, 0.25);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1.2rem;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
            padding-bottom: 0.8rem;
        }

        .card-title {
            font-size: 1.1rem;
            font-weight: 600;
            color: #fff;
            display: flex;
            align-items: center;
            gap: 0.6rem;
        }

        .stats-num {
            font-size: 2.5rem;
            font-weight: 800;
            margin: 0.5rem 0;
            background: linear-gradient(135deg, #fff, var(--text-muted));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .stats-label {
            font-size: 0.85rem;
            color: var(--text-muted);
        }

        /* Lists and Tables */
        .list-container {
            max-height: 400px;
            overflow-y: auto;
            scrollbar-width: thin;
            scrollbar-color: rgba(255, 255, 255, 0.1) transparent;
        }

        .list-container::-webkit-scrollbar {
            width: 6px;
        }

        .list-container::-webkit-scrollbar-thumb {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
        }

        .table {
            width: 100%;
            border-collapse: collapse;
            table-layout: fixed;
        }

        .table th, .table td {
            text-align: left;
            padding: 0.8rem 0.5rem;
            font-size: 0.85rem;
            word-wrap: break-word;
            word-break: break-all;
            white-space: normal;
        }

        .table th {
            color: var(--text-muted);
            font-weight: 600;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .table tr:not(:last-child) {
            border-bottom: 1px solid rgba(255, 255, 255, 0.03);
        }

        .table tr:hover {
            background: rgba(255, 255, 255, 0.02);
        }

        .address-link {
            color: var(--primary);
            text-decoration: none;
            font-family: monospace;
            transition: color 0.2s ease;
        }

        .address-link:hover {
            color: #a78bfa;
            text-decoration: underline;
        }

        .badge {
            padding: 0.2rem 0.6rem;
            border-radius: 6px;
            font-size: 0.75rem;
            font-weight: 600;
        }

        .badge-resolved {
            background: rgba(16, 185, 129, 0.15);
            color: var(--success);
        }

        .badge-unresolved {
            background: rgba(245, 158, 11, 0.15);
            color: var(--warning);
        }

        .search-bar {
            width: 100%;
            background: rgba(255, 255, 255, 0.04);
            border: 1px solid var(--panel-border);
            padding: 0.8rem 1.2rem;
            border-radius: 12px;
            color: #fff;
            font-family: inherit;
            font-size: 0.9rem;
            margin-bottom: 1rem;
            outline: none;
            transition: border-color 0.3s ease;
        }

        .search-bar:focus {
            border-color: var(--primary);
            background: rgba(255, 255, 255, 0.06);
        }

        .token-symbol-glow {
            font-weight: 800;
            color: #34d399;
            text-shadow: 0 0 10px rgba(52, 211, 153, 0.2);
        }

        /* Timeline styles */
        .timeline-item {
            padding: 1rem;
            border-left: 2px solid var(--primary);
            margin-left: 0.5rem;
            position: relative;
            background: rgba(255, 255, 255, 0.02);
            border-radius: 0 12px 12px 0;
            margin-bottom: 1rem;
        }

        .timeline-item::before {
            content: '';
            width: 10px;
            height: 10px;
            background: var(--primary);
            border-radius: 50%;
            position: absolute;
            left: -6px;
            top: 1.4rem;
            box-shadow: 0 0 8px var(--primary);
        }

        .timeline-time {
            font-size: 0.75rem;
            color: var(--text-muted);
            margin-bottom: 0.4rem;
        }

        .timeline-title {
            font-size: 0.95rem;
            font-weight: 600;
            margin-bottom: 0.3rem;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .timeline-val {
            color: var(--success);
            font-weight: 700;
        }

        .timeline-details {
            font-size: 0.85rem;
            color: var(--text-muted);
        }

        .btn {
            padding: 0.3rem 0.8rem;
            border-radius: 8px;
            font-size: 0.8rem;
            font-weight: 600;
            cursor: pointer;
            border: 1px solid transparent;
            transition: all 0.2s ease;
        }
        .btn-ignore {
            background: rgba(239, 68, 68, 0.15);
            color: var(--danger);
            border-color: rgba(239, 68, 68, 0.25);
        }
        .btn-ignore:hover {
            background: rgba(239, 68, 68, 0.25);
            box-shadow: 0 0 8px rgba(239, 68, 68, 0.2);
        }
        .btn-track {
            background: rgba(16, 185, 129, 0.15);
            color: var(--success);
            border-color: rgba(16, 185, 129, 0.25);
        }
        .btn-track:hover {
            background: rgba(16, 185, 129, 0.25);
            box-shadow: 0 0 8px rgba(16, 185, 129, 0.2);
        }
        .tabs-header {
            display: flex;
            gap: 0.4rem;
            margin-bottom: 0.8rem;
            border-bottom: 1px solid rgba(255, 255, 255, 0.08);
            padding-bottom: 0.4rem;
            overflow-x: auto;
            scrollbar-width: none;
        }
        .tabs-header::-webkit-scrollbar {
            display: none;
        }
        .tab-btn {
            background: transparent;
            border: none;
            color: var(--text-muted);
            padding: 0.4rem 0.8rem;
            font-family: inherit;
            font-size: 0.8rem;
            font-weight: 600;
            cursor: pointer;
            border-radius: 8px;
            transition: all 0.2s ease;
            white-space: nowrap;
        }
        .tab-btn:hover {
            color: #fff;
            background: rgba(255, 255, 255, 0.04);
        }
        .tab-btn.active {
            color: #fbbf24;
            background: rgba(251, 191, 36, 0.08);
            box-shadow: 0 0 12px rgba(251, 191, 36, 0.1);
        }

        /* Modal Styles */
        .modal-overlay {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(11, 15, 25, 0.8);
            backdrop-filter: blur(8px);
            display: none;
            justify-content: center;
            align-items: center;
            z-index: 1000;
            opacity: 0;
            transition: opacity 0.3s ease;
        }
        .modal-overlay.active {
            display: flex;
            opacity: 1;
        }
        .modal-content {
            background: rgba(17, 25, 40, 0.95);
            border: 1px solid rgba(255, 255, 255, 0.15);
            box-shadow: 0 20px 50px rgba(0, 0, 0, 0.5), 0 0 30px rgba(139, 92, 246, 0.15);
            border-radius: 24px;
            width: 90%;
            max-width: 700px;
            padding: 2rem;
            transform: scale(0.9);
            transition: transform 0.3s ease;
            position: relative;
        }
        .modal-overlay.active .modal-content {
            transform: scale(1);
        }
        .modal-close {
            position: absolute;
            top: 1.2rem;
            right: 1.5rem;
            background: none;
            border: none;
            color: var(--text-muted);
            font-size: 1.5rem;
            cursor: pointer;
            transition: color 0.2s;
        }
        .modal-close:hover {
            color: #fff;
        }
        .modal-title {
            font-size: 1.4rem;
            font-weight: 800;
            color: #fff;
            margin-bottom: 0.5rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        .modal-subtitle {
            font-size: 0.85rem;
            color: var(--text-muted);
            margin-bottom: 1.5rem;
            word-break: break-all;
        }
        .modal-loader {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 3rem;
            gap: 1rem;
            color: var(--text-muted);
        }
        .spinner {
            width: 40px;
            height: 40px;
            border: 4px solid rgba(255, 255, 255, 0.1);
            border-top-color: var(--primary);
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
        .clickable-token {
            cursor: pointer;
            transition: transform 0.2s;
        }
        .clickable-token:hover {
            transform: translateX(4px);
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="logo-section">
            <div class="pulse-indicator"></div>
            <div>
                <h1 class="title">PulseChain Price Oracle</h1>
                <div class="subtitle">Real-Time PulseX Liquidity & Swap Resolver</div>
            </div>
        </div>
        <div class="status-badge">
            <span style="display:inline-block; width:8px; height:8px; background-color:var(--success); border-radius:50%"></span>
            Monitoring Block Logs
        </div>
    </div>

    <div class="grid">
        <!-- Stats Card 1 -->
        <div class="card">
            <div class="card-title">💵 Market Coverage</div>
            <div class="stats-num" id="stat-tokens-count">0</div>
            <div class="stats-label">Unique Currencies Tracked and Priced in USD</div>
        </div>
        <!-- Stats Card 2 -->
        <div class="card">
            <div class="card-title">⏳ Resolution Queue</div>
            <div class="stats-num" id="stat-unresolved-count" style="color: var(--warning)">0</div>
            <div class="stats-label">Unresolved Trades Awaiting Price Paths</div>
        </div>
        <!-- Stats Card 3 -->
        <div class="card">
            <div class="card-title">🚀 Swaps Resolved</div>
            <div class="stats-num" id="stat-resolved-count" style="color: var(--success)">0</div>
            <div class="stats-label">Total Swap Paths Successfully Coordinated</div>
        </div>
    </div>

    <div class="grid" style="grid-template-columns: 2fr 1.3fr">
        <!-- Main Price Catalog -->
        <div class="card">
            <div class="card-header">
                <div class="card-title">📋 Live USD Token Catalog</div>
                <div class="badge badge-resolved" id="catalog-count">0 Tokens</div>
            </div>
            <input type="text" id="catalog-search" class="search-bar" placeholder="Filter by Name, Symbol, or Address...">
            <div class="list-container" style="max-height: 480px">
                <table class="table" id="catalog-table">
                    <thead>
                        <tr>
                            <th onclick="setSort('symbol')" style="cursor: pointer; user-select: none;">Token<span id="sort-symbol"> ⇅</span></th>
                            <th onclick="setSort('address')" style="cursor: pointer; user-select: none;">Address<span id="sort-address"> ⇅</span></th>
                            <th onclick="setSort('price')" style="cursor: pointer; user-select: none;">USD Price<span id="sort-price"> ⇅</span></th>
                            <th onclick="setSort('volume')" style="cursor: pointer; user-select: none;">Traded Volume<span id="sort-volume" style="color: var(--primary)"> ▼</span></th>
                            <th onclick="setSort('amount')" style="cursor: pointer; user-select: none;">Amount for $1.00 USD<span id="sort-amount"> ⇅</span></th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="5" style="text-align: center; color: var(--text-muted)">Loading oracle prices...</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Recent Realtime Resolutions -->
        <div class="card">
            <div class="card-header">
                <div class="card-title">⚡ Real-Time Resolution Activity</div>
            </div>
            <div class="list-container" id="timeline-list" style="max-height: 540px">
                <div style="text-align: center; color: var(--text-muted); padding: 2rem">Waiting for events...</div>
            </div>
        </div>
    </div>

    <div class="grid" style="grid-template-columns: 1.2fr 1fr">
        <!-- Unresolved Swaps Table -->
        <div class="card">
            <div class="card-header">
                <div class="card-title" style="color: var(--warning)">⏳ Pending Resolution Queue</div>
                <div class="badge badge-unresolved" id="pending-count">0 Swap events</div>
            </div>
            <div class="list-container" style="max-height: 300px">
                <table class="table" id="unresolved-table">
                    <thead>
                        <tr>
                            <th>Tx Hash</th>
                            <th>From Token</th>
                            <th>To Token</th>
                            <th>Swap Amount</th>
                            <th>Timestamp</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="5" style="text-align: center; color: var(--text-muted)">No pending swaps in queue.</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Treasury Token Registry Card -->
        <div class="card">
            <div class="card-header">
                <div class="card-title" style="color: #fbbf24">👑 Treasury Token Registry</div>
                <div class="badge badge-resolved" id="treasury-tokens-count" style="background: rgba(251, 191, 36, 0.15); color: #fbbf24">0 Minted</div>
            </div>
            <div style="margin-bottom: 0.8rem; font-size: 0.85rem; color: var(--text-muted)">
                Active tracking of standard tokens minted by minters. Ignored tokens are hidden from the price catalog.
            </div>
            <div class="tabs-header" id="treasury-tabs-header"></div>
            <div class="list-container" id="treasury-registry-container" style="max-height: 420px; padding-right: 0.3rem;">
                <div style="text-align: center; color: var(--text-muted); padding: 2rem">No treasury tokens identified yet.</div>
            </div>
        </div>
    </div>

    <!-- LP Details Modal -->
    <div class="modal-overlay" id="lp-modal" onclick="closeLpModal(event)">
        <div class="modal-content" onclick="event.stopPropagation()">
            <button class="modal-close" onclick="closeLpModal(event)">&times;</button>
            <div class="modal-title" id="lp-modal-title">Token LP Reserves</div>
            <div class="modal-subtitle" id="lp-modal-subtitle">0x00000...</div>
            <div id="lp-modal-body">
                <!-- Dynamic Content or Loader -->
            </div>
        </div>
    </div>

    <script>
        let cachedData = { prices: {}, unresolved: [], resolved: [] };
        let activeMinterTab = null;

        function selectMinterTab(minterName) {
            activeMinterTab = minterName;
            updateDashboard();
        }

        async function fetchData() {
            try {
                const response = await fetch('/api/data');
                if (!response.ok) return;
                const data = await response.json();
                
                cachedData = data;
                updateDashboard();
            } catch (err) {
                console.error("Failed fetching data:", err);
            }
        }

        function formatAddress(addr) {
            if (!addr) return '';
            return addr.slice(0, 6) + '...' + addr.slice(-4);
        }

        function updateDashboard() {
            // Update Stats
            const priceEntries = Object.entries(cachedData.prices);
            document.getElementById('stat-tokens-count').innerText = priceEntries.length;
            document.getElementById('stat-unresolved-count').innerText = cachedData.unresolved.length;
            document.getElementById('stat-resolved-count').innerText = cachedData.resolved.length;
            document.getElementById('catalog-count').innerText = `${priceEntries.length} Tokens`;
            document.getElementById('pending-count').innerText = `${cachedData.unresolved.length} Swap events`;

            // Update Price Catalog
            renderCatalog();

            // Update Realtime Timeline
            const timeline = document.getElementById('timeline-list');
            if (cachedData.resolved.length === 0) {
                timeline.innerHTML = '<div style="text-align: center; color: var(--text-muted); padding: 2rem">No resolved swap events recorded.</div>';
            } else {
                timeline.innerHTML = cachedData.resolved.slice().reverse().map(item => {
                    const t = new Date(item.timestamp * 1000).toLocaleTimeString();
                    return `
                        <div class="timeline-item">
                            <div class="timeline-time">${t}</div>
                            <div class="timeline-title">
                                <span>${item.token0.symbol} ➡️ ${item.token1.symbol}</span>
                                <span class="timeline-val">$${parseFloat(item.usd_value).toFixed(2)} USD</span>
                            </div>
                            <div class="timeline-details">
                                Tx: <a class="address-link" target="_blank" href="https://otter.pulsechain.com/tx/0x${item.tx_hash}">${formatAddress(item.tx_hash)}</a><br>
                                Amount: ${parseFloat(item.amount0).toLocaleString()} ${item.token0.symbol} ➡️ ${parseFloat(item.amount1).toLocaleString()} ${item.token1.symbol}
                            </div>
                        </div>
                    `;
                }).join('');
            }

            // Update Unresolved Queue Table
            const unresolvedBody = document.querySelector('#unresolved-table tbody');
            if (cachedData.unresolved.length === 0) {
                unresolvedBody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: var(--text-muted)">No pending swaps in queue.</td></tr>';
            } else {
                unresolvedBody.innerHTML = cachedData.unresolved.map(item => {
                    const t = new Date(item.timestamp * 1000).toLocaleTimeString();
                    return `
                        <tr>
                            <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/tx/0x${item.tx_hash}">${formatAddress(item.tx_hash)}</a></td>
                            <td><span class="token-symbol-glow">${item.token0.symbol}</span> <span style="font-size:0.8rem; color:var(--text-muted)">(${item.token0.name})</span></td>
                            <td><span class="token-symbol-glow">${item.token1.symbol}</span> <span style="font-size:0.8rem; color:var(--text-muted)">(${item.token1.name})</span></td>
                            <td>${parseFloat(item.amount0).toLocaleString()} ➡️ ${parseFloat(item.amount1).toLocaleString()}</td>
                            <td>${t}</td>
                        </tr>
                    `;
                }).join('');
            }

            // Update Treasury Token Registry Card (grouped by minter tabs)
            const tabsHeader = document.getElementById('treasury-tabs-header');
            const registryContainer = document.getElementById('treasury-registry-container');
            const treasuryEntries = Object.entries(cachedData.treasury_tokens || {});
            
            // Update treasury count badge
            const activeCount = treasuryEntries.filter(([a, t]) => t.ignored !== true).length;
            document.getElementById('treasury-tokens-count').innerText = `${activeCount} Tracked`;

            if (treasuryEntries.length === 0) {
                tabsHeader.innerHTML = '';
                registryContainer.innerHTML = '<div style="text-align: center; color: var(--text-muted); padding: 2rem">No treasury tokens identified yet.</div>';
            } else {
                // Group by minter name
                const grouped = {};
                
                // Pre-define preferred minters order to ensure consistent layout
                const preferredMinters = [
                    "PKIMinter",
                    "TreasuryMinter",
                    "FederalMinter",
                    "BureauMinter",
                    "IndexMinter",
                    "PersonalMinter",
                    "MRPKMinter",
                    "Unknown Minter"
                ];
                
                preferredMinters.forEach(m => grouped[m] = []);
                
                treasuryEntries.forEach(([addr, token]) => {
                    const minterName = token.minter_name || 'Unknown Minter';
                    if (!grouped[minterName]) {
                        grouped[minterName] = [];
                    }
                    grouped[minterName].push([addr, token]);
                });
                
                // Find all minters that actually have active tokens
                const activeMinters = preferredMinters.filter(m => grouped[m] && grouped[m].filter(([a, t]) => t.ignored !== true).length > 0);
                
                // Set default active tab if none selected or if selected tab has no tokens
                if (!activeMinterTab || !activeMinters.includes(activeMinterTab)) {
                    activeMinterTab = activeMinters[0] || null;
                }
                
                // Render tab buttons
                tabsHeader.innerHTML = activeMinters.map(minterName => {
                    const count = grouped[minterName].filter(([a, t]) => t.ignored !== true).length;
                    const isActive = minterName === activeMinterTab;
                    return `<button class="tab-btn ${isActive ? 'active' : ''}" onclick="selectMinterTab('${minterName}')">${minterName} (${count})</button>`;
                }).join('');
                
                if (activeMinterTab && grouped[activeMinterTab]) {
                    const tokens = grouped[activeMinterTab];
                    
                    // Sort tokens within group by symbol
                    tokens.sort((a, b) => {
                        const symA = (a[1].symbol || '').toLowerCase();
                        const symB = (b[1].symbol || '').toLowerCase();
                        return symA.localeCompare(symB);
                    });
                    
                    const rowsHtml = tokens
                        .filter(([addr, token]) => token.ignored !== true)
                        .map(([addr, token]) => {
                        const isIgnored = token.ignored === true;
                        const name = token.name || 'Unknown';
                        const symbol = token.symbol || 'UNKNOWN';
                        
                        const displaySymbol = isIgnored ? `<del style="color: var(--text-muted)">${symbol}</del>` : `👑 <span class="token-symbol-glow" style="color: #fbbf24; text-shadow: 0 0 10px rgba(251, 191, 36, 0.4)">${symbol}</span>`;
                        const displayName = isIgnored ? `<del style="color: var(--text-muted)">${name}</del>` : name;
                        
                        const buttonText = isIgnored ? "Track" : "Ignore";
                        const buttonClass = isIgnored ? "btn-track" : "btn-ignore";
                        const nextIgnored = !isIgnored;

                        const parentSymbol = token.parent_symbol || 'None';
                        const parentAddr = token.parent_address || '';
                        const parentDisplay = parentAddr ? `<a class="address-link" style="font-size:0.75rem" target="_blank" href="https://otter.pulsechain.com/address/${parentAddr}" title="${parentAddr}">${parentSymbol}</a>` : parentSymbol;
                        
                        const b64Symbol = btoa(unescape(encodeURIComponent(symbol)));
                        const b64Name = btoa(unescape(encodeURIComponent(name)));
                        
                        return `
                            <tr>
                                <td class="clickable-token" onclick="showLpModal('${addr}', '${b64Symbol}', '${b64Name}', true)">
                                    <div>${displaySymbol}</div>
                                    <div style="font-size:0.75rem; color:var(--text-muted)">${displayName}</div>
                                    <div style="font-size:0.7rem; color:var(--text-muted); margin-top: 0.1rem">
                                        <span style="color: #34d399">Parent:</span> ${parentDisplay}
                                    </div>
                                </td>
                                <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${addr}">${formatAddress(addr)}</a></td>
                                <td>
                                    <button class="btn ${buttonClass}" onclick="toggleIgnoreToken('${addr}', ${nextIgnored})">${buttonText}</button>
                                </td>
                            </tr>
                        `;
                    }).join('');
                    
                    registryContainer.innerHTML = `
                        <div class="minter-group" style="margin-bottom: 0;">
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th style="width: 45%">Token</th>
                                        <th style="width: 30%">Address</th>
                                        <th style="width: 25%">Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    ${rowsHtml}
                                </tbody>
                            </table>
                        </div>
                    `;
                } else {
                    registryContainer.innerHTML = '<div style="text-align: center; color: var(--text-muted); padding: 2rem">No tokens found.</div>';
                }
            }
        }

        let currentSort = { column: 'volume', direction: 'desc' };

        function setSort(column) {
            if (currentSort.column === column) {
                currentSort.direction = currentSort.direction === 'asc' ? 'desc' : 'asc';
            } else {
                currentSort.column = column;
                currentSort.direction = 'desc';
            }
            updateSortIndicators();
            renderCatalog();
        }

        function updateSortIndicators() {
            const columns = ['symbol', 'address', 'price', 'volume', 'amount'];
            columns.forEach(col => {
                const el = document.getElementById(`sort-${col}`);
                if (!el) return;
                if (currentSort.column === col) {
                    el.innerText = currentSort.direction === 'asc' ? ' ▲' : ' ▼';
                    el.style.color = 'var(--primary)';
                } else {
                    el.innerText = ' ⇅';
                    el.style.color = 'var(--text-muted)';
                }
            });
        }

        function renderCatalog() {
            const searchVal = document.getElementById('catalog-search').value.toLowerCase();
            const catalogBody = document.querySelector('#catalog-table tbody');
            const priceEntries = Object.entries(cachedData.prices);

            // Calculate volumes for each token address
            const volumes = {};
            if (cachedData.resolved) {
                cachedData.resolved.forEach(swap => {
                    const addr0 = swap.token0.address.toLowerCase();
                    const addr1 = swap.token1.address.toLowerCase();
                    const val = parseFloat(swap.usd_value) || 0;
                    volumes[addr0] = (volumes[addr0] || 0) + val;
                    volumes[addr1] = (volumes[addr1] || 0) + val;
                });
            }

            const filtered = priceEntries.filter(([addr, val]) => {
                const addrLower = addr.toLowerCase();
                if (cachedData.treasury_tokens && cachedData.treasury_tokens[addrLower] && cachedData.treasury_tokens[addrLower].ignored) {
                    return false;
                }
                return addrLower.includes(searchVal) || 
                       (val && val.symbol && val.symbol.toLowerCase().includes(searchVal)) ||
                       (val && val.name && val.name.toLowerCase().includes(searchVal));
            });

            // Sort logic: Treasury tokens first, then sorted by chosen column
            filtered.sort((a, b) => {
                const addrA = a[0].toLowerCase();
                const addrB = b[0].toLowerCase();
                
                const isTreasuryA = !!(cachedData.treasury_tokens && cachedData.treasury_tokens[addrA] && !cachedData.treasury_tokens[addrA].ignored);
                const isTreasuryB = !!(cachedData.treasury_tokens && cachedData.treasury_tokens[addrB] && !cachedData.treasury_tokens[addrB].ignored);
                
                if (isTreasuryA && !isTreasuryB) return -1;
                if (!isTreasuryA && isTreasuryB) return 1;
                
                let valA, valB;
                const priceA = parseFloat(typeof a[1] === 'object' ? a[1].price : a[1]) || 0;
                const priceB = parseFloat(typeof b[1] === 'object' ? b[1].price : b[1]) || 0;
                
                if (currentSort.column === 'price') {
                    valA = priceA;
                    valB = priceB;
                } else if (currentSort.column === 'volume') {
                    valA = volumes[addrA] || 0;
                    valB = volumes[addrB] || 0;
                } else if (currentSort.column === 'amount') {
                    valA = priceA > 0 ? (1 / priceA) : 0;
                    valB = priceB > 0 ? (1 / priceB) : 0;
                } else if (currentSort.column === 'symbol') {
                    const matchA = cachedData.resolved.find(r => r.token0.address === a[0]) || cachedData.resolved.find(r => r.token1.address === a[0]);
                    const symA = (matchA ? (matchA.token0.address === a[0] ? matchA.token0.symbol : matchA.token1.symbol) : ((a[1] && a[1].symbol) || '')).toLowerCase();
                    
                    const matchB = cachedData.resolved.find(r => r.token0.address === b[0]) || cachedData.resolved.find(r => r.token1.address === b[0]);
                    const symB = (matchB ? (matchB.token0.address === b[0] ? matchB.token0.symbol : matchB.token1.symbol) : ((b[1] && b[1].symbol) || '')).toLowerCase();
                    
                    return currentSort.direction === 'asc' ? symA.localeCompare(symB) : symB.localeCompare(symA);
                } else if (currentSort.column === 'address') {
                    return currentSort.direction === 'asc' ? addrA.localeCompare(addrB) : addrB.localeCompare(addrA);
                }
                
                if (currentSort.direction === 'asc') {
                    return valA - valB;
                } else {
                    return valB - valA;
                }
            });

            if (filtered.length === 0) {
                catalogBody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: var(--text-muted)">No matching tokens found.</td></tr>';
                return;
            }

            catalogBody.innerHTML = filtered.map(([addr, val]) => {
                let name = "Unknown Token";
                let symbol = "UNKNOWN";
                
                if (val && typeof val === 'object') {
                    if (val.name) name = val.name;
                    if (val.symbol) symbol = val.symbol;
                }
                
                // If it's seed stablecoins, display metadata or defaults
                if (addr.includes("15d38573d2feeb82e7ad5187ab8c1d52810b1f07")) { name = "USD Coin from Ethereum"; symbol = "USDC"; }
                else if (addr.includes("a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48")) { name = "USD Coin"; symbol = "USDC"; }
                else if (addr.includes("0xcb81b54a05e0547d2d08c4a9e273a7d4c72b9eb")) { name = "Tether USD"; symbol = "USDT"; }
                else if (addr.includes("dac17f958d2ee523a2206206994597c13d831ec7")) { name = "Tether USD"; symbol = "USDT"; }
                else if (addr.includes("efd766ccb8c15e5e9f813af7b2809857baa53a1f")) { name = "Dai Stablecoin"; symbol = "DAI"; }
                else if (addr.includes("6b175474e89094c44da98b954eedeac495271d0f")) { name = "Dai Stablecoin"; symbol = "DAI"; }
                
                // If the cache contains price object with metadata
                let price = typeof val === 'object' ? val.price : val;
                
                // Search resolved/unresolved to match symbol/name if it was a dynamically resolved token
                const matchResolved = cachedData.resolved.find(r => r.token0.address === addr) || 
                                      cachedData.resolved.find(r => r.token1.address === addr);
                if (matchResolved) {
                    const match = matchResolved.token0.address === addr ? matchResolved.token0 : matchResolved.token1;
                    name = match.name;
                    symbol = match.symbol;
                }

                let isTreasury = false;
                let treasuryOwner = null;
                const addrLower = addr.toLowerCase();
                if (cachedData.treasury_tokens && cachedData.treasury_tokens[addrLower]) {
                    isTreasury = true;
                    treasuryOwner = cachedData.treasury_tokens[addrLower].owner;
                } else if (val && typeof val === 'object') {
                    if (val.is_treasury) isTreasury = val.is_treasury;
                    if (val.treasury_owner) treasuryOwner = val.treasury_owner;
                }

                let priceFloat = parseFloat(price);
                let amountForOneUsd = priceFloat > 0 ? (1 / priceFloat).toLocaleString(undefined, {minimumFractionDigits: 4, maximumFractionDigits: 8}) : 'N/A';
                
                const vol = volumes[addrLower] || 0;
                const volStr = vol > 0 ? `$${vol.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2})}` : '$0.00';

                let symbolHtml = isTreasury ? `<span class="token-symbol-glow" style="color: #fbbf24; text-shadow: 0 0 10px rgba(251, 191, 36, 0.4)">👑 ${symbol}</span>` : `<span class="token-symbol-glow">${symbol}</span>`;
                let nameHtml = isTreasury ? `<div style="font-size:0.75rem; color:var(--text-muted)">${name} <span style="color:#fbbf24; font-size:0.7rem; font-weight:600" title="Treasury Owner: ${treasuryOwner}">(Treasury)</span></div>` : `<div style="font-size:0.75rem; color:var(--text-muted)">${name}</div>`;

                const b64Symbol = btoa(unescape(encodeURIComponent(symbol)));
                const b64Name = btoa(unescape(encodeURIComponent(name)));

                return `
                    <tr style="${isTreasury ? 'background: rgba(251, 191, 36, 0.02); border-left: 3px solid #fbbf24;' : ''}">
                        <td class="clickable-token" onclick="showLpModal('${addr}', '${b64Symbol}', '${b64Name}', true)">
                            ${symbolHtml}
                            ${nameHtml}
                        </td>
                        <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${addr}">${formatAddress(addr)}</a></td>
                        <td style="font-family: monospace; font-weight: 700; color: #fff">$${priceFloat.toFixed(8)}</td>
                        <td style="font-family: monospace; font-weight: 700; color: #a78bfa">${volStr}</td>
                        <td style="font-family: monospace; font-weight: 700; color: #34d399">${amountForOneUsd} ${symbol}</td>
                    </tr>
                `;
            }).join('');
        }

        async function toggleIgnoreToken(address, ignored) {
            try {
                const response = await fetch(`/api/ignore?address=${encodeURIComponent(address)}&ignored=${ignored}`);
                if (response.ok) {
                    fetchData();
                } else {
                    console.error("Failed to toggle ignore status");
                }
            } catch (err) {
                console.error("Error toggling ignore status:", err);
            }
        }

        async function showLpModal(address, symbol, name, isB64) {
            if (isB64) {
                try {
                    symbol = decodeURIComponent(escape(atob(symbol)));
                    name = decodeURIComponent(escape(atob(name)));
                } catch(e) {
                    console.error("Base64 decode error:", e);
                }
            }
            const modal = document.getElementById('lp-modal');
            const titleEl = document.getElementById('lp-modal-title');
            const subtitleEl = document.getElementById('lp-modal-subtitle');
            const bodyEl = document.getElementById('lp-modal-body');

            titleEl.innerText = `${symbol} Liquidity Pools`;
            subtitleEl.innerHTML = `Address: <a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${address}">${address}</a>`;
            
            bodyEl.innerHTML = `
                <div class="modal-loader">
                    <div class="spinner"></div>
                    <div>Querying PulseChain nodes for active PulseX V1 & V2 pools...</div>
                </div>
            `;
            
            modal.classList.add('active');

            try {
                const res = await fetch(`/api/pools?address=${encodeURIComponent(address)}`);
                if (!res.ok) throw new Error("API request failed");
                const data = await res.json();
                
                if (!data.success) {
                    bodyEl.innerHTML = `<div style="text-align: center; color: var(--danger); padding: 2rem">Error: ${data.error}</div>`;
                    return;
                }

                if (data.pools.length === 0) {
                    bodyEl.innerHTML = `<div style="text-align: center; color: var(--text-muted); padding: 2rem">No liquidity pools found matching standard base assets on PulseX V1 or V2.</div>`;
                    return;
                }

                const rowsHtml = data.pools.map((pool, idx) => {
                    return `
                        <tr>
                            <td>
                                <div style="font-weight: 700; color: #fff;">${symbol} / ${pool.base_symbol}</div>
                                <div style="font-size: 0.75rem; color: var(--text-muted);">
                                    <span style="color:#fbbf24; font-weight:600;">${pool.version}</span> Pool: <a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${pool.pair_address}">${formatAddress(pool.pair_address)}</a>
                                </div>
                            </td>
                            <td style="font-family: monospace; text-align: right; color: #34d399; font-weight: 700;">
                                ${pool.token_reserve.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 4})}
                            </td>
                            <td style="font-family: monospace; text-align: right; color: #a78bfa; font-weight: 700;">
                                ${pool.base_reserve.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 4})}
                            </td>
                        </tr>
                    `;
                }).join('');

                bodyEl.innerHTML = `
                    <div style="max-height: 400px; overflow-y: auto;">
                        <table class="table">
                            <thead>
                                <tr>
                                    <th>Pool Pair</th>
                                    <th style="text-align: right;">${symbol} Reserve</th>
                                    <th style="text-align: right;">Base Asset Reserve</th>
                                </tr>
                            </thead>
                            <tbody>
                                ${rowsHtml}
                            </tbody>
                        </table>
                    </div>
                `;
            } catch (err) {
                bodyEl.innerHTML = `<div style="text-align: center; color: var(--danger); padding: 2rem">Failed fetching pool reserves. Ensure daemon connection is active.</div>`;
            }
        }

        function closeLpModal(event) {
            const modal = document.getElementById('lp-modal');
            modal.classList.remove('active');
        }

        document.getElementById('catalog-search').addEventListener('input', renderCatalog);

        // Fetch updates every 2 seconds
        fetchData();
        setInterval(fetchData, 2000);
    </script>
</body>
</html>
"""

if __name__ == "__main__":
    handler = DashboardHandler
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), handler) as httpd:
        print(f"Dashboard web server active on http://localhost:{PORT}")
        httpd.serve_forever()
