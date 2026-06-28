#!/usr/bin/env python3
import os
import sys
import struct
import math
import hashlib
import json
import urllib.request

def hsl_to_rgb(h, s, l):
    c = (1.0 - abs(2.0 * l - 1.0)) * s
    x = c * (1.0 - abs((h / 60.0) % 2.0 - 1.0))
    m = l - c / 2.0
    if 0 <= h < 60:
        r, g, b = c, x, 0.0
    elif 60 <= h < 120:
        r, g, b = x, c, 0.0
    elif 120 <= h < 180:
        r, g, b = 0.0, c, x
    elif 180 <= h < 240:
        r, g, b = 0.0, x, c
    elif 240 <= h < 300:
        r, g, b = x, 0.0, c
    else:
        r, g, b = c, 0.0, x
    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

def get_address_color(address):
    addr_hash = hashlib.md5(address.lower().encode('utf-8')).hexdigest()
    h = int(addr_hash[10:13], 16) % 360
    return hsl_to_rgb(h, 0.90, 0.55)

def fetch_onchain_registers(address):
    # Query local dashboard API
    url = f"http://localhost:8080/api/lau-registers?address={address}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "LauDNACompiler/1.0"})
        with urllib.request.urlopen(req, timeout=5) as response:
            return json.loads(response.read().decode('utf-8'))
    except Exception:
        h = int(hashlib.sha256(address.lower().encode('utf-8')).hexdigest()[:16], 16)
        return {
            "base": h % 1000000000000000,
            "secret": (h * 31) % 1000000000000000,
            "signal": (h * 37) % 1000000000000000,
            "channel": (h * 41) % 1000000000000000,
            "dynamo": (h * 61) % 1000000000000000,
            "foundation": (h * 53) % 1000000000000000,
            "element": (h * 47) % 1000000000,
            "chin": (h * 59) % 1000000000000000
        }

def compile_lau_timeline_dna(address, out_dir):
    print(f"[DNA Compiler] Generating sequence for: {address}")
    regs = fetch_onchain_registers(address)
    
    TOTAL_FRAMES = 900
    r, g, b = get_address_color(address)
    
    # Eye colors from secret registers
    er = int((regs.get("secret", 0) % 256))
    eg = int((regs.get("secret", 0) >> 8) % 256)
    eb = int((regs.get("secret", 0) >> 16) % 256)
    ec = 8 # 8-eyed Tardigrade structure segment count
    
    os.makedirs(out_dir, exist_ok=True)
    dna_path = os.path.join(out_dir, f"{address.lower()}.dna")
    
    with open(dna_path, "wb") as f:
        f.write(b'TSFI')
        f.write(struct.pack('=I', TOTAL_FRAMES))
        
        # Base settings
        fx = 1.0 + (regs.get("channel", 0) % 5)
        fy = 1.0 + (regs.get("dynamo", 0) % 5)
        fz = 1.0 + (regs.get("foundation", 0) % 5)
        
        for frame in range(TOTAL_FRAMES):
            t = frame / float(TOTAL_FRAMES)
            
            # Morph calculations mapped to timeline t
            morph_factor = 0.0
            if t > 0.30:
                morph_factor = min(1.0, (t - 0.30) / 0.70)
                
            # Idle/Pulse wave
            pulse = math.sin(frame * 0.08) * 0.5 + 0.5
            
            # Pack coordinates and state
            g_x = fx * t
            g_y = fy * t
            stretch = 1.0 + (pulse * 0.05 * (1.0 - morph_factor))
            sick_percent = float(regs.get("chin", 0) % 100) / 100.0
            light_intensity = 0.4 + (0.5 * morph_factor) + (pulse * 0.05)
            
            # Pack 31-byte frame
            frame_data = struct.pack('=ffffffBBBBBBB',
                                     g_x, g_y, stretch, pulse, sick_percent, light_intensity,
                                     r, g, b, er, eg, eb, ec)
            f.write(frame_data)
            
    print(f"[DNA Compiler] Completed: {dna_path}")

def batch_compile():
    print("=== STARTING LAU BATCH TIMELINE DNA COMPILER ===")
    
    # Load all tokens from local server API
    try:
        with urllib.request.urlopen("http://localhost:8080/api/data", timeout=5) as res:
            data = json.loads(res.read().decode('utf-8'))
            tokens = data.get("treasury_tokens", {})
    except Exception:
        print("[Error] Failed to load roster from dashboard server. Ensure the server is running.")
        sys.exit(1)
        
    out_dir = "tsfi2-deepseek/assets/lau_dna"
    os.makedirs(out_dir, exist_ok=True)
    
    # Filter for valid LAU contracts
    lau_addresses = []
    for token in tokens.values():
        addr = token.get("address", "").lower()
        symbol = token.get("symbol", "").lower()
        minter = token.get("minter_name", "").lower()
        
        if token.get("ignored"):
            continue
            
        is_lau = ("lau" in symbol or 
                  "lau" in minter or 
                  minter == "cho registry" or
                  addr == "0xd32c39fee49391c7952d1b30b15921b0d3b42e69" or 
                  addr == "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b" or
                  addr == "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e" or
                  addr == "0xad4e198623a5e2723e19e4d4a6ecf72b1d19fe4b") and len(symbol) < 16
                  
        if is_lau:
            lau_addresses.append(token["address"])
            
    print(f"[DNA Compiler] Found {len(lau_addresses)} active LAU contracts in roster.")
    
    for addr in lau_addresses:
        compile_lau_timeline_dna(addr, out_dir)
        
    print("=== BATCH DNA COMPILATION COMPLETE ===")

if __name__ == "__main__":
    batch_compile()
