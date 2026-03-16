import os
import requests
import json

def fetch_operations():
    api_key = os.environ.get("GOOGLE_API_KEY")
    if not api_key:
        print("Error: GOOGLE_API_KEY not set.")
        return

    # Updated URL to target specific model (removed limit param)
    url = f"https://generativelanguage.googleapis.com/v1beta/models/veo-3.1-fast-generate-preview/operations?key={api_key}"
    print(f"Fetching operations from {url.split('?')[0]}...")
    
    try:
        resp = requests.get(url)
        if resp.status_code != 200:
            print(f"Error {resp.status_code}: {resp.text}")
            return

        data = resp.json()
        ops = data.get("operations", [])
        print(f"Found {len(ops)} operations.")

        with open("creative/raw_operations_dump.json", "w") as f:
            json.dump(data, f, indent=2)

        print("\n--- Extracted Prompt History ---")
        for op in ops:
            # Metadata dump
            print(f"ID: {op.get('name')}")
            print(f"Metadata: {json.dumps(op.get('metadata', {}), indent=2)}")
            print("-" * 20)

    except Exception as e:
        print(f"Exception: {e}")

if __name__ == "__main__":
    fetch_operations()
