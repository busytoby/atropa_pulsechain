# Asynchronous Timeout Hardening for Winchester MQ

This document details the design and implementation of the real-time clock (RTC) based timeout mechanism for the Winchester MQ 6502 driver layer. This approach addresses the limitations of instruction-loop-based timeouts in offline or high-latency transaction systems.

## The Problem: Instruction-Loop Timeouts

In previous versions of the driver, low-level SASI handshakes (waiting for BSY or REQ signals) were implemented using instruction countdown loops:

```assembly
WaitReqW:
        LDA SASI_SIG_IN
        AND #$02
        BNE GotReqW
        DEX
        BNE WaitReqW
```

While functional in a synchronous, real-time hardware environment, this model fails in a virtualized EVM CPU emulator for two reasons:

1. **Extreme Execution Speed**: An emulated 6502 CPU executes thousands of instructions in a fraction of a millisecond. A 255-iteration decrement loop (`DEX \n BNE`) completes almost instantly (less than 4,000 clock cycles, or ~3-4ms of virtual CPU time).
2. **Asynchronous EVM Time Passage**: In a user-connected, high-latency, or offline transaction system, operations (like writing/reading blocks or committing queues) are processed asynchronously and depend on EVM block times (e.g., 12 seconds per block on PulseChain). 

If the emulator runs the guest CPU code, it will exhaust the instruction-loop countdown and trigger a premature timeout (error code `$80`) before the EVM has had time to mine a block and update the SASI controller signals.

---

## The Solution: RTC-Based Modulo-256 Timers

The emulated `cpu6502.yul` CPU exposes the EVM block timestamp through a memory-mapped Real-Time Clock (RTC) register at address **`$D933`** (`55603` in decimal):

```yul
case 55603 { // RTC Epoch Timestamp
    val := timestamp()
}
```

Since guest memory reads are 8-bit, reading `$D933` returns `and(timestamp(), 0xFF)`, which is the lowest byte of the current Unix epoch timestamp in seconds.

### 1. Robust Time Difference Check
Instead of counting loop cycles, we record the start timestamp and compare it against the current timestamp:

```assembly
        LDA RTC_EPOCH           ; Read low byte of current timestamp
        STA SASI_TMP            ; Store as start time in Zero Page
WaitReq:
        LDA SASI_SIG_IN
        AND #$02                ; Check REQ
        BNE GotReq              ; If ready, proceed
        
        ; Calculate elapsed time and check timeout
        LDA RTC_EPOCH
        SEC
        SBC SASI_TMP            ; A = current - start (modulo 256)
        CMP #SASI_TIMEOUT       ; Compare with timeout limit (e.g., 15s)
        BCC WaitReq             ; Loop if elapsed < timeout
```

### 2. Wrap-Around Arithmetic
Because the timestamp low byte is modulo 256, it wraps around to `0` every 256 seconds. By using standard 6502 two's complement subtraction (`SEC` followed by `SBC`), the borrow bit acts as a borrow flag, correctly computing the positive difference even across the wrap-around boundary:

| Start Time (`$05`) | Current Time (`$D933`) | Unsigned Subtraction (`SBC`) | Elapsed (Seconds) | Result |
| :--- | :--- | :--- | :--- | :--- |
| `250` | `5` (wrapped) | `5 - 250 = 11` | `11` | `< 15` (Proceed) |
| `250` | `25` (wrapped) | `25 - 250 = 31` | `31` | `>= 15` (Timeout) |

---

## Driver and ISR Hardening

The updated files have been placed in the artifact scratch directory:

* **[winchester_mq_driver.asm](file:///home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/scratch/winchester_mq_driver.asm)**: Implements the RTC-based long timeout loops for `WRITE_BYTE`, `READ_BYTE`, and Selection Phases.
* **[winchester_mq_isr.asm](file:///home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/scratch/winchester_mq_isr.asm)**: Refactored to save and restore the zero-page timer temporary (`SASI_TMP` / `$05`) on the stack to prevent corruption during nested interrupts.
* **[mq_daemon.asm](file:///home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/scratch/mq_daemon.asm)**: Uses the updated driver and replaces the non-standard `TXY` instruction with a standard `TXA \n TAY` sequence.

> [!TIP]
> This design ensures that the CPU can execute millions of instructions waiting for an asynchronous event across multiple scheduler time slices without timing out, failing only when the block timestamp advances past the limit.
