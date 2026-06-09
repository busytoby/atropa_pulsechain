# Enhancing the System for Agentic CPU Testing

This document specifies the integration of debug ports, telemetry outputs, and input injection vectors designed to enhance an agent's ability to test and validate the virtual CPU (`cpu6502.yul` and `FolkloreCPU`).

---

## 1. Emulated Debug Registers & Registers Map ($D6C0–$D6CF)

To enable deep state inspection during test execution, we map the virtual CPU's internal registers directly to a memory-mapped I/O debug window:

| Address (Hex) | Dec Address | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6C0` | `54976` | Read-Only | **Program Counter Low (PC_LO)**: Current instruction pointer. |
| `$D6C1` | `54977` | Read-Only | **Program Counter High (PC_HI)** |
| `$D6C2` | `54978` | Read-Only | **Accumulator (A)** |
| `$D6C3` | `54979` | Read-Only | **X Index Register** |
| `$D6C4` | `54980` | Read-Only | **Y Index Register** |
| `$D6C5` | `54981` | Read-Only | **Processor Status (P)**: Carry, Zero, Interrupt, Decimal, Break, Overflow, Negative flags. |
| `$D6C6` | `54982` | Write-Only | **Debug Control Strobe**: Writing `1` executes a single instruction (Step), `2` resumes execution, `3` halts CPU. |
| `$D6C7` | `54983` | Read-Write | **Assert Strobe**: Writing expected status registers (e.g. comparing A register). Breaks execution if mismatched. |

---

## 2. Mock Coordinate Injection (Sensory Stream Simulation)

For testing pathfinding and vector collision loops in `FolkloreCPU`, we implement a sensory injection vector:
* **The Injection Port (`$D6C8`–`$D6CB` / `54984`–`54987`)**: Allows the testing harness to push simulated coordinates (simulating NMEA GPS sentences or telemetry streams) directly into the CPU's memory:
  * `54984`: Inject $X$ Coordinate.
  * `54985`: Inject $Y$ Coordinate.
  * `54986`: Inject Epoch / Timestamp.
  * `54987`: Trigger Interrupt (simulate NMI/IRQ on input arrival).

---

## 3. Visual Telemetry & Overlays (B-Graph Sync)

To audit the vector calculations visually without reading raw storage dumps:
* **Vector Plotting**: The CPU updates coordinates at `$D6C8`–`$D6CB`, which the [bGraph.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/bGraph.yul) contract automatically reads.
* **Telemetry Output**: It plots the active **Path Trajectory** (draws dots showing the agent's path history) and **Attractor/Repeller positions** directly on the visual display console, allowing visual auditing of the pathing engine.

---

## 4. On-Chain Assertion Engine (EVM Test Hooks)

To automate validation in Node.js test scripts (like `test_flotsam.js` or `test_koala.js`), the CPU contract exposes an assertion callback:
* **Assertion Failures**: If the CPU executes an invalid opcode or hits an instruction state mismatch (defined by register `$D6C7`), it:
  1. Writes the error reason code to address `$D6C7`.
  2. Emits an event:
     ```solidity
     event CpuAssertionFailed(uint16 programCounter, uint8 errorCode);
     ```
     Topic: `0xbc848da07a7ad8b94df8c8a14b0368b634882dfba51912bc5527e0291f24d1c`.
  3. Reverts the transaction to preserve state and highlight the execution path failure.
