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
                
                // Track the Auncient owner of the card in storage slot 0x7000 + cardId if not yet set
                let cardOwner := sload(add(0x7000, yueCardId))
                if iszero(cardOwner) {
                    cardOwner := caller()
                    sstore(add(0x7000, yueCardId), cardOwner)
                }
                
                // Verify caller is the Auncient owner or an approved accessor
                mstore(0x00, yueCardId)
                mstore(0x20, caller())
                let isApproved := sload(keccak256(0x00, 0x40))
                
                if iszero(isApproved) {
                    if xor(caller(), cardOwner) { revert(0, 0) }
                }
                
                let totalPlayers := sload(0x200)
                
                // Append player Card ID to registry
                sstore(add(0x210, totalPlayers), yueCardId)
                sstore(0x200, add(totalPlayers, 1))
                
                return(0, 0)
            }

             // ----------------------------------------------------------------
             // METHOD 4: approveAccessorForCard(uint256 cardId, address accessor, uint256 approved) -> void
             // Selector: 0x19a84a60 (typo) / 0x21e6303f (standard)
             // ----------------------------------------------------------------
             if or(eq(selector, 0x19a84a60), eq(selector, 0x21e6303f)) {
                let cardId := calldataload(4)
                let accessor := calldataload(36)
                let approved := calldataload(68)
                
                // Only the registered Auncient owner of the Card/Yue can approve accessors
                let cardOwner := sload(add(0x7000, cardId))
                if xor(caller(), cardOwner) { revert(0, 0) }
                
                mstore(0x00, cardId)
                mstore(0x20, accessor)
                sstore(keccak256(0x00, 0x40), approved)
                return(0, 0)
             }

             // ----------------------------------------------------------------
             // METHOD 5: systemEquipQing(uint256 cardId, uint256 pageIdx, uint256 u1, uint256 u2) -> void
             // Selector: 0xb8e3a241 (typo) / 0xdcde94f8 (standard)
             // ----------------------------------------------------------------
             if or(eq(selector, 0xb8e3a241), eq(selector, 0xdcde94f8)) {
                let cardId := calldataload(4)
                
                // Verify caller has been approved for this specific Yue/Card by the Auncient owner
                mstore(0x00, cardId)
                mstore(0x20, caller())
                let isApproved := sload(keccak256(0x00, 0x40))
                
                if iszero(isApproved) {
                    // Fallback: Auncient owner of the card is always authorized
                    let cardOwner := sload(add(0x7000, cardId))
                    if xor(caller(), cardOwner) { revert(0, 0) }
                }
                
                let pageIdx := calldataload(36)
                let u1 := calldataload(68)
                let u2 := calldataload(100)
                
                // Write the 2-bar values directly into the Card's page memory slots
                let destOffset := add(0x8000, mul(cardId, 0x1000))
                let pageOffset := add(destOffset, mul(pageIdx, 256))
                
                sstore(pageOffset, u1)
                sstore(add(pageOffset, 32), u2)
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

             // ----------------------------------------------------------------
             // METHOD 7: tag(address qingA, address qingB) -> void
             // Selector: 0xb901e242
             // ----------------------------------------------------------------
             if eq(selector, 0xb901e242) {
                 let qingA := calldataload(4)
                 let qingB := calldataload(36)
                 
                 // Link key check: check if already linked to prevent duplicate entries
                 mstore(0x00, qingA)
                 mstore(0x20, qingB)
                 let linkSlot := keccak256(0x00, 0x40)
                 let exists := sload(linkSlot)
                 
                 if iszero(exists) {
                     // Get current linked count for qingA
                     mstore(0x00, qingA)
                     mstore(0x20, 0)
                     let countSlot := keccak256(0x00, 0x40)
                     let count := sload(countSlot)
                     
                     // Store link status
                     sstore(linkSlot, 1)
                     
                     // Store linked QING address in list
                     mstore(0x00, qingA)
                     mstore(0x20, add(count, 1))
                     let indexSlot := keccak256(0x00, 0x40)
                     sstore(indexSlot, qingB)
                     
                     // Update total count
                     sstore(countSlot, add(count, 1))
                 }
                 return(0, 0)
             }

             // ----------------------------------------------------------------
             // METHOD 8: tags(address qing) -> address[]
             // Selector: 0x58b8f842
             // ----------------------------------------------------------------
             if eq(selector, 0x58b8f842) {
                 let qing := calldataload(4)
                 
                 // Load count
                 mstore(0x00, qing)
                 mstore(0x20, 0)
                 let countSlot := keccak256(0x00, 0x40)
                 let count := sload(countSlot)
                 
                 let ptr := mload(0x40)
                 let addrPtr := add(ptr, 64) // Leave space for array offset and length
                 let weightPtr := add(addrPtr, mul(count, 32))
                 
                 // Load linked QINGs and calculate their weights
                 let pageIdx := 0x70
                 for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                     mstore(0x00, qing)
                     mstore(0x20, add(i, 1))
                     let indexSlot := keccak256(0x00, 0x40)
                     let linkedQing := sload(indexSlot)
                     mstore(add(addrPtr, mul(i, 32)), linkedQing)
                     
                     // Calculate weight: u2 - u1 from Page 0x70
                     let destOffset := add(0x8000, mul(linkedQing, 0x1000))
                     let pageOffset := add(destOffset, mul(pageIdx, 256))
                     let u1 := sload(pageOffset)
                     let u2 := sload(add(pageOffset, 32))
                     let width := 0
                     if gt(u2, u1) {
                         width := sub(u2, u1)
                     }
                     mstore(add(weightPtr, mul(i, 32)), width)
                 }
                 
                 // Selection sort descending
                 for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                     let maxIdx := i
                     let maxWeight := mload(add(weightPtr, mul(maxIdx, 32)))
                     for { let j := add(i, 1) } lt(j, count) { j := add(j, 1) } {
                         let w := mload(add(weightPtr, mul(j, 32)))
                         if gt(w, maxWeight) {
                             maxIdx := j
                             maxWeight := w
                         }
                     }
                     if iszero(eq(maxIdx, i)) {
                         // Swap weights
                         let tempW := mload(add(weightPtr, mul(i, 32)))
                         mstore(add(weightPtr, mul(i, 32)), maxWeight)
                         mstore(add(weightPtr, mul(maxIdx, 32)), tempW)
                         // Swap addresses
                         let tempAddr := mload(add(addrPtr, mul(i, 32)))
                         mstore(add(addrPtr, mul(i, 32)), mload(add(addrPtr, mul(maxIdx, 32))))
                         mstore(add(addrPtr, mul(maxIdx, 32)), tempAddr)
                     }
                 }
                 
                 // Encode ABI response
                 mstore(ptr, 32)        // Array offset
                 mstore(add(ptr, 32), count) // Array length
                 
                 let totalReturnSize := add(64, mul(count, 32))
                 return(ptr, totalReturnSize)
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
