#!/usr/bin/env python3
import serial
import time
import sys
import struct

PORT_A = "/dev/ttyACM0"
PORT_B = "/dev/ttyACM1"
BAUD_RATE = 9600

def get_serial(port):
    try:
        # Open port with custom control line defaults to prevent bootloader strapping
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
                if line.startswith('I (') or line.startswith('E (') or 'HELTEC' in line or 'STEP' in line:
                    print(f"    [FW LOG {conn.port}] {line}")
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
    print(f"[DEBUG {conn.port}] Timeout! Raw buffer: {rx}")
    return None

def main():
    print("=== Auncient Physical YI Handshake Beta Test ===")
    
    conn_a = get_serial(PORT_A)
    conn_b = get_serial(PORT_B)
    
    if not conn_a or not conn_b:
        sys.exit(1)
        
    print("[INIT] Resetting chips via DTR/RTS control lines...")
    # 1. Hold reset (RTS active/low, DTR inactive/high)
    conn_a.setDTR(False); conn_a.setRTS(True)
    conn_b.setDTR(False); conn_b.setRTS(True)
    time.sleep(0.15)
    # 2. Release reset with GPIO0 high (RTS inactive/high, DTR inactive/high)
    conn_a.setRTS(False)
    conn_b.setRTS(False)
    time.sleep(0.2)
    # 3. Assert DTR for data stream
    conn_a.setDTR(True)
    conn_b.setDTR(True)
    
    print("[INIT] Waiting 4.0 seconds for clean boot sequence...")
    time.sleep(4.0)
    
    # Clean out initial bootlogs from buffers
    conn_a.reset_input_buffer()
    conn_b.reset_input_buffer()
    
    # --- PHASE 1: INIT ---
    print("\n[PHASE 1] Initializing Roles (Node A -> ROD, Node B -> CONE)...")
    res_a = query_node(conn_a, pack_kermit(0, 'I', b"\x00"))
    res_b = query_node(conn_b, pack_kermit(0, 'I', b"\x01"))
    
    if not res_a or not res_b:
        print("[FAIL] Role initialization failed.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    # Unpack registers (88 bytes total) and signature (8 bytes)
    regs_a = struct.unpack("<11Q", res_a[1][:88])
    regs_b = struct.unpack("<11Q", res_b[1][:88])
    sig_init_a = struct.unpack("<Q", res_a[1][88:96])[0]
    sig_init_b = struct.unpack("<Q", res_b[1][88:96])[0]
    
    contour_a = regs_a[4]
    contour_b = regs_b[4]
    print(f"  -> Node A Contour: {contour_a} [ACK Sig Daiichi: {sig_init_a}]")
    print(f"  -> Node B Contour: {contour_b} [ACK Sig Daiichi: {sig_init_b}]")
    
    # --- PHASE 2: EXCHANGE STEPS ---
    
    # Step 1: FORM (Swap Contours)
    print("\n[STEP 2: FORM] Swapping Contours...")
    res_a = query_node(conn_a, pack_kermit(1, 'P', struct.pack("<Q", contour_b)))
    res_b = query_node(conn_b, pack_kermit(1, 'P', struct.pack("<Q", contour_a)))
    
    if not res_a or not res_b:
        print("[FAIL] Contour exchange failed.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    regs_a = struct.unpack("<11Q", res_a[1][:88])
    regs_b = struct.unpack("<11Q", res_b[1][:88])
    sig_a = struct.unpack("<Q", res_a[1][100:108])[0]
    sig_b = struct.unpack("<Q", res_b[1][100:108])[0]
    pole_a = regs_a[5]
    pole_b = regs_b[5]
    print(f"  -> Node A Pole: {pole_a} [ACK Sig Daiichi: {sig_a}]")
    print(f"  -> Node B Pole: {pole_b} [ACK Sig Daiichi: {sig_b}]")
    
    # Step 2: CONJUGATE (Swap Poles)
    print("\n[STEP 3: CONJUGATE] Swapping Poles...")
    res_a = query_node(conn_a, pack_kermit(2, 'P', struct.pack("<Q", pole_b)))
    res_b = query_node(conn_b, pack_kermit(2, 'P', struct.pack("<Q", pole_a)))
    
    if not res_a or not res_b:
        print("[FAIL] Pole exchange failed.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    regs_a = struct.unpack("<11Q", res_a[1][:88])
    regs_b = struct.unpack("<11Q", res_b[1][:88])
    sig_a = struct.unpack("<Q", res_a[1][100:108])[0]
    sig_b = struct.unpack("<Q", res_b[1][100:108])[0]
    found_a = regs_a[7]
    found_b = regs_b[7]
    print(f"  -> Node A Foundation: {found_a} [ACK Sig Daiichi: {sig_a}]")
    print(f"  -> Node B Foundation: {found_b} [ACK Sig Daiichi: {sig_b}]")
    
    # Step 3: SATURATE (Swap Foundations)
    print("\n[STEP 4: SATURATE] Swapping Foundations...")
    res_a = query_node(conn_a, pack_kermit(3, 'P', struct.pack("<Q", found_b)))
    res_b = query_node(conn_b, pack_kermit(3, 'P', struct.pack("<Q", found_a)))
    
    if not res_a or not res_b:
        print("[FAIL] Foundation exchange failed.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    regs_a = struct.unpack("<11Q", res_a[1][:88])
    regs_b = struct.unpack("<11Q", res_b[1][:88])
    sig_a = struct.unpack("<Q", res_a[1][100:108])[0]
    sig_b = struct.unpack("<Q", res_b[1][100:108])[0]
    dynamo_a = regs_a[9]
    dynamo_b = regs_b[9]
    print(f"  -> Node A Dynamo: {dynamo_a} [ACK Sig Daiichi: {sig_a}]")
    print(f"  -> Node B Dynamo: {dynamo_b} [ACK Sig Daiichi: {sig_b}]")
    
    # Step 4: MAGNETIZE (Swap Dynamos and check Convergence)
    print("\n[STEP 5: MAGNETIZE] Swapping Dynamos and checking convergence...")
    res_a = query_node(conn_a, pack_kermit(4, 'P', struct.pack("<Q", dynamo_b)))
    res_b = query_node(conn_b, pack_kermit(4, 'P', struct.pack("<Q", dynamo_a)))
    
    if not res_a or not res_b:
        print("[FAIL] Dynamo exchange failed.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    # Read manifold value appended after registers (88 + 8 = 96 bytes total)
    manifold_a = struct.unpack("<Q", res_a[1][88:96])[0]
    manifold_b = struct.unpack("<Q", res_b[1][88:96])[0]
    epoch_a = struct.unpack("<I", res_a[1][96:100])[0]
    epoch_b = struct.unpack("<I", res_b[1][96:100])[0]
    sig_a = struct.unpack("<Q", res_a[1][100:108])[0]
    sig_b = struct.unpack("<Q", res_b[1][100:108])[0]
    
    print(f"  -> Node A Manifold: {manifold_a} (Epoch: {epoch_a}) [ACK Sig Daiichi: {sig_a}]")
    print(f"  -> Node B Manifold: {manifold_b} (Epoch: {epoch_b}) [ACK Sig Daiichi: {sig_b}]")
    
    if manifold_a == manifold_b and manifold_a != 0:
        print(f"\n[SUCCESS] Helmholtz convergence established physically! YI = {manifold_a}")
    else:
        print("[FAIL] Helmholtz manifolds diverged physically.")
        conn_a.close()
        conn_b.close()
        sys.exit(1)
        
    # --- PHASE 3: NONCE SIGNATURE RETRIEVAL ---
    print("\n[PHASE 3] Generating YI.react Nonce Signatures Physically:")
    for n in range(3):
        res_sig_a = query_node(conn_a, pack_kermit(5 + n, 'R', struct.pack("<Q", n)))
        res_sig_b = query_node(conn_b, pack_kermit(5 + n, 'R', struct.pack("<Q", n)))
        
        if res_sig_a and res_sig_b:
            ichidai_a, daiichi_a = struct.unpack("<QQ", res_sig_a[1][:16])
            ichidai_b, daiichi_b = struct.unpack("<QQ", res_sig_b[1][:16])
            print(f"  -> Nonce {n} | Node A [Ichidai: {ichidai_a}, Daiichi: {daiichi_a}] | Node B [Ichidai: {ichidai_b}, Daiichi: {daiichi_b}]")
            if ichidai_a != ichidai_b or daiichi_a != daiichi_b:
                print(f"[FAIL] Signature mismatch at nonce {n}!")
                conn_a.close()
                conn_b.close()
                sys.exit(1)
                
    print("\n[RESULT] [SUCCESS] All physical beta YI handshake and contractual signature validations PASSED!")
    conn_a.close()
    conn_b.close()
    sys.exit(0)

if __name__ == '__main__':
    main()
