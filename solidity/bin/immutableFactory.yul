object "VerboseImmutableFactory" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // 1. Copy the core runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // 4. Return the combined runtime bytecode + your custom master key
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
            // PUBLIC MAIN ENTRYPOINTS & LIFECYCLE MANAGEMENT
            // ================================================================
            
            // ----------------------------------------------------------------
            // METHOD 1: resolve(bytes32 salt, bytes32 bytecodeHash) -> address
            // Selector: 0x05de9943 
            // ----------------------------------------------------------------
            if eq(selector, 0x05de9943) {
                let argSalt := calldataload(4)
                let argHash := calldataload(36)
                
                packResolveBuffer(argSalt, argHash)
                
                mstore(0x00, and(keccak256(0x00, 0x55), 0xffffffffffffffffffffffffffffffffffffffff))
                return(12, 20)
            }

            // ----------------------------------------------------------------
            // METHOD 2: createAuthorized(bytes32 salt, bytes32 r, bytes32 s, uint8 v)
            // Selector: 0xa9f4305f 
            // ----------------------------------------------------------------
            if eq(selector, 0xb5ba0c68) {
                // Determine the size of the dynamic trailing initialization bytecode.
                // Subtract 164 bytes (4-byte selector + 5 static 32-byte parameters).
                let bytecodeSize := sub(calldatasize(), 164)
                if iszero(bytecodeSize) { revert(0, 0) }
    
                // Copy the raw deployment bytecode payload into memory slot 0x00 to extract its hash.
                // Notice the source pointer shifts to 164.
                calldatacopy(0, 164, bytecodeSize)
                let bytecodeHash := keccak256(0, bytecodeSize)

                // Unpack variables directly from calldata layout matrices
                let mainSalt := calldataload(4)
                let sigR := calldataload(36)
                let sigS := calldataload(68)
                let sigV := shr(248, calldataload(100))
                let providedMasterKey := shr(96, calldataload(132)) // Clean 20-byte address format

                // --- PHASE 1: verifySignature (Macro-Packed & Called with Dynamic User Parameter) ---
                // Target Method ID: 0x489069d1 (verifySignature(bytes32,bytes32,bytes32,uint8,address))
                // The factory code-reading copy loop is gone; providedMasterKey is passed directly.
                pop(pack128AndCall(0x489069d1, bytecodeHash, sigR, sigS, shl(248, sigV), shl(96, providedMasterKey), 0xa4, 0))

                // --- PHASE 2: resolve ---
                let targetAddress := pack32AndCall(0x05de9943, mainSalt, bytecodeHash, 0, 0x44, 0)

                // --- PHASE 3: verifyIdempotency ---
                let idemResult := pack32AndCall(0x4b9e903d, targetAddress, mainSalt, 0, 0x44, 1)
                if iszero(eq(idemResult, 1)) {
                    mstore(0x00, shr(96, targetAddress))
                    return(12, 20)
                }

                // --- PHASE 4: executeDeployment ---
                pack32(0x93d9b8f0, 0x40, mainSalt, bytecodeSize)
                // Stream dynamic initialization code into execution memory from its new 164 offset
                calldatacopy(0x64, 164, bytecodeSize)
                mstore(0x00, executeSelfCall(add(0x64, bytecodeSize), 1))
                return(12, 20)
            }

            // ================================================================
            // PUBLIC SUB-ROUTINE LIFECYCLE INTERFACES (ENCAPSULATION LAYERS)
            // ================================================================

            // ----------------------------------------------------------------
            // METHOD 3: verifySignature(bytes32 targetHash, bytes32 r, bytes32 s, uint8 v, address masterKey)
            // Selector: 0x489069d1
            // ----------------------------------------------------------------
            if eq(selector, 0x489069d1) {
                let inputHash := calldataload(4)
                let subR := calldataload(36)
                let subS := calldataload(68)
                let subV := shr(248, calldataload(100))
                let inputKey := shr(96, calldataload(132))
                
                pack96(0, inputHash, subV, subR, subS)
                
                if iszero(staticcall(gas(), 0x01, 0, 128, 0, 32)) { revert(0, 0) }
                if iszero(eq(mload(0), inputKey)) { revert(0, 0) }
                
                mstore(0x00, 1)
                return(0, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: verifyIdempotency(address targetAddress, bytes32 salt)
            // Selector: 0x4b9e903d
            // ----------------------------------------------------------------
            if eq(selector, 0x4b9e903d) {
                let targetAddress := shr(96, calldataload(4))
                let salt := calldataload(36)
                
                if gt(extcodesize(targetAddress), 0) {
                    let idemTopic := 0xb4e1cb2a00000000000000000000000000000000000000000000000000000000
                    mstore(0, targetAddress)
                    log3(0, 32, idemTopic, targetAddress, salt)
                    return(0, 32)
                }
                mstore(0x00, 1)
                return(0, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: executeDeployment(bytes bytecode, bytes32 salt) -> address
            // Selector: 0x93d9b8f0
            // ----------------------------------------------------------------
            if eq(selector, 0x93d9b8f0) {
                let offset := calldataload(4)
                let salt := calldataload(36)
                let size := calldataload(add(4, offset))
                
                calldatacopy(0, add(36, offset), size)
                
                let deployed := create2(0, 0, size, salt)
                if iszero(deployed) { revert(0, 0) }
                
                let successTopic := 0x1d36d75c00000000000000000000000000000000000000000000000000000000
                mstore(0, deployed)
                log3(0, 32, successTopic, deployed, salt)
                return(0, 32)
            }

            revert(0, 0)

            // ================================================================
            // HIGH-DENSITY PACK-AND-CALL MACROS (STRUCTURAL UTILITIES)
            // ================================================================
            
            function pack32AndCall(sel, a1, a2, a3, calldataSize, isWrite) -> res {
                pack32(sel, a1, a2, a3)
                res := executeSelfCall(calldataSize, isWrite)
            }

            function pack128AndCall(sel, a1, a2, a3, a4, a5, calldataSize, isWrite) -> res {
                mstore(0x00, shl(224, sel))
                mstore(0x04, a1)
                mstore(0x24, a2)
                mstore(0x44, a3)
                mstore(0x64, a4)
                mstore(0x84, a5)
                res := executeSelfCall(calldataSize, isWrite)
            }

            function packResolveBuffer(_salt, _hash) {
                mstore(0x00, or(0xff00000000000000000000000000000000000000000000000000000000000000, shl(88, address())))
                mstore(0x15, _salt)  
                mstore(0x35, _hash)  
            }

            function pack32(sel, a1, a2, a3) {
                mstore(0x00, shl(224, sel))
                mstore(0x04, a1)
                if n_z(a2) { mstore(0x24, a2) }
                if n_z(a3) { mstore(0x44, a3) }
            }

            function pack96(sel, a1, a2, a3, a4) {
                if n_z(sel) { mstore(0x00, shl(224, sel)) } 
                mstore(if_z_add(sel, 0x00, 0x04), a1)
                mstore(if_z_add(sel, 0x20, 0x24), a2)
                mstore(if_z_add(sel, 0x40, 0x44), a3)
                mstore(if_z_add(sel, 0x60, 0x64), a4)
            }

            function n_z(val) -> check { check := iszero(iszero(val)) }
            
            function if_z_add(cond, targetZ, targetNZ) -> dest {
                dest := targetNZ
                if iszero(cond) { dest := targetZ }
            }

            function executeSelfCall(calldataSize, isWrite) -> response {
                let success := 0
                switch isWrite
                case 1 { success := call(gas(), address(), 0, 0x00, calldataSize, 0x00, 0x20) }
                default { success := staticcall(gas(), address(), 0x00, calldataSize, 0x00, 0x20) }
                if iszero(success) {
                    returndatacopy(0x00, 0x00, returndatasize())
                    revert(0x00, returndatasize())
                }
                response := mload(0x00)
            }
        }
    }
}