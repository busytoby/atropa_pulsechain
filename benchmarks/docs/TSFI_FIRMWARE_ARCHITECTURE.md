# TSFi Firmware Architecture: The Verilog Transpiler & HLS

## 1. Overview
Project Mariner Phase 1 establishes **Absolute Determinism** by shifting the foundational system logic from C-level software structures to Hardware Description Language (HDL). The core system manifold is now defined in pure Verilog (`src/firmware/LauWireFirmware.v`) and synthesized into a cycle-accurate C model via our native transpiler, `tsfi_v2c`.

This guarantees that the TSFi execution boundary is mathematically rigorous. Software pointers cannot corrupt hardware registers; execution only advances on explicit clock strobes.

## 2. The `HELMHOLTZ_LIST` as Hardware Buses
In the software iteration, the `HELMHOLTZ_LIST` provided function pointers for execution, state polling, and directives. In the firmware architecture, these are mapped to explicit, physical buses on the top-level Verilog module:

*   **The Epoch Bus (`step_safety_epoch` -> `epoch_strobe`):** The primary clock driver. Asserting this wire advances the sequential logic (flip-flops) of the firmware by exactly one cycle.
*   **The State Bus (`step_safety_state` -> `state_ready`):** A combinatorial readout. Allows the host C runtime to poll the hardware's readiness without advancing the clock.
*   **The Directive Bus (`step_executor_directive` -> `directive_cmd`, `directive_strobe`):** A 32-bit command bus. Pushes structured signals (e.g., 32-bit hashes for "STOP" or "FLUSH") into the hardware state machine.
*   **The Provenance Bus (`provenance` -> `provenance_hash` or `wavefront_ref`):** Originally envisioned to supply continuous cryptographic affirmation, this bus is expanded to supply the **zero-copy reference wavefronts**. It allows the firmware to directly access external 512-bit vectors (like `vec_a`, `vec_b`) without duplicating memory, establishing a mathematically pure interface for the macro-cells.

## 3. Peripheral Isolation (The PTY IP Block)
Peripherals, such as the `LauWirePty` (which manages the Gemini CLI subshell), are treated as external hardware IP blocks. They do not share memory with the firmware.

Communication is strictly defined by structural pins:
*   **Rx Pins (`pty_rx_data`, `pty_rx_valid`, `pty_rx_ready`):** The PTY drives data into the firmware's internal FIFOs using a standard valid/ready handshake, providing hardware-level backpressure.
*   **Tx Pins (`pty_tx_data`, `pty_tx_valid`, `pty_tx_ready`):** The firmware drives data back out to the PTY.

This eliminates POSIX pipe deadlocks and terminal hijacking (`SIGTTOU`). The Vulkan display merely reads the `BASE` memory updated combinatorially by the firmware.

## 4. High-Level Synthesis (HLS) and Macro-Cells
The most powerful aspect of the `tsfi_v2c` transpiler is its ability to seamlessly integrate Gemini-scale performance primitives into the Verilog simulation.

We define empty "Black Box" macro-cells in Verilog:
```verilog
module Wave512_Add (
    input wire [511:0] vec_a,
    input wire [511:0] vec_b,
    output wire [511:0] vec_out
);
```

Crucially, these 512-bit vectors do not need to be duplicated inside the firmware's internal memory. The **Provenance Bus** (which originally supplied the cryptographic affirmation) is expanded to act as the supplier of the zero-copy **reference wavefront**. The `vec_a`, `vec_b`, and `vec_out` wires in the Verilog module map directly to the zero-copy pointers maintained by the overarching software.

When `tsfi_v2c` parses the instantiation of this module, it does not attempt to simulate the logic gates. Instead, it emits the literal, zero-overhead AVX-512 intrinsic directly into the C implementation of the combinatorial logic, operating directly on the zero-copy wavefront:
```c
state->wave_out = _mm512_add_epi32(state->vec_a, state->vec_b);
```

This merges the strict formal verification of Verilog state machines with the unbridled throughput of CPU vector units, creating a hybrid compute fabric capable of extreme performance without sacrificing determinism.