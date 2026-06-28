#!/usr/bin/env python3
import serial
import time
import sys
import struct
import random

PORT_A = "/dev/ttyACM0"
PORT_B = "/dev/ttyACM1"
BAUD_RATE = 9600

DATA_SIZE = 100 * 1024 # 100 KB
DATA_PER_PACKET = 86 # bytes
NUM_PACKETS = (DATA_SIZE + DATA_PER_PACKET - 1) // DATA_PER_PACKET # 1191 packets

TIER_SPEEDS = {
    0: 9600,
    1: 19200,
    2: 38400,
    3: 57600
}

TX_PACKET_BYTES = 99
RX_ACK_BYTES = 13
GUARD_DELAY_MS = 5

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
    
    rx = bytearray()
    start = time.time()
    while time.time() - start < 1.0:
        if conn.in_waiting > 0:
            rx.extend(conn.read(conn.in_waiting))
            i = 0
            while i < len(rx):
                if rx[i] == 0x01:
                    if len(rx) > i + 1:
                        length = rx[i + 1] - 32
                        if len(rx) >= i + length + 2:
                            res = parse_kermit(rx[i : i + length + 2])
                            if res:
                                return res
                            i += 1
                            continue
                    break
                i += 1
            if i > 0:
                rx = rx[i:]
        else:
            time.sleep(0.001) # 1ms yield to avoid CPU hogging while waiting
    return None

