# Technical Proposal: Yul CPU Upgrades for VIC-20 Emulation

To support authentic VIC-20 hardware programs (such as A.J. Kwitowski's *Sound Concept*) natively on-chain within the Yul-compiled `cpu6502.yul` contract, several memory and peripheral address upgrades should be considered.

---

## 1. VIC-I (MOS 6560/6561) MMIO Space Mapping

Currently, the Yul CPU emulator intercepts Commodore 64 registers (VIC-II at `$D000` and SID at `$D400`). To run VIC-20 code, we must map the VIC-I registers:

*   **Address Range**: `$9000`–`$900F` (36864–36879 decimal).
*   **Registers to Intercept**:
    *   `$9000`–`$9003`: Screen columns, rows, character size, and horizontal/vertical raster offsets.
    *   `$9004`: Current raster line.
    *   `$900A`–`$900D`: Bass, Tenor, Soprano, and Noise voice registers (the target for the Kwitowski-Harris *Sound Concept*).
    *   `$900E`: Auxiliary color and master volume.
*   **Yul Code Upgrade**: In `cpu6502.yul` memory writes, implement address routing:
    ```yul
    if and(gte(addr, 0x9000), lte(addr, 0x900F)) {
        // Store in dedicated VIC-I state variables/storage keys
        sstore(add(VIC_I_STORAGE_OFFSET, sub(addr, 0x9000)), val)
        leave
    }
    ```

---

## 2. VIC-20 RAM Expansion Layouts

The standard VIC-20 has only 5K of RAM. Upgrading memory configurations to support external RAM cartridges requires adding bank-switching boundaries in Yul memory lookups:

| Memory Blocks | Function / Cartridge Slot |
| :--- | :--- |
| `$0000`–`$03FF` | System Areas (Zero page, Stack, Vector maps) |
| `$0400`–`$0FFF` | 3K RAM Expansion (Block 0) |
| `$1000`–`$1FFF` | Standard User BASIC RAM (without expansions) |
| `$2000`–`$7FFF` | 8K RAM Expansion Blocks (1, 2, and 3) |
| `$A000`–`$BFFF` | 8K ROM Expansion (Game Cartridges) |

*   **Upgraded Lookup Logic**: If a RAM expansion is active, the `cpu6502` load/store logic must map addresses dynamically based on configured registers, allowing standard assemblies to run without clashing with the basic zero page.

---

## 3. Dual MOS 6522 VIA (Versatile Interface Adapter) Timers

The VIC-20 relies on dual 6522 VIAs at `$9110`–`$912F` for timers and keyboard interrupts.

*   **On-Chain Decrementing Timers**: Implement decrementing counters based on the VM step cycles. For example, during every `step()` execution block in Yul:
    ```yul
    let timer_a := sload(VIA1_TIMER_A)
    if gt(timer_a, 0) {
        timer_a := sub(timer_a, 1)
        sstore(VIA1_TIMER_A, timer_a)
        if eq(timer_a, 0) {
            trigger_via_interrupt()
        }
    }
    ```
    This achieves deterministic timer interrupts directly on-chain without requiring external clock synchronization from the host system.
