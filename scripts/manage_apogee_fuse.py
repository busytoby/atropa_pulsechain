#!/usr/bin/env python3
import serial
import time
import sys
import struct
import argparse

BAUD_RATE = 9600

def get_serial(port):
    try:
        conn = serial.Serial(port, BAUD_RATE, timeout=3)
        return conn
    except Exception as e:
        print(f"[ERROR] Port {port} unavailable: {e}")
        return None

def kermit_checksum(buf):
    sum_val = sum(buf)
    return (((sum_val + ((sum_val & 0xC0) >> 6)) & 0x3F) + 32)

def pack_kermit(seq, type_char, data=b""):
    out = bytearray([0x01, len(data) + 3 + 32, seq + 32, ord(type_char)])
    out.extend(data)
    out.append(kermit_checksum(out[1:]))
    return out

def parse_kermit(buf):
    if len(buf) < 5 or buf[0] != 0x01:
        return None
    length = buf[1] - 32
    seq = buf[2] - 32
    type_char = chr(buf[3])
    data_len = length - 3
    if len(buf) < data_len + 5:
        return None
    data = buf[4 : 4 + data_len]
    check = buf[4 + data_len]
    calc = kermit_checksum(buf[1 : 4 + data_len])
    if calc != check:
        return None
    return type_char, data

def query_node(conn, tx_frame):
    conn.write(tx_frame)
    conn.flush()
    time.sleep(0.15)
    
    rx = bytearray()
    start = time.time()
    while time.time() - start < 3.0:
        if conn.in_waiting > 0:
            new_bytes = conn.read(conn.in_waiting)
            rx.extend(new_bytes)
            
            # Print complete ASCII log lines
            while b'\n' in rx:
                idx = rx.index(b'\n')
                line = rx[:idx+1].decode('utf-8', errors='ignore').strip()
                if line.startswith('I (') or line.startswith('E (') or 'HELTEC' in line or 'APOGEE' in line or 'STEP' in line:
                    print(f"    [FW LOG] {line}")
                rx = rx[idx+1:]
                
            for i in range(len(rx)):
                if rx[i] == 0x01:
                    if len(rx) > i + 1:
                        length = rx[i + 1] - 32
                        if len(rx) >= i + length + 2:
                            res = parse_kermit(rx[i : i + length + 2])
                            if res:
                                return res
        time.sleep(0.05)
    return None

def main():
    parser = argparse.ArgumentParser(description="Retrieve, Set, and Store APOGEE YI FUSE Configuration on Heltec Devices")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Target serial port (e.g. /dev/ttyACM0)")
    parser.add_argument("--action", choices=["get", "set"], default="get", help="Action to perform (get or set)")
    parser.add_argument("--base", type=int, help="APOGEE FUSE Base value (uint64_t)")
    parser.add_argument("--secret", type=int, help="APOGEE FUSE Secret value (uint64_t)")
    parser.add_argument("--signal", type=int, help="APOGEE FUSE Signal value (uint64_t)")
    parser.add_argument("--prime", type=int, default=953473, help="FUSE Prime modulus (default APOGEE: 953473)")
    parser.add_argument("--file", help="Path to .dys configuration file to load parameters from")
    
    args = parser.parse_args()
    
    # Load from file if specified
    f_base, f_secret, f_signal, f_prime = args.base, args.secret, args.signal, args.prime
    if args.file:
        print(f"[FILE] Loading parameters from {args.file}...")
        try:
            with open(args.file, "r") as f:
                for line in f:
                    line = line.strip()
                    if not line or "=" not in line:
                        continue
                    k, v = line.split("=", 1)
                    k, v = k.strip().lower(), int(v.strip())
                    if k == "base": f_base = v
                    elif k == "secret": f_secret = v
                    elif k == "signal": f_signal = v
                    elif k == "prime": f_prime = v
        except Exception as e:
            print(f"[ERROR] Failed to read file {args.file}: {e}")
            sys.exit(1)
            
    conn = get_serial(args.port)
    if not conn:
        sys.exit(1)
        
    print(f"[CONNECT] Connected to device on port {args.port}")
    
    # Assert DTR to enable USB communication
    conn.setDTR(True)
    time.sleep(0.1)
    
    if args.action == "get":
        print(f"[GET] Querying current stored APOGEE FUSE parameters...")
        tx = pack_kermit(0, 'V')
        res = query_node(conn, tx)
        if res:
            type_char, data = res
            if type_char == 'Y' and len(data) >= 32:
                base, secret, signal, converged_yi = struct.unpack("<QQQQ", data[:32])
                print("\n=== Stored APOGEE FUSE Configuration ===")
                print(f"  Base:     {base}")
                print(f"  Secret:   {secret}")
                print(f"  Signal:   {signal}")
                print(f"  YI (Manifold): {converged_yi}")
                print("========================================")
            else:
                print(f"[ERROR] Unexpected response: Type: {type_char}, Data length: {len(data)}")
        else:
            print("[ERROR] No response received from device.")
            
    elif args.action == "set":
        if f_base is None or f_secret is None or f_signal is None:
            print("[ERROR] Action 'set' requires --base, --secret, and --signal arguments or a valid --file.")
            sys.exit(1)
            
        print(f"[SET] Fusing device with Base={f_base}, Secret={f_secret}, Signal={f_signal}, Prime={f_prime}...")
        payload = struct.pack("<QQQQ", f_base, f_secret, f_signal, f_prime)
        tx = pack_kermit(0, 'F', payload)
        res = query_node(conn, tx)
        if res:
            type_char, data = res
            if type_char == 'Y' and len(data) >= 8:
                converged_yi = struct.unpack("<Q", data[:8])[0]
                print("\n=== APOGEE YI FUSE Configuration Successful ===")
                print(f"  New Converged YI: {converged_yi}")
                print("===============================================")
            else:
                print(f"[ERROR] Unexpected response: Type: {type_char}, Data length: {len(data)}")
        else:
            print("[ERROR] No response or verification received from device.")
            
    conn.close()

if __name__ == "__main__":
    main()
