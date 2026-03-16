import requests
import json
import sys
import os
from pathlib import Path

# Add current directory to path to find secure_config
sys.path.append(os.getcwd())
from tools import secure_config

BUILD_ID = "0QTD5IT7rAmrobzxOA4bS"
# Target the Library page JSON, which should contain the project list
BASE_URL = f"https://labs.google/fx/_next/data/{BUILD_ID}/en/library.json"

def fetch_history():
    # 1. Get Credentials Securely
    cookie = secure_config.get_secret("FLOW_COOKIE")
    if not cookie:
        print("Error: FLOW_COOKIE not found.")
        sys.exit(1)

    # 2. Setup Headers
    headers = {
        "Cookie": cookie,
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Accept": "*/*",
        "Referer": "https://labs.google/fx/library",
        "x-nextjs-data": "1"
    }

    print(f"Fetching from {BASE_URL}...")
    
    try:
        resp = requests.get(BASE_URL, headers=headers)
        
        if resp.status_code == 200:
            data = resp.json()
            print("✅ Success! Library Data retrieved.")
            
            # Save raw dump
            output_path = Path("creative/raw_library_dump.json")
            with open(output_path, "w") as f:
                json.dump(data, f, indent=2)
            print(f"Saved to {output_path}")
            
            # Quick check for projects
            # Inspect structure: props -> pageProps -> ...
            page_props = data.get('pageProps', {})
            # Look for keys like 'projects', 'library', 'assets'
            print("Keys in pageProps:", list(page_props.keys()))
            
        else:
            print(f"❌ Failed: HTTP {resp.status_code}")
            # If 404, maybe the locale path is different or build ID rotated?
            # Try without locale?
            print("Response:", resp.text[:200])
            
    except Exception as e:
        print(f"Error during fetch: {e}")

if __name__ == "__main__":
    fetch_history()
