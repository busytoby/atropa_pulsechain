import requests
import json
import sys
import os
from urllib.parse import quote

# Add current directory to path to find secure_config
sys.path.append(os.getcwd())
from tools import secure_config

def probe_trpc():
    cookie = secure_config.get_secret("FLOW_COOKIE")
    if not cookie:
        print("Error: FLOW_COOKIE not found.")
        sys.exit(1)

    headers = {
        "Cookie": cookie,
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Content-Type": "application/json"
    }

    # Focus on the most likely ones
    procedures = ["user.library", "project.list"]
    base_url = "https://labs.google/api/trpc"

    for proc in procedures:
        input_simple = quote('{"json":{}}')
        url = f"{base_url}/{proc}?batch=1&input={input_simple}"
        
        print(f"Probing {proc}...")
        try:
            resp = requests.get(url, headers=headers)
            print(f"Status: {resp.status_code}")
            print(f"Content-Type: {resp.headers.get('Content-Type')}")
            print(f"Text Preview: {resp.text[:200]}") # Show raw text
            
        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    probe_trpc()
