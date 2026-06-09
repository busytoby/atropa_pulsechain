# Plan: Migrating Z-Machine Rooms to ZMM VM Layout

This plan outlines the refactoring of [zmachine.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/zmachine.yul) to migrate dynamic rooms out of persistent EVM storage (`sload`/`sstore` mappings) and into a transient, high-speed ZMM VM linear memory architecture.

---

## 1. Objectives

1. **Remove EVM Call Overhead**: Replace external contract interactions (`call`/`staticcall` to room contract addresses) with direct operations on 16-bit big-endian linear registers.
2. **Page-Aligned Memory Mapping**: Map rooms, items, player stats, and chat buffers to fixed ZMM register addresses starting at `0x8000` (e.g., matching VIC-II and ANTIC styles).
3. **Volatile State Registers**: Store dynamic room states (active users, bouncer divisors, guest list counts, and cover charges) inside volatile memory pages instead of persistent EVM storage slots.

---

## 2. Memory Map Design (ZMM MMIO)

| Address Range | Component | Representation | Purpose |
| :--- | :--- | :--- | :--- |
| `0x0000 - 0x7FFF` | **Story ROM Segment** | Static 8-bit array | Z-Machine game bytecode & text assets. |
| `0x8000 - 0x80FF` | **Room Registry Page** | 16-bit registers | Active Room IDs, active bouncers count, cover charge value. |
| `0x8100 - 0x82FF` | **Room Metadata (descriptions)** | Aligned text buffers | Room description strings decoded on-demand. |
| `0x8300 - 0x85FF` | **Subdomain State Registers** | 64-bit status flags | Chat buffers, active user lists, and attribute key-value pairs. |
| `0x8600 - 0x8FFF` | **Guest List / Staff Registry** | Bitmaps | guest/staff registration (1 = active, 0 = inactive). |

---

## 3. Implementation Steps

### Step 1: Update ZMM Memory Helpers
Add JIT memory helpers in Yul to handle 16-bit and 64-bit aligned reads/writes out of the ZMM memory bank:
```yul
function readZmm16(addr) -> val {
    val := shr(240, mload(addr))
}
function writeZmm16(addr, val) {
    mstore(addr, shl(240, val))
}
```

### Step 2: Implement ZMM Bridge
Write `bridgeEvmToZmm` to decode existing persistent token states on startup and page them directly into the ZMM MMIO space.

### Step 3: Refactor Command Parser
Rewrite `parseCommand` inside Yul to consume and modify the registers at `0x8000+` directly instead of calling external EVM contracts.

---

> [!NOTE]
> This refactoring will significantly optimize the terminal's responsiveness and lay the groundwork for high-performance JIT execution loops.
