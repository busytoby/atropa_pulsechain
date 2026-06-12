object "Assembler" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helper to check if a character is whitespace
            function isSpace(c) -> space {
                space := or(or(eq(c, 32), eq(c, 9)), or(eq(c, 10), eq(c, 13)))
            }

            // 1. assemble(string code) -> bytes bytecode
            // Selector: 0xccc713d2
            if eq(selector, 0xccc713d2) {
                let codeOffset := calldataload(4)
                let codeLen := calldataload(add(4, codeOffset))
                let codeStart := add(36, codeOffset)

                mstore(0x00, 0x20) // output offset
                // output length will be filled at 0x20 later
                let memPtr := 0x40

                let i := 0
                for { } lt(i, codeLen) { } {
                    let char := byte(0, calldataload(add(codeStart, i)))
                    if isSpace(char) {
                        i := add(i, 1)
                        continue
                    }

                    // Load up to 4 chars for matching mnemonics
                    let c0 := byte(0, calldataload(add(codeStart, i)))
                    let c1 := 0
                    if lt(add(i, 1), codeLen) { c1 := byte(0, calldataload(add(codeStart, add(i, 1)))) }
                    let c2 := 0
                    if lt(add(i, 2), codeLen) { c2 := byte(0, calldataload(add(codeStart, add(i, 2)))) }
                    let c3 := 0
                    if lt(add(i, 3), codeLen) { c3 := byte(0, calldataload(add(codeStart, add(i, 3)))) }

                    // NOP (78, 79, 80)
                    if and(and(eq(c0, 78), eq(c1, 79)), eq(c2, 80)) {
                        mstore8(memPtr, 0x00)
                        memPtr := add(memPtr, 1)
                        i := add(i, 3)
                        continue
                    }

                    // ADD (65, 68, 68)
                    if and(and(eq(c0, 65), eq(c1, 68)), eq(c2, 68)) {
                        mstore8(memPtr, 0x03)
                        memPtr := add(memPtr, 1)
                        i := add(i, 5) // Skip "ADD B"
                        continue
                    }

                    // SUB (83, 85, 66)
                    if and(and(eq(c0, 83), eq(c1, 85)), eq(c2, 66)) {
                        mstore8(memPtr, 0x04)
                        memPtr := add(memPtr, 1)
                        i := add(i, 5) // Skip "SUB B"
                        continue
                    }

                    // HLT (72, 76, 84)
                    if and(and(eq(c0, 72), eq(c1, 76)), eq(c2, 84)) {
                        mstore8(memPtr, 0xFF)
                        memPtr := add(memPtr, 1)
                        i := add(i, 3)
                        continue
                    }

                    // MVI (77, 86, 73)
                    if and(and(eq(c0, 77), eq(c1, 86)), eq(c2, 73)) {
                        let reg := byte(0, calldataload(add(codeStart, add(i, 4))))
                        let op := 0x01 // MVI A
                        if eq(reg, 66) { op := 0x02 } // MVI B
                        
                        mstore8(memPtr, op)
                        memPtr := add(memPtr, 1)

                        let nextIdx := add(i, 5)
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if iszero(isSpace(tc)) { break }
                        }
                        
                        let num := 0
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if or(lt(tc, 48), gt(tc, 57)) { break }
                            num := add(mul(num, 10), sub(tc, 48))
                        }
                        
                        mstore8(memPtr, num)
                        memPtr := add(memPtr, 1)
                        i := nextIdx
                        continue
                    }

                    // SNT (83, 78, 84) - Set Synthesizer Parameter
                    if and(and(eq(c0, 83), eq(c1, 78)), eq(c2, 84)) {
                        mstore8(memPtr, 0x09)
                        memPtr := add(memPtr, 1)

                        let nextIdx := add(i, 3)
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if iszero(isSpace(tc)) { break }
                        }
                        
                        let num := 0
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if or(lt(tc, 48), gt(tc, 57)) { break }
                            num := add(mul(num, 10), sub(tc, 48))
                        }
                        
                        mstore8(memPtr, num)
                        memPtr := add(memPtr, 1)
                        i := nextIdx
                        continue
                    }

                    // SFX (83, 70, 88) - Trigger Synth Effect
                    if and(and(eq(c0, 83), eq(c1, 70)), eq(c2, 88)) {
                        mstore8(memPtr, 0x0A)
                        memPtr := add(memPtr, 1)

                        let nextIdx := add(i, 3)
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if iszero(isSpace(tc)) { break }
                        }
                        
                        let num := 0
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if or(lt(tc, 48), gt(tc, 57)) { break }
                            num := add(mul(num, 10), sub(tc, 48))
                        }
                        
                        mstore8(memPtr, num)
                        memPtr := add(memPtr, 1)
                        i := nextIdx
                        continue
                    }

                    // JMP, JZ, LDA, STA
                    let op := 0
                    let skipCount := 3
                    if and(and(eq(c0, 74), eq(c1, 77)), eq(c2, 80)) { op := 0x05 }
                    if and(eq(c0, 74), eq(c1, 90)) {
                        op := 0x06
                        skipCount := 2
                    }
                    if and(and(eq(c0, 76), eq(c1, 68)), eq(c2, 65)) { op := 0x07 }
                    if and(and(eq(c0, 83), eq(c1, 84)), eq(c2, 65)) { op := 0x08 }

                    if gt(op, 0) {
                        mstore8(memPtr, op)
                        memPtr := add(memPtr, 1)

                        let nextIdx := add(i, skipCount)
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if iszero(isSpace(tc)) { break }
                        }

                        let num := 0
                        for { } lt(nextIdx, codeLen) { nextIdx := add(nextIdx, 1) } {
                            let tc := byte(0, calldataload(add(codeStart, nextIdx)))
                            if or(lt(tc, 48), gt(tc, 57)) { break }
                            num := add(mul(num, 10), sub(tc, 48))
                        }

                        mstore8(memPtr, shr(8, num))
                        memPtr := add(memPtr, 1)
                        mstore8(memPtr, and(num, 0xFF))
                        memPtr := add(memPtr, 1)
                        i := nextIdx
                        continue
                    }

                    // Ignore other characters
                    i := add(i, 1)
                }

                let finalLen := sub(memPtr, 0x40)
                mstore(0x20, finalLen)

                let paddedWords := div(add(finalLen, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }

            revert(0, 0)
        }
    }
}
