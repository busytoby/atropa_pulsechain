object "ConsensusPKI" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // Return runtime bytecode
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
            // METHOD 1: proposeInput11(bytes32 msgHash, bytes32[11] r, bytes32[11] s, uint8[11] v) -> uint256 passed
            // Selector: 0x9093ab4f
            // ----------------------------------------------------------------
            if eq(selector, 0x9093ab4f) {
                let msgHash := calldataload(4)
                let validCount := 0

                for { let i := 0 } lt(i, 11) { i := add(i, 1) } {
                    // Offsets for r, s, v arrays in calldata
                    // calldataload(4 + 32) -> r[0]
                    // calldataload(4 + 32 + 11*32) -> s[0]
                    // calldataload(4 + 32 + 22*32) -> v[0]
                    let rOffset := add(36, mul(i, 32))
                    let sOffset := add(388, mul(i, 32))
                    let vOffset := add(740, mul(i, 32))

                    let r := calldataload(rOffset)
                    let s := calldataload(sOffset)
                    let v := and(calldataload(vOffset), 0xFF)

                    // Call precompiled ecrecover (address 1)
                    // Input structure: [msgHash, v, r, s] (128 bytes)
                    mstore(0x100, msgHash)
                    mstore(0x120, v)
                    mstore(0x140, r)
                    mstore(0x160, s)

                    let success := call(gas(), 1, 0, 0x100, 128, 0x180, 32)
                    let recoveredSigner := 0
                    if success {
                        recoveredSigner := mload(0x180)
                    }

                    // Retrieve expected validator key from storage slot (10 + i)
                    let expectedKey := sload(add(10, i))

                    let isMatch := and(success, eq(recoveredSigner, expectedKey))
                    if isMatch {
                        validCount := add(validCount, 1)
                    }
                    if iszero(isMatch) {
                        // Slash validator stake in storage slot (30 + i)
                        let currentStake := sload(add(30, i))
                        let slashedStake := 0
                        if gt(currentStake, 4) {
                            slashedStake := sub(currentStake, 4) // Deduct 4 PLS stake
                        }
                        sstore(add(30, i), slashedStake)
                    }
                }

                // If all 11 validators passed, return 1, else 0
                let passed := eq(validCount, 11)
                mstore(0x00, passed)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: getValidatorAddress(uint256 index) -> address
            // Selector: 0x05f01202
            // ----------------------------------------------------------------
            if eq(selector, 0x05f01202) {
                let index := calldataload(4)
                mstore(0x00, sload(add(10, index)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: getValidatorStake(uint256 index) -> uint256
            // Selector: 0x3d02a013
            // ----------------------------------------------------------------
            if eq(selector, 0x3d02a013) {
                let index := calldataload(4)
                mstore(0x00, sload(add(30, index)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: setValidator(uint256 index, address addr, uint256 stake) -> void
            // Selector: 0xd88a05c3
            // ----------------------------------------------------------------
            if eq(selector, 0xd88a05c3) {
                let index := calldataload(4)
                let addr := calldataload(36)
                let stake := calldataload(68)
                sstore(add(10, index), addr)
                sstore(add(30, index), stake)
                return(0, 0)
            }

            revert(0, 0)
        }
    }
}
