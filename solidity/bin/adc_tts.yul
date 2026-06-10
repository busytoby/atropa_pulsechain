object "ADCTTS" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // 1. encodeDeltaMod(bytes pcmData, uint8 stepSize) -> bytes encoded
            // Selector: 0x076ce0a7
            if eq(selector, 0x076ce0a7) {
                let pcmOffset := calldataload(4)
                let stepSize := and(calldataload(36), 0xFF)
                if iszero(stepSize) { stepSize := 8 } // Default step

                let pcmLen := calldataload(add(4, pcmOffset))
                let pcmStart := add(36, pcmOffset)

                // 8 PCM samples are packed into 1 encoded byte
                let outputLen := div(add(pcmLen, 7), 8)

                mstore(0x00, 0x20) // offset
                mstore(0x20, outputLen) // length

                let memPtr := 0x40
                let accumulator := 128 // midpoint of 8-bit unsigned PCM
                let byteVal := 0

                for { let i := 0 } lt(i, pcmLen) { i := add(i, 1) } {
                    let sample := byte(0, calldataload(add(pcmStart, i)))
                    let bit := 0
                    
                    if gt(sample, accumulator) {
                        bit := 1
                        accumulator := add(accumulator, stepSize)
                        if gt(accumulator, 255) { accumulator := 255 }
                    }
                    if iszero(gt(sample, accumulator)) {
                        bit := 0
                        if lt(accumulator, stepSize) {
                            accumulator := 0
                        }
                        if iszero(lt(accumulator, stepSize)) {
                            accumulator := sub(accumulator, stepSize)
                        }
                    }

                    let bitIdx := mod(i, 8)
                    byteVal := or(byteVal, shl(sub(7, bitIdx), bit))

                    if or(eq(bitIdx, 7), eq(i, sub(pcmLen, 1))) {
                        mstore8(memPtr, byteVal)
                        memPtr := add(memPtr, 1)
                        byteVal := 0
                    }
                }

                let paddedWords := div(add(outputLen, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }

            // 2. decodeDeltaMod(bytes encodedData, uint8 stepSize, uint256 length) -> bytes pcmData
            // Selector: 0xa60fba60
            if eq(selector, 0xa60fba60) {
                let encodedOffset := calldataload(4)
                let stepSize := and(calldataload(36), 0xFF)
                let originalLen := calldataload(68)
                if iszero(stepSize) { stepSize := 8 }

                let encodedStart := add(36, encodedOffset)

                mstore(0x00, 0x20)
                mstore(0x20, originalLen)

                let memPtr := 0x40
                let accumulator := 128

                for { let i := 0 } lt(i, originalLen) { i := add(i, 1) } {
                    let byteIdx := div(i, 8)
                    let bitIdx := mod(i, 8)
                    
                    let byteVal := byte(0, calldataload(add(encodedStart, byteIdx)))
                    let bit := and(shr(sub(7, bitIdx), byteVal), 1)

                    if bit {
                        accumulator := add(accumulator, stepSize)
                        if gt(accumulator, 255) { accumulator := 255 }
                    }
                    if iszero(bit) {
                        if lt(accumulator, stepSize) {
                            accumulator := 0
                        }
                        if iszero(lt(accumulator, stepSize)) {
                            accumulator := sub(accumulator, stepSize)
                        }
                    }

                    mstore8(memPtr, accumulator)
                    memPtr := add(memPtr, 1)
                }

                let paddedWords := div(add(originalLen, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }

            revert(0, 0)
        }
    }
}
