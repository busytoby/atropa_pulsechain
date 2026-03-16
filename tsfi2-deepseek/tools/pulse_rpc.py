import urllib.request
import json
import sys

RPC_URL = "https://rpc.pulsechain.com"

def eth_call(to_address, data_hex):
    payload = {
        "jsonrpc": "2.0",
        "method": "eth_call",
        "params": [{
            "to": to_address,
            "data": data_hex
        }, "latest"],
        "id": 1
    }
    try:
        req = urllib.request.Request(RPC_URL, data=json.dumps(payload).encode('utf-8'), headers={'Content-Type': 'application/json'})
        with urllib.request.urlopen(req) as response:
            res = json.loads(response.read().decode())
            if 'error' in res:
                return f"Error: {res['error']['message']}"
            return res.get('result', 'No result')
    except Exception as e:
        return f"Request failed: {str(e)}"

def eth_get_storage_at(address, slot_hex):
    payload = {
        "jsonrpc": "2.0",
        "method": "eth_getStorageAt",
        "params": [address, slot_hex, "latest"],
        "id": 1
    }
    try:
        req = urllib.request.Request(RPC_URL, data=json.dumps(payload).encode('utf-8'), headers={'Content-Type': 'application/json'})
        with urllib.request.urlopen(req) as response:
            res = json.loads(response.read().decode())
            if 'error' in res:
                return f"Error: {res['error']['message']}"
            return res.get('result', 'No result')
    except Exception as e:
        return f"Request failed: {str(e)}"

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage:")
        print("  python3 pulse_rpc.py call <address> <data_hex>")
        print("  python3 pulse_rpc.py storage <address> <slot_hex>")
        sys.exit(1)
        
    action = sys.argv[1]
    address = sys.argv[2]
    
    if action == "call" and len(sys.argv) == 4:
        print(eth_call(address, sys.argv[3]))
    elif action == "storage" and len(sys.argv) == 4:
        print(eth_get_storage_at(address, sys.argv[3]))
    else:
        print("Invalid arguments.")
