object "Genesis" {
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

            // ----------------------------------------------------------------
            // METHOD 1: getComplexity() -> uint256
            // Selector: 0x3df0a09e
            // ----------------------------------------------------------------
            if eq(selector, 0x3df0a09e) {
                // Returns the complexity metadata of the Genesis launch
                mstore(0x00, 8) // Default complexity mapping
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: getLaunchTimestamp() -> uint256
            // Selector: 0xa92350ef
            // ----------------------------------------------------------------
            if eq(selector, 0xa92350ef) {
                mstore(0x00, timestamp())
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
