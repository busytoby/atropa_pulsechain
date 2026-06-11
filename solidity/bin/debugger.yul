object "Debugger" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for register reads/writes
            function getReg(regId) -> val {
                if eq(regId, 200) { val := sload(200) leave }
                if eq(regId, 201) { val := sload(201) leave }
                if eq(regId, 202) { val := sload(60200) leave }
                if eq(regId, 203) { val := sload(60203) leave }
                val := sload(add(60000, regId))
            }
            function setReg(regId, val) {
                sstore(add(60000, regId), val)
            }

            // Helpers for virtual memory (size 65536)
            function getMem(addr) -> val {
                val := and(sload(add(60100, addr)), 0xFF)
            }
            function setMem(addr, val) {
                sstore(add(60100, addr), and(val, 0xFF))
            }

            // Breakpoint helpers
            function isBreakpoint(addr) -> found {
                let count := sload(60020)
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    if eq(sload(add(60021, i)), addr) {
                        found := 1
                        i := count // Break loop
                    }
                }
            }

            // 1. writeRegister(uint8 regId, uint256 val)
            // Selector: 0x50af8f71
            if eq(selector, 0x50af8f71) {
                let regId := and(calldataload(4), 0xFF)
                let val := calldataload(36)
                setReg(regId, val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 2. readRegister(uint8 regId) -> uint256
            // Selector: 0x4a7bf00f
            if eq(selector, 0x4a7bf00f) {
                let regId := and(calldataload(4), 0xFF)
                mstore(0x00, getReg(regId))
                return(0x00, 32)
            }

            // 3. writeMemory(uint256 addr, uint8 val)
            // Selector: 0x5517b193
            if eq(selector, 0x5517b193) {
                let addr := calldataload(4)
                let val := and(calldataload(36), 0xFF)
                setMem(addr, val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 4. readMemory(uint256 addr) -> uint8
            // Selector: 0xe0075f25
            if eq(selector, 0xe0075f25) {
                let addr := calldataload(4)
                mstore(0x00, getMem(addr))
                return(0x00, 32)
            }

            // 5. setBreakpoint(uint256 addr)
            // Selector: 0x4f3adf9d
            if eq(selector, 0x4f3adf9d) {
                let addr := calldataload(4)
                if iszero(isBreakpoint(addr)) {
                    let count := sload(60020)
                    sstore(add(60021, count), addr)
                    sstore(60020, add(count, 1))
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 6. clearBreakpoint(uint256 addr) -> uint256
            // Selector: 0x0b3464e6
            if eq(selector, 0x0b3464e6) {
                let addr := calldataload(4)
                let count := sload(60020)
                let foundIdx := 0xFFFF
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    if eq(sload(add(60021, i)), addr) {
                        foundIdx := i
                        i := count
                    }
                }
                if lt(foundIdx, count) {
                    let lastIdx := sub(count, 1)
                    if iszero(eq(foundIdx, lastIdx)) {
                        sstore(add(60021, foundIdx), sload(add(60021, lastIdx)))
                    }
                    sstore(add(60021, lastIdx), 0)
                    sstore(60020, lastIdx)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }
                mstore(0x00, 0)
                return(0x00, 32)
            }

            // 7. getBreakpoints() -> uint256[]
            // Selector: 0xcec5fe08
            if eq(selector, 0xcec5fe08) {
                let count := sload(60020)
                mstore(0x00, 0x20) // Array offset
                mstore(0x20, count) // Array length
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    mstore(add(0x40, mul(i, 32)), sload(add(60021, i)))
                }
                return(0x00, add(64, mul(count, 32)))
            }

            // 8. runDebugger(uint256 maxSteps) -> (uint256 exitCode, uint256 stepsExecuted)
            // Selector: 0xd18f4eb6
            if eq(selector, 0xd18f4eb6) {
                let maxSteps := calldataload(4)
                let stepsExecuted := 0
                let exitCode := 0 // 0 = max steps reached

                let regPC := getReg(0) // reg 0 is PC
                let sp := getReg(1) // reg 1 is SP
                let a := getReg(2)  // reg 2 is A
                let b := getReg(3)  // reg 3 is B
                let flags := getReg(4) // reg 4 is Flags

                for { } lt(stepsExecuted, maxSteps) { stepsExecuted := add(stepsExecuted, 1) } {
                    // Check if current PC is a breakpoint (if stepsExecuted > 0 to allow stepping off a breakpoint)
                    if gt(stepsExecuted, 0) {
                        if isBreakpoint(regPC) {
                            exitCode := 1 // Hit Breakpoint
                            break
                        }
                    }

                    let opcode := getMem(regPC)

                    // NOP (0x00)
                    if eq(opcode, 0x00) {
                        regPC := add(regPC, 1)
                        continue
                    }

                    // MVI A, data (0x01)
                    if eq(opcode, 0x01) {
                        a := getMem(add(regPC, 1))
                        regPC := add(regPC, 2)
                        continue
                    }

                    // MVI B, data (0x02)
                    if eq(opcode, 0x02) {
                        b := getMem(add(regPC, 1))
                        regPC := add(regPC, 2)
                        continue
                    }

                    // ADD B (0x03)
                    if eq(opcode, 0x03) {
                        let res := add(a, b)
                        a := and(res, 0xFF)
                        let z := iszero(a)
                        let c := gt(res, 0xFF)
                        flags := or(z, shl(1, c))
                        regPC := add(regPC, 1)
                        continue
                    }

                    // SUB B (0x04)
                    if eq(opcode, 0x04) {
                        let carry := 0
                        if lt(a, b) { carry := 1 }
                        a := and(sub(a, b), 0xFF)
                        let z := iszero(a)
                        flags := or(z, shl(1, carry))
                        regPC := add(regPC, 1)
                        continue
                    }

                    // JMP addr (0x05)
                    if eq(opcode, 0x05) {
                        let target := or(shl(8, getMem(add(regPC, 1))), getMem(add(regPC, 2)))
                        regPC := target
                        continue
                    }

                    // JZ addr (0x06)
                    if eq(opcode, 0x06) {
                        let target := or(shl(8, getMem(add(regPC, 1))), getMem(add(regPC, 2)))
                        let z := and(flags, 1)
                        if z {
                            regPC := target
                            continue
                        }
                        regPC := add(regPC, 3)
                        continue
                    }

                    // LDA addr (0x07)
                    if eq(opcode, 0x07) {
                        let target := or(shl(8, getMem(add(regPC, 1))), getMem(add(regPC, 2)))
                        a := getMem(target)
                        regPC := add(regPC, 3)
                        continue
                    }

                    // STA addr (0x08)
                    if eq(opcode, 0x08) {
                        let target := or(shl(8, getMem(add(regPC, 1))), getMem(add(regPC, 2)))
                        setMem(target, a)
                        regPC := add(regPC, 3)
                        continue
                    }

                    // CALL_OSC (0x09)
                    if eq(opcode, 0x09) {
                        let scaledBias := mul(a, 100000000000000)    // scale to mA (10^14)
                        let scaledPitch := mul(b, 1000)          // scale to pitch units
                        mstore(0x00, 0x07a96d8c00000000000000000000000000000000000000000000000000000000)
                        mstore(0x04, scaledBias)
                        mstore(0x24, scaledPitch)
                        let success := call(gas(), 0x0000000000000000000000000000000000000042, 0, 0x00, 0x44, 0x00, 0x20)
                        sstore(60203, success)
                        if success {
                            let outVal := mload(0x00)
                            sstore(60200, outVal) // Debug: Save raw output value
                            let positiveOut := add(outVal, 500000000000000000)
                            a := and(div(positiveOut, 10000000000000000), 0xFF)
                        }
                        if iszero(success) {
                            sstore(60200, 0xDEADBEEF) // Debug: Indicate call failure
                        }
                        regPC := add(regPC, 1)
                        continue
                    }

                    // HLT (0xFF)
                    if eq(opcode, 0xFF) {
                        exitCode := 2 // HLT encountered
                        stepsExecuted := add(stepsExecuted, 1)
                        break
                    }

                    // Invalid Opcode
                    exitCode := 3
                    break
                }

                // Write registers back to storage
                setReg(0, regPC)
                setReg(1, sp)
                setReg(2, a)
                setReg(3, b)
                setReg(4, flags)

                mstore(0x00, exitCode)
                mstore(0x20, stepsExecuted)
                return(0x00, 64)
            }

            revert(0, 0)
        }
    }
}
