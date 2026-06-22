object "CardcoPrinterBridge" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // translateIecToCentronics(bytes serialPayload) -> (bytes asciiOutput, uint256 strobeCount, uint256 cycles)
            // Selector: 0xa77ca92f
            if eq(selector, 0xa77ca92f) {
                let offset := calldataload(4)
                let length := calldataload(add(4, offset))

                let destOffset := 0x80
                let strobeCount := 0
                let cycles := 0

                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let petsciiByte := byte(0, calldataload(add(add(36, offset), i)))

                    let asciiByte := petsciiByte
                    cycles := add(cycles, 8) // Bit-shft extraction cost

                    // PETSCII to ASCII Conversion rules:
                    if and(gt(petsciiByte, 0x40), lt(petsciiByte, 0x5b)) {
                        // Standard uppercase letters: pass-through
                        asciiByte := petsciiByte
                    }
                    if and(gt(petsciiByte, 0xc0), lt(petsciiByte, 0xdb)) {
                        // Lowercase letters: map $C1-$DA down to $61-$7A
                        asciiByte := sub(petsciiByte, 0x60)
                    }
                    
                    // Centronics parallel latching simulation
                    strobeCount := add(strobeCount, 1)
                    cycles := add(cycles, 12) // Latching cycle overhead

                    // Store translated ASCII character in memory buffer
                    mstore8(add(destOffset, i), asciiByte)
                }

                // Return payload
                mstore(0x00, 0x60) // Offset to dynamic bytes
                mstore(0x20, strobeCount) // Strobe pulse count
                mstore(0x40, cycles) // CPU cycle cost
                mstore(0x60, length) // Length of returned bytes
                return(0x00, add(0x80, mul(div(add(length, 31), 32), 32)))
            }

            // produceSpriteData(bytes asciiInput) -> (bytes spriteOutput)
            // Selector: 0x02f10424
            if eq(selector, 0x02f10424) {
                let offset := calldataload(4)
                let length := calldataload(add(4, offset))
                
                let destOffset := 0x40
                let outputLength := mul(length, 8)
                
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let char := byte(0, calldataload(add(add(36, offset), i)))
                    let font := getCharFont(char)
                    mstore(add(destOffset, mul(i, 8)), font)
                }
                
                mstore(0x00, 0x20)
                mstore(0x20, outputLength)
                let returnSize := add(0x40, mul(div(add(outputLength, 31), 32), 32))
                return(0x00, returnSize)
            }

            revert(0, 0)

            function getCharFont(char) -> font {
                switch char
                case 0x20 { font := 0x0000000000000000000000000000000000000000000000000000000000000000 } // Space
                case 0x41 { font := 0x183c667e66666600000000000000000000000000000000000000000000000000 } // A
                case 0x42 { font := 0x7c66667c66667c00000000000000000000000000000000000000000000000000 } // B
                case 0x43 { font := 0x3c66606060663c00000000000000000000000000000000000000000000000000 } // C
                case 0x44 { font := 0x786c6666666c7800000000000000000000000000000000000000000000000000 } // D
                case 0x45 { font := 0x7e60607c60607e00000000000000000000000000000000000000000000000000 } // E
                case 0x46 { font := 0x7e60607c60606000000000000000000000000000000000000000000000000000 } // F
                case 0x47 { font := 0x3c66606e66663c00000000000000000000000000000000000000000000000000 } // G
                case 0x48 { font := 0x6666667e66666600000000000000000000000000000000000000000000000000 } // H
                case 0x49 { font := 0x7e18181818187e00000000000000000000000000000000000000000000000000 } // I
                case 0x4a { font := 0x1e0c0c0c0c6c3800000000000000000000000000000000000000000000000000 } // J
                case 0x4b { font := 0x666c7870786c6600000000000000000000000000000000000000000000000000 } // K
                case 0x4c { font := 0x6060606060607e00000000000000000000000000000000000000000000000000 } // L
                case 0x4d { font := 0x667efec6c6c6c6000000000000000000000000000000000000000000000000 } // M
                case 0x4e { font := 0x66767e7666666600000000000000000000000000000000000000000000000000 } // N
                case 0x4f { font := 0x3c66666666663c00000000000000000000000000000000000000000000000000 } // O
                case 0x50 { font := 0x7c66667c60606000000000000000000000000000000000000000000000000000 } // P
                case 0x51 { font := 0x3c666666666c361c000000000000000000000000000000000000000000000000 } // Q
                case 0x52 { font := 0x7c66667c786c6600000000000000000000000000000000000000000000000000 } // R
                case 0x53 { font := 0x3c6630180c663c00000000000000000000000000000000000000000000000000 } // S
                case 0x54 { font := 0x7e18181818181800000000000000000000000000000000000000000000000000 } // T
                case 0x55 { font := 0x6666666666663c00000000000000000000000000000000000000000000000000 } // U
                case 0x56 { font := 0x66666666663c1800000000000000000000000000000000000000000000000000 } // V
                case 0x57 { font := 0x66c6c6d6fee666000000000000000000000000000000000000000000000000 } // W
                case 0x58 { font := 0x66663c183c666600000000000000000000000000000000000000000000000000 } // X
                case 0x59 { font := 0x66663c1818181800000000000000000000000000000000000000000000000000 } // Y
                case 0x5a { font := 0x7e060c1830607e00000000000000000000000000000000000000000000000000 } // Z
                default   { font := 0xffffffffffffffff000000000000000000000000000000000000000000000000 }
            }
        }
    }
}