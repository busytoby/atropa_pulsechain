import urllib.request
import json
import sys
import time

PULSESCAN_API = "https://scan.pulsechain.com/api"

def get_source_code(address):
    url = f"{PULSESCAN_API}?module=contract&action=getsourcecode&address={address}"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            res = json.loads(response.read().decode())
            if res.get("status") == "1" and res.get("result"):
                return res["result"][0]
            return f"Error: {res.get('message', 'Unknown error')}"
    except Exception as e:
        return f"Request failed: {str(e)}"

def get_creation_data(address):
    url = f"{PULSESCAN_API}?module=contract&action=getcontractcreation&contractaddresses={address}"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            res = json.loads(response.read().decode())
            if res.get("status") == "1" and res.get("result"):
                return res["result"][0]
            return f"Error: {res.get('message', 'Unknown error')}"
    except Exception as e:
        return f"Request failed: {str(e)}"

def get_token_info(address):
    url = f"{PULSESCAN_API}?module=token&action=getToken&contractaddress={address}"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            res = json.loads(response.read().decode())
            if res.get("status") == "1" and res.get("result"):
                return res["result"]
            return f"Error: {res.get('message', 'Unknown error')}"
    except Exception as e:
        return f"Request failed: {str(e)}"


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage:")
        print("  python3 pulse_scan.py source <address>")
        print("  python3 pulse_scan.py creation <address>")
        print("  python3 pulse_scan.py token <address>")
        sys.exit(1)
        
    action = sys.argv[1]
    address = sys.argv[2]
    
    if action == "source":
        data = get_source_code(address)
        if isinstance(data, dict):
            print(f"Contract Name: {data.get('ContractName')}")
            print(f"Compiler Version: {data.get('CompilerVersion')}")
            print(f"Optimization Used: {data.get('OptimizationUsed')}")
            print("\nABI:")
            print(data.get('ABI'))
        else:
            print(data)
    elif action == "creation":
        data = get_creation_data(address)
        if isinstance(data, dict):
            print(f"Creator (TxSender): {data.get('contractCreator')}")
            print(f"Creation TxHash: {data.get('txHash')}")
        else:
            print(data)
    elif action == "token":
        data = get_token_info(address)
        print(json.dumps(data, indent=2))
    else:
        print("Invalid arguments.")
