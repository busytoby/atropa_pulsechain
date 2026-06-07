object "CPU6502Emulator" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy runtime code to memory and return it
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    // ========================================================================
    // RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Storage Map for CPU Registers:
            // Slot 0x80: Accumulator A
            // Slot 0x81: X Register
            // Slot 0x82: Y Register
            // Slot 0x83: Stack Pointer (SP)
            // Slot 0x84: Status Register (SR) - Carry (bit 0), Zero (bit 1), Negative (bit 7)
            // Slot 0x85: Program Counter (PC)

            
            // Helper to get non-overlapping memory offset for C64 registers

            // Helper to get active context user address
            function getContextUser() -> user {
                user := mload(0x1F0)
                if iszero(user) {
                    user := caller()
                }
            }

            // Helper to get namespaced storage slot for a specific user and C64 address
            function getUserSlot(addr) -> slot {
                mstore(0x00, getContextUser())
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }
            function regMemOffset(slot) -> offset {
                switch slot
                case 0x80 { offset := 0x80 }
                case 0x81 { offset := 0xA0 }
                case 0x82 { offset := 0xC0 }
                case 0x83 { offset := 0xE0 }
                case 0x84 { offset := 0x100 }
                case 0x85 { offset := 0x120 }
                default { offset := 0 }
            }

            function getReg(slot) -> val {
                val := mload(regMemOffset(slot))
            }

            function setReg(slot, val) {
                mstore(regMemOffset(slot), val)
            }

            // Load registers from storage to memory cache
            function loadRegisterCache() {
                mstore(0x80, sload(getUserSlot(0x80)))
                mstore(0xA0, sload(getUserSlot(0x81)))
                mstore(0xC0, sload(getUserSlot(0x82)))
                mstore(0xE0, sload(getUserSlot(0x83)))
                mstore(0x100, sload(getUserSlot(0x84)))
                mstore(0x120, sload(getUserSlot(0x85)))
            }

            // Write back registers from memory cache to storage
            function saveRegisterCache() {
                sstore(getUserSlot(0x80), mload(0x80))
                sstore(getUserSlot(0x81), mload(0xA0))
                sstore(getUserSlot(0x82), mload(0xC0))
                sstore(getUserSlot(0x83), mload(0xE0))
                sstore(getUserSlot(0x84), mload(0x100))
                sstore(getUserSlot(0x85), mload(0x120))
            }
