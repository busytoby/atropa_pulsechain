object "Intel8080" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // execute(bytes bytecode, uint256 cycles) -> returns (uint8 a, uint8 b, uint8 c, uint8 d, uint8 e, uint8 h, uint8 l, uint16 pc, uint16 sp, uint8 flags)
            // selector: 0xab5898e8
            if eq(selector, 0xab5898e8) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                let cycles := calldataload(36)
                
                // Copy bytecode to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                let a := 0
                let b := 0
                let c := 0
                let d := 0
                let e := 0
                let h := 0
                let l := 0
                let regPC := 0
                let sp := 0xF000 // Stack pointer default
                
                // Flags state
                let fSign := 0
                let fZero := 1
                let fAuxCarry := 0
                let fParity := 1
                let fCarry := 0
                
                function computeParity(v) -> p {
                    let count := 0
                    for { let temp := and(v, 0xFF) } temp { temp := shr(1, temp) } {
                        count := add(count, and(temp, 1))
                    }
                    p := iszero(and(count, 1))
                }
                
                for { let cyc := 0 } and(lt(cyc, cycles), lt(regPC, len)) { cyc := add(cyc, 1) } {
                    let opcode := byte(0, mload(add(0x1000, regPC)))
                    
                    switch opcode
                    
                    // 0x00: NOP
                    case 0x00 {
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x06: MVI B, d8
                    case 0x06 {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        b := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x0E: MVI C, d8
                    case 0x0E {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        c := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x16: MVI D, d8
                    case 0x16 {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        d := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x1E: MVI E, d8
                    case 0x1E {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        e := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x26: MVI H, d8
                    case 0x26 {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        h := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x2E: MVI L, d8
                    case 0x2E {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        l := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x3E: MVI A, d8
                    case 0x3E {
                        if iszero(lt(add(regPC, 1), len)) { break }
                        a := byte(0, mload(add(0x1001, regPC)))
                        regPC := add(regPC, 2)
                    }
                    
                    // 0x78: MOV A, B
                    case 0x78 {
                        a := b
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x47: MOV B, A
                    case 0x47 {
                        b := a
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x80: ADD B
                    case 0x80 {
                        let res := add(a, b)
                        fCarry := gt(res, 0xFF)
                        a := and(res, 0xFF)
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x81: ADD C
                    case 0x81 {
                        let res := add(a, c)
                        fCarry := gt(res, 0xFF)
                        a := and(res, 0xFF)
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x90: SUB B
                    case 0x90 {
                        let res := sub(a, b)
                        fCarry := lt(a, b)
                        a := and(res, 0xFF)
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0xA0: ANA B
                    case 0xA0 {
                        a := and(a, b)
                        fCarry := 0
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0xB0: ORA B
                    case 0xB0 {
                        a := or(a, b)
                        fCarry := 0
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0xA8: XRA B
                    case 0xA8 {
                        a := xor(a, b)
                        fCarry := 0
                        fSign := shr(7, and(a, 0x80))
                        fZero := iszero(a)
                        fParity := computeParity(a)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x04: INR B
                    case 0x04 {
                        b := and(add(b, 1), 0xFF)
                        fSign := shr(7, and(b, 0x80))
                        fZero := iszero(b)
                        fParity := computeParity(b)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0x05: DCR B
                    case 0x05 {
                        b := and(sub(b, 1), 0xFF)
                        fSign := shr(7, and(b, 0x80))
                        fZero := iszero(b)
                        fParity := computeParity(b)
                        regPC := add(regPC, 1)
                    }
                    
                    // 0xC3: JMP adr
                    case 0xC3 {
                        if iszero(lt(add(regPC, 2), len)) { break }
                        let low := byte(0, mload(add(0x1001, regPC)))
                        let high := byte(0, mload(add(0x1002, regPC)))
                        regPC := or(low, shl(8, high))
                    }
                    
                    default {
                        // Unsupported opcode, halt execution
                        break
                    }
                }
                
                // Pack flags byte
                // PSW format: S Z 0 AC 0 P 1 CY
                let flagsByte := 2
                if fSign { flagsByte := or(flagsByte, 0x80) }
                if fZero { flagsByte := or(flagsByte, 0x40) }
                if fAuxCarry { flagsByte := or(flagsByte, 0x10) }
                if fParity { flagsByte := or(flagsByte, 0x04) }
                if fCarry { flagsByte := or(flagsByte, 0x01) }
                
                mstore(0x00, a)
                mstore(0x20, b)
                mstore(0x40, c)
                mstore(0x60, d)
                mstore(0x80, e)
                mstore(0xa0, h)
                mstore(0xc0, l)
                mstore(0xe0, regPC)
                mstore(0x100, sp)
                mstore(0x120, flagsByte)
                
                return(0x00, 0x140)
            }
            
            revert(0, 0)
        }
    }
}
