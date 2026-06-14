# Dr. Dobb's Journal Issue #3 (March 1976) & TSFi2 Breakpoint Emulation

This document reviews **Dr. Dobb's Journal** (Vol. 1, No. 2, March 1976), focusing on code samples such as the **MOS Technology 6502 Breakpoint Routine**, and demonstrates how these retro debugging concepts are implemented in the **TSFi2 AI Debugger and ZMM VM**.

---

## 1. Key Articles & Architectural Alignment

| Dr. Dobb's Issue #3 Concept | Original 1976 Technology | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **MOS 6502 Breakpoint Routine** | Using the `BRK` hardware instruction (opcode `$00`) to save register states (A, X, Y, P, S) and yield control to a monitor program. | **AI Debugger Verification Suite** | Intercepting execution breakpoints, saving virtual register frames, and logging state telemetry. |
| **Keyboard Loader via TVT-2** | Loading octal machine code directly into RAM via a serial terminal interface. | **RPC & Calldata Loader** | Dynamic VM memory injection interfaces via RPC payload decoding and memory-mapped inputs. |
| **Tiny BASIC with 1D Arrays** | Extended interpreter support for single-dimension array index allocation and boundaries. | **Yul Memory Allocator** | Dynamic offset calculation and boundary assertions for arrays in VM memory. |

---

## 2. Emulating the MOS 6502 `BRK` Breakpoint in Yul

On the MOS 6502, executing a `BRK` instruction triggers a software interrupt. The CPU automatically pushes the program counter and status register to the stack, then vectors to the interrupt service routine (ISR). The debugger’s ISR saves the accumulator, X, Y, and stack pointer registers to a designated memory zone so that the programmer can inspect the CPU state.

In **TSFi2**, we emulate this process inside our Yul instruction decoder loop. When the VM encounters a `BRK` instruction (`0x00`), it suspends execution, captures the virtual register state, and writes it to an on-chain ledger log:

```yul
// Yul 6502 instruction decoder loop with BRK breakpoint interception
function executeInstruction(pc, acc, regX, regY, sp, status) -> newPc, newAcc, newRegX, newRegY, newSp, newStatus {
    // Read the current opcode from VM memory
    let opcode := mload(pc)
    
    switch opcode
    case 0x00 { // BRK (Software Interrupt / Breakpoint)
        // Set the Break (B) flag in the status register (bit 4)
        newStatus := or(status, 0x10)
        
        // Push PC + 2 and Status to the virtual stack
        let stackOffset := add(0x0100, sp)
        mstore(stackOffset, add(pc, 2))
        mstore(sub(stackOffset, 1), newStatus)
        newSp := sub(sp, 3)
        
        // Vector to Interrupt Handler ($FFFE / $FFFF)
        newPc := readWord(0xfffe)
        
        // Intercept execution and report debug registers to the host debugger
        triggerHostDebugCallback(pc, acc, regX, regY, newSp, newStatus)
    }
    // Handle other opcodes...
    default {
        // Normal instruction execution
        newPc := add(pc, 1)
        newAcc := acc
        newRegX := regX
        newRegY := regY
        newSp := sp
        newStatus := status
    }
}

function triggerHostDebugCallback(pc, acc, rx, ry, sp, status) {
    // Write register state to debug telemetry logs (memory-mapped debug port)
    sstore(0xdeb0001, pc)
    sstore(0xdeb0002, acc)
    sstore(0xdeb0003, rx)
    sstore(0xdeb0004, ry)
    sstore(0xdeb0005, sp)
    sstore(0xdeb0006, status)
}

function readWord(addr) -> val {
    val := and(mload(addr), 0xffff)
}
```

---

## 3. Keyboard Loaders and Dynamic Storage Injection

The TVT-2 keyboard loader allowed early hobbyists to key in code line by line. In the Synthesis Studio frontend, we implement this as an interactive HEX input console, where users can type in custom Yul assemblies, compile them in real-time, and hot-load the bytecode into the running VM slots.

---

## 4. Conclusion

*Dr. Dobb's Journal* Issue #3 laid the foundation for microcomputer software debugging techniques. By incorporating MOS 6502 `BRK` interrupt behavior into the TSFi2 debugger suite, we gain complete visibility into virtual contract execution, allowing our automated verification routines to pinpoint arithmetic overflow or dynamic range violations instantly.