// Helper function to update Zero and Negative flags based on value
            function updateFlags(val) {
                let sr := getReg(0x84)
                
                // Clear Zero (bit 1) and Negative (bit 7) flags
                sr := and(sr, 0x7D)

                // Set Zero flag if val is 0
                if iszero(val) {
                    sr := or(sr, 0x02)
                }

                // Set Negative flag if bit 7 of val is set
                if and(val, 0x80) {
                    sr := or(sr, 0x80)
                }

                setReg(0x84, sr)
            }

            // Helper function to get Carry flag
            function getCarry() -> carry {
                carry := and(getReg(0x84), 0x01)
            }

            // Helper function to set Carry flag
            function setCarry(val) {
                let sr := getReg(0x84)
                sr := and(sr, 0xFE) // Clear Carry
                if val {
                    sr := or(sr, 0x01)
                }
                setReg(0x84, sr)
            }

            // Helper function to push to stack
            function pushStack(val) {
                let sp := getReg(0x83)
                // C64 Stack mapped to 0x100 - 0x1FF
                sstore(getUserSlot(add(0x100, sp)), and(val, 0xFF))
                setReg(0x83, and(sub(sp, 1), 0xFF))
            }

            // Helper function to pull from stack
            function pullStack() -> val {
                let sp := and(add(getReg(0x83), 1), 0xFF)
                val := sload(getUserSlot(add(0x100, sp)))
                setReg(0x83, sp)
            }

            // Helper function to read from memory/registers (handling memory-mapped I/O)
            function readMemory(addr) -> val {
                // Apply cartridge bank-switching for range $8000-$9FFF (32768-40959) using bank register at $D500 (54528)
                if and(iszero(lt(addr, 32768)), lt(addr, 40960)) {
                    let bank := sload(getUserSlot(54528))
                    addr := add(addr, mul(bank, 8192))
                }

                switch addr
                case 54552 { // AABB Collision Detection coprocessor output ($D518)
                    let x1 := sload(getUserSlot(54544))
                    let y1 := sload(getUserSlot(54545))
                    let w1 := sload(getUserSlot(54546))
                    let h1 := sload(getUserSlot(54547))
                    let x2 := sload(getUserSlot(54548))
                    let y2 := sload(getUserSlot(54549))
                    let w2 := sload(getUserSlot(54550))
                    let h2 := sload(getUserSlot(54551))
                    let cond1 := lt(x1, add(x2, w2))
                    let cond2 := lt(x2, add(x1, w1))
                    let cond3 := lt(y1, add(y2, h2))
                    let cond4 := lt(y2, add(y1, h1))
                    if and(and(cond1, cond2), and(cond3, cond4)) {
                        val := 1
                    }
                }
                case 54562 { // Ring Buffer Size ($D522)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    val := and(sub(add(tail, 32), head), 31)
                }
                case 54564 { // Last Popped Value ($D524)
                    val := sload(getUserSlot(54564))
                }
                case 56321 { // $DC01 (CIA 1 Data Port B - Keyboard row read)
                    let colMask := sload(getUserSlot(56320))
                    let rowMask := 0xFF
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        // Shift 1 by i bits to check active column
                        if iszero(and(colMask, shl(i, 1))) {
                            let rowVal := sload(getUserSlot(add(56350, i)))
                            if iszero(rowVal) {
                                rowVal := 0xFF
                            }
                            rowMask := and(rowMask, rowVal)
                        }
                    }
                    val := rowMask
                }
                default {
                    val := sload(getUserSlot(addr))
                }
            }

            // Helper function to write to memory/registers (handling memory-mapped I/O)
            function writeMemory(addr, val) {
                // Apply cartridge bank-switching for range $8000-$9FFF (32768-40959) using bank register at $D500 (54528)
                if and(iszero(lt(addr, 32768)), lt(addr, 40960)) {
                    let bank := sload(getUserSlot(54528))
                    addr := add(addr, mul(bank, 8192))
                }

                switch addr
                case 54560 { // Ring Buffer Push ($D520)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    let nextTail := and(add(tail, 1), 31)
                    if iszero(eq(nextTail, head)) {
                        sstore(getUserSlot(add(54567, tail)), val)
                        sstore(getUserSlot(54566), nextTail)
                    }
                }
                case 54561 { // Trigger Ring Buffer Pop transaction ($D521)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    if iszero(eq(head, tail)) {
                        let poppedVal := sload(getUserSlot(add(54567, head)))
                        sstore(getUserSlot(54564), poppedVal) // Save to last popped value
                        head := and(add(head, 1), 31)
                        sstore(getUserSlot(54565), head) // Advance head
                    }
                }
                case 54563 { // Ring Buffer Clear ($D523)
                    sstore(getUserSlot(54565), 0)
                    sstore(getUserSlot(54566), 0)
                }
                case 53265 { // $D011 (VIC-II Control Register 1)
                    // Bit 7 is the high bit of the raster compare target
                    sstore(getUserSlot(53301), and(val, 0x80))
                    sstore(getUserSlot(53265), val)
                }
                case 53266 { // $D012 (VIC-II Raster Line compare / read raster)
                    // Low 8 bits of the raster compare target
                    sstore(getUserSlot(53300), and(val, 0xFF))
                }
                case 53273 { // $D019 (Interrupt Flag Register - Write 1 to clear)
                    let currentVal := sload(getUserSlot(53273))
                    let newVal := and(and(currentVal, not(val)), 0x7F) // Clear bit 7 first
                    let d01aVal := sload(getUserSlot(53274))
                    if and(newVal, d01aVal) {
                        newVal := or(newVal, 0x80)
                    }
                    sstore(getUserSlot(53273), newVal)
                }
                case 54784 { // $D600 (Votrax Phoneme register)
                    sstore(getUserSlot(54784), val)
                    sstore(getUserSlot(54786), 1) // Set Busy to 1
                    let cnt := sload(getUserSlot(54787))
                    sstore(getUserSlot(54787), add(cnt, 1))
                }
                default {
                    sstore(getUserSlot(addr), val)
                }
            }

            // Helper function to update flags for comparison instructions
            function updateCompareFlags(regVal, operandVal) {
                let sr := getReg(0x84)
                sr := and(sr, 0x7C) // Clear N, Z, C
                if or(gt(regVal, operandVal), eq(regVal, operandVal)) {
                    sr := or(sr, 0x01)
                }
                if eq(regVal, operandVal) {
                    sr := or(sr, 0x02)
                }
                let diff := and(sub(regVal, operandVal), 0xFF)
                if and(diff, 0x80) {
                    sr := or(sr, 0x80)
                }
                setReg(0x84, sr)
            }

            // Core Execution Dispatcher
            function executeInternal(opcode, operand) -> branchTaken {
                branchTaken := 0
                
                switch opcode
                // LDA # (Load Accumulator Immediate)
                case 0xA9 {
                    let val := and(operand, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LDA absolute (Load Accumulator from memory)
                case 0xAD {
                    let val := and(readMemory(operand), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA absolute (Store Accumulator to address)
                case 0x8D {
                    writeMemory(operand, getReg(0x80))
                }
                // AND # (Logical AND with Accumulator Immediate)
                case 0x29 {
                    let val := and(getReg(0x80), and(operand, 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // AND absolute (Logical AND with Accumulator from memory)
                case 0x2D {
                    let val := and(getReg(0x80), and(readMemory(operand), 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // CMP # (Compare Accumulator Immediate)
                case 0xC9 {
                    updateCompareFlags(getReg(0x80), and(operand, 0xFF))
                }
                // CMP absolute (Compare Accumulator from memory)
                case 0xCD {
                    updateCompareFlags(getReg(0x80), and(readMemory(operand), 0xFF))
                }
                // CMP zero-page (Compare Accumulator from Zero Page)
                case 0xC5 {
                    updateCompareFlags(getReg(0x80), and(sload(getUserSlot(and(operand, 0xFF))), 0xFF))
                }
                // CPX # (Compare X Register Immediate)
                case 0xE0 {
                    updateCompareFlags(getReg(0x81), and(operand, 0xFF))
                }
                // CPY # (Compare Y Register Immediate)
                case 0xC0 {
                    updateCompareFlags(getReg(0x82), and(operand, 0xFF))
                }
                // INC absolute (Increment memory address)
                case 0xEE {
                    let val := and(add(readMemory(operand), 1), 0xFF)
                    writeMemory(operand, val)
                    updateFlags(val)
                }
                // DEC absolute (Decrement memory address)
                case 0xCE {
                    let val := and(sub(readMemory(operand), 1), 0xFF)
                    writeMemory(operand, val)
                    updateFlags(val)
                }
                // DEC zero-page (Decrement zero page address)
                case 0xC6 {
                    let addr := and(operand, 0xFF)
                    let val := and(sub(readMemory(addr), 1), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // INC zero-page (Increment zero page address)
                case 0xE6 {
                    let addr := and(operand, 0xFF)
                    let val := and(add(readMemory(addr), 1), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // LDA zero-page (Load Accumulator from Zero Page)
                case 0xA5 {
                    let val := and(sload(getUserSlot(and(operand, 0xFF))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA zero-page (Store Accumulator to Zero Page)
                case 0x85 {
                    sstore(getUserSlot(and(operand, 0xFF)), getReg(0x80))
                }
                // LDX # (Load X Register Immediate)
                case 0xA2 {
                    let val := and(operand, 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LDX absolute (Load X Register from memory)
                case 0xAE {
                    let val := and(readMemory(operand), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // STX absolute (Store X Register to address)
                case 0x8E {
                    writeMemory(operand, getReg(0x81))
                }
                // LDY # (Load Y Register Immediate)
                case 0xA0 {
                    let val := and(operand, 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // STY absolute (Store Y Register to address)
                case 0x8C {
                    writeMemory(operand, getReg(0x82))
                }
                // CLC (Clear Carry Flag)
                case 0x18 {
                    setCarry(0)
                }
                // SEC (Set Carry Flag)
                case 0x38 {
                    setCarry(1)
                }
                // ADC # (Add with Carry)
                case 0x69 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let sum := add(add(a, and(operand, 0xFF)), carry)
                    setCarry(gt(sum, 255))
                    let val := and(sum, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // SBC # (Subtract with Carry)
                case 0xE9 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let diff := sub(sub(a, and(operand, 0xFF)), sub(1, carry))
                    let noBorrow := iszero(and(diff, 0x100))
                    setCarry(noBorrow)
                    let val := and(diff, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // INX (Increment X)
                case 0xE8 {
                    let val := and(add(getReg(0x81), 1), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // DEX (Decrement X)
                case 0xCA {
                    let val := and(sub(getReg(0x81), 1), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // INY (Increment Y)
                case 0xC8 {
                    let val := and(add(getReg(0x82), 1), 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // DEY (Decrement Y)
                case 0x88 {
                    let val := and(sub(getReg(0x82), 1), 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // TAX (Transfer A to X)
                case 0xAA {
                    let val := getReg(0x80)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // TXA (Transfer X to A)
                case 0x8A {
                    let val := getReg(0x81)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // TAY (Transfer A to Y)
                case 0xA8 {
                    let val := getReg(0x80)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // TYA (Transfer Y to A)
                case 0x98 {
                    let val := getReg(0x82)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // PHA (Push Accumulator)
                case 0x48 {
                    pushStack(getReg(0x80))
                }
                // PLA (Pull Accumulator)
                case 0x68 {
                    let val := pullStack()
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // PHP (Push Processor Status)
                case 0x08 {
                    pushStack(getReg(0x84))
                }
                // PLP (Pull Processor Status)
                case 0x28 {
                    let val := pullStack()
                    setReg(0x84, val)
                }
                // JMP absolute
                case 0x4C {
                    setReg(0x85, operand)
                    branchTaken := 1
                }
                // JSR (Jump to Subroutine)
                case 0x20 {
                    let currentPC := getReg(0x85)
                    let handled := 0
                    
                    switch operand
                    case 65490 { // $FFD2 - CHROUT (Output character in A to screen)
                        let char := getReg(0x80)
                        let cursor := sload(getUserSlot(211)) // $D3 - cursor column
                        let row := sload(getUserSlot(214))    // $D6 - cursor row
                        // Calculate screen RAM address (1024 + row * 40 + cursor)
                        let screenAddr := add(1024, add(mul(row, 40), cursor))
                        
                        // Handle special characters
                        switch char
                        case 13 { // Return key
                            row := add(row, 1)
                            cursor := 0
                        }
                        default {
                            let screenCode := char
                            if and(iszero(lt(char, 65)), iszero(gt(char, 90))) {
                                screenCode := sub(char, 64) // A-Z -> 1-26
                            }
                            if and(iszero(lt(char, 97)), iszero(gt(char, 122))) {
                                screenCode := sub(char, 96) // a-z -> 1-26
                            }
                            writeMemory(screenAddr, screenCode)
                            // Also write white color (1) to color RAM
                            writeMemory(add(55296, add(mul(row, 40), cursor)), 1)
                            
                            cursor := add(cursor, 1)
                            if iszero(lt(cursor, 40)) {
                                cursor := 0
                               row := add(row, 1)
                            }
                        }
                        
                        if iszero(lt(row, 25)) {
                            row := 0
                        }
                        
                        sstore(getUserSlot(211), cursor)
                        sstore(getUserSlot(214), row)
                        handled := 1
                    }
                    case 65508 { // $FFE4 - GETIN (Get character from keyboard buffer into A)
                        let bufLen := sload(getUserSlot(198))
                        let char := 0
                        if gt(bufLen, 0) {
                            char := sload(getUserSlot(637))
                            for { let i := 0 } lt(i, sub(bufLen, 1)) { i := add(i, 1) } {
                                sstore(getUserSlot(add(637, i)), sload(getUserSlot(add(638, i))))
                            }
                            sstore(getUserSlot(198), sub(bufLen, 1))
                        }
                        setReg(0x80, char)
                        updateFlags(char)
                        handled := 1
                    }
                    
                    if iszero(handled) {
                        pushStack(shr(8, currentPC))
                        pushStack(and(currentPC, 0xFF))
                        setReg(0x85, operand)
                        branchTaken := 1
                    }
                }
                // RTS (Return from Subroutine)
                case 0x60 {
                    let low := pullStack()
                    let high := pullStack()
                    let target := or(shl(8, high), low)
                    setReg(0x85, target)
                    branchTaken := 1
                }
                // BPL (Branch on Plus / Negative flag clear)
                case 0x10 {
                    if iszero(and(getReg(0x84), 0x80)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BMI (Branch on Minus / Negative flag set)
                case 0x30 {
                    if and(getReg(0x84), 0x80) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BVC (Branch on Overflow clear)
                case 0x50 {
                    if iszero(and(getReg(0x84), 0x40)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BVS (Branch on Overflow set)
                case 0x70 {
                    if and(getReg(0x84), 0x40) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BCC (Branch on Carry clear)
                case 0x90 {
                    if iszero(getCarry()) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BCS (Branch on Carry set)
                case 0xB0 {
                    if getCarry() {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BNE (Branch on Not Equal / Zero flag clear)
                case 0xD0 {
                    if iszero(and(getReg(0x84), 0x02)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BEQ (Branch on Equal / Zero flag set)
                case 0xF0 {
                    if and(getReg(0x84), 0x02) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // CLI (Clear Interrupt Disable)
                case 0x58 {
                    setReg(0x84, and(getReg(0x84), 0xFB))
                }
                // SEI (Set Interrupt Disable)
                case 0x78 {
                    setReg(0x84, or(getReg(0x84), 0x04))
                }
                // RTI (Return from Interrupt)
                case 0x40 {
                    let sr := pullStack()
                    setReg(0x84, sr)
                    let low := pullStack()
                    let high := pullStack()
                    setReg(0x85, or(shl(8, high), low))
                    branchTaken := 1
                }
                // LDA abs,X
                case 0xBD {
                    let val := and(readMemory(add(operand, getReg(0x81))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LDA abs,Y
                case 0xB9 {
                    let val := and(readMemory(add(operand, getReg(0x82))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA abs,X
                case 0x9D {
                    writeMemory(add(operand, getReg(0x81)), getReg(0x80))
                }
                // STA abs,Y
                case 0x99 {
                    writeMemory(add(operand, getReg(0x82)), getReg(0x80))
                }
                // LDA zp,X
                case 0xB5 {
                    let addr := and(add(operand, getReg(0x81)), 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA zp,X
                case 0x95 {
                    let addr := and(add(operand, getReg(0x81)), 0xFF)
                    writeMemory(addr, getReg(0x80))
                }
                // LDA (zp),Y
                case 0xB1 {
                    let zp := and(operand, 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemory(add(base, getReg(0x82))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA (zp),Y
                case 0x91 {
                    let zp := and(operand, 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    writeMemory(add(base, getReg(0x82)), getReg(0x80))
                }
                // LDA (zp,X)
                case 0xA1 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemory(base), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA (zp,X)
                case 0x81 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    writeMemory(base, getReg(0x80))
                }
                default {
                    revert(0, 0)
                }
            }

            // Helper to compute and apply relative branch offset
            function executeBranch(operand) -> taken {
                let offset := and(operand, 0xFF)
                if and(offset, 0x80) {
                    offset := sub(offset, 256)
                }
                let currentPC := getReg(0x85)
                setReg(0x85, add(add(currentPC, 2), offset))
                taken := 1
            }

            // Helper to get instruction length
            function getOpLength(op) -> len {
                len := 1 // default implicit/accumulator/stack
                
                // 2-byte immediate, zero-page or relative branch
                if or(or(or(eq(op, 0xA9), eq(op, 0xA2)), or(eq(op, 0xA0), eq(op, 0x69))),
                      or(or(eq(op, 0xE9), eq(op, 0x10)), or(eq(op, 0x30), or(eq(op, 0x50), or(eq(op, 0x70), or(eq(op, 0x29), or(eq(op, 0xC9), or(eq(op, 0xC5), or(eq(op, 0xE0), or(eq(op, 0xC0), or(eq(op, 0xA5), or(eq(op, 0x85), or(eq(op, 0xC6), eq(op, 0xE6)))))))))))))) {
                    len := 2
                }
                if or(or(eq(op, 0x90), eq(op, 0xB0)), or(eq(op, 0xD0), eq(op, 0xF0))) {
                    len := 2
                }
                // Indexed zero-page / indirect Y / indexed indirect X (2-byte)
                if or(or(or(eq(op, 0xB1), eq(op, 0x91)), or(eq(op, 0xA1), eq(op, 0x81))), or(eq(op, 0xB5), eq(op, 0x95))) {
                    len := 2
                }

                // 3-byte absolute address/JMP/JSR
                if or(or(or(eq(op, 0xAD), eq(op, 0x8D)), or(eq(op, 0xAE), eq(op, 0x8E))),
                      or(or(eq(op, 0x8C), eq(op, 0x4C)), or(eq(op, 0x20), or(eq(op, 0x2D), or(eq(op, 0xCD), or(eq(op, 0xEE), eq(op, 0xCE))))))) {
                    len := 3
                }
                // Indexed absolute (3-byte)
                if or(or(eq(op, 0xBD), eq(op, 0xB9)), or(eq(op, 0x9D), eq(op, 0x99))) {
                    len := 3
                }
            }

            // ----------------------------------------------------------------
            // Method 1: executeOp(uint8 opcode, uint256 operand)
            // Selector: 0x2a9cd5d1
            // ----------------------------------------------------------------
            if eq(selector, 0x2a9cd5d1) {
                loadRegisterCache()
                let opcode := shr(248, calldataload(4))
                let operand := calldataload(36)

                pop(executeInternal(opcode, operand))
                saveRegisterCache()

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 2: getCPUState() -> (uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 SP, uint256 PC)
            // Selector: 0xcbe94d1f
            // ----------------------------------------------------------------
            if eq(selector, 0xe0f77c57) {
                loadRegisterCache()
                mstore(0x00, getReg(0x80))
                mstore(0x20, getReg(0x81))
                mstore(0x40, getReg(0x82))
                mstore(0x60, getReg(0x84))
                mstore(0x80, getReg(0x83))
                mstore(0xA0, getReg(0x85))
                return(0x00, 192)
            }

            // ----------------------------------------------------------------
            // Method 3: runSteps(uint256 maxSteps) -> uint256
            // Selector: 0xc45b1808
            // ----------------------------------------------------------------
            if eq(selector, 0xc45b1808) {
                loadRegisterCache()
                let maxSteps := calldataload(4)
                let stepsExecuted := 0
                let halted := 0

                for { } and(lt(stepsExecuted, maxSteps), iszero(halted)) { stepsExecuted := add(stepsExecuted, 1) } {
                    // Check for pending IRQ interrupt
                    let irqPending := and(sload(getUserSlot(53273)), 0x80)
                    let irqDisable := and(getReg(0x84), 0x04)
                    if and(iszero(iszero(irqPending)), iszero(irqDisable)) {
                        let currentPC := getReg(0x85)
                        // Push PC high and low
                        pushStack(shr(8, currentPC))
                        pushStack(and(currentPC, 0xFF))
                        // Push SR status
                        pushStack(getReg(0x84))
                        // Set I flag
                        setReg(0x84, or(getReg(0x84), 0x04))
                        // Load IRQ vector from $FFFE / $FFFF (65534 / 65535)
                        let lowVector := and(sload(getUserSlot(65534)), 0xFF)
                        let highVector := and(sload(getUserSlot(65535)), 0xFF)
                        setReg(0x85, or(shl(8, highVector), lowVector))
                    }

                    let currentPC := getReg(0x85)
                    let opcode := and(sload(getUserSlot(currentPC)), 0xFF)

                    // BRK instruction (0x00) halts execution
                    if iszero(opcode) {
                        halted := 1
                    }

                    if iszero(halted) {
                        // Read operand based on instruction length (2-byte vs 3-byte)
                        let operand := and(sload(getUserSlot(add(currentPC, 1))), 0xFF)
                        let len := getOpLength(opcode)
                        if eq(len, 3) {
                            let highByte := and(sload(getUserSlot(add(currentPC, 2))), 0xFF)
                            operand := or(shl(8, highByte), operand)
                        }

                        // Execute instruction
                        let branchTaken := executeInternal(opcode, operand)

                        // If branch/jump did not modify PC, increment it by instruction length
                        if iszero(branchTaken) {
                            setReg(0x85, add(currentPC, len))
                        }

                        // --- Simulate VIC-II Raster Line Progression ---
                        let line := sload(getUserSlot(53266))
                        // Fetch bit 7 of $D011 to construct the full 9-bit raster line value
                        let d011Val := sload(getUserSlot(53265))
                        if and(d011Val, 0x80) {
                            line := or(line, 0x100)
                        }

                        let nextLine := mod(add(line, 1), 312)
                        sstore(getUserSlot(53266), and(nextLine, 0xFF))

                        let newD011 := and(d011Val, 0x7F)
                        if and(nextLine, 0x100) {
                            newD011 := or(newD011, 0x80)
                        }
                        sstore(getUserSlot(53265), newD011)

                        // --- Raster Interrupt Comparison ---
                        let compareLow := sload(getUserSlot(53300))
                        let compareHigh := sload(getUserSlot(53301))
                        let targetLine := compareLow
                        if compareHigh {
                            targetLine := or(targetLine, 0x100)
                        }

                        if eq(nextLine, targetLine) {
                            let d019Val := sload(getUserSlot(53273))
                            d019Val := or(d019Val, 0x01) // Set bit 0 (Raster Interrupt Flag)
                            
                            // Check if Raster Interrupt is enabled (bit 0 of $D01A / 53274)
                            let d01aVal := sload(getUserSlot(53274))
                            if and(d01aVal, 0x01) {
                                d019Val := or(d019Val, 0x80) // Set bit 7 (IRQ flag)
                            }
                            sstore(getUserSlot(53273), d019Val)
                        }
                    }
                }

                saveRegisterCache()
                mstore(0x00, stepsExecuted)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 4: poke(uint256 addr, uint256 val)
            // Selector: 0x8029e7c0
            // ----------------------------------------------------------------
            if eq(selector, 0x8029e7c0) {
                let addr := calldataload(4)
                let val := calldataload(36)
                writeMemory(addr, val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 5: getScreenRAM() -> bytes
            // Selector: 0x4a5d1e5c
            // ----------------------------------------------------------------
            if eq(selector, 0x4a5d1e5c) {
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(getUserSlot(add(1024, i))), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            // ----------------------------------------------------------------
            // Method 6: getColorRAM() -> bytes
            // Selector: 0xace7b8ce
            // ----------------------------------------------------------------
            if eq(selector, 0xace7b8ce) {
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(getUserSlot(add(55296, i))), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            
            // ----------------------------------------------------------------
            // Method 7: pokeBytes(uint256 startAddr, bytes calldata data)
            // Selector: 0xf7e8e81b
            // ----------------------------------------------------------------
            if eq(selector, 0xf7e8e81b) {
                let startAddr := calldataload(4)
                let offset := calldataload(36)
                let length := calldataload(add(4, offset))
                let dataStart := add(36, offset)
                
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let val := shr(248, calldataload(add(dataStart, i)))
                    writeMemory(add(startAddr, i), val)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 8: fillMemory(uint256 startAddr, uint256 length, uint256 val)
            // Selector: 0x69a989d1
            // ----------------------------------------------------------------
            if eq(selector, 0x69a989d1) {
                let startAddr := calldataload(4)
                let length := calldataload(36)
                let val := and(calldataload(68), 0xFF)
                
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    writeMemory(add(startAddr, i), val)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            
            // ----------------------------------------------------------------
            // Method 9: peek(uint256 addr) -> uint256
            // Selector: 0x7861d269
            // ----------------------------------------------------------------
            if eq(selector, 0x7861d269) {
                let addr := calldataload(4)
                let val := readMemory(addr)
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 10: batchPoke(uint256[] calldata addrs, uint256[] calldata vals)
            // Selector: 0xe5c80097
            // ----------------------------------------------------------------
            if eq(selector, 0xe5c80097) {
                let addrsOffset := calldataload(4)
                let valsOffset := calldataload(36)
                let addrsLen := calldataload(add(4, addrsOffset))
                let valsLen := calldataload(add(4, valsOffset))
                
                if iszero(eq(addrsLen, valsLen)) { revert(0, 0) }
                
                let addrsDataStart := add(36, addrsOffset)
                let valsDataStart := add(36, valsOffset)
                
                for { let i := 0 } lt(i, addrsLen) { i := add(i, 1) } {
                    let addr := calldataload(add(addrsDataStart, mul(i, 32)))
                    let val := calldataload(add(valsDataStart, mul(i, 32)))
                    writeMemory(addr, val)
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 12: peekUser(address user, uint256 addr) -> uint256
            // Selector: 0x0ccd522c
            // ----------------------------------------------------------------
            if eq(selector, 0x0ccd522c) {
                let user := calldataload(4)
                let addr := calldataload(36)
                
                // Save context user
                let oldUser := mload(0x1F0)
                mstore(0x1F0, user)
                
                // Read via readMemory to trigger bank switching and memory-mapped IO behaviors
                let val := readMemory(addr)
                
                // Restore context user
                mstore(0x1F0, oldUser)
                
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 13: pokeUser(address user, uint256 addr, uint256 val) -> uint256
            // Selector: 0xa7260f02
            // ----------------------------------------------------------------
            if eq(selector, 0xa7260f02) {
                let user := calldataload(4)
                let addr := calldataload(36)
                let val := calldataload(68)
                
                // Save context user
                let oldUser := mload(0x1F0)
                mstore(0x1F0, user)
                
                // Write via writeMemory to trigger bank switching and side-effects
                writeMemory(addr, val)
                
                // Restore context user
                mstore(0x1F0, oldUser)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 11: initializeGame(uint256 startPC, bytes calldata program, uint256[] calldata addrs, uint256[] calldata vals)
            // Selector: 0x914eae91
            // ----------------------------------------------------------------
            if eq(selector, 0x914eae91) {
                let startPC := calldataload(4)
                
                // 1. Reset registers
                writeMemory(0x80, 0) // A
                writeMemory(0x81, 0) // X
                writeMemory(0x82, 0) // Y
                writeMemory(0x83, 0xFF) // SP
                writeMemory(0x84, 0) // SR
                writeMemory(0x85, startPC) // PC

                // 2. Upload program
                let progOffset := calldataload(36)
                let progLen := calldataload(add(4, progOffset))
                let progStart := add(36, progOffset)
                for { let i := 0 } lt(i, progLen) { i := add(i, 1) } {
                    let val := shr(248, calldataload(add(progStart, i)))
                    writeMemory(add(startPC, i), val)
                }

                // 3. Clear remaining RAM up to 256 bytes
                let remaining := 0
                if lt(progLen, 256) {
                    remaining := sub(256, progLen)
                }
                if gt(remaining, 0) {
                    let fillStart := add(startPC, progLen)
                    for { let i := 0 } lt(i, remaining) { i := add(i, 1) } {
                        writeMemory(add(fillStart, i), 0)
                    }
                }

                // 4. Apply batch pokes for sprites, enable mask, banner
                let addrsOffset := calldataload(68)
                let valsOffset := calldataload(100)
                let addrsLen := calldataload(add(4, addrsOffset))
                let valsLen := calldataload(add(4, valsOffset))
                if iszero(eq(addrsLen, valsLen)) { revert(0, 0) }

                let addrsDataStart := add(36, addrsOffset)
                let valsDataStart := add(36, valsOffset)
                for { let i := 0 } lt(i, addrsLen) { i := add(i, 1) } {
                    let addr := calldataload(add(addrsDataStart, mul(i, 32)))
                    let val := calldataload(add(valsDataStart, mul(i, 32)))
                    writeMemory(addr, val)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
