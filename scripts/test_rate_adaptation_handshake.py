#!/usr/bin/env python3
import sys
import random
import time

TOTAL_DATA_BYTES = 1024 * 1024 # 1 MB
MAX_KERMIT_PAYLOAD = 94
SEAL1_TX_OVERHEAD = 8 # Ichidai signature
DATA_PER_PACKET = MAX_KERMIT_PAYLOAD - SEAL1_TX_OVERHEAD # 86 bytes
NUM_PACKETS = (TOTAL_DATA_BYTES + DATA_PER_PACKET - 1) // DATA_PER_PACKET # 12193 packets

# Rate Tiers
TIER_SPEEDS = {
    0: 9600,   # Tier 0: 9.6 kbps
    1: 19200,  # Tier 1: 19.2 kbps
    2: 38400,  # Tier 2: 38.4 kbps
    3: 57600   # Tier 3: 57.6 kbps
}

TX_PACKET_BYTES = 5 + SEAL1_TX_OVERHEAD + DATA_PER_PACKET # 99 bytes
RX_ACK_BYTES = 5 + 8 # 13 bytes (includes Daiichi signature)
GUARD_DELAY_MS = 5 # 5ms each way = 10ms round trip

def run_simulation(loss_rate=0.02):
    print(f"=== Kermit-over-OOK Dynamic Rate Adaptation (DRA) Simulator ===")
    print(f"  Total Data: {TOTAL_DATA_BYTES / 1024:.0f} KB ({NUM_PACKETS} packets)")
    print(f"  Injected Channel Packet Loss Rate: {loss_rate * 100:.1f}%\n")
    
    current_tier = 0
    consecutive_successes = 0
    total_sim_time = 0.0
    packets_transmitted = 0
    retransmissions = 0
    
    successful_packets = 0
    
    while successful_packets < NUM_PACKETS:
        speed_bps = TIER_SPEEDS[current_tier]
        packets_transmitted += 1
        
        # Calculate time for this packet transaction at current speed
        t_tx = (TX_PACKET_BYTES * 8) / speed_bps
        t_ack = (RX_ACK_BYTES * 8) / speed_bps
        t_guards = (GUARD_DELAY_MS * 2) / 1000.0
        t_transaction = t_tx + t_ack + t_guards
        
        # Inject simulated RF drop
        is_corrupt = random.random() < loss_rate
        
        if is_corrupt:
            # Packet dropped/failed validation -> Timeout retransmit + throttle back
            total_sim_time += t_transaction + 1.0 # Add 1s timeout delay
            retransmissions += 1
            consecutive_successes = 0
            old_tier = current_tier
            current_tier = 0 # Instant throttle back
            if old_tier != current_tier:
                print(f"  [RF DROP] Packet {successful_packets} corrupted! Throttling {TIER_SPEEDS[old_tier]/1000:.1f}k -> 9.6k")
        else:
            # Successful transaction
            total_sim_time += t_transaction
            successful_packets += 1
            consecutive_successes += 1
            
            # Check step up condition (10 successes in a row)
            if consecutive_successes >= 10:
                if current_tier < 3:
                    current_tier += 1
                    consecutive_successes = 0
                    print(f"  [STEP UP] 10 consecutive clean packets. Accelerating -> {TIER_SPEEDS[current_tier]/1000:.1f} kbps")
                    
    # Compare with static 9.6k baseline
    baseline_time = 1259.94 # calculated static time
    speedup = baseline_time / total_sim_time
    
    print("\n=== Simulation Results ===")
    print(f"  Total Packets Sent:      {packets_transmitted} (including {retransmissions} retries)")
    print(f"  Simulated Transfer Time: {total_sim_time:.2f} seconds ({total_sim_time/60.0:.2f} minutes)")
    print(f"  Static 9.6k Baseline:    {baseline_time:.2f} seconds ({baseline_time/60.0:.2f} minutes)")
    print(f"  Effective Throughput:    {TOTAL_DATA_BYTES / total_sim_time / 1024:.2f} KB/s")
    print(f"  Performance Speedup:     {speedup:.2f}x")
    print("==========================")

if __name__ == "__main__":
    # Seed for reproducibility
    random.seed(42)
    run_simulation(loss_rate=0.02) # 2% loss rate
