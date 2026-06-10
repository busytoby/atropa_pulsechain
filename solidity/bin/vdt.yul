object "VDTTerminal" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // Helper to get screen character: logical index 0 to 1919
            function getChar(idx) -> val {
                let startRow := sload(58002)
                let logicalRow := div(idx, 80)
                let logicalCol := mod(idx, 80)
                let physicalRow := mod(add(startRow, logicalRow), 24)
                let physicalIdx := add(mul(physicalRow, 80), logicalCol)
                val := sload(add(58010, physicalIdx))
                if iszero(val) {
                    val := 0x20 // Default to space
                }
            }
            
            // Helper to set screen character: logical index 0 to 1919
            function setChar(idx, val) {
                let startRow := sload(58002)
                let logicalRow := div(idx, 80)
                let logicalCol := mod(idx, 80)
                let physicalRow := mod(add(startRow, logicalRow), 24)
                let physicalIdx := add(mul(physicalRow, 80), logicalCol)
                if eq(val, 0x20) {
                    val := 0
                }
                sstore(add(58010, physicalIdx), val)
            }
            
            // 1. writeChar(uint8 char)
            // selector: 0x1bccde3b
            if eq(selector, 0x1bccde3b) {
                let char := and(calldataload(4), 0xFF)
                
                let col := sload(58000)
                let row := sload(58001)
                
                // Backspace (0x08)
                if eq(char, 0x08) {
                    if gt(col, 0) {
                        col := sub(col, 1)
                        setChar(add(mul(row, 80), col), 0x20)
                    }
                    sstore(58000, col)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }
                
                // Carriage Return (0x0D)
                if eq(char, 0x0D) {
                    col := 0
                    sstore(58000, col)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }
                
                // Newline (0x0A)
                if eq(char, 0x0A) {
                    col := 0
                    row := add(row, 1)
                    
                    // Handle scrolling
                    if iszero(lt(row, 24)) {
                        let startRow := sload(58002)
                        startRow := mod(add(startRow, 1), 24)
                        sstore(58002, startRow)
                        
                        // Clear bottom row (logical row 23)
                        for { let c := 0 } lt(c, 80) { c := add(c, 1) } {
                            setChar(add(mul(23, 80), c), 0x20)
                        }
                        row := 23
                    }
                    sstore(58000, col)
                    sstore(58001, row)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }
                
                // Normal printable character
                let writeIdx := add(mul(row, 80), col)
                setChar(writeIdx, char)
                
                col := add(col, 1)
                if iszero(lt(col, 80)) {
                    col := 0
                    row := add(row, 1)
                    
                    // Handle scrolling
                    if iszero(lt(row, 24)) {
                        let startRow := sload(58002)
                        startRow := mod(add(startRow, 1), 24)
                        sstore(58002, startRow)
                        
                        // Clear bottom row (logical row 23)
                        for { let c := 0 } lt(c, 80) { c := add(c, 1) } {
                            setChar(add(mul(23, 80), c), 0x20)
                        }
                        row := 23
                    }
                }
                
                sstore(58000, col)
                sstore(58001, row)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            // 2. getScreenRAM() -> returns (bytes)
            // selector: 0x4a5d1e5c
            if eq(selector, 0x4a5d1e5c) {
                let outputSize := 1920
                mstore(0x00, 0x20) // Offset
                mstore(0x20, outputSize) // Length
                
                for { let i := 0 } lt(i, outputSize) { i := add(i, 1) } {
                    mstore8(add(0x40, i), getChar(i))
                }
                
                let paddedWords := div(add(outputSize, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x00, returnSize)
            }
            
            // 3. getCursor() -> returns (uint256 col, uint256 row)
            // selector: 0x380169ea
            if eq(selector, 0x380169ea) {
                mstore(0x00, sload(58000))
                mstore(0x20, sload(58001))
                return(0x00, 64)
            }
            
            // 4. clearScreen()
            // selector: 0xc8967e3e
            if eq(selector, 0xc8967e3e) {
                // Clear all 1920 character cells
                for { let i := 0 } lt(i, 1920) { i := add(i, 1) } {
                    sstore(add(58010, i), 0)
                }
                sstore(58000, 0) // reset cursor
                sstore(58001, 0)
                sstore(58002, 0) // reset startRow
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
