#!/usr/bin/env python3
import serial
import time
import sys
import struct

PORT = "/dev/ttyACM0"
BAUD_RATE = 9600

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
    time.sleep(0.2)
    
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
                if line.startswith('I (') or line.startswith('E (') or 'HELTEC' in line or 'APOGEE' in line or 'STEP' in line or 'Fused' in line:
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
    print("=== Auncient APOGEE YI FUSE & Nonce Signature Verification Test ===")
    
    try:
        conn = serial.Serial(PORT, BAUD_RATE, timeout=3)
    except Exception as e:
        print(f"[ERROR] Target port {PORT} not available: {e}")
        sys.exit(1)
        
    print(f"[CONNECT] Connected to device on port {PORT}")
    conn.setDTR(True)
    time.sleep(0.2)
    
    # 1. Initialize role as ROD_A
    print("\n[STEP 1] Initializing role to ROD_A...")
    tx = pack_kermit(1, 'I', b"\x00")
    res = query_node(conn, tx)
    if not res or res[0] != 'Y':
        print("[FAIL] Role initialization failed.")
        sys.exit(1)
    print("  [PASS] Role initialized successfully.")
    
    # 2. Send known partner FUSE (under MOTZKIN_PRIME = 953467954114363ULL)
    print("\n[STEP 2] Setting known FUSE parameters (base=1234567, secret=9876543, signal=5555555)...")
    base = 1234567
    secret = 9876543
    signal = 5555555
    prime = 953467954114363
    
    payload = struct.pack("<QQQQ", base, secret, signal, prime)
    tx = pack_kermit(2, 'F', payload)
    res = query_node(conn, tx)
    
    if not res:
        print("[FAIL] No response to FUSE command.")
        sys.exit(1)
        
    type_char, data = res
    if type_char != 'Y' or len(data) < 8:
        print(f"[FAIL] Unexpected FUSE response: type={type_char}, len={len(data)}")
        sys.exit(1)
        
    converged_yi = struct.unpack("<Q", data[:8])[0]
    expected_yi = 5041950426255
    print(f"  -> Converged YI: {converged_yi}")
    if converged_yi == expected_yi:
        print(f"  [PASS] YI matches expected value: {expected_yi}")
    else:
        print(f"[FAIL] YI mismatch! Expected: {expected_yi}, Got: {converged_yi}")
        sys.exit(1)
        
    # 3. Resolve nonces 0, 1, and 2
    print("\n[STEP 3] Resolving signature nonces...")
    expected_signatures = {
        0: (507164254988891, 507164254988891),
        1: (517510242241500, 517510242241500),
        2: (535903767884292, 535903767884292)
    }
    
    all_passed = True
    for nonce in range(3):
        print(f"  Querying Nonce {nonce}...")
        payload = struct.pack("<Q", nonce)
        tx = pack_kermit(3 + nonce, 'R', payload)
        res = query_node(conn, tx)
        
        if not res:
            print(f"    [FAIL] No response for Nonce {nonce}")
            all_passed = False
            continue
            
        type_char, data = res
        if type_char != 'Y' or len(data) < 16:
            print(f"    [FAIL] Unexpected response format for Nonce {nonce}")
            all_passed = False
            continue
            
        ichidai, daiichi = struct.unpack("<QQ", data[:16])
        exp_ichidai, exp_daiichi = expected_signatures[nonce]
        print(f"    -> Ichidai: {ichidai} | Daiichi: {daiichi}")
        
        if ichidai == exp_ichidai and daiichi == exp_daiichi:
            print(f"    [PASS] Nonce {nonce} signatures match expected values.")
        else:
            print(f"    [FAIL] Nonce {nonce} signatures mismatch! Expected Ichidai: {exp_ichidai}, Daiichi: {exp_daiichi}")
            all_passed = False
            
    conn.close()
    
    if all_passed:
        print("\n[RESULT] [SUCCESS] All FUSE reset and Nonce signature validation checks PASSED successfully!")
        sys.exit(0)
    else:
        print("\n[RESULT] [FAILED] One or more verification checks failed.")
        sys.exit(1)

if __name__ == "__main__":
    main()
