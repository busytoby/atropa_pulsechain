object "TTYSerialLoop" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helper to count set bits in a byte
            function countBits(val) -> count {
                count := 0
                for { let temp := val } gt(temp, 0) { temp := shr(1, temp) } {
                    count := add(count, and(temp, 1))
                }
            }

            // Helper to compute parity bit (0 or 1)
            // mode: 0=None, 1=Even, 2=Odd
            function computeParity(val, mode) -> bit {
                bit := 0
                if eq(mode, 1) { // Even
                    bit := and(countBits(val), 1)
                }
                if eq(mode, 2) { // Odd
                    bit := iszero(and(countBits(val), 1))
                }
            }

            // 1. encodeSerial(bytes data, uint8 parityMode) -> bytes pulses
            // Selector: 0x95fe7833
            if eq(selector, 0x95fe7833) {
                let dataOffset := calldataload(4)
                let parityMode := and(calldataload(36), 0xFF)

                let dataLen := calldataload(add(4, dataOffset))
                let dataStart := add(36, dataOffset) // Start of characters data

                let frameSize := 10 // 1 start + 8 data + 1 stop
                if gt(parityMode, 0) {
                    frameSize := 11 // 1 start + 8 data + 1 parity + 1 stop
                }

                let outputLen := mul(dataLen, frameSize)

                // Return ABI: [offset to bytes][bytes array length][bytes values...]
                mstore(0x00, 0x20) // offset
                mstore(0x20, outputLen) // length

                let memPtr := 0x40

                for { let i := 0 } lt(i, dataLen) { i := add(i, 1) } {
                    let char := byte(0, calldataload(add(dataStart, i)))

                    // 1. Start bit (0)
                    mstore8(memPtr, 0)
                    memPtr := add(memPtr, 1)

                    // 2. 8 Data bits (LSB first)
                    for { let b := 0 } lt(b, 8) { b := add(b, 1) } {
                        let bit := and(shr(b, char), 1)
                        mstore8(memPtr, bit)
                        memPtr := add(memPtr, 1)
                    }

                    // 3. Parity bit
                    if gt(parityMode, 0) {
                        let pBit := computeParity(char, parityMode)
                        mstore8(memPtr, pBit)
                        memPtr := add(memPtr, 1)
                    }

                    // 4. Stop bit (1)
                    mstore8(memPtr, 1)
                    memPtr := add(memPtr, 1)
                }

                let paddedWords := div(add(outputLen, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }

            // 2. decodeSerial(bytes pulses, uint8 parityMode) -> bytes decoded
            // Selector: 0xba5d27da
            if eq(selector, 0xba5d27da) {
                let pulsesOffset := calldataload(4)
                let parityMode := and(calldataload(36), 0xFF)

                let pulsesLen := calldataload(add(4, pulsesOffset))
                let pulsesStart := add(36, pulsesOffset)

                let frameSize := 10
                if gt(parityMode, 0) {
                    frameSize := 11
                }

                let maxDecodedLen := div(pulsesLen, frameSize)
                let decodedLen := 0

                mstore(0x00, 0x20) // Offset to return bytes
                let memPtr := 0x40

                let i := 0
                for { } lt(i, pulsesLen) { } {
                    // Look for start bit (0)
                    let pVal := byte(0, calldataload(add(pulsesStart, i)))
                    
                    if eq(pVal, 0) {
                        // Check if we have enough bits left for a full frame
                        if lt(add(i, frameSize), add(pulsesLen, 1)) {
                            // Decode 8 data bits
                            let char := 0
                            for { let b := 0 } lt(b, 8) { b := add(b, 1) } {
                                let bitVal := byte(0, calldataload(add(pulsesStart, add(add(i, 1), b))))
                                if bitVal {
                                    char := or(char, shl(b, 1))
                                }
                            }

                            // Write decoded character to memory
                            mstore8(memPtr, char)
                            memPtr := add(memPtr, 1)
                            decodedLen := add(decodedLen, 1)

                            i := add(i, frameSize)
                            continue
                        }
                    }
                    
                    i := add(i, 1)
                }

                mstore(0x20, decodedLen)
                let paddedWords := div(add(decodedLen, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }

            revert(0, 0)
        }
    }
}
