#!/usr/bin/env python3
import os
import sys
import struct
import hashlib
import json
import urllib.request

def get_address_color(address):
    # Deterministic HSL color derivation matching evolution dashboard
    addr_hash = hashlib.md5(address.lower().encode('utf-8')).hexdigest()
    h = int(addr_hash[10:13], 16) % 360
    s = 0.90
    l = 0.55
    
    # HSL to RGB
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

def fetch_onchain_registers(address):
    # Query our local dashboard server API to resolve on-chain registers
    url = f"http://localhost:8080/api/lau-registers?address={address}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "AuncientDNACompiler/1.0"})
        with urllib.request.urlopen(req, timeout=5) as response:
            return json.loads(response.read().decode('utf-8'))
    except Exception as e:
        print(f"[Warning] Failed to fetch live registers, using hash-based fallback: {e}")
        h = int(hashlib.sha256(address.lower().encode('utf-8')).hexdigest()[:16], 16)
        return {
            "base": h % 1000000000000000,
            "secret": (h * 31) % 1000000000000000,
            "signal": (h * 37) % 1000000000000000,
            "channel": (h * 41) % 1000000000000000,
            "dynamo": (h * 61) % 1000000000000000,
            "foundation": (h * 53) % 1000000000000000,
            "element": (h * 47) % 1000000000
        }

def compile_xray_dna(address):
    print(f"=== Compiling DNA from Aligned X-Ray/Telescope: {address} ===")
    regs = fetch_onchain_registers(address)
    
    # 1. Base/Fur Color from address hash
    fur_r, fur_g, fur_b = get_address_color(address)
    
    # 2. Eye Color derived from Pole/Secret registers
    eye_r = int((regs.get("secret", 0) % 256))
    eye_g = int((regs.get("secret", 0) >> 8) % 256)
    eye_b = int((regs.get("secret", 0) >> 16) % 256)
    
    # 3. Sickness details mapping
    base_sickness = int((regs.get("chin", 0) % 100))
    
    # 4. Scale boundaries
    base_scale = int(100 + (regs.get("foundation", 0) % 50))
    
    # 5. Segment/Fur Length
    base_fur_length = int(80 + (regs.get("element", 0) % 80))
    
    # 6. Light orbit phase
    light_angle_deg = int((regs.get("base", 0) % 256))
    
    # 7. Wavelet and twitch registers
    breathing_freq = int((regs.get("channel", 0) % 256))
    twitch_intensity = int((regs.get("dynamo", 0) % 256))
    
    # Pack parameters to 12-byte binary format
    dna_data = struct.pack('=BBBBBBBBBBBB',
                           fur_r, fur_g, fur_b,
                           eye_r, eye_g, eye_b,
                           base_sickness,
                           base_scale,
                           base_fur_length,
                           light_angle_deg,
                           breathing_freq,
                           twitch_intensity)
                           
    out_dir = "tsfi2-deepseek/assets"
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "bear_genome.dna")
    
    with open(out_path, "wb") as f:
        f.write(dna_data)
        
    print(f"[SUCCESS] DNA genome written to: {out_path}")
    print(f"          Base RGB: ({fur_r},{fur_g},{fur_b}) | Eyes RGB: ({eye_r},{eye_g},{eye_b})")
    print(f"          Sickness: {base_sickness}% | Twitch: {twitch_intensity}")

if __name__ == "__main__":
    addr = sys.argv[1] if len(sys.argv) > 1 else "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e"
    compile_xray_dna(addr)
