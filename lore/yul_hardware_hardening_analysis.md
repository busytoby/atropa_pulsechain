# Yul Hardware Multi-User Hardening Analysis

This document outlines the analysis of multi-user concurrency concerns and transient state corruption vulnerabilities across the Yul-emulated virtual hardware stack on public EVM networks, preserving the historical legacy of the **Auncient** folklore.

## 1. Core Contention Mechanism
In Winchester MQ, the SASI/SCSI bus handshake requires multiple sequential EVM transactions (SEL -> Command Phase -> Data Phase -> Status Phase). Because these transactions are sent individually by the client, concurrent users will interleave their calls. If the transient handshake registers (signals, CDB buffer, data port) are stored in global storage slots, the states will collide, leading to stuck machines or data corruption.

A similar vulnerability exists in any emulated hardware device where:
- State is modified across multiple sequential EVM transactions.
- Execution parameters (registers, screen memory, coordinates) are globally accessible instead of virtualized per calling identity.

---

## 2. Identified Vulnerabilities in Yul Hardware

### A. Video Display Terminal (VDTTerminal - `vdt.yul`)
* **Vulnerability**: The terminal emulates an 80x24 video screen. It stores cursor columns (slot `58000`), cursor rows (slot `58001`), the screen scrolling offset (`58002`), and 1920 character cells (`58010` to `58010 + 1919`) directly in global storage.
* **Impact**: If two users stream character print transactions concurrently, their characters interleave on the same physical slots, and their cursor positions overwrite each other, corrupting the screen stdout for both users.
* **Mitigation**: Virtualize the cursor positions and screen RAM cells by namespacing the storage slots under `caller()`.

### B. Graphics System (GraphicsSystem - `graphicsSystem.yul`)
* **Vulnerability**: Emulates Commodore 64 VIC-II sprite registers (slots `53248` to `53263` for X/Y coordinates, `53269` for sprite enable masks, etc.). Writes and collision checks are performed globally.
* **Impact**: Multiple users updating sprite positions and checking collisions simultaneously will interfere with one another's games. A collision check triggered by User A might run against coordinates just overwritten by User B.
* **Mitigation**: Namespace the sprite coordinate registers and enable masks by `caller()`, ensuring isolated sprite states for concurrent players.

### C. Commodore 64 CPU (CPU6502Emulator - `cpu6502.yul`)
* **Analysis**: The CPU emulator is already hardened; it namespaces register bank indices and CPU RAM pages per calling wallet (`getContextUser()`) via the namespaced `getUserSlot` function. 
* **Remaining Concern**: If `sharedMode` (slot `54541`) is enabled, memory-mapped Screen RAM and VIC-II ranges are redirected to a globally shared address (`0x5555555555555555555555555555555555555555`). While this is a deliberate design choice for cooperative multiplayer games, any multi-transaction sequences executed by guests (such as drawing lines or updating block graphics) remain subject to race conditions and must be coordinated at the application/game logic layer.

---

## 3. Implementation Plan for Namespacing
To resolve these issues, we will introduce a standard virtualizing hash key generator inside `vdt.yul` and `graphicsSystem.yul`:

```yul
function getTransientSlot(index) -> slot {
    mstore(0x280, caller())
    mstore(0x2A0, index)
    slot := keccak256(0x280, 64)
}
```

This isolates the transient and screen registers by wallet, virtualizing the hardware for public EVM execution.
