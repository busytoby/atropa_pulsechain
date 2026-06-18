object "ConsensusPKI" {
    code {
        // Constructor: Set owner to deployer (transaction origin)
        sstore(0, origin())
        
        // Copy runtime code to memory and return
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            
            let selector := shr(224, calldataload(0))
            
            switch selector
            
            // initializeValidatorKeys(address[11]) -> selector: 0xf7df1939
            case 0xf7df1939 {
                // Only contract owner can initialize keys
                let owner := sload(0)
                if iszero(eq(caller(), owner)) { revert(0, 0) }
                
                // Load 11 validator addresses from calldata and store in slots 10 to 20
                for { let i := 0 } lt(i, 11) { i := add(i, 1) } {
                    let addr := calldataload(add(4, mul(i, 32)))
                    sstore(add(10, i), addr)
                    // Initialize each validator's stake to 32.0 PLS in slots 30 to 40
                    sstore(add(30, i), 32)
                }
                return(0, 0)
            }
            
            // proposeInput11(bytes32,bytes[11]) -> selector: 0xd560a12c
            // MessageHash: The hash of the proposed emulator direction state.
            // Signatures: 11 concatenated raw signatures (65 bytes each).
            case 0xd560a12c {
                let msgHash := calldataload(4)
                let sigsOffset := calldataload(36) // Calldata offset to signatures array
                let absoluteSigsOffset := add(4, sigsOffset)
                
                let validCount := 0
                let ptr := mload(0x40) // Free memory pointer
                
                // Loop to verify all 11 validator signatures
                for { let i := 0 } lt(i, 11) { i := add(i, 1) } {
                    // Calculate pointer to current signature's raw data
                    // signature offset inside array = 32 + (i * 32)
                    let sigOffset := add(add(absoluteSigsOffset, 32), calldataload(add(absoluteSigsOffset, add(32, mul(i, 32)))))
                    
                    // Decode v, r, s
                    let r := calldataload(add(sigOffset, 32))
                    let s := calldataload(add(sigOffset, 64))
                    let v := byte(0, calldataload(add(sigOffset, 96)))
                    
                    // Staticcall ecrecover precompile (address 0x01)
                    mstore(ptr, msgHash)
                    mstore(add(ptr, 0x20), v)
                    mstore(add(ptr, 0x40), r)
                    mstore(add(ptr, 0x60), s)
                    
                    let success := staticcall(gas(), 1, ptr, 0x80, ptr, 0x20)
                    
                    let signer := 0
                    if success {
                        signer := mload(ptr)
                    }
                    
                    let expectedKey := sload(add(10, i))
                    
                    // Verify signer matches the expected PKI validator key
                    if and(success, eq(signer, expectedKey)) {
                        validCount := add(validCount, 1)
                    }
                    
                    // Byzantine control: Slash stake if key check fails
                    if or(iszero(success), iszero(eq(signer, expectedKey))) {
                        let currentStake := sload(add(30, i))
                        let newStake := 0
                        if gt(currentStake, 4) {
                            newStake := sub(currentStake, 4)
                        }
                        sstore(add(30, i), newStake)
                    }
                }
                
                // Return passed status (1 if all 11 signatures are cryptographically valid, otherwise 0)
                let passed := eq(validCount, 11)
                mstore(0, passed)
                mstore(32, validCount)
                return(0, 64)
            }
            
            // getValidatorStake(uint256) -> selector: 0xa86e81dc
            case 0xa86e81dc {
                let index := calldataload(4)
                if iszero(lt(index, 11)) { revert(0, 0) }
                let stake := sload(add(30, index))
                mstore(0, stake)
                return(0, 32)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
