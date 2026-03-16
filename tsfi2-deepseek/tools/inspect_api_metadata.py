import os
import json
from google import genai

def inspect_metadata():
    # Load API Key Securely
    api_key = os.environ.get("GOOGLE_API_KEY")
    if not api_key:
        print("Error: GOOGLE_API_KEY environment variable not set.")
        return

    client = genai.Client(api_key=api_key)

    print("--- Inspecting File Metadata ---")
    try:
        files = list(client.files.list())
        print(f"Found {len(files)} files.")
        
        for f in files:
            print(f"\nFile: {f.name}")
            # Dump all attributes
            try:
                # Attempt to convert to dict/json if supported
                if hasattr(f, "to_dict"):
                    print(json.dumps(f.to_dict(), indent=2, default=str))
                else:
                    # Manually list common fields
                    print(f"  Display Name: {getattr(f, 'display_name', 'N/A')}")
                    print(f"  Mime Type: {getattr(f, 'mime_type', 'N/A')}")
                    print(f"  State: {getattr(f, 'state', 'N/A')}")
                    print(f"  URI: {getattr(f, 'uri', 'N/A')}")
            except Exception as e:
                print(f"  Error dumping attributes: {e}")

    except Exception as e:
        print(f"Error listing files: {e}")

if __name__ == "__main__":
    inspect_metadata()
