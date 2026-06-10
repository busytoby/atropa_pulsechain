object "MahoneyTypesetter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // justify(bytes text, uint256 lineLength) -> returns (bytes justified)
            // selector: 0xbb22d2b7
            if eq(selector, 0xbb22d2b7) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                let lineLength := calldataload(36)
                if iszero(lineLength) { revert(0, 0) }
                
                // Copy source text to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // We will store word metadata (startOffset, length) at 0x10000
                // Each word entry is 64 bytes (32 bytes start, 32 bytes len)
                let wordCount := 0
                let insideWord := 0
                let wordStart := 0
                
                for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                    let char := byte(0, mload(add(0x1000, i)))
                    let isSpace := or(eq(char, 0x20), eq(char, 0x0a)) // space or newline
                    
                    if isSpace {
                        if insideWord {
                            let wLen := sub(i, wordStart)
                            mstore(add(0x10000, mul(wordCount, 64)), wordStart)
                            mstore(add(0x10000, add(mul(wordCount, 64), 32)), wLen)
                            wordCount := add(wordCount, 1)
                            insideWord := 0
                        }
                    }
                    if iszero(isSpace) {
                        if iszero(insideWord) {
                            wordStart := i
                            insideWord := 1
                        }
                    }
                }
                
                // Handle the last word if text doesn't end with space
                if insideWord {
                    let wLen := sub(len, wordStart)
                    mstore(add(0x10000, mul(wordCount, 64)), wordStart)
                    mstore(add(0x10000, add(mul(wordCount, 64), 32)), wLen)
                    wordCount := add(wordCount, 1)
                }
                
                // Output buffer will start at 0x40040
                // Reserve 0x40000 for output metadata offsets
                let outPtr := 0x40040
                
                let lineStartWord := 0
                let currentLineWordsLength := 0
                let currentLineWordCount := 0
                
                for { let w := 0 } lt(w, wordCount) { w := add(w, 1) } {
                    let wStart := mload(add(0x10000, mul(w, 64)))
                    let wLen := mload(add(0x10000, add(mul(w, 64), 32)))
                    
                    // Check if adding this word (and 1 space separator) exceeds the lineLength
                    let nextLen := wLen
                    if gt(currentLineWordCount, 0) {
                        nextLen := add(nextLen, 1)
                    }
                    
                    if gt(add(currentLineWordsLength, nextLen), lineLength) {
                        // Justify the accumulated line
                        let totalSpaces := sub(lineLength, currentLineWordsLength)
                        
                        // Add back the virtual spaces we subtracted for calculation
                        if gt(currentLineWordCount, 1) {
                            totalSpaces := add(totalSpaces, sub(currentLineWordCount, 1))
                        }
                        
                        let baseSpaces := 0
                        let extraSpaces := 0
                        if gt(currentLineWordCount, 1) {
                            let gaps := sub(currentLineWordCount, 1)
                            baseSpaces := div(totalSpaces, gaps)
                            extraSpaces := mod(totalSpaces, gaps)
                        }
                        
                        for { let lw := 0 } lt(lw, currentLineWordCount) { lw := add(lw, 1) } {
                            let activeWordIdx := add(lineStartWord, lw)
                            let currWStart := mload(add(0x10000, mul(activeWordIdx, 64)))
                            let currWLen := mload(add(0x10000, add(mul(activeWordIdx, 64), 32)))
                            
                            // Copy word characters
                            for { let c := 0 } lt(c, currWLen) { c := add(c, 1) } {
                                mstore8(outPtr, byte(0, mload(add(0x1000, add(currWStart, c)))))
                                outPtr := add(outPtr, 1)
                            }
                            
                            // Write spaces if not the last word on the line
                            if lt(lw, sub(currentLineWordCount, 1)) {
                                let spacesToInsert := baseSpaces
                                if lt(lw, extraSpaces) {
                                    spacesToInsert := add(spacesToInsert, 1)
                                }
                                for { let s := 0 } lt(s, spacesToInsert) { s := add(s, 1) } {
                                    mstore8(outPtr, 0x20)
                                    outPtr := add(outPtr, 1)
                                }
                            }
                        }
                        
                        // Append newline character at end of justified line
                        mstore8(outPtr, 0x0a)
                        outPtr := add(outPtr, 1)
                        
                        // Reset line accumulators
                        lineStartWord := w
                        currentLineWordsLength := 0
                        currentLineWordCount := 0
                    }
                    
                    currentLineWordCount := add(currentLineWordCount, 1)
                    if eq(currentLineWordCount, 1) {
                        currentLineWordsLength := wLen
                    }
                    if gt(currentLineWordCount, 1) {
                        currentLineWordsLength := add(add(currentLineWordsLength, 1), wLen)
                    }
                }
                
                // Write the last line (left-justified, standard space-separated)
                if gt(currentLineWordCount, 0) {
                    for { let lw := 0 } lt(lw, currentLineWordCount) { lw := add(lw, 1) } {
                        let activeWordIdx := add(lineStartWord, lw)
                        let currWStart := mload(add(0x10000, mul(activeWordIdx, 64)))
                        let currWLen := mload(add(0x10000, add(mul(activeWordIdx, 64), 32)))
                        
                        // Copy word characters
                        for { let c := 0 } lt(c, currWLen) { c := add(c, 1) } {
                            mstore8(outPtr, byte(0, mload(add(0x1000, add(currWStart, c)))))
                            outPtr := add(outPtr, 1)
                        }
                        
                        // Write single space if not the last word
                        if lt(lw, sub(currentLineWordCount, 1)) {
                            mstore8(outPtr, 0x20)
                            outPtr := add(outPtr, 1)
                        }
                    }
                    // Append final newline character
                    mstore8(outPtr, 0x0a)
                    outPtr := add(outPtr, 1)
                }
                
                // Return justified text (ABI bytes structure)
                let outputSize := sub(outPtr, 0x40040)
                mstore(0x40000, 0x20) // offset to start of dynamic array
                mstore(0x40020, outputSize) // length of array
                
                // Pad return size to 32-byte multiple
                let paddedWords := div(add(outputSize, 31), 32)
                let returnSize := add(64, mul(paddedWords, 32))
                return(0x40000, returnSize)
            }
            
            revert(0, 0)
        }
    }
}
