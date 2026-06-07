object "DiskSystem" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy the core runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // Return the runtime bytecode
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN PRODUCTION RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // ================================================================
            // FEATURE 1: SWITCH BOUNCE DEBOUNCER (SECURITY LAYER)
            // ================================================================
            mstore(0x00, caller())
            mstore(0x20, 0x99)
            let debounceSlot := keccak256(0x00, 0x40)
            let lastCallBlock := sload(debounceSlot)

            // Revert if call bounced in the same block
            if eq(lastCallBlock, number()) { revert(0, 0) }
            sstore(debounceSlot, number())

            // Helper to convert 20-byte address to 40 hex ASCII characters in memory
            function writeAddressHex(memPtr, addr) {
                for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                    let b := byte(add(12, i), addr)
                    let h1 := shr(4, b)
                    let h2 := and(b, 0x0F)
                    
                    let c1 := add(h1, 48)
                    if gt(h1, 9) { c1 := add(h1, 55) }
                    
                    let c2 := add(h2, 48)
                    if gt(h2, 9) { c2 := add(h2, 55) }
                    
                    mstore8(add(memPtr, mul(i, 2)), c1)
                    mstore8(add(add(memPtr, mul(i, 2)), 1), c2)
                }
            }

            // Helper to parse track and sector from a command string.
            // Scans the command starting at index 2 to bypass 'U1'/'U2' command prefix digits.
            function parseTrackSector(cmdOffset, cmdLen) -> track, sector {
                let count := 0
                let curNum := 0
                let inNum := 0
                for { let i := 2 } lt(i, cmdLen) { i := add(i, 1) } {
                    let char := byte(0, calldataload(add(cmdOffset, i)))
                    if iszero(char) { break }
                    if and(gt(char, 47), lt(char, 58)) {
                        curNum := add(mul(curNum, 10), sub(char, 48))
                        inNum := 1
                    }
                    if or(lt(char, 48), gt(char, 57)) {
                        if inNum {
                            mstore(add(0x260, mul(count, 32)), curNum)
                            count := add(count, 1)
                            curNum := 0
                            inNum := 0
                        }
                    }
                }
                if inNum {
                    mstore(add(0x260, mul(count, 32)), curNum)
                    count := add(count, 1)
                }
                if eq(count, 2) {
                    track := mload(0x260)
                    sector := mload(0x280)
                }
                if eq(count, 3) {
                    track := mload(0x280)
                    sector := mload(0x2A0)
                }
                if or(eq(count, 4), gt(count, 4)) {
                    track := mload(add(0x260, mul(sub(count, 2), 32)))
                    sector := mload(add(0x260, mul(sub(count, 1), 32)))
                }
            }

            // ----------------------------------------------------------------
            // METHOD 1: getJiffies() -> uint256
            // Selector: 0xb5123d47
            // ----------------------------------------------------------------
            if eq(selector, 0xb5123d47) {
                let elapsedJiffies := mul(number(), 60)
                mstore(0x00, elapsedJiffies)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 1.5: configureSystem(address token)
            // Selector: 0x6b9c2fdb
            // ----------------------------------------------------------------
            if eq(selector, 0x6b9c2fdb) {
                sstore(0x9999, calldataload(4))
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: executeDiskCommand(bytes calldata cmd)
            // Selector: 0x9812a4df
            // ----------------------------------------------------------------
            if or(eq(selector, 0x9812a4df), eq(selector, 0xeb603a54)) {
                let cmdLength := calldataload(36)
                if lt(cmdLength, 3) { revert(0, 0) }
                let cmdType := shr(240, calldataload(68))

                // Block Read Command (U1)
                if eq(cmdType, 0x5531) { // ASCII "U1"
                    let track, sector := parseTrackSector(68, cmdLength)
                    if or(iszero(track), gt(track, 40)) { revert(0, 0) }
                    if gt(sector, 30) { revert(0, 0) }
                    
                    mstore(0x00, track)
                    mstore(0x20, sector)
                    let sectorKey := keccak256(0x00, 64)
                    mstore(0x00, caller())
                    mstore(0x20, sectorKey)
                    let sectorSlot := keccak256(0x00, 64)
                    
                    mstore(0x00, 32)
                    mstore(0x20, 256)
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        mstore(add(0x40, mul(i, 32)), sload(add(sectorSlot, i)))
                    }
                    return(0x00, 320)
                }

                // Block Write Command (U2)
                if eq(cmdType, 0x5532) { // ASCII "U2"
                    let track, sector := parseTrackSector(68, cmdLength)
                    if or(iszero(track), gt(track, 40)) { revert(0, 0) }
                    if gt(sector, 30) { revert(0, 0) }
                    
                    let charCount := 0
                    for {} lt(charCount, cmdLength) { charCount := add(charCount, 1) } {
                        let char := byte(0, calldataload(add(68, charCount)))
                        if iszero(char) { break }
                    }
                    let dataStartOffset := add(68, add(charCount, 1))
                    
                    mstore(0x00, track)
                    mstore(0x20, sector)
                    let sectorKey := keccak256(0x00, 64)
                    mstore(0x00, caller())
                    mstore(0x20, sectorKey)
                    let sectorSlot := keccak256(0x00, 64)
                    
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        let word := calldataload(add(dataStartOffset, mul(i, 32)))
                        sstore(add(sectorSlot, i), word)
                    }
                    
                    let sectorID := or(shl(8, track), sector)
                    log3(dataStartOffset, 256, 0x18278cbdbe8ed62bf134676966ade77d7871afa34eabd366196c1e9547b08e63, caller(), sectorID)
                    
                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                // Parse filename until null terminator
                let nameHash := 0
                let fileOffset := 70
                let charCount := 0
                
                for {} lt(charCount, 32) { charCount := add(charCount, 1) } {
                    let char := byte(charCount, calldataload(fileOffset))
                    if iszero(char) { break }
                    nameHash := add(mul(nameHash, 31), char)
                }

                // Check if filename is "bbs.txt" (0x6262732e747874)
                let nameWord := calldataload(add(fileOffset, 1))
                let filenameLen := sub(charCount, 1)
                let nameMask := not(sub(exp(2, mul(8, sub(32, filenameLen))), 1))
                nameWord := and(nameWord, nameMask)
                let isBbs := eq(shr(200, nameWord), 0x6262732e747874)

                let baseSlot := 0
                let lengthSlot := 0

                if isBbs {
                    // Public Global namespace for BBS message board
                    mstore(0x00, nameHash)
                    baseSlot := keccak256(0x00, 32)
                    
                    mstore(0x00, nameHash)
                    mstore(0x20, 0x999)
                    lengthSlot := keccak256(0x00, 64)
                }
                if iszero(isBbs) {
                    // Sandbox-isolated private namespace per caller wallet
                    mstore(0x00, caller())
                    mstore(0x20, nameHash)
                    baseSlot := keccak256(0x00, 64)
                    
                    mstore(0x00, caller())
                    mstore(0x20, add(nameHash, 0x999))
                    lengthSlot := keccak256(0x00, 64)
                }

                // Duplicate Command (D0:destfile=srcfile)
                if eq(cmdType, 0x4430) { // ASCII "D0"
                    let destHash := 0
                    let fOffset := 70
                    let cCount := 0
                    let eqIdx := 0
                    
                    for {} lt(cCount, 32) { cCount := add(cCount, 1) } {
                        let char := byte(cCount, calldataload(fOffset))
                        if eq(char, 0x3d) {
                            eqIdx := cCount
                            break
                        }
                        if iszero(char) { break }
                        destHash := add(mul(destHash, 31), char)
                    }
                    
                    let srcHash := 0
                    let srcOffset := add(fOffset, add(eqIdx, 1))
                    let srcCharCount := 0
                    for {} lt(srcCharCount, 32) { srcCharCount := add(srcCharCount, 1) } {
                        let char := byte(srcCharCount, calldataload(srcOffset))
                        if iszero(char) { break }
                        srcHash := add(mul(srcHash, 31), char)
                    }
                    
                    mstore(0x00, caller())
                    mstore(0x20, srcHash)
                    let srcBaseSlot := keccak256(0x00, 64)
                    
                    mstore(0x00, caller())
                    mstore(0x20, add(srcHash, 0x999))
                    let srcLengthSlot := keccak256(0x00, 64)
                    
                    mstore(0x00, caller())
                    mstore(0x20, destHash)
                    let destBaseSlot := keccak256(0x00, 64)
                    
                    mstore(0x00, caller())
                    mstore(0x20, add(destHash, 0x999))
                    let destLengthSlot := keccak256(0x00, 64)
                    
                    let srcLen := sload(srcLengthSlot)
                    sstore(destLengthSlot, srcLen)
                    
                    let numSlots := div(add(srcLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        let data := sload(add(srcBaseSlot, i))
                        sstore(add(destBaseSlot, i), data)
                    }
                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                // Write Command (W0)
                if eq(cmdType, 0x5730) { // ASCII "W0"
                    let tokenAddr := sload(0x9999)
                    if and(iszero(iszero(tokenAddr)), isBbs) {
                        mstore(0x200, shl(224, 0x70a08231))
                        mstore(0x204, caller())
                        let success := staticcall(gas(), tokenAddr, 0x200, 36, 0x244, 32)
                        if iszero(success) { revert(0, 0) }
                        let bal := mload(0x244)
                        if lt(bal, 1000000000000000000) { revert(0, 0) } // Must have at least 1 token
                    }

                    let dataStartOffset := add(fileOffset, charCount)
                    dataStartOffset := add(dataStartOffset, 1) // skip null terminator

                    let dataLen := sub(cmdLength, sub(dataStartOffset, 68))
                    if gt(dataLen, 131072) {
                        dataLen := 131072
                    }

                    if isBbs {
                        // Append-only behavior for global BBS
                        let globalLen := sload(lengthSlot)
                        if gt(globalLen, 8192) {
                            globalLen := 0 // clear/reset if exceeds 8KB
                        }

                        // Load current global content to memory at 0x100
                        let numSlots := div(add(globalLen, 31), 32)
                        for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                            mstore(add(0x100, mul(i, 32)), sload(add(baseSlot, i)))
                        }

                        let writePtr := add(0x100, globalLen)

                        // 1. Newline divider
                        if gt(globalLen, 0) {
                            mstore8(writePtr, 10) // '\n'
                            writePtr := add(writePtr, 1)
                        }

                        // 2. Author prefix "[0xHEX]: "
                        mstore8(writePtr, 91) // '['
                        mstore8(add(writePtr, 1), 48) // '0'
                        mstore8(add(writePtr, 2), 120) // 'x'
                        writePtr := add(writePtr, 3)

                        writeAddressHex(writePtr, caller())
                        writePtr := add(writePtr, 40)

                        mstore8(writePtr, 93) // ']'
                        mstore8(add(writePtr, 1), 58) // ':'
                        mstore8(add(writePtr, 2), 32) // ' '
                        writePtr := add(writePtr, 3)

                        // 3. Append incoming message
                        for { let i := 0 } lt(i, dataLen) { i := add(i, 1) } {
                            let char := byte(0, calldataload(add(dataStartOffset, i)))
                            mstore8(add(writePtr, i), char)
                        }
                        writePtr := add(writePtr, dataLen)

                        // 4. Save consolidated content
                        let newLen := sub(writePtr, 0x100)
                        sstore(lengthSlot, newLen)

                        let newSlots := div(add(newLen, 31), 32)
                        for { let i := 0 } lt(i, newSlots) { i := add(i, 1) } {
                            sstore(add(baseSlot, i), mload(add(0x100, mul(i, 32))))
                        }
                    }
                    if iszero(isBbs) {
                        // Standard overwrite write for private sandbox partitions
                        sstore(lengthSlot, dataLen)
                        let numSlots := div(add(dataLen, 31), 32)
                        for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                            let word := calldataload(add(dataStartOffset, mul(i, 32)))
                            sstore(add(baseSlot, i), word)
                        }

                        // Track private directory listing
                        let numFilesSlot := 0
                        mstore(0x00, caller())
                        mstore(0x20, 0x1000)
                        numFilesSlot := keccak256(0x00, 64)
                        let numFiles := sload(numFilesSlot)
                        let found := 0
                        for { let i := 0 } lt(i, numFiles) { i := add(i, 1) } {
                            mstore(0x00, caller())
                            mstore(0x20, add(0x1001, i))
                            if eq(sload(keccak256(0x00, 64)), nameWord) {
                                found := 1
                                break
                            }
                        }
                        if iszero(found) {
                            mstore(0x00, caller())
                            mstore(0x20, add(0x1001, numFiles))
                            sstore(keccak256(0x00, 64), nameWord)
                            sstore(numFilesSlot, add(numFiles, 1))
                        }
                    }

                    // Emit event logs
                    log3(dataStartOffset, dataLen, 0x18278cbdbe8ed62bf134676966ade77d7871afa34eabd366196c1e9547b08e63, caller(), nameWord)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                // Read Command (R0)
                if eq(cmdType, 0x5230) { // ASCII "R0"
                    // Check if filename is "$"
                    let secondChar := byte(1, calldataload(fileOffset))
                    if and(eq(charCount, 2), eq(secondChar, 0x24)) {
                        // Directory listing (isolated per wallet)
                        mstore(0x00, 32)
                        mstore(0x20, 256)
                        let numFilesSlot := 0
                        mstore(0x40, caller())
                        mstore(0x60, 0x1000)
                        numFilesSlot := keccak256(0x40, 64)
                        let numFiles := sload(numFilesSlot)
                        for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                            let fileNameVal := 0
                            if lt(i, numFiles) {
                                mstore(0x40, caller())
                                mstore(0x60, add(0x1001, i))
                                fileNameVal := sload(keccak256(0x40, 64))
                            }
                            mstore(add(0x40, mul(i, 32)), fileNameVal)
                        }
                        return(0x00, 320)
                    }

                    // Read target file content (either global BBS or sandbox file)
                    let dataLen := sload(lengthSlot)
                    mstore(0x00, 32)
                    mstore(0x20, dataLen)

                    let numSlots := div(add(dataLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        mstore(add(0x40, mul(i, 32)), sload(add(baseSlot, i)))
                    }
                    return(0x00, add(64, mul(numSlots, 32)))
                }

                // Scratch Command (S0)
                if eq(cmdType, 0x5330) { // ASCII "S0"
                    if isBbs {
                        // Public BBS scratch not allowed for entire file, only private partition is cleanable
                        revert(0, 0)
                    }
                    
                    let dataLen := sload(lengthSlot)
                    if iszero(dataLen) {
                        dataLen := 256
                    }

                    let numSlots := div(add(dataLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        sstore(add(baseSlot, i), 0)
                    }
                    sstore(lengthSlot, 0)

                    // Remove from directory list
                    let numFilesSlot := 0
                    mstore(0x00, caller())
                    mstore(0x20, 0x1000)
                    numFilesSlot := keccak256(0x00, 64)
                    let numFiles := sload(numFilesSlot)
                    for { let i := 0 } lt(i, numFiles) { i := add(i, 1) } {
                        mstore(0x00, caller())
                        mstore(0x20, add(0x1001, i))
                        let fileSlot := keccak256(0x00, 64)
                        if eq(sload(fileSlot), nameWord) {
                            sstore(fileSlot, 0)
                            break
                        }
                    }

                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                revert(0, 0)
            }

            revert(0, 0)
        }
    }
}
