# TSFi2 Virtual Interrupt Standards (Ahoy! / C64 Emulation)

This document establishes the hardware-level emulation standards for handling interrupts (IRQs and NMIs) in the TSFi2 virtual ecosystem, ensuring deterministic real-time musical playback, graphics synchronization, and timer routines between the on-chain Yul contracts and the browser simulation console.

---

## 1. Interrupt Vector Allocation

To emulate 6502 interrupt vectors inside the EVM space, we map key pointers from RAM ($0314–$031B and $FFFA–$FFFF) to virtual EVM execution hooks:

| Target Vector | Hardware Address | Vector Pointer | Purpose | TSFi2 Mapping Handler |
| :--- | :--- | :--- | :--- | :--- |
| **NMI** (Non-Maskable) | `$FFFA–$FFFB` | `$0318–$0319` | Restore key, CIA 2 Timer A/B interrupts | `cpu6502.yul` state check for NMI flag |
| **RESET** | `$FFFC–$FFFD` | N/A | Cold/Warm boot vector | Contract initial constructor state |
| **IRQ / BRK** | `$FFFE–$FFFF` | `$0314–$0315` | Raster beam, CIA 1 Timers, Playback | Main periodic loop handler `runSteps` |

---

## 2. Standardized Timing Intervals

To prevent frame rate drift and guarantee musical tempo consistency, we standardize on two primary interrupt rates:

### A. The 60Hz System Jiffy (Raster / Vertical Blank)
* **Real-world frequency**: 60 Hz (NTSC) / 50 Hz (PAL).
* **Virtual Rate**: 1 jiffy per block or simulated time increment.
* **Purpose**: Video beam updates, keyboard matrix scanning, and general music pitch/gate step modulation.

### B. Custom Timer Ticks (CIA 1 Timer A / B)
* **Real-world frequency**: Programmable via 16-bit timer intervals at 1.02 MHz CPU speed.
* **Virtual Emulation Rate**: Memory locations `56324` (`$DC04`) and `56325` (`$DC05`) store the Timer A latch. When timer reaches zero, a bit is set in `$DC0D` (`56333`), triggering the virtual interrupt flag.

---

## 3. The Yul Interrupt Loop Standard

For the Yul CPU to process interrupts deterministically without blocking the Ethereum Virtual Machine, we utilize a status-flag register map:

```mermaid
graph TD
    Step[Run CPU Step] --> CheckIRQ{IRQ Flag Active?}
    CheckIRQ -- Yes & I_Bit=0 -- > SaveState[Push PC & SR to Stack]
    SaveState --> JumpVector[Jump to Address at $0314]
    JumpVector --> ExecHandler[Execute Interrupt Handler]
    ExecHandler --> RTI[Return from Interrupt - RTI]
    CheckIRQ -- No / I_Bit=1 -- > ExecNormal[Execute Normal Opcode]
```

### Virtual Register Definitions
*   **IRQ Disable Flag (`I` bit in Status Register)**: Mask interrupts when set to `1`.
*   **Interrupt Request Register (`$D019` / `53273`)**: Emulates VIC-II raster interrupt status. Writing `1` clears the pending flag.
*   **Interrupt Control Register (`$DC0D` / `56333`)**: Emulates CIA 1 interrupt sources.

---

## 4. Web Audio & Synth Buffer Standards

When the browser-side simulation console captures SID voice states, it aligns Web Audio scheduling with the virtual interrupt cadence:

1. **Jiffy Scheduler**: Browser poll loops check the block timestamp or frame count.
2. **Buffer Ahead**: Playback transitions are scheduled `15ms` ahead to avoid audio glitches or skips during high-latency web3 operations.
3. **Mute State Isolation**: Ensure oscillator instances are recycled or zeroed out immediately if the gate bit drops to `0`.
