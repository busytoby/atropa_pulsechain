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
            // State: Storage slots mapped by caller address store last block number.
            // Address mapping base = keccak256(caller, 0x99) to avoid slot collisions.
            mstore(0x00, caller())
            mstore(0x20, 0x99)
            let debounceSlot := keccak256(0x00, 0x40)
            let lastCallBlock := sload(debounceSlot)

            // Revert if call bounced in the same block
            if eq(lastCallBlock, number()) { revert(0, 0) }
            sstore(debounceSlot, number())

            // ----------------------------------------------------------------
            // METHOD 1: getJiffies() -> uint256
            // Selector: 0xb5123d47
            // ----------------------------------------------------------------
            if eq(selector, 0xb5123d47) {
                // EVM block height maps to Jiffies (1 block = 60 jiffies / 1 second)
                let elapsedJiffies := mul(number(), 60)
                mstore(0x00, elapsedJiffies)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: executeDiskCommand(bytes calldata cmd)
            // Selector: 0x9812a4df
            // ----------------------------------------------------------------
            if or(eq(selector, 0x9812a4df), eq(selector, 0xeb603a54)) {
                // Calldata layout:
                // offset 4: offset of dynamic bytes array (usually 32)
                // offset 36: length of command bytes array
                // offset 68: start of raw bytes
                let cmdLength := calldataload(36)
                if lt(cmdLength, 3) { revert(0, 0) }

                // Read command characters (first 2 bytes)
                // W0 = Write, R0 = Read, S0 = Scratch
                let cmdType := shr(240, calldataload(68))

                // Parse filename until null terminator '\x00'
                // We start reading characters from offset 70
                let nameHash := 0
                let fileOffset := 70
                let charCount := 0
                
                for {} lt(charCount, 32) { charCount := add(charCount, 1) } {
                    let char := byte(charCount, calldataload(fileOffset))
                    if iszero(char) { break } // null terminator reached
                    // simple hash calculation
                    nameHash := add(mul(nameHash, 31), char)
                }

                // Base slot location calculated from filename hash
                mstore(0x00, nameHash)
                let baseSlot := keccak256(0x00, 32)

                // Write Command (W0)
                if eq(cmdType, 0x5730) { // ASCII for "W0"
                    let dataStartOffset := add(fileOffset, charCount)
                    dataStartOffset := add(dataStartOffset, 1) // skip null terminator

                    // Data length in bytes: cmdLength - (dataStartOffset - 68)
                    let dataLen := sub(cmdLength, sub(dataStartOffset, 68))
                    
                    // Cap maximum data length at 131072 bytes (128KB)
                    if gt(dataLen, 131072) {
                        dataLen := 131072
                    }

                    // Save the file length in a namespaced slot
                    mstore(0x00, nameHash)
                    mstore(0x20, 0x999)
                    let lengthSlot := keccak256(0x00, 64)
                    sstore(lengthSlot, dataLen)

                    // Write data in 32-byte words
                    let numSlots := div(add(dataLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        let word := calldataload(add(dataStartOffset, mul(i, 32)))
                        sstore(add(baseSlot, i), word)
                    }

                    // Track directory:
                    // Read file name as a 32-byte word starting at offset 71 (removing leading ':')
                    let nameWord := calldataload(add(fileOffset, 1))
                    let numFiles := sload(0x1000)
                    let found := 0
                    for { let i := 0 } lt(i, numFiles) { i := add(i, 1) } {
                        if eq(sload(add(0x1001, i)), nameWord) {
                            found := 1
                            break
                        }
                    }
                    if iszero(found) {
                        sstore(add(0x1001, numFiles), nameWord)
                        sstore(0x1000, add(numFiles, 1))
                    }

                    // Emit Log Event: DiskWrite(address indexed writer, bytes32 indexed filename)
                    log3(0x00, 0, 0x18278cbdbe8ed62bf134676966ade77d7871afa34eabd366196c1e9547b08e63, caller(), nameWord)

                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                // Read Command (R0)
                if eq(cmdType, 0x5230) { // ASCII for "R0"
                    // Check if filename is "$"
                    // ASCII for "$" is 0x24 (36). Filename starting at offset 70 has ':' at index 0 and '$' at index 1.
                    let secondChar := byte(1, calldataload(fileOffset))
                    if and(eq(charCount, 2), eq(secondChar, 0x24)) {
                        // Directory listing requested: return list of filenames
                        let numFiles := sload(0x1000)
                        for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                            let fileNameVal := 0
                            if lt(i, numFiles) {
                                fileNameVal := sload(add(0x1001, i))
                            }
                            mstore(mul(i, 32), fileNameVal)
                        }
                        return(0x00, 256)
                    }

                    // Standard file read: return stored data from storage slots based on size
                    mstore(0x00, nameHash)
                    mstore(0x20, 0x999)
                    let lengthSlot := keccak256(0x00, 64)
                    let dataLen := sload(lengthSlot)
                    if iszero(dataLen) {
                        dataLen := 256
                    }

                    let numSlots := div(add(dataLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        mstore(mul(i, 32), sload(add(baseSlot, i)))
                    }
                    return(0x00, mul(numSlots, 32))
                }

                // Scratch Command (S0)
                if eq(cmdType, 0x5330) { // ASCII for "S0"
                    mstore(0x00, nameHash)
                    mstore(0x20, 0x999)
                    let lengthSlot := keccak256(0x00, 64)
                    let dataLen := sload(lengthSlot)
                    if iszero(dataLen) {
                        dataLen := 256
                    }

                    let numSlots := div(add(dataLen, 31), 32)
                    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
                        sstore(add(baseSlot, i), 0)
                    }
                    sstore(lengthSlot, 0)

                    // Remove from directory list:
                    let nameWord := calldataload(add(fileOffset, 1))
                    let numFiles := sload(0x1000)
                    for { let i := 0 } lt(i, numFiles) { i := add(i, 1) } {
                        if eq(sload(add(0x1001, i)), nameWord) {
                            sstore(add(0x1001, i), 0)
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
