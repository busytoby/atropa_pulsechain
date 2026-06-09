# Podless Multi-User Shared CPU Architecture in Yul

This document describes the design for a shared-contract virtual C64 platform where users maintain distinct CPU execution states (registers, program counter, and isolated local RAM pages) but interact dynamically through shared virtual hardware spaces, collaborative framebuffers, and inter-user interrupt signals.

---

## 1. State Isolation vs. Collective Sharing

In the current virtual machine design, each user's RAM and registers are namespaced by their calling Ethereum address (e.g., `user` prefixing storage slots). To support multi-user collaboration without isolating players into independent "pods", we divide the $64\text{KB}$ memory map into two classes of memory:

1. **Namespaced User Memory (Isolated)**:
   * `$0000`–`$BFFF`: Contains page zero, the stack, user program binaries, and private application variables. This state is unique to each user address.
2. **Global Shared Memory (Collective)**:
   * Enabled via the **Shared Screen Mode** toggle at register `$D50D` (`54541`) in [cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul).
   * When active, Screen RAM (1024-2047), VIC registers (53248-53311), and Color RAM (55296-56295) bypass namespacing and map to a static global address (`0x5555555555555555555555555555555555555555`), allowing multiplayer collaborative screens.

---

## 2. On-Chain Shared Screen Test Verification

We verified the namespacing bypass using the JS test script [test_shared_screen.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_shared_screen.js).

### Verification Steps:
1. **Default Namespaced Check**: User 1 writes `88` to Screen RAM location `1024`. User 2 reads `1024` and gets `0` (confirming default isolation).
2. **Enabling Shared Mode**: Both User 1 and User 2 enable Shared Screen mode by writing `1` to register `54541`.
3. **Collaborative Read/Write**:
   * User 1 writes character `65` ('A') to position `1024`. User 2 immediately peeks `1024` and reads `65`.
   * User 2 writes character `66` ('B') to position `1025`. User 1 immediately peeks `1025` and reads `66`.

### Result:
```
=== Testing Shared Screen Mode on EVM ===
CPU Contract Address: 0x8A3a48De741DDDC9D44dEE32565A54C30Bcd43AA
User 1: 0x70997970C51812dc3A010C7d01b50e0d17dc79C8
User 2: 0x3C44CdDdB6a900fa2b585dd299e03d12FA4293BC

--- Step 1: Default namespaced state test ---
Default mode - User 1 reads: 88, User 2 reads: 0
-> Success: Default namespaced behavior confirmed (User 2 reads 0).

--- Step 2: Enabling Shared Screen Mode at $D50D (54541) ---
Enabled shared screen mode for both users.

--- Step 3: Performing cross-user write/read verification ---
User 1 wrote 65 ('A') to position 1024.
User 2 reads position 1024: 65
User 2 wrote 66 ('B') to position 1025.
User 1 reads position 1025: 66

>>> SUCCESS: Shared Screen Mode successfully bypassed namespacing! <<<
```

---

## 3. Frontend Integration

In [datamost.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/datamost.html), we added a **Join Shared Screen** toggle button in the **Game Cartridge Loader** panel.

* When clicked, the button updates the on-chain shared screen mode state (`poke(54541, 1)`) for the connected wallet, logs status updates to the retro dashboard log, and alters the button status to **Shared Screen: Active** (glowing retro neon blue).
* This allows players to dynamically enter and exit shared multiplayer screen spaces directly from the C64 emulator frontend.
