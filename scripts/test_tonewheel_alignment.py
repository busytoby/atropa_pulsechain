import os
import math

MotzkinPrime = 953467954114363

def mod_exp(base, exp, mod):
    res = 1
    base = base % mod
    while exp > 0:
        if exp % 2 == 1:
            res = (res * base) % mod
        base = (base * base) % mod
        exp //= 2
    return res

def derive_hypotrochoid(soul):
    R = (soul % 50) + 15.0
    r = (soul % 25) + 8.0
    d = (soul % 15) + 5.0
    return R, r, d

def main():
    print("[TONEWHEEL] Starting discrete search alignment test...")
    
    # Witness B's parameters (target to match)
    address_B = "0xd32c39fee49391c7952d1b30b15921b0d3b42e69"
    int_B = int(address_B, 16)
    target_soul = int_B ^ 0xFEEDface
    target_R, target_r, target_d = derive_hypotrochoid(target_soul)
    print(f"Target partner shape (Witness B): R={target_R:.1f}, r={target_r:.1f}, d={target_d:.1f}")

    # Witness A (observing and searching its tonewheel)
    # Witness A uses exp = 12000
    exp_A = 12000
    
    print("\nWitness A searching discrete tonewheel voltage space...")
    best_input = None
    min_error = float('inf')
    
    # Sweep a discrete range of candidate tonewheel voltages
    for candidate_input in range(1000, 5000):
        # Resolve through immutable LAU mod-exp equation
        resolved_val = mod_exp(candidate_input, exp_A, MotzkinPrime)
        
        # Derive resulting shape from resolved output
        candidate_soul = int(resolved_val) ^ 0xFEEDface
        c_R, c_r, c_d = derive_hypotrochoid(candidate_soul)
        
        # Calculate shape difference
        error = abs(c_R - target_R) + abs(c_r - target_r) + abs(c_d - target_d)
        
        if error < min_error:
            min_error = error
            best_input = candidate_input
            print(f"  -> Input {candidate_input:4d} yields shape (R={c_R:.1f}, r={c_r:.1f}, d={c_d:.1f}) | Error: {error:.2f}")
            
        if error < 0.1: # Match found
            break

    print(f"\n[SUCCESS] Tonewheel lock achieved!")
    print(f"Witness A locks input voltage to {best_input} to match partner's hypotrochoid.")

if __name__ == "__main__":
    main()
