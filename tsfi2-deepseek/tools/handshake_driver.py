import sys
import os
import json
import httpx
import time

# Host and Port for TSFi RPC (assuming MCP bridge)
RPC_URL = "http://127.0.0.1:10042"

def rpc_call(method, params=None):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or {},
        "id": 1
    }
    try:
        response = httpx.post(RPC_URL, json=payload, timeout=120.0)
        return response.json()
    except Exception as e:
        return {"error": str(e)}

def epoch_name(e):
    names = ["STAT", "AVAIL", "FORM", "POLARIZE", "CONJUGATE", "CONIFY", "SATURATE", "IONIZE", "MAGNETIZE"]
    return names[e] if e < len(names) else "DONE"

def main():
    print("=== TSFi Rigid Handshake Driver (9-Epoch Trilateral) ===")
    
    # 1. Initialize Handshake
    print(f"-> Phase 0: Initializing Handshake...")
    res = rpc_call("manifold.start_handshake")
    if "error" in res:
        print(f"[FRACTURE] Failed to start handshake: {res['error']}")
        return

    for epoch in range(9):
        print(f"\n--- EPOCH {epoch}: {epoch_name(epoch)} ---")
        
        # User Action: Manual Norm/Key Injection
        print(f"Input required for User Pole (Epoch {epoch}).")
        try:
            norm = input(f"  Enter Norm value (default {137+epoch}): ") or str(137+epoch)
            key = input(f"  Enter Phase Key (hex, default 0x{epoch:02x}): ") or f"0x{epoch:02x}"
            
            print(f"-> Injecting User Pole: norm={norm}, key={key}")
            rpc_call("manifold.inject_user_pole", {"norm": norm, "phase_key": key})
        except KeyboardInterrupt:
            print("\nAborting handshake.")
            break

        # DeepSeek Action: Step Resonance
        print(f"-> Pulsing DeepSeek for Epoch {epoch} resonance...")
        rpc_call("manifold.handshake_step")
        
        # Poll for DeepSeek confirmation
        print("Waiting for DeepSeek resonance...", end="", flush=True)
        confirmed = False
        while not confirmed:
            poll = rpc_call("manifold.poll_handshake")
            if "result" in poll and poll["result"].get("status") == "confirmed":
                print(" [RESORANT]")
                print(f"  Receipt: {poll['result'].get('receipt_id')}")
                confirmed = True
            else:
                print(".", end="", flush=True)
                time.sleep(2)
        
        # Gemini/System Action: Verification
        heat_res = rpc_call("manifold.get_heat")
        heat = heat_res.get("result", {}).get("heat", 0.0)
        print(f"-> System Heat: {heat:.4f} (Banach Rigidity)")
        
        if heat > 0.5:
            print("[WARNING] High entropy detected. Rigid path is fracturing.")

    print("\n=== Handshake Complete: MAGNETIZE Phase Rigidified. ===")

if __name__ == "__main__":
    main()
