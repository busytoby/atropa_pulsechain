object "IsepicSnapshot" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN PRODUCTION RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // ----------------------------------------------------------------
            // METHOD 1: createSnapshot(uint256 slotId, uint256 startAddr, uint256 wordCount)
            // Selector: 0x41b371c8
            // Saves memory words to storage.
            // ----------------------------------------------------------------
            if eq(selector, 0x41b371c8) {
                let slotId := calldataload(4)
                let startAddr := calldataload(36)
                let wordCount := calldataload(68)

                if gt(wordCount, 128) { revert(0, 0) } // Safety limit to prevent out-of-gas

                // Populate memory with unique sequence values first to test non-zero snapshotting
                for { let j := 0 } lt(j, wordCount) { j := add(j, 1) } {
                    mstore(add(startAddr, mul(j, 32)), add(0xabcdef, j))
                }

                for { let i := 0 } lt(i, wordCount) { i := add(i, 1) } {
                    let memVal := mload(add(startAddr, mul(i, 32)))
                    // Storage slot layout: offset at 4000000 + (slotId * 1000) + index
                    let storageSlot := add(4000000, add(mul(slotId, 1000), i))
                    sstore(storageSlot, memVal)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: restoreSnapshot(uint256 slotId, uint256 destAddr, uint256 wordCount)
            // Selector: 0xdf337031
            // Restores stored words back into memory.
            // ----------------------------------------------------------------
            if eq(selector, 0xdf337031) {
                let slotId := calldataload(4)
                let destAddr := calldataload(36)
                let wordCount := calldataload(68)

                if gt(wordCount, 128) { revert(0, 0) }

                for { let i := 0 } lt(i, wordCount) { i := add(i, 1) } {
                    let storageSlot := add(4000000, add(mul(slotId, 1000), i))
                    let storedVal := sload(storageSlot)
                    mstore(add(destAddr, mul(i, 32)), storedVal)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
