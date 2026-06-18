object "ArenaProcessor" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Set owner to deployer
        sstore(0, caller())

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
            // METHOD 1: setTargetQingViaPKI(address pkiContract, uint256 qingId, bytes32 msgHash, bytes sigs) -> void
            // Selector: 0x5e2cf9f1
            // ----------------------------------------------------------------
            if eq(selector, 0x5e2cf9f1) {
                let pkiContract := calldataload(4)
                let qingId := calldataload(36)
                let msgHash := calldataload(68)
                
                // Call ConsensusPKI.proposeInput11(msgHash, sigs)
                let ptr := mload(0x40)
                mstore(ptr, 0xd560a12c00000000000000000000000000000000000000000000000000000000) // proposeInput11 selector
                mstore(add(ptr, 4), msgHash)
                mstore(add(ptr, 36), 64) // Signature array offset (relative to calldata start in subcall)
                
                // Copy signatures from current calldata to subcall payload
                // Signatures array size is 11 * 32 bytes of offsets, then signature data (concatenated v,r,s)
                let sigsSize := sub(calldatasize(), 100)
                calldatacopy(add(ptr, 68), 100, sigsSize)
                
                let totalSize := add(68, sigsSize)
                let success := call(gas(), pkiContract, 0, ptr, totalSize, ptr, 64)
                if iszero(success) { revert(0, 0) }
                
                let passed := mload(ptr)
                if iszero(passed) { revert(0, 0) } // Reject if PKI consensus fails
                
                // Update target Qing and reset round states
                sstore(0x100, qingId)            // Target Qing ID
                sstore(0x101, 0)                 // Processed Cursor = 0
                sstore(0x102, 0xFFFFFFFF)        // Leader Card ID = None
                sstore(0x103, 0)                 // Max Bar Width = 0
                sstore(0x104, 0)                 // Round Completed = 0
                
                return(0, 0)
            }

             // ----------------------------------------------------------------
             // METHOD 2: registerPlayerYue(uint256 yueCardId) -> void
             // Selector: 0xd8fce734
             // ----------------------------------------------------------------
             if eq(selector, 0xd8fce734) {
                let yueCardId := calldataload(4)
                let totalPlayers := sload(0x200)
                
                // Append player Card ID to registry
                sstore(add(0x210, totalPlayers), yueCardId)
                sstore(0x200, add(totalPlayers, 1))
                
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 3: processBatch(uint256 batchSize) -> (uint256 processed, uint256 cursor, uint256 completed)
             // Selector: 0x6309463b
             // ----------------------------------------------------------------
             if eq(selector, 0x6309463b) {
                let batchSize := calldataload(4)
                let targetQing := sload(0x100)
                let roundCompleted := sload(0x104)
                
                if roundCompleted {
                    // Round already complete, return status
                    let ptr := mload(0x40)
                    mstore(ptr, 0)
                    mstore(add(ptr, 32), sload(0x101))
                    mstore(add(ptr, 64), 1)
                    return(ptr, 96)
                }
                
                let totalPlayers := sload(0x200)
                let cursor := sload(0x101)
                let maxWidth := sload(0x103)
                let currentLeader := sload(0x102)
                
                let nextBoundary := add(cursor, batchSize)
                if gt(nextBoundary, totalPlayers) {
                    nextBoundary := totalPlayers
                }
                
                // Target Page Index for Qings (let's assume Qings map to Page 0x70)
                let pageIdx := 0x70
                
                for { let i := cursor } lt(i, nextBoundary) { i := add(i, 1) } {
                    let yueCardId := sload(add(0x210, i))
                    
                    // Fetch the 2-bar parameters from the Card's memory slot space
                    // Slot calculation matching WMQ_Scheduler: destOffset + pageIdx * 256
                    let destOffset := add(0x8000, mul(yueCardId, 0x1000))
                    let pageOffset := add(destOffset, mul(pageIdx, 256))
                    
                    let u1 := sload(pageOffset)
                    let u2 := sload(add(pageOffset, 32))
                    
                    let width := 0
                    if gt(u2, u1) {
                        width := sub(u2, u1)
                    }
                    
                    // If this Yue has higher bars, update the leading status
                    if gt(width, maxWidth) {
                        maxWidth := width
                        currentLeader := yueCardId
                    }
                }
                
                // Update persistent batch tracking states
                sstore(0x101, nextBoundary)
                sstore(0x102, currentLeader)
                sstore(0x103, maxWidth)
                
                let completed := 0
                if eq(nextBoundary, totalPlayers) {
                    completed := 1
                    sstore(0x104, 1) // Set round completed
                    
                    // Write final winning state directly to Shared Arena State Slots
                    // Slot 0x300 holds winner Card ID, Slot 0x301 holds winning bar width
                    sstore(0x300, currentLeader)
                    sstore(0x301, maxWidth)
                }
                
                let ptr := mload(0x40)
                mstore(ptr, sub(nextBoundary, cursor)) // processed in this batch
                mstore(add(ptr, 32), nextBoundary)      // current cursor position
                mstore(add(ptr, 64), completed)         // completed flag
                return(ptr, 96)
            }

            revert(0, 0)
            
            function getMaxSlots() -> maxSlots {
                maxSlots := sload(0x11)
            }
            
            function getPcbOffset(cardId) -> offset {
                offset := add(0x6000, mul(cardId, 16))
            }
        }
    }
}