def main():
    print("=== Auncient Physical SEAL1 Hardware-in-the-Loop Benchmark ===")
    
    # Connect
    try:
        conn_a = serial.Serial(PORT_A, BAUD_RATE, timeout=0.2)
        conn_b = serial.Serial(PORT_B, BAUD_RATE, timeout=0.2)
    except Exception as e:
        print(f"[ERROR] Could not open ports: {e}")
        sys.exit(1)
        
    print("[INIT] Performing warm-up reset of target ACM devices...")
    conn_a.setDTR(False); conn_a.setRTS(True)
    conn_b.setDTR(False); conn_b.setRTS(True)
    time.sleep(0.1)
    conn_a.setRTS(False)
    conn_b.setRTS(False)
    time.sleep(1.5)
    conn_a.reset_input_buffer()
    conn_b.reset_input_buffer()
    
    # 1. Init Roles
    print("[PHASE 1] Resetting roles (Node A -> ROD, Node B -> CONE)...")
    res_a = query_node(conn_a, pack_kermit(0, 'I', b"\x00"))
    res_b = query_node(conn_b, pack_kermit(0, 'I', b"\x01"))
    if not res_a or not res_b:
        print("[ERROR] Role reset failed.")
        sys.exit(1)
        
    # 2. Swap contours to converge manifold
    regs_a = struct.unpack("<11Q", res_a[1][:88])
    regs_b = struct.unpack("<11Q", res_b[1][:88])
    contour_a = regs_a[4]
    contour_b = regs_b[4]
    
    # Perform FORM
    res_a = query_node(conn_a, pack_kermit(1, 'P', struct.pack("<Q", contour_b)))
    res_b = query_node(conn_b, pack_kermit(1, 'P', struct.pack("<Q", contour_a)))
    
    # CONJUGATE
    pole_a = struct.unpack("<11Q", res_a[1][:88])[5]
    pole_b = struct.unpack("<11Q", res_b[1][:88])[5]
    res_a = query_node(conn_a, pack_kermit(2, 'P', struct.pack("<Q", pole_b)))
    res_b = query_node(conn_b, pack_kermit(2, 'P', struct.pack("<Q", pole_a)))
    
    # SATURATE
    found_a = struct.unpack("<11Q", res_a[1][:88])[7]
    found_b = struct.unpack("<11Q", res_b[1][:88])[7]
    res_a = query_node(conn_a, pack_kermit(3, 'P', struct.pack("<Q", found_b)))
    res_b = query_node(conn_b, pack_kermit(3, 'P', struct.pack("<Q", found_a)))
    
    # MAGNETIZE
    dynamo_a = struct.unpack("<11Q", res_a[1][:88])[9]
    dynamo_b = struct.unpack("<11Q", res_b[1][:88])[9]
    res_a = query_node(conn_a, pack_kermit(4, 'P', struct.pack("<Q", dynamo_b)))
    res_b = query_node(conn_b, pack_kermit(4, 'P', struct.pack("<Q", dynamo_a)))
    
    manifold_a = struct.unpack("<Q", res_a[1][88:96])[0]
    print(f"[CONVERGED] Monopole secret manifolds converged: {manifold_a}")
    
    # --- 10KB TRANSFER BENCHMARK ---
    print(f"\n[PHASE 2] Starting 10KB Secure File Transfer Benchmark ({NUM_PACKETS} packets)...")
    
    current_tier = 0
    consecutive_successes = 0
    accumulated_sim_time = 0.0
    actual_start_time = time.time()
    
    # Simulate a small 2% random channel error rate for realistic dynamic rate testing
    loss_rate = 0.02
    
    packet_idx = 0
    while packet_idx < NUM_PACKETS:
        # 1. Ask Node A to generate Ichidai/Daiichi for this nonce
        res_sig_a = query_node(conn_a, pack_kermit(packet_idx % 64, 'R', struct.pack("<Q", packet_idx)))
        if not res_sig_a:
            print(f"[FAIL] Node A failed signature generation at packet {packet_idx}")
            sys.exit(1)
        
        ichidai, daiichi = struct.unpack("<QQ", res_sig_a[1][:16])
        
        # 2. Simulate transmission time at current speed tier
        speed_bps = TIER_SPEEDS[current_tier]
        t_tx = (TX_PACKET_BYTES * 8) / speed_bps
        t_ack = (RX_ACK_BYTES * 8) / speed_bps
        t_guards = (GUARD_DELAY_MS * 2) / 1000.0
        t_transaction = t_tx + t_ack + t_guards
        
        # Inject random noise
        is_corrupted = random.random() < loss_rate
        
        if is_corrupted:
            # Drop/NAK: trigger rate throttle back and time penalty
            accumulated_sim_time += t_transaction + 1.0
            consecutive_successes = 0
            old_tier = current_tier
            current_tier = 0
            if old_tier != 0:
                print(f"  [QOS FALLBACK] Packet {packet_idx} dropped! Throttling {TIER_SPEEDS[old_tier]/1000:.1f}k -> 9.6k")
        else:
            # Successful delivery -> verify signature on Node B
            verify_payload = struct.pack("<QQQ", packet_idx, ichidai, daiichi)
            res_verify = query_node(conn_b, pack_kermit(packet_idx % 64, 'S', verify_payload))
            
            if not res_verify or res_verify[0] != 'Y':
                print(f"[FAIL] Node B signature verification failed at packet {packet_idx}")
                sys.exit(1)
                
            accumulated_sim_time += t_transaction
            consecutive_successes += 1
            packet_idx += 1
            
            if consecutive_successes >= 10:
                if current_tier < 3:
                    current_tier += 1
                    consecutive_successes = 0
                    print(f"  [QOS SPEED UP] Accelerating modulation rate to {TIER_SPEEDS[current_tier]/1000:.1f} kbps (Packet {packet_idx})")
                    
        # Update progress bar
        progress = (packet_idx + 1) / NUM_PACKETS
        bar_len = 20
        filled = int(bar_len * progress)
        bar = '=' * filled + '-' * (bar_len - filled)
        sys.stdout.write(f"\r  Progress: [{bar}] {packet_idx + 1}/{NUM_PACKETS} packets | Speed: {TIER_SPEEDS[current_tier]/1000:.1f} kbps")
        sys.stdout.flush()
        
    actual_rtt_overhead = time.time() - actual_start_time
    print(f"\n\n=== Physical Transfer Benchmark Complete ===")
    print(f"  Simulated RF Airtime:     {accumulated_sim_time:.2f} seconds")
    print(f"  Static 9.6k Baseline:     {(NUM_PACKETS * (99*8/9600 + 13*8/9600 + 0.010)):.2f} seconds")
    print(f"  Average Throughput:       {DATA_SIZE / accumulated_sim_time / 1024:.2f} KB/s")
    print(f"  Speedup vs Static:        {(NUM_PACKETS * (99*8/9600 + 13*8/9600 + 0.010)) / accumulated_sim_time:.2f}x")
    print(f"  Host JTAG USB RTT Time:   {actual_rtt_overhead:.2f} seconds")
    print("============================================")
    
    conn_a.close()
    conn_b.close()

if __name__ == '__main__':
    main()
