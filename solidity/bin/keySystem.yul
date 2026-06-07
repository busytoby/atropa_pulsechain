object "KeySystem" {
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

            // Helper to get Namespaced Storage Slot for a user
            function getUserSlot(user, index) -> slot {
                mstore(0x00, user)
                mstore(0x20, index)
                slot := keccak256(0x00, 64)
            }

            // ----------------------------------------------------------------
            // METHOD 1: setPublicKey160(address user, address pubAddr)
            // Selector: 0x9357a57f
            // ----------------------------------------------------------------
            if eq(selector, 0x9357a57f) {
                let user := calldataload(4)
                let pubAddr := calldataload(36)
                sstore(getUserSlot(user, 0), pubAddr)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: getPublicKey160(address user) -> address
            // Selector: 0xa925d170
            // ----------------------------------------------------------------
            if eq(selector, 0xa925d170) {
                let user := calldataload(4)
                mstore(0x00, sload(getUserSlot(user, 0)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: setKey256(address user, bytes32 key)
            // Selector: 0x9ac3ed6f
            // ----------------------------------------------------------------
            if eq(selector, 0x9ac3ed6f) {
                let user := calldataload(4)
                let key := calldataload(36)
                sstore(getUserSlot(user, 1), key)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: getKey256(address user) -> bytes32
            // Selector: 0xa5f5ddb3
            // ----------------------------------------------------------------
            if eq(selector, 0xa5f5ddb3) {
                let user := calldataload(4)
                mstore(0x00, sload(getUserSlot(user, 1)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: hashKey(address user) -> bytes32
            // Selector: 0x93b8c92b
            // ----------------------------------------------------------------
            if eq(selector, 0x93b8c92b) {
                let user := calldataload(4)
                let key := sload(getUserSlot(user, 1))
                mstore(0x00, key)
                mstore(0x20, sload(getUserSlot(user, 0))) // Mix in 160-bit address
                let hashed := keccak256(0x00, 64)
                mstore(0x00, hashed)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
