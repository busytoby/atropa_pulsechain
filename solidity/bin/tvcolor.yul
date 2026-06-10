object "TVColorGenerator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // generatePattern(uint256 patternType) -> returns (bytes colorSignals)
            // selector: 0x4590dcc3
            if eq(selector, 0x4590dcc3) {
                let patternType := calldataload(4)
                
                // 32 * 32 pixels * 3 bytes/pixel = 3072 bytes
                let outputSize := 3072
                
                let outPtr := 0x40040
                
                // Pattern 0: Color Bars
                if iszero(patternType) {
                    for { let r := 0 } lt(r, 32) { r := add(r, 1) } {
                        for { let c := 0 } lt(c, 32) { c := add(c, 1) } {
                            let bar := div(c, 4)
                            let yVal := 0
                            let phaseVal := 0
                            let burstVal := 0
                            
                            switch bar
                            case 0 { // White
                                yVal := 255
                                phaseVal := 0
                            }
                            case 1 { // Yellow
                                yVal := 180
                                phaseVal := 135
                            }
                            case 2 { // Cyan
                                yVal := 160
                                phaseVal := 270
                            }
                            case 3 { // Green
                                yVal := 130
                                phaseVal := 225
                            }
                            case 4 { // Magenta
                                yVal := 100
                                phaseVal := 45
                            }
                            case 5 { // Red
                                yVal := 80
                                phaseVal := 90
                            }
                            case 6 { // Blue
                                yVal := 40
                                phaseVal := 0
                            }
                            default { // Black
                                yVal := 0
                                phaseVal := 0
                            }
                            
                            mstore8(outPtr, yVal)
                            mstore8(add(outPtr, 1), phaseVal)
                            mstore8(add(outPtr, 2), burstVal)
                            outPtr := add(outPtr, 3)
                        }
                    }
                }
                
                // Pattern 1: Rainbow Phase Sweep
                if eq(patternType, 1) {
                    for { let r := 0 } lt(r, 32) { r := add(r, 1) } {
                        for { let c := 0 } lt(c, 32) { c := add(c, 1) } {
                            let yVal := 128
                            let phaseVal := div(mul(c, 360), 32)
                            let burstVal := 0
                            
                            mstore8(outPtr, yVal)
                            mstore8(add(outPtr, 1), div(phaseVal, 2)) // Store as phase/2 to fit in 8-bit byte
                            mstore8(add(outPtr, 2), burstVal)
                            outPtr := add(outPtr, 3)
                        }
                    }
                }
                
                // Pattern 2: Grid with Horizontal Color Burst Sync
                if eq(patternType, 2) {
                    for { let r := 0 } lt(r, 32) { r := add(r, 1) } {
                        for { let c := 0 } lt(c, 32) { c := add(c, 1) } {
                            let yVal := 0
                            let phaseVal := 0
                            let burstVal := 0
                            
                            // First 2 pixels of each line: Color Burst Sync active (180 degrees)
                            if lt(c, 2) {
                                yVal := 0
                                phaseVal := 180
                                burstVal := 1
                            }
                            
                            // Grid pattern
                            if iszero(lt(c, 2)) {
                                let onGrid := or(iszero(mod(r, 8)), iszero(mod(c, 8)))
                                if onGrid {
                                    yVal := 255
                                    phaseVal := 0
                                    burstVal := 0
                                }
                            }
                            
                            mstore8(outPtr, yVal)
                            mstore8(add(outPtr, 1), div(phaseVal, 2)) // Store as phase/2
                            mstore8(add(outPtr, 2), burstVal)
                            outPtr := add(outPtr, 3)
                        }
                    }
                }
                
                // ABI dynamic bytes output structure
                mstore(0x40000, 0x20)
                mstore(0x40020, outputSize)
                
                let paddedWords := div(add(outputSize, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x40000, returnSize)
            }
            
            revert(0, 0)
        }
    }
}
