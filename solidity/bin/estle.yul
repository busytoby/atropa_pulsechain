object "EstleCompressor" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // compress(bytes rawData) -> returns (bytes rleData)
            // selector: 0x7e2224bd
            if eq(selector, 0x7e2224bd) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                calldatacopy(0x1000, start, len)
                
                let outPtr := 0x40040
                
                if gt(len, 0) {
                    let currentVal := byte(0, mload(0x1000))
                    let currentCount := 1
                    
                    for { let i := 1 } lt(i, len) { i := add(i, 1) } {
                        let nextVal := byte(0, mload(add(0x1000, i)))
                        
                        if and(eq(nextVal, currentVal), lt(currentCount, 255)) {
                            currentCount := add(currentCount, 1)
                            continue
                        }
                        
                        // Output run
                        mstore8(outPtr, currentCount)
                        mstore8(add(outPtr, 1), currentVal)
                        outPtr := add(outPtr, 2)
                        
                        currentVal := nextVal
                        currentCount := 1
                    }
                    
                    // Output final run
                    mstore8(outPtr, currentCount)
                    mstore8(add(outPtr, 1), currentVal)
                    outPtr := add(outPtr, 2)
                }
                
                let outputSize := sub(outPtr, 0x40040)
                mstore(0x40000, 0x20)
                mstore(0x40020, outputSize)
                
                let paddedWords := div(add(outputSize, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x40000, returnSize)
            }
            
            // decompress(bytes rleData) -> returns (bytes rawData)
            // selector: 0x5cd3f3a1
            if eq(selector, 0x5cd3f3a1) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                calldatacopy(0x1000, start, len)
                
                let outPtr := 0x40040
                
                for { let i := 0 } lt(i, len) { i := add(i, 2) } {
                    let count := byte(0, mload(add(0x1000, i)))
                    let val := byte(0, mload(add(0x1000, add(i, 1))))
                    
                    for { let c := 0 } lt(c, count) { c := add(c, 1) } {
                        mstore8(outPtr, val)
                        outPtr := add(outPtr, 1)
                    }
                }
                
                let outputSize := sub(outPtr, 0x40040)
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
