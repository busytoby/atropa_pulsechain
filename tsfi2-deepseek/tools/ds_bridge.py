import sys
import httpx

class DeepSeekBridge:
    def __init__(self, host='http://127.0.0.1:11435', model='deepseek-coder-v2:16b'):
        self.host = host
        self.model = model

    def query(self, prompt, system="You are a direct, steady assistant from Zhumadian. You specialize in C code, AVX-512, and internal manifold rigidity."):
        url = f"{self.host}/api/chat"
        payload = {
            "model": self.model,
            "messages": [
                {"role": "system", "content": system},
                {"role": "user", "content": prompt}
            ],
            "stream": False
        }
        try:
            response = httpx.post(url, json=payload, timeout=120.0)
            if response.status_code == 200:
                return response.json()['message']['content']
            else:
                return f"[FRACTURE] DeepSeek query failed with status {response.status_code}: {response.text}"
        except Exception as e:
            return f"[FRACTURE] DeepSeek query failed: {e}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 ds_bridge.py <prompt>")
        sys.exit(1)
    
    bridge = DeepSeekBridge()
    print(bridge.query(sys.argv[1]))
