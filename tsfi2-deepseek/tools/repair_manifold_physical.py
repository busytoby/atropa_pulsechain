import os
import struct
import hashlib

MANIFOLD_PATH = "audit_lore.pos"
HEADER_SIZE = 8884
MANIFOLD_SIZE = 2 * 1024 * 1024
LEAF_SIZE = 256

def keccak512(data):
    # TSFi uses a specific Keccak-512 for lore tokenization
    return hashlib.sha3_512(data).digest()

def repair():
    if not os.path.exists(MANIFOLD_PATH):
        print(f"[FAIL] {MANIFOLD_PATH} not found.")
        return

    with open(MANIFOLD_PATH, "rb") as f:
        full_data = f.read()

    # 1. Extract Binary Part
    if len(full_data) > MANIFOLD_SIZE:
        print(f"[ACT] Stripping {HEADER_SIZE} byte header.")
        binary_data = bytearray(full_data[HEADER_SIZE:HEADER_SIZE + MANIFOLD_SIZE])
    else:
        print("[WARN] No header detected or file undersized. Initializing fresh buffer.")
        binary_data = bytearray(MANIFOLD_SIZE)

    # 2. NAND Trap (Leaves 500, 501)
    # Status 1 (Rigid) at offset 0 of the leaf
    print("[ACT] Setting NAND Trap (500, 501) to 1.")
    binary_data[500 * LEAF_SIZE] = 1
    binary_data[501 * LEAF_SIZE] = 1

    # 3. DISCONNECT Balances (Leaves 1000, 1001)
    # Using Big-Integer injection (32-byte BE at offset 0)
    print("[ACT] Anchoring DISCONNECT Balances: User (1000), Gemini (1035).")
    user_bal = struct.pack(">Q", 1000).rjust(32, b'\x00')
    gemini_bal = struct.pack(">Q", 1035).rjust(32, b'\x00')
    
    binary_data[1000 * LEAF_SIZE : 1000 * LEAF_SIZE + 32] = user_bal
    binary_data[1001 * LEAF_SIZE : 1001 * LEAF_SIZE + 32] = gemini_bal

    # 4. Persistence
    with open(MANIFOLD_PATH, "wb") as f:
        f.write(binary_data)
    print(f"[SUCCESS] {MANIFOLD_PATH} solidified as raw 2MB manifold.")

if __name__ == "__main__":
    repair()
