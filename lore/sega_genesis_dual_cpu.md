# Sega Genesis Dual-CPU Bus Sharing Architecture

The **Sega Genesis** features a asymmetric multiprocessing layout containing a primary **Motorola 68000** CPU (running game logic) and a secondary **Zilog Z80** CPU (running sound drivers and controlling audio chips).

Because both CPUs share access to the system ROM and expansion ports, Sega engineers implemented a hardware handshake mechanism to prevent bus collisions.

---

## 1. The BUSREQ & BUSACK Handshake

The Z80 operates on its own local 8KB RAM, but frequently needs to read audio data or sample streams directly from the main cartridge ROM.

To access the main bus:
1. **Bus Request (`BUSREQ`)**: The Z80 (or main system controller) asserts the `BUSREQ` signal.
2. **Bus Acknowledge (`BUSACK`)**: The 68000 finishes its current bus cycle, releases its control over the address/data buses, and asserts `BUSACK` to signal that the bus is free.
3. **Z80 Access**: The Z80 executes its read/write cycle over the main bus.
4. **Release**: The Z80 releases `BUSREQ`, the 68000 drops `BUSACK`, and the 68000 resumes executing game code.

```
       Z80 CPU                           68000 CPU
      ┌───────┐                         ┌─────────┐
      │       │ ──── BUSREQ (Halt) ───► │  Busy   │
      │       │                         └────┬────┘
      │       │                              │ (Yield Bus)
      │       │ ◄─── BUSACK (Ack) ───────────┘
      │       │
      │       │ ──► [Read Cartridge ROM]
      │       │
      │       │ ──── Release BUSREQ ──► [Resumes]
      └───────┘                         └─────────┘
```

---

## 2. 68000 Control of Z80 Reset

To prevent the Z80 from accessing the bus during crucial memory operations, the 68000 can control the Z80 directly:
* **Reset Register (`$A11200`)**: The 68000 can write `0x0000` to this register to hold the Z80 in a hard reset state (halting it), or `0x0100` to release it.
* **Bus Request Register (`$A11100`)**: The 68000 can write `0x0100` to request the bus from the Z80, allowing the 68000 to write directly into the Z80's local 8KB RAM.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_sega_dual_cpu.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_sega_dual_cpu.js).

This script:
1. Simulates memory-mapped control registers for `$A11100` and `$A11200`.
2. Models the state machine transitions of both the 68000 and Z80 during a bus request event.
3. Verifies that memory access blocks execute only when the proper lock conditions are met.
