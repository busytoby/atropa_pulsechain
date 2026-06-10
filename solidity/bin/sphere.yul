object "SphereSystem" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for character formatting
            function parseHexChar(c) -> val {
                val := 0
                if and(iszero(lt(c, 48)), iszero(gt(c, 57))) { val := sub(c, 48) }
                if and(iszero(lt(c, 65)), iszero(gt(c, 70))) { val := add(10, sub(c, 65)) }
                if and(iszero(lt(c, 97)), iszero(gt(c, 102))) { val := add(10, sub(c, 97)) }
            }

            function getHexChar(val) -> c {
                if lt(val, 10) { c := add(val, 48) }
                if iszero(lt(val, 10)) { c := add(sub(val, 10), 65) }
            }

            // Helpers for CPU registers
            function getPC() -> pcVal { pcVal := sload(95000) }
            function setPC(pcVal) { sstore(95000, pcVal) }

            // Helpers for Video RAM (512 bytes: 95100 to 95611)
            function getVRAM(offset) -> char {
                char := sload(add(95100, offset))
                if iszero(char) { char := 0x20 } // default space
            }
            function setVRAM(offset, char) {
                sstore(add(95100, offset), and(char, 0xFF))
            }

            // 1. writeDisplay(uint256 offset, uint8 char)
            // Selector: 0x1ad58968
            if eq(selector, 0x1ad58968) {
                let offset := calldataload(4)
                let char := and(calldataload(36), 0xFF)
                if lt(offset, 512) {
                    setVRAM(offset, char)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 2. executeMonitorCommand(string cmd) -> string output
            // Selector: 0xc1133d67
            if eq(selector, 0xc1133d67) {
                let cmdOffset := calldataload(4)
                let cmdLen := calldataload(add(4, cmdOffset))
                let cmdStart := add(36, cmdOffset)

                mstore(0x00, 0x20) // output offset
                // output length will be filled at 0x20 later
                let memPtr := 0x40

                let c0 := byte(0, calldataload(cmdStart))

                // Command 'L' (Load / Examine memory: "L ABCD")
                if eq(c0, 76) {
                    let addr := 0
                    if gt(cmdLen, 5) {
                        let a0 := parseHexChar(byte(0, calldataload(add(cmdStart, 2))))
                        let a1 := parseHexChar(byte(0, calldataload(add(cmdStart, 3))))
                        let a2 := parseHexChar(byte(0, calldataload(add(cmdStart, 4))))
                        let a3 := parseHexChar(byte(0, calldataload(add(cmdStart, 5))))
                        addr := or(or(shl(12, a0), shl(8, a1)), or(shl(4, a2), a3))
                    }
                    
                    let val := sload(add(96000, addr))

                    // Construct return string: "ABCD: XX"
                    let a0 := getHexChar(and(shr(12, addr), 0xF))
                    let a1 := getHexChar(and(shr(8, addr), 0xF))
                    let a2 := getHexChar(and(shr(4, addr), 0xF))
                    let a3 := getHexChar(and(addr, 0xF))
                    let hVal := getHexChar(and(shr(4, val), 0xF))
                    let lVal := getHexChar(and(val, 0xF))

                    mstore8(memPtr, a0)
                    mstore8(add(memPtr, 1), a1)
                    mstore8(add(memPtr, 2), a2)
                    mstore8(add(memPtr, 3), a3)
                    mstore8(add(memPtr, 4), 58) // ':'
                    mstore8(add(memPtr, 5), 32) // ' '
                    mstore8(add(memPtr, 6), hVal)
                    mstore8(add(memPtr, 7), lVal)

                    mstore(0x20, 8) // output string length
                    return(0x00, 128)
                }

                // Command 'D' (Deposit memory: "D ABCD XX")
                if eq(c0, 68) {
                    let addr := 0
                    let val := 0
                    if gt(cmdLen, 8) {
                        let a0 := parseHexChar(byte(0, calldataload(add(cmdStart, 2))))
                        let a1 := parseHexChar(byte(0, calldataload(add(cmdStart, 3))))
                        let a2 := parseHexChar(byte(0, calldataload(add(cmdStart, 4))))
                        let a3 := parseHexChar(byte(0, calldataload(add(cmdStart, 5))))
                        addr := or(or(shl(12, a0), shl(8, a1)), or(shl(4, a2), a3))

                        let v0 := parseHexChar(byte(0, calldataload(add(cmdStart, 7))))
                        let v1 := parseHexChar(byte(0, calldataload(add(cmdStart, 8))))
                        val := or(shl(4, v0), v1)
                    }

                    sstore(add(96000, addr), val)

                    mstore8(memPtr, 79) // 'O'
                    mstore8(add(memPtr, 1), 75) // 'K'
                    mstore(0x20, 2)
                    return(0x00, 128)
                }

                // Command 'G' (Go / Start: "G ABCD")
                if eq(c0, 71) {
                    let addr := 0
                    if gt(cmdLen, 5) {
                        let a0 := parseHexChar(byte(0, calldataload(add(cmdStart, 2))))
                        let a1 := parseHexChar(byte(0, calldataload(add(cmdStart, 3))))
                        let a2 := parseHexChar(byte(0, calldataload(add(cmdStart, 4))))
                        let a3 := parseHexChar(byte(0, calldataload(add(cmdStart, 5))))
                        addr := or(or(shl(12, a0), shl(8, a1)), or(shl(4, a2), a3))
                    }

                    setPC(addr)

                    // Return "STARTING AT ABCD"
                    let msg := "STARTING AT "
                    for { let offset := 0 } lt(offset, 12) { offset := add(offset, 1) } {
                        mstore8(add(memPtr, offset), byte(offset, msg))
                    }
                    let a0 := getHexChar(and(shr(12, addr), 0xF))
                    let a1 := getHexChar(and(shr(8, addr), 0xF))
                    let a2 := getHexChar(and(shr(4, addr), 0xF))
                    let a3 := getHexChar(and(addr, 0xF))

                    mstore8(add(memPtr, 12), a0)
                    mstore8(add(memPtr, 13), a1)
                    mstore8(add(memPtr, 14), a2)
                    mstore8(add(memPtr, 15), a3)

                    mstore(0x20, 16)
                    return(0x00, 128)
                }

                // Command 'V' (View screen display: "V")
                if eq(c0, 86) {
                    mstore(0x20, 512) // String length is 512 characters
                    
                    for { let offset := 0 } lt(offset, 512) { offset := add(offset, 1) } {
                        mstore8(add(memPtr, offset), getVRAM(offset))
                    }

                    let paddedWords := div(add(512, 31), 32)
                    let returnSize := add(64, mul(paddedWords, 32))
                    return(0x00, returnSize)
                }

                // Default output "ERR"
                mstore8(memPtr, 69) // 'E'
                mstore8(add(memPtr, 1), 82) // 'R'
                mstore8(add(memPtr, 2), 82) // 'R'
                mstore(0x20, 3)
                return(0x00, 128)
            }

            revert(0, 0)
        }
    }
}
