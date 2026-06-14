# Technical Report: 6502 Custom Binary Assembly & `counter.bin` Specification

This document details the architectural layout, assembly compilation mechanism, and bit-for-bit structure of the custom 6502 machine bytecode outputs designed for execution on the **CPU6502Emulator** and the **ZMM VM (Zero-Memory-Map Virtual Machine)**, as well as the **Folklore CPU**.

---

## 1. Architectural Overview & Address Space

The emulator implements a standard 8-bit MOS Technology 6502 processor executing in a virtualized, namespaced EVM/storage environment.

- **Accumulator ($A$)**: Addressable in EVM storage at slot `0x80` (128).
- **X Index Register ($X$)**: Addressable in EVM storage at slot `0x81` (129).
- **Y Index Register ($Y$)**: Addressable in EVM storage at slot `0x82` (130).
- **Stack Pointer ($SP$)**: Addressable in EVM storage at slot `0x83` (131).
- **Status Register ($SR$)**: Addressable in EVM storage at slot `0x84` (132).
- **Program Counter ($PC$)**: Addressable in EVM storage at slot `0x85` (133).
- **System RAM**: Addressable directly from `$0000` to `$FFFF` (0 - 65535).
  - **Zero Page**: `$0000` to `$00FF` (0 - 255).
  - **Program Origin**: Loaded at `$0200` (512).

---

## 2. Compilation Assembly Syntax & Generation of `.bin`

Our JS-based assembler compiles standard 6502 mnemonics using target addressing modes:
- **Immediate (`#$val`)**: Inserts the numeric literal value directly into the instruction payload.
- **Zero Page (`$val`)**: Addresses the first 256 bytes of RAM (requiring only a single-byte address argument).
- **Absolute (`$val`)**: Addresses the entire 64KB range (requiring a two-byte, little-endian address argument).

To generate `counter.bin` using the node script:
```javascript
const fs = require("fs");
const counterBytes = [0xA5, 0x10, 0x69, 0x01, 0x85, 0x10, 0x60];
fs.writeFileSync("solidity/bin/counter.bin", Buffer.from(counterBytes));
```

---

## 3. Bit-for-Bit Specification of `counter.bin`

The `counter.bin` executable is exactly **7 bytes** in size. Below is a detailed, bit-by-bit breakdown of its executable content:

| Byte Index | Hex Code | Binary Value | Instruction | Mode / Operand | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **0** | `0xA5` | `10100101` | `LDA` | Zero Page | Opcode to Load Accumulator from Zero Page memory. |
| **1** | `0x10` | `00010000` | — | `$10` | The target Zero Page address containing the counter. |
| **2** | `0x69` | `01101001` | `ADC` | Immediate | Opcode to Add with Carry to the Accumulator. |
| **3** | `0x01` | `00000001` | — | `#$01` | Literal value to add (increment of 1). |
| **4** | `0x85` | `10000101` | `STA` | Zero Page | Opcode to Store Accumulator in Zero Page memory. |
| **5** | `0x10` | `00010000` | — | `$10` | The target Zero Page address to save the counter state. |
| **6** | `0x60` | `01100000` | `RTS` | Implied | Return from Subroutine. Halts execution loop. |

### Byte 1 & 2: `A5 10` (Load Counter)
- **Opcode**: `A5` (Binary `10100101`). Loads the Accumulator with the value of the byte residing at the specified Zero Page memory address.
- **Operand**: `10` (Binary `00010000`). Specifies the Zero Page RAM location `$0010`.

### Byte 3 & 4: `69 01` (Increment Value)
- **Opcode**: `69` (Binary `01101001`). Adds the immediate value plus the state of the Carry flag to the Accumulator.
- **Operand**: `01` (Binary `00000001`). The value to be added. (Increments the register state by 1).

### Byte 5 & 6: `85 10` (Save State)
- **Opcode**: `85` (Binary `10000101`). Copies the contents of the Accumulator back into Zero Page RAM.
- **Operand**: `10` (Binary `00010000`). Writes the value back to target Zero Page RAM location `$0010`, updating the persistent state.

### Byte 7: `60` (Halt/Return)
- **Opcode**: `60` (Binary `01100000`). Pops the program counter from the stack and returns. In the execution harness of `runSteps()`, this opcode serves as the subroutine termination marker, signaling that the execution slice is complete.

---

## 4. Dual-Platform Compilation & Deployment

The compiled `counter.bin` binary is cross-compatible and runs natively on both platforms because they share instruction decoder paradigms and memory layout structures:

### Platform A: CPU6502Emulator (`cpu6502.yul`)
- **Execution model**: Evaluates assembly cycles line-by-line via direct hardware simulation.
- **8-Bit Constraints**: The memory is handled in standard 8-bit bytes (`uint8`). When the counter hits `255` (`0xFF`), the next `ADC #$01` increments the value to `256` which overflows/wraps back to `0` in zero-page memory location `$10` (with the Carry status flag set in Register `$SR`).

#### Deployment & Run Code:
```javascript
const cpuABI = ["function poke(uint256,uint256)", "function runSteps(uint256)"];
const contract = new ethers.Contract(cpuAddress, cpuABI, signer);

// Load counter.bin bytes into CPU RAM at $0200
const bin = fs.readFileSync("solidity/bin/counter.bin");
for (let i = 0; i < bin.length; i++) {
    await contract.poke(0x0200 + i, bin[i]);
}
// Set Program Counter to origin
await contract.poke(0x85, 0x0200);
// Execute steps
await contract.runSteps(10);
```

### Platform B: Folklore CPU (`folklore.yul`)
- **Execution model**: Executes cycles as part of a high-level game physics update cycle.
- **256-Bit Value Counting Advantage**: Because `folklore.yul` implements storage slots using full **256-bit EVM words (`uint256`)** instead of standard physical 8-bit cells, operations targeting its memory spaces do not overflow at `255`. The addition logic continues counting persistently up to $2^{256}-1$ ($\approx 1.15 \times 10^{77}$), allowing arbitrary high counters on-chain without register-wrap limitations.

#### Deployment & Run Code:
```javascript
const folkABI = ["function poke(uint256,uint256)", "function peek(uint256)"];
const contract = new ethers.Contract(folkAddress, folkABI, signer);

// Write code bytes
const bin = fs.readFileSync("solidity/bin/counter.bin");
for (let i = 0; i < bin.length; i++) {
    await contract.poke(0x0200 + i, bin[i]);
}
// Set Program Counter to origin
await contract.poke(0x85, 0x0200);
// Trigger execution loop tick
await contract.poke(55024, 1);
```

