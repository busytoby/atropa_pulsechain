#!/usr/bin/env python3
import os
import json
import http.server
import socketserver
import urllib.parse
import threading
import re
import glob

PORT = int(os.environ.get("PORT", 8080))
http.server.BaseHTTPRequestHandler.max_line_size = 1048576
PRICE_CACHE_FILE = "price_cache.json"
UNRESOLVED_FILE = "unresolved_swaps.json"
RESOLVED_FILE = "resolved_swaps.json"

reserves_files = glob.glob(os.path.expanduser("~/.gemini/antigravity-cli/brain/*/scratch/nonukes_pulsex_reserves.json"))
default_reserves_path = reserves_files[0] if reserves_files else "nonukes_pulsex_reserves.json"
RESERVES_FILE_PATH = os.environ.get("RESERVES_FILE_PATH", default_reserves_path)

IGNORE_LOCK = threading.Lock()
QING_CHAT_LOGS = [
    {
        "username": "s㉾vereign",
        "soul": 91831900924270,
        "aura": 682857673341037,
        "message": "vibes",
        "block": 26551929,
        "tx": "0xf64e78f57315f9f5064521ba8f21d3ac4ee6c245f81796fea5d6a5b540317396"
    }
]

def start_qing_chat_poller():
    global QING_CHAT_LOGS
    import urllib.request
    import json
    import time
    
    QING_ADDR = "0x4dd0371c02631bfd17ad10ab7c0e35a047ff2d20"
    TOPIC_0 = "0xb916a3aaef1d0f80cd78f5fca0067f557f3db4a63748929d2e5e8d0014549374"
    RPC_URL = "https://rpc.pulsechain.com"
    
    def decode_abi_log_event(data_hex):
        try:
            if data_hex.startswith("0x"):
                data_hex = data_hex[2:]
            words = [data_hex[i:i+64] for i in range(0, len(data_hex), 64)]
            if len(words) < 4:
                return None
            soul = int(words[1], 16)
            aura = int(words[2], 16)
            username_word_idx = int(words[0], 16) // 32
            msg_word_idx = int(words[3], 16) // 32
            username_len = int(words[username_word_idx], 16)
            username_hex = "".join(words[username_word_idx + 1 : username_word_idx + 1 + (username_len + 31) // 32])
            username = bytes.fromhex(username_hex[:username_len * 2]).decode('utf-8', errors='ignore')
            msg_len = int(words[msg_word_idx], 16)
            msg_hex = "".join(words[msg_word_idx + 1 : msg_word_idx + 1 + (msg_len + 31) // 32])
            msg = bytes.fromhex(msg_hex[:msg_len * 2]).decode('utf-8', errors='ignore')
            return {
                "username": username,
                "soul": soul,
                "aura": aura,
                "message": msg
            }
        except Exception:
            return None

    def poll():
        global QING_CHAT_LOGS
        last_block = 0
        while True:
            try:
                req = urllib.request.Request(
                    RPC_URL,
                    data=json.dumps({"jsonrpc": "2.0", "method": "eth_blockNumber", "params": [], "id": 1}).encode('utf-8'),
                    headers={"Content-Type": "application/json"}
                )
                with urllib.request.urlopen(req, timeout=5) as res:
                    resp = json.loads(res.read().decode('utf-8'))
                    current_block = int(resp['result'], 16)
                
                if last_block == 0:
                    logs = []
                    scan_sizes = [15000, 100000, 500000, 2000000, 5000000]
                    for size in scan_sizes:
                        start_block = max(0, current_block - size)
                        req_logs = urllib.request.Request(
                            RPC_URL,
                            data=json.dumps({
                                "jsonrpc": "2.0",
                                "method": "eth_getLogs",
                                "params": [{
                                    "fromBlock": hex(start_block),
                                    "toBlock": hex(current_block),
                                    "address": QING_ADDR,
                                    "topics": [TOPIC_0]
                                }],
                                "id": 2
                            }).encode('utf-8'),
                            headers={"Content-Type": "application/json"}
                        )
                        try:
                            with urllib.request.urlopen(req_logs, timeout=8) as res:
                                resp = json.loads(res.read().decode('utf-8'))
                                logs = resp.get('result', [])
                            if logs:
                                break
                        except Exception:
                            pass
                else:
                    start_block = last_block + 1
                    logs = []
                
                if (last_block > 0 and start_block <= current_block) or (last_block == 0 and logs):
                    if last_block > 0:
                        req_logs = urllib.request.Request(
                            RPC_URL,
                            data=json.dumps({
                                "jsonrpc": "2.0",
                                "method": "eth_getLogs",
                                "params": [{
                                    "fromBlock": hex(start_block),
                                    "toBlock": hex(current_block),
                                    "address": QING_ADDR,
                                    "topics": [TOPIC_0]
                                }],
                                "id": 2
                            }).encode('utf-8'),
                            headers={"Content-Type": "application/json"}
                        )
                        with urllib.request.urlopen(req_logs, timeout=8) as res:
                            resp = json.loads(res.read().decode('utf-8'))
                            logs = resp.get('result', [])
                        
                    new_chats = []
                    for log in logs:
                        decoded = decode_abi_log_event(log.get('data', ''))
                        if decoded:
                            decoded["block"] = int(log.get("blockNumber", "0"), 16)
                            decoded["tx"] = log.get("transactionHash", "")
                            new_chats.append(decoded)
                        
                        if new_chats:
                            # Use lock to update safely
                            QING_CHAT_LOGS.extend(new_chats)
                            seen = set()
                            unique_chats = []
                            for c in QING_CHAT_LOGS:
                                if c["tx"] not in seen:
                                    seen.add(c["tx"])
                                    unique_chats.append(c)
                            QING_CHAT_LOGS = unique_chats[-100:]
                    
                    last_block = current_block
            except Exception:
                pass
            time.sleep(8)
            
    t = threading.Thread(target=poll, daemon=True)
    t.start()

start_qing_chat_poller()

def get_qings_mapping():
    mapping_path = "config/nonukes_qings_status.json"
    if not os.path.exists(mapping_path):
        mapping_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "config", "nonukes_qings_status.json")
    
    qings = {}
    if os.path.exists(mapping_path):
        try:
            with open(mapping_path, "r") as f:
                data = json.load(f)
                if isinstance(data, list):
                    for item in data:
                        addr = item.get("address")
                        qing = item.get("qing")
                        if addr and qing:
                            qings[addr.lower()] = qing
        except Exception:
            pass
    return qings

def fetch_logs_for_qing(qing_address):
    import urllib.request
    import json
    
    TOPIC_0 = "0xb916a3aaef1d0f80cd78f5fca0067f557f3db4a63748929d2e5e8d0014549374"
    RPC_URL = "https://rpc.pulsechain.com"
    
    def decode_abi_log_event(data_hex):
        try:
            if data_hex.startswith("0x"):
                data_hex = data_hex[2:]
            words = [data_hex[i:i+64] for i in range(0, len(data_hex), 64)]
            if len(words) < 4:
                return None
            soul = int(words[1], 16)
            aura = int(words[2], 16)
            username_word_idx = int(words[0], 16) // 32
            msg_word_idx = int(words[3], 16) // 32
            username_len = int(words[username_word_idx], 16)
            username_hex = "".join(words[username_word_idx + 1 : username_word_idx + 1 + (username_len + 31) // 32])
            username = bytes.fromhex(username_hex[:username_len * 2]).decode('utf-8', errors='ignore')
            msg_len = int(words[msg_word_idx], 16)
            msg_hex = "".join(words[msg_word_idx + 1 : msg_word_idx + 1 + (msg_len + 31) // 32])
            msg = bytes.fromhex(msg_hex[:msg_len * 2]).decode('utf-8', errors='ignore')
            return {
                "username": username,
                "soul": soul,
                "aura": aura,
                "message": msg
            }
        except Exception:
            return None

    try:
        req = urllib.request.Request(
            RPC_URL,
            data=json.dumps({"jsonrpc": "2.0", "method": "eth_blockNumber", "params": [], "id": 1}).encode('utf-8'),
            headers={"Content-Type": "application/json"}
        )
        with urllib.request.urlopen(req, timeout=5) as res:
            resp = json.loads(res.read().decode('utf-8'))
            current_block = int(resp['result'], 16)
        
        logs = []
        scan_sizes = [20000, 100000, 500000, 2000000, 5000000]
        for size in scan_sizes:
            start_block = max(0, current_block - size)
            req_logs = urllib.request.Request(
                RPC_URL,
                data=json.dumps({
                    "jsonrpc": "2.0",
                    "method": "eth_getLogs",
                    "params": [{
                        "fromBlock": hex(start_block),
                        "toBlock": hex(current_block),
                        "address": qing_address,
                        "topics": [TOPIC_0]
                    }],
                    "id": 2
                }).encode('utf-8'),
                headers={"Content-Type": "application/json"}
            )
            try:
                with urllib.request.urlopen(req_logs, timeout=8) as res:
                    resp = json.loads(res.read().decode('utf-8'))
                    logs = resp.get('result', [])
                if logs:
                    break
            except Exception:
                pass
            
        chats = []
        for log in logs:
            decoded = decode_abi_log_event(log.get('data', ''))
            if decoded:
                decoded["block"] = int(log.get("blockNumber", "0"), 16)
                decoded["tx"] = log.get("transactionHash", "")
                chats.append(decoded)
        if not chats:
            chats = [{
                "username": "Auncient Advisor",
                "soul": 9,
                "aura": 9,
                "message": f"Welcome to the Auncient VM telemetry stream for QING {qing_address}. Communication channel initialized.",
                "block": 0,
                "tx": ""
            }]
        return chats
    except Exception as e:
        print(f"Error fetching logs for dynamic QING {qing_address}: {e}")
        return [{
            "username": "Auncient Advisor",
            "soul": 0,
            "aura": 0,
            "message": f"Welcome to the Auncient VM telemetry stream for QING {qing_address}. Communication channel initialized.",
            "block": 0,
            "tx": ""
        }]


def get_price(prices, addr):
    if not isinstance(prices, dict) or not addr:
        return 0.0
    val = prices.get(addr)
    price_val = 0.0
    if isinstance(val, dict):
        price_val = val.get("price")
    else:
        price_val = val
    if price_val is None:
        return 0.0
    try:
        return float(price_val)
    except (ValueError, TypeError):
        return 0.0

def get_adjusted_and_aligned_reserves(pool_addr, reserves_data, pool_info):
    NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
    partner_addr = pool_info.get("other_addr", "").lower() if isinstance(pool_info, dict) else ""
    partner_symbol = pool_info.get("symbol", "UNKNOWN") if isinstance(pool_info, dict) else "UNKNOWN"
    
    res = reserves_data.get(pool_addr.lower(), {})
    if not isinstance(res, dict):
        res = {}
        
    t0 = res.get("token0", "").lower()
    t1 = res.get("token1", "").lower()
    
    r0_val = res.get("reserve0")
    r1_val = res.get("reserve1")
    try:
        r0 = float(r0_val) if r0_val is not None else 0.0
    except (ValueError, TypeError):
        r0 = 0.0
    try:
        r1 = float(r1_val) if r1_val is not None else 0.0
    except (ValueError, TypeError):
        r1 = 0.0
        
    if not t0 or not t1:
        if partner_addr < NONUKES_ADDR:
            t0, t1 = partner_addr, NONUKES_ADDR
        else:
            t0, t1 = NONUKES_ADDR, partner_addr
            
    if t0 == NONUKES_ADDR:
        nonukes_res = r0
        partner_res = r1
        partner_token_key = "1"
    elif t1 == NONUKES_ADDR:
        nonukes_res = r1
        partner_res = r0
        partner_token_key = "0"
    else:
        nonukes_res = r0
        partner_res = r1
        partner_token_key = "1"
        
    partner_decimals = 18
    raw_r_val = res.get(f"raw_reserve{partner_token_key}")
    adj_r_val = res.get(f"reserve{partner_token_key}")
    if raw_r_val is not None and adj_r_val is not None:
        try:
            raw_r = float(raw_r_val)
            adj_r = float(adj_r_val)
            if raw_r > 0.0 and adj_r > 0.0:
                import math
                derived_dec = round(math.log10(raw_r / adj_r))
                if 0 <= derived_dec <= 36:
                    partner_decimals = derived_dec
        except Exception:
            pass
            
    partner_res_adjusted = partner_res * (10 ** (18 - partner_decimals))
    
    return {
        "token0": nonukes_res,
        "token1": partner_res_adjusted,
        "token0_symbol": "NoNukes",
        "token1_symbol": partner_symbol
    }

def derive_prices(pools_data, reserves_data, cached_prices=None):
    if not isinstance(pools_data, dict):
        pools_data = {}
    if not isinstance(reserves_data, dict):
        reserves_data = {}
        
    NONUKES_PRICE = 1.74
    NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
    
    nonukes_price_val = 0.0
    if isinstance(cached_prices, dict):
        for k, v in cached_prices.items():
            if isinstance(k, str) and k.lower() == NONUKES_ADDR:
                price_field = None
                if isinstance(v, dict):
                    price_field = v.get("price")
                else:
                    price_field = v
                if price_field is not None:
                    try:
                        nonukes_price_val = float(price_field)
                    except (ValueError, TypeError):
                        pass
                break
                
    if nonukes_price_val <= 0.0:
        nonukes_price_val = NONUKES_PRICE
        
    partner_pools = {}
    for pool_addr, info in pools_data.items():
        if not isinstance(info, dict):
            continue
        partner_addr = info.get("other_addr")
        if isinstance(partner_addr, str):
            p_addr_lower = partner_addr.lower()
            if p_addr_lower not in partner_pools:
                partner_pools[p_addr_lower] = []
            partner_pools[p_addr_lower].append((pool_addr.lower(), info))
            
    derived_prices = {}
    for partner_addr, pools_list in partner_pools.items():
        total_reserve_partner = 0.0
        total_reserve_nonukes = 0.0
        symbol = "UNKNOWN"
        name = "Unknown Token"
        for pool_addr, info in pools_list:
            if info.get("symbol"):
                symbol = info.get("symbol")
            if info.get("name"):
                name = info.get("name")
            
            res = reserves_data.get(pool_addr.lower())
            if not isinstance(res, dict):
                continue
                
            adjusted = get_adjusted_and_aligned_reserves(pool_addr, reserves_data, info)
            total_reserve_nonukes += adjusted["token0"]
            total_reserve_partner += adjusted["token1"]
            
        price_usd = 0.0
        if total_reserve_partner > 0.0:
            price_usd = (total_reserve_nonukes / total_reserve_partner) * nonukes_price_val
            
        liquidity_usd = total_reserve_nonukes * nonukes_price_val * 2.0
        
        derived_prices[partner_addr] = {
            "price": price_usd,
            "symbol": symbol,
            "name": name,
            "liquidity": liquidity_usd
        }
        
    # Add NoNukes itself
    derived_prices[NONUKES_ADDR] = {
        "price": nonukes_price_val,
        "symbol": "NoNukes",
        "name": "NoNukes",
        "liquidity": sum(item.get("liquidity", 0.0) for item in derived_prices.values() if isinstance(item, dict))
    }
    return derived_prices

class DashboardHandler(http.server.SimpleHTTPRequestHandler):
    def send_error(self, code, message=None, explain=None):
        if code == 414:
            code = 400
        super().send_error(code, message, explain)

    def log_message(self, format, *args):
        # Suppress request logging to keep stderr clean
        pass

    def do_GET(self):
        if self.path == '/lau-gallery' or self.path == '/lau-gallery/' or self.path == '/lau-gallery/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'lau_gallery', 'index.html'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'lau_gallery', 'index.html')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/nonukes' or self.path == '/nonukes/' or self.path == '/nonukes/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'nonukes_dashboard', 'index.html'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'index.html')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/h111' or self.path == '/h111/' or self.path == '/h111/index.html' or self.path == '/hammond_h111_dashboard.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'hammond_h111_dashboard.html'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'hammond_h111_dashboard.html')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/evolution' or self.path == '/evolution/' or self.path == '/evolution/index.html' or self.path == '/yi_lau_evolution_dashboard.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'yi_lau_evolution_dashboard.html'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'yi_lau_evolution_dashboard.html')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/nonukes/style.css':
            self.send_response(200)
            self.send_header('Content-Type', 'text/css; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'nonukes_dashboard', 'style.css'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'style.css')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/nonukes/app.js':
            self.send_response(200)
            self.send_header('Content-Type', 'application/javascript; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.abspath(os.path.join('frontend', 'nonukes_dashboard', 'app.js'))
            if not os.path.exists(file_path):
                file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'app.js')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/api/data':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            pools_data = {}
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(pools_data, dict): pools_data = {}
            
            reserves_data = {}
            res_path = RESERVES_FILE_PATH
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(reserves_data, dict): reserves_data = {}
            
            prices = derive_prices(pools_data, reserves_data)
            
            if os.path.exists(PRICE_CACHE_FILE):
                try:
                    with open(PRICE_CACHE_FILE, "r") as f:
                        cached_prices = json.load(f)
                        if isinstance(cached_prices, dict):
                            for k, v in cached_prices.items():
                                k_lower = k.lower()
                                if k_lower not in prices:
                                    prices[k_lower] = v
                except Exception:
                    pass
            if not isinstance(prices, dict): prices = {}
                    
            unresolved = []
            if os.path.exists(UNRESOLVED_FILE):
                try:
                    with open(UNRESOLVED_FILE, "r") as f:
                        unresolved = json.load(f)
                except Exception:
                    pass
            if not isinstance(unresolved, list): unresolved = []
                    
            resolved = []
            if os.path.exists(RESOLVED_FILE):
                try:
                    with open(RESOLVED_FILE, "r") as f:
                        resolved = json.load(f)
                except Exception:
                    pass
            if not isinstance(resolved, list): resolved = []
            
            treasury_tokens = {}
            import glob
            with IGNORE_LOCK:
                for fpath in glob.glob("treasury_tokens_*.json"):
                    try:
                        with open(fpath, "r") as f:
                            t_data = json.load(f)
                            if isinstance(t_data, dict):
                                treasury_tokens.update(t_data)
                    except Exception:
                        pass
            
            response = {
                "prices": prices,
                "unresolved": unresolved,
                "resolved": resolved,
                "treasury_tokens": treasury_tokens
            }
            self.wfile.write(json.dumps(response, indent=2).encode('utf-8'))
        elif self.path == '/api/nonukes/pools':
            pools_data = None
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(pools_data, dict):
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Invalid pools data"}).encode('utf-8'))
                return
                    
            reserves_data = None
            res_path = RESERVES_FILE_PATH
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(reserves_data, dict):
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Invalid reserves data"}).encode('utf-8'))
                return
                    
            resolved_swaps = None
            if os.path.exists("resolved_swaps.json"):
                try:
                    with open("resolved_swaps.json", "r") as f:
                        resolved_swaps = json.load(f)
                except Exception:
                    pass
            if not isinstance(resolved_swaps, list):
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Invalid swaps data"}).encode('utf-8'))
                return
            
            cached_prices = {}
            if os.path.exists("price_cache.json"):
                try:
                    with open("price_cache.json", "r") as f:
                        cached_prices = json.load(f)
                except Exception:
                    pass
            if not isinstance(cached_prices, dict): cached_prices = {}
            
            prices = derive_prices(pools_data, reserves_data, cached_prices)
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            # Aggregate stats from swaps
            pool_stats = {}
            for swap in resolved_swaps:
                if not isinstance(swap, dict):
                    continue
                p_addr = swap.get("pool_address", "").lower()
                if not p_addr:
                    continue
                if p_addr not in pool_stats:
                    pool_stats[p_addr] = {"count": 0, "volume": 0.0}
                
                usd_val = float(swap.get("usd_value", 0.0))
                # Fallback if usd_value is 0.0
                if usd_val == 0.0:
                    t0_info = swap.get("token0")
                    t1_info = swap.get("token1")
                    t0_addr = t0_info.get("address", "").lower() if isinstance(t0_info, dict) else ""
                    t1_addr = t1_info.get("address", "").lower() if isinstance(t1_info, dict) else ""
                    p0 = get_price(prices, t0_addr)
                    p1 = get_price(prices, t1_addr)
                    if p0 > 0:
                        usd_val = float(swap.get("amount0", 0.0)) * p0
                    elif p1 > 0:
                        usd_val = float(swap.get("amount1", 0.0)) * p1
                        
                pool_stats[p_addr]["count"] += 1
                pool_stats[p_addr]["volume"] += usd_val
                
            qings_map = get_qings_mapping()
            pools_list = []
            for addr, info in pools_data.items():
                if not isinstance(info, dict):
                    continue
                addr_lower = addr.lower()
                res = reserves_data.get(addr_lower, {})
                if not isinstance(res, dict):
                    res = {}
                stats = pool_stats.get(addr_lower, {"count": 0, "volume": 0.0})
                
                partner_addr = info.get("other_addr", "")
                qing_addr = qings_map.get(partner_addr.lower())
                
                pools_list.append({
                    "address": addr,
                    "symbol": info.get("symbol", ""),
                    "name": info.get("name", ""),
                    "target_group": info.get("target_group", ""),
                    "version": info.get("version", ""),
                    "partner_address": partner_addr,
                    "qing_address": qing_addr,
                    "reserves": get_adjusted_and_aligned_reserves(addr, reserves_data, info),
                    "swap_count": stats["count"],
                    "volume_usd": stats["volume"]
                })
                
            success = bool(pools_data)
            self.wfile.write(json.dumps({"success": success, "pools": pools_list}, indent=2).encode('utf-8'))
        elif self.path.startswith('/api/nonukes/pool_details'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            
            address_lower = address.lower() if address else ""
            
            pools_data = {}
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(pools_data, dict): pools_data = {}
                    
            pools_data_lower = {k.lower(): v for k, v in pools_data.items() if isinstance(k, str) and isinstance(v, dict)}
            if not address or len(address) < 42 or not address.startswith("0x") or address_lower not in pools_data_lower:
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Invalid pool address or pool not found"}).encode('utf-8'))
                return
                
            pool_info = pools_data_lower[address_lower]
            
            reserves_data = {}
            res_path = RESERVES_FILE_PATH
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves_data = json.load(f)
                except Exception:
                    pass
            if not isinstance(reserves_data, dict): reserves_data = {}
                    
            res = reserves_data.get(address_lower, {})
            if not isinstance(res, dict): res = {}
            
            cached_prices = {}
            price_cache_corrupt = False
            if os.path.exists("price_cache.json"):
                try:
                    with open("price_cache.json", "r") as f:
                        cached_prices = json.load(f)
                except Exception:
                    price_cache_corrupt = True
            if not isinstance(cached_prices, dict): cached_prices = {}
            
            prices = derive_prices(pools_data, reserves_data, cached_prices)
                    
            token0_addr = res.get("token0", "0x174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd62").lower()
            token1_addr = res.get("token1", pool_info.get("other_addr", "")).lower()
            
            def get_symbol(addr):
                if addr == "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62":
                    return "NoNukes"
                if addr == pool_info.get("other_addr", "").lower():
                    return pool_info.get("symbol", "UNKNOWN")
                entry = prices.get(addr, {})
                if isinstance(entry, dict):
                    return entry.get("symbol", "UNKNOWN")
                return "UNKNOWN"
                
            token0_symbol = get_symbol(token0_addr)
            token1_symbol = get_symbol(token1_addr)
            
            resolved_swaps = []
            if os.path.exists("resolved_swaps.json"):
                try:
                    with open("resolved_swaps.json", "r") as f:
                        resolved_swaps = json.load(f)
                except Exception:
                    pass
            if not isinstance(resolved_swaps, list): resolved_swaps = []
                    
            matching_swaps = []
            for swap in resolved_swaps:
                if isinstance(swap, dict) and swap.get("pool_address", "").lower() == address_lower:
                    matching_swaps.append(swap)
            
            # Sort chronologically for trends
            matching_swaps.sort(key=lambda s: float(s.get("timestamp", 0)))
            
            price_trends = []
            swap_history = []
            minter_addr = pool_info.get("other_addr", "").lower()
            
            for swap in matching_swaps:
                timestamp = int(swap.get("timestamp", 0))
                amt0 = float(swap.get("amount0", 0.0))
                amt1 = float(swap.get("amount1", 0.0))
                usd_val = float(swap.get("usd_value", 0.0))
                
                # Dynamic USD fallback
                if usd_val == 0.0:
                    t0_info = swap.get("token0")
                    t1_info = swap.get("token1")
                    t0_addr = t0_info.get("address", "").lower() if isinstance(t0_info, dict) else ""
                    t1_addr = t1_info.get("address", "").lower() if isinstance(t1_info, dict) else ""
                    p0 = get_price(prices, t0_addr)
                    p1 = get_price(prices, t1_addr)
                    if p0 > 0:
                        usd_val = amt0 * p0
                    elif p1 > 0:
                        usd_val = amt1 * p1
                
                t0_info = swap.get("token0")
                t1_info = swap.get("token1")
                t0_addr = t0_info.get("address", "").lower() if isinstance(t0_info, dict) else ""
                t1_addr = t1_info.get("address", "").lower() if isinstance(t1_info, dict) else ""
                
                amt_m = 0.0
                if t0_addr == minter_addr:
                    amt_m = amt0
                elif t1_addr == minter_addr:
                    amt_m = amt1
                else:
                    amt_m = amt1
                    
                price = 0.0
                if usd_val > 0 and amt_m > 0:
                    price = usd_val / amt_m
                
                price_trends.append({
                    "timestamp": timestamp,
                    "price": price
                })
                
                tx_hash = swap.get("tx_hash", "")
                if tx_hash and not tx_hash.startswith("0x"):
                    tx_hash = "0x" + tx_hash
                    
                swap_history.append({
                    "tx_hash": tx_hash,
                    "timestamp": timestamp,
                    "amount0": amt0,
                    "amount1": amt1,
                    "usd_value": usd_val
                })
                
            # History is returned newest first
            swap_history.reverse()
            
            if price_cache_corrupt:
                price_trends = [0.0, 0.0, 0.0]
 
            yue_scores = {
                "token0": {"hypobar": 0, "epibar": 0},
                "token1": {"hypobar": 0, "epibar": 0}
            }
 
            qings_map = get_qings_mapping()
            partner_addr = pool_info.get("other_addr", "")
            qing_addr = qings_map.get(partner_addr.lower())

            # Retrieve lore description
            lore_desc = "Primary minter contract paired in the NoNukes pool family."
            import glob
            card_files = glob.glob(f"solidity/dysnomia/domain/data/{partner_addr.lower()}.json")
            if card_files:
                try:
                    with open(card_files[0], "r") as f:
                        card_data = json.load(f)
                        lore_desc = card_data.get("desc", lore_desc)
                except Exception:
                    pass
            try:
                cache_file = "tmp/rich_descriptions_cache.json"
                if os.path.exists(cache_file):
                    with open(cache_file, "r") as f:
                        cache = json.load(f)
                        if partner_addr.lower() in cache:
                            lore_desc = cache[partner_addr.lower()]
            except Exception:
                pass

            import hashlib
            addr_hash = hashlib.md5(partner_addr.lower().encode('utf-8')).hexdigest()
            h_val = int(addr_hash, 16)
            atk_val = (h_val % 8) + 3
            def_val = ((h_val >> 4) % 8) + 3
            liq_val = ((h_val >> 8) % 8) + 3

            response = {
                "success": True,
                "address": address,
                "partner_address": partner_addr,
                "qing_address": qing_addr,
                "lore": lore_desc,
                "price_trends": price_trends,
                "reserves": get_adjusted_and_aligned_reserves(address, reserves_data, pool_info),
                "yue_scores": yue_scores,
                "stats": {
                    "atk": atk_val,
                    "def": def_val,
                    "liq": liq_val
                },
                "swap_history": swap_history,
                "swaps": swap_history
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
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
            # Load reserves from scratch directory or fallback to current directory
            res_path = RESERVES_FILE_PATH
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
                if not re.match(r"^0x[0-9a-fA-F]{40}$", address):
                    self.send_response(400)
                    self.send_header('Content-Type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    self.wfile.write(json.dumps({"success": False, "error": "Invalid address format"}).encode('utf-8'))
                    return
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
            
            if not address or not re.match(r"^0x[0-9a-fA-F]{40}$", address):
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Invalid address parameter"}).encode('utf-8'))
                return
            
            address = address.lower()
            
            with IGNORE_LOCK:
                import glob
                target_fpath = None
                tokens = {}
                
                # Search all files to find which files have the token and update them
                found_any = False
                for fpath in glob.glob("treasury_tokens_*.json"):
                    try:
                        with open(fpath, "r") as f:
                            data = json.load(f)
                        if isinstance(data, dict) and address in data:
                            data[address]["ignored"] = ignored_val
                            with open(fpath, "w") as f:
                                json.dump(data, f, indent=4)
                            found_any = True
                    except Exception:
                        pass
                
                if not found_any:
                    # Default to unknown_minter if not found anywhere
                    target_fpath = "treasury_tokens_unknown_minter.json"
                    tokens = None
                    if os.path.exists(target_fpath):
                        try:
                            with open(target_fpath, "r") as f:
                                tokens = json.load(f)
                        except Exception:
                            pass
                    if os.path.exists(target_fpath) and not isinstance(tokens, dict):
                        self.send_response(400)
                        self.send_header('Content-Type', 'application/json')
                        self.send_header('Access-Control-Allow-Origin', '*')
                        self.end_headers()
                        self.wfile.write(json.dumps({"success": False, "error": "Invalid tokens format"}).encode('utf-8'))
                        return
                    if not isinstance(tokens, dict):
                        tokens = {}
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
        elif self.path.startswith('/api/nonukes/qing-chat'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            qing_addr = params.get('address', [None])[0]
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            NONUKES_QING = "0x4dd0371c02631bfd17ad10ab7c0e35a047ff2d20"
            if not qing_addr or qing_addr.lower() == NONUKES_QING.lower():
                self.wfile.write(json.dumps({"success": True, "chat": QING_CHAT_LOGS, "address": NONUKES_QING}).encode('utf-8'))
            else:
                dynamic_chats = fetch_logs_for_qing(qing_addr)
                self.wfile.write(json.dumps({"success": True, "chat": dynamic_chats, "address": qing_addr}).encode('utf-8'))
            return
        elif self.path.startswith('/api/nonukes/generate-card-art'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            
            if not address or not address.startswith("0x"):
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b"Invalid address")
                return
                
            address = address.lower()
            card_data = None
            import glob
            card_files = glob.glob(f"solidity/dysnomia/domain/data/{address}.json")
            if card_files:
                try:
                    with open(card_files[0], "r") as f:
                        card_data = json.load(f)
                except Exception:
                    pass
            
            if not card_data:
                pools_data = {}
                if os.path.exists("nonukes_pools.json"):
                    try:
                        with open("nonukes_pools.json", "r") as f:
                            pools_data = json.load(f)
                    except Exception:
                        pass
                
                symbol = "UNKNOWN"
                name = "Unknown Token"
                for p_addr, p_info in pools_data.items():
                    if p_info.get("other_addr", "").lower() == address:
                        symbol = p_info.get("symbol", "UNKNOWN")
                        name = p_info.get("name", "Unknown Token")
                        break
                        
                card_data = {
                    "address": address,
                    "symbol": symbol,
                    "name": name,
                    "type": "Minter Asset",
                    "desc": f"Primary minter contract paired in the NoNukes pool family.",
                    "color": "#a02be2"
                }
                
            os.makedirs("assets", exist_ok=True)
            output_path = f"assets/{address}.png"
            
            try:
                import sys
                scripts_dir = os.path.dirname(os.path.abspath(__file__))
                if scripts_dir not in sys.path:
                    sys.path.append(scripts_dir)
                from batch_generate_art import render_card_art
                render_card_art(card_data, output_path)
                success = True
                error_msg = ""
            except Exception as e:
                success = False
                error_msg = str(e)
                
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            lore_desc = card_data.get("desc", "")
            try:
                cache_file = "tmp/rich_descriptions_cache.json"
                if os.path.exists(cache_file):
                    with open(cache_file, "r") as f:
                        cache = json.load(f)
                        if address in cache:
                            lore_desc = cache[address]
            except Exception:
                pass
                
            self.wfile.write(json.dumps({
                "success": success,
                "error": error_msg,
                "art_url": f"/assets/{address}.png",
                "lore": lore_desc
            }).encode('utf-8'))
            return
        elif self.path.startswith('/api/nonukes/chat'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            query = params.get('q', [''])[0]
            
            import sys
            scripts_dir = os.path.dirname(os.path.abspath(__file__))
            if scripts_dir not in sys.path:
                sys.path.append(scripts_dir)
            try:
                from telemetry_advisor import query_telemetry
                response_text = query_telemetry(query)
            except Exception as e:
                response_text = f"Telemetry engine error: {str(e)}"
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"success": True, "response": response_text}).encode('utf-8'))
            return
        elif self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_CONTENT.encode('utf-8'))
        else:
            # Check if the requested path corresponds to a file relative to the server's working directory
            clean_path = self.path.split('?')[0].lstrip('/')
            if clean_path:
                frontend_dir = os.path.abspath("frontend")
                assets_dir = os.path.abspath("assets")
                abs_clean = os.path.abspath(clean_path)
                try:
                    is_safe_frontend = os.path.commonpath([frontend_dir, abs_clean]) == frontend_dir
                    is_safe_assets = os.path.commonpath([assets_dir, abs_clean]) == assets_dir
                    is_safe = is_safe_frontend or is_safe_assets
                except ValueError:
                    is_safe = False
                
                if is_safe and os.path.exists(abs_clean) and os.path.isfile(abs_clean):
                    self.send_response(200)
                    if abs_clean.endswith('.html'):
                        self.send_header('Content-Type', 'text/html; charset=utf-8')
                    elif abs_clean.endswith('.js'):
                        self.send_header('Content-Type', 'application/javascript')
                    elif abs_clean.endswith('.css'):
                        self.send_header('Content-Type', 'text/css')
                    elif abs_clean.endswith('.json'):
                        self.send_header('Content-Type', 'application/json')
                    elif abs_clean.endswith('.png'):
                        self.send_header('Content-Type', 'image/png')
                    self.end_headers()
                    with open(abs_clean, 'rb') as f:
                        self.wfile.write(f.read())
                    return
                else:
                    self.send_response(400)
                    self.end_headers()
                    self.wfile.write(b"Bad Request or File not found")
                    return
            self.send_error(404, "File not found")

    def do_POST(self):
        if self.path == '/api/nonukes/set-clipboard':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                text = data.get("text", "")
            except Exception:
                text = post_data
            
            import subprocess
            try:
                p = subprocess.Popen(["python3", "scripts/set_clipboard.py"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = p.communicate(input=text.encode('utf-8'))
                success = p.returncode == 0
                error_msg = stderr.decode('utf-8') if not success else ""
            except Exception as e:
                success = False
                error_msg = str(e)
                
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"success": success, "error": error_msg}).encode('utf-8'))
            return
        elif self.path == '/api/nonukes/chat':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                query = data.get("query", "")
            except Exception:
                query = ""
            
            import sys
            scripts_dir = os.path.dirname(os.path.abspath(__file__))
            if scripts_dir not in sys.path:
                sys.path.append(scripts_dir)
            try:
                from telemetry_advisor import query_telemetry
                response_text = query_telemetry(query)
            except Exception as e:
                response_text = f"Telemetry engine error: {str(e)}"
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"success": True, "response": response_text}).encode('utf-8'))
            return
        elif self.path == '/api/nonukes/zmachine-console':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                command = data.get("command", "")
            except Exception:
                command = ""
                
            has_keys = any(os.environ.get(k) for k in ["GOOGLE_API_KEY", "GEMINI_API_KEY", "OPENAI_API_KEY"])
            response_text = ""
            
            if has_keys:
                try:
                    from langchain_core.prompts import ChatPromptTemplate
                    from langchain_core.output_parsers import StrOutputParser
                    
                    system_prompt = (
                        "You are the Z-Machine AI Game Co-Pilot. The player is playing an on-chain interactive text adventure "
                        "story game on PulseChain. Translate their natural language intent into standard game action verbs "
                        "if possible (e.g. 'look', 'take [item]', 'use [item]', 'Reaction [address]', 'east', 'west', 'north', 'south', 'help'). "
                        "Reply with a retro sci-fi interactive fiction response. Start by showing the translated Z-Machine command "
                        "like: '>> TRANSLATED: look' or '>> GUIDANCE: Type east to proceed', followed by a short immersive description "
                        "of what they see or do. Keep it under 250 characters, in clean Share Tech Mono text style."
                    )
                    
                    prompt = ChatPromptTemplate.from_messages([
                        ("system", system_prompt),
                        ("user", "Player Input: {command}\n\nAI Guide Response:")
                    ])
                    
                    llm = None
                    if os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY"):
                        from langchain_google_genai import ChatGoogleGenerativeAI
                        api_key = os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY")
                        llm = ChatGoogleGenerativeAI(model="gemini-1.5-flash", google_api_key=api_key)
                    elif os.environ.get("OPENAI_API_KEY"):
                        from langchain_openai import ChatOpenAI
                        llm = ChatOpenAI(model="gpt-4o-mini")
                        
                    if llm:
                        chain = prompt | llm | StrOutputParser()
                        response_text = chain.invoke({"command": command}).strip()
                except Exception as e:
                    response_text = f">> AI Co-Pilot Error: {str(e)}"
                    
            if not response_text:
                cmd_low = command.lower().strip()
                if "help" in cmd_low:
                    response_text = ">> Z-Machine Verbs: look, take [item], use [item], Reaction [address], north, south, east, west, help."
                elif "look" in cmd_low:
                    response_text = ">> TRANSLATED: look\nAn empty room looms with metallic voxel panels. Exit paths lead East and West."
                elif "take" in cmd_low or "pick" in cmd_low:
                    response_text = f">> TRANSLATED: take\nAttempting to acquire target asset. Inventory bounds verified."
                elif "east" in cmd_low or "go east" in cmd_low:
                    response_text = ">> TRANSLATED: east\nSteering corridor corridor. Transiting to Segment 2."
                elif "west" in cmd_low or "go west" in cmd_low:
                    response_text = ">> TRANSLATED: west\nSteering corridor corridor. Transiting to Segment 1."
                else:
                    response_text = f">> COMMAND PARSED: {command}\nNo active thunk attached. Verify Z-Machine console verbs or enable LLM keys."
                    
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"success": True, "response": response_text}).encode('utf-8'))
            return
        elif self.path == '/api/nonukes/zmachine-lore':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                terminal_logs = data.get("logs", "")
            except Exception:
                terminal_logs = ""
                
            has_keys = any(os.environ.get(k) for k in ["GOOGLE_API_KEY", "GEMINI_API_KEY", "OPENAI_API_KEY"])
            response_text = ""
            
            if has_keys:
                try:
                    from langchain_core.prompts import ChatPromptTemplate
                    from langchain_core.output_parsers import StrOutputParser
                    
                    system_prompt = (
                        "You are the Z-Machine Lore Master and Guide. The player is playing an on-chain text adventure "
                        "story game on PulseChain. Look at the terminal logs of their game session. "
                        "Provide a helpful hint, a secret, or guide them on what action verbs to try next "
                        "(e.g. going north/south/east/west, taking items, or using attributes). "
                        "Write in an immersive retro-sci-fi style under 250 characters, in clean Share Tech Mono text style."
                    )
                    
                    prompt = ChatPromptTemplate.from_messages([
                        ("system", system_prompt),
                        ("user", "Console Terminal Logs:\n{logs}\n\nHelpful Lore Guidance:")
                    ])
                    
                    llm = None
                    if os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY"):
                        from langchain_google_genai import ChatGoogleGenerativeAI
                        api_key = os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY")
                        llm = ChatGoogleGenerativeAI(model="gemini-1.5-flash", google_api_key=api_key)
                    elif os.environ.get("OPENAI_API_KEY"):
                        from langchain_openai import ChatOpenAI
                        llm = ChatOpenAI(model="gpt-4o-mini")
                        
                    if llm:
                        chain = prompt | llm | StrOutputParser()
                        response_text = chain.invoke({"logs": terminal_logs}).strip()
                except Exception as e:
                    response_text = f">> AI Lore Guide Error: {str(e)}"
                    
            if not response_text:
                response_text = (
                    ">> LORE HINT: You are trapped in a system of dark corridors. "
                    "Type 'look' to scan the current segment, and try moving 'east' or 'west' to explore new segments."
                )
                
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"success": True, "response": response_text}).encode('utf-8'))
            return
        else:
            self.send_response(404)
            self.end_headers()


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

class ThreadingHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    pass

if __name__ == "__main__":
    handler = DashboardHandler
    ThreadingHTTPServer.allow_reuse_address = True
    with ThreadingHTTPServer(("", PORT), handler) as httpd:
        print(f"Dashboard web server active on http://localhost:{PORT}")
        httpd.serve_forever()
