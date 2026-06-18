/*
 * Unix 1 Core Utilities - EVM-Native Yul Implementation
 * 
 * Implements the user-space utility behavior directly in Yul:
 * - ls() -> returns list of all file name hashes
 * - cat(nameHash) -> returns content bytes of the target file
 * - write_file(nameHash, word0, word1) -> writes content bytes
 * - rm(nameHash) -> deletes the target file inode entry
 */

object "Unix1Utils" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            switch shr(224, calldataload(0))
            
            // 0x8ab095c3: ls() -> returns list of nameHashes
            case 0x8ab095c3 {
                let count := sload(0x20)
                mstore(0x00, count)
                
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let base := add(0x2000, mul(i, 100))
                    let nameHash := sload(base)
                    mstore(add(0x20, mul(i, 32)), nameHash)
                }
                
                return(0x00, add(0x20, mul(count, 32)))
            }
            
            // 0x24249a2a: cat(nameHash) -> returns raw bytes
            case 0x24249a2a {
                let nameHash := calldataload(4)
                let inode := findInode(nameHash)
                if iszero(inode) { revert(0, 0) }
                
                let word0 := sload(add(inode, 1))
                let word1 := sload(add(inode, 2))
                mstore(0x00, word0)
                mstore(0x20, word1)
                
                return(0x00, 0x40)
            }
            
            // 0xb8b6a3ef: write_file(nameHash, word0, word1) -> success
            case 0xb8b6a3ef {
                let nameHash := calldataload(4)
                let word0 := calldataload(36)
                let word1 := calldataload(68)
                
                let inode := findOrCreateInode(nameHash)
                sstore(add(inode, 1), word0)
                sstore(add(inode, 2), word1)
                
                mstore(0x00, 1)
                return(0x00, 0x20)
            }
            
            // 0x0f2fa44a: rm(nameHash) -> success
            case 0x0f2fa44a {
                let nameHash := calldataload(4)
                let count := sload(0x20)
                let found := 0
                
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let base := add(0x2000, mul(i, 100))
                    if eq(sload(base), nameHash) {
                        sstore(base, 0)
                        sstore(add(base, 1), 0)
                        sstore(add(base, 2), 0)
                        found := 1
                        break
                    }
                }
                
                mstore(0x00, found)
                return(0x00, 0x20)
            }

            // 0xc3a012ff: ipcs() -> returns active msg count for LUNs 0-7
            case 0xc3a012ff {
                let ptr := mload(0x40)
                for { let lun := 0 } lt(lun, 8) { lun := add(lun, 1) } {
                    // Load queue head (0x2000 + lun) and tail (0x2050 + lun) from controller state
                    let head := sload(add(0x2000, lun))
                    let tail := sload(add(0x2050, lun))
                    let pending := 0
                    if lt(head, tail) { pending := sub(tail, head) }
                    mstore(add(ptr, mul(lun, 32)), pending)
                }
                return(ptr, 256)
            }

            // 0x5b8a0fc1: mqsend(lun, priority, word0, word1) -> success
            case 0x5b8a0fc1 {
                let lun := calldataload(4)
                let priority := calldataload(36)
                let w0 := calldataload(68)
                let w1 := calldataload(100)

                let tail := sload(add(0x2050, lun))
                let cacheKey := keccak256(add(0x1000, tail), 32)
                
                // Pack Priority byte (Byte 6) into word0
                let headerWord := or(w0, shl(200, and(priority, 0xFF)))
                sstore(cacheKey, headerWord)
                sstore(add(cacheKey, 1), w1)

                // Increment tail index
                sstore(add(0x2050, lun), add(tail, 1))
                
                mstore(0x00, 1)
                return(0x00, 0x20)
            }

            // 0x4d710ab0: mqrecv(lun) -> returns message payload (word0, word1) and commits ACK
            case 0x4d710ab0 {
                let lun := calldataload(4)
                let head := sload(add(0x2000, lun))
                let tail := sload(add(0x2050, lun))

                if iszero(lt(head, tail)) {
                    // Empty queue status
                    mstore(0x00, 0)
                    mstore(0x20, 0)
                    return(0x00, 0x40)
                }

                let cacheKey := keccak256(add(0x1000, head), 32)
                let w0 := sload(cacheKey)
                let w1 := sload(add(cacheKey, 1))

                // Perform 2-phase commit automatically for shell utility: Increment head
                sstore(add(0x2000, lun), add(head, 1))

                mstore(0x00, w0)
                mstore(0x20, w1)
                return(0x00, 0x40)
            }

            // 0x494e4954: init_complete() -> broadcasts INIT_COMPLETE event to LUN 5 topic 0x494e4954
            case 0x494e4954 {
                // Fetch LUN 5 tail
                let tail := sload(0x2055) // LUN 5 tail index slot (0x2050 + 5)
                let cacheKey := keccak256(add(0x1000, tail), 32)
                
                // Format block: Magic 'INIT' (0x494e4954) topic header
                sstore(cacheKey, 0x494e495400000000000000000000000000000000000000000000000000000000)
                sstore(add(cacheKey, 1), 0) // Padding

                // Call internal fan-out manually (normally triggered by WinchesterMQ write block)
                performTopicFanOutForUtils(0x494e4954, tail)

                mstore(0x00, 1)
                return(0x00, 0x20)
            }

            // 0x72656769: register_init_listener(lun) -> subscribes target LUN to INIT_COMPLETE topic
            case 0x72656769 {
                let lun := calldataload(4)
                let topicId := 0x494e4954 // Topic: 'INIT'

                let subscriberCountKey := keccak256(add(0x7000, topicId), 32)
                let subCount := sload(subscriberCountKey)
                
                let targetKey := keccak256(add(add(0x7100, topicId), subCount), 32)
                sstore(targetKey, lun)
                sstore(subscriberCountKey, add(subCount, 1))

                mstore(0x00, 1)
                return(0x00, 0x20)
            }

            // 0x70730000: ps() -> returns PCB status array (IP, SP, status, idle) for all 6 process cards
            case 0x70730000 {
                let ptr := mload(0x40)
                
                for { let card := 0 } lt(card, 6) { card := add(card, 1) } {
                    // Calculate PCB base offset matching WMQ_Scheduler layout: 0x6000 + card * 16
                    let pcbBase := add(0x6000, mul(card, 16))
                    
                    let ip := sload(add(pcbBase, 0))
                    let sp := sload(add(pcbBase, 1))
                    let status := sload(add(pcbBase, 6))
                    let idle := sload(add(pcbBase, 7))

                    // Pack into 32-byte slots in return payload memory
                    let cardOffset := mul(card, 128) // Allocates 4 words per card
                    mstore(add(ptr, cardOffset), ip)
                    mstore(add(ptr, add(cardOffset, 32)), sp)
                    mstore(add(ptr, add(cardOffset, 64)), status)
                    mstore(add(ptr, add(cardOffset, 96)), idle)
                }
                
                return(ptr, mul(6, 128))
            }
            
            default {
                revert(0, 0)
            }
            
            /*
             * Helpers
             */
            function performTopicFanOutForUtils(topicId, tail) {
                let cacheKey := keccak256(add(0x1000, tail), 32)
                let w0 := sload(cacheKey)
                let w1 := sload(add(cacheKey, 1))

                let subscriberCountKey := keccak256(add(0x7000, topicId), 32)
                let count := sload(subscriberCountKey)
                if iszero(count) { leave }

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let targetKey := keccak256(add(add(0x7100, topicId), i), 32)
                    let destLun := sload(targetKey)

                    // Write message block to subscriber's inbox queue tail
                    let destTail := sload(add(0x2050, destLun))
                    let destKey := keccak256(add(0x1000, destTail), 32)
                    
                    sstore(destKey, w0)
                    sstore(add(destKey, 1), w1)

                    // Increment the subscriber's tail index
                    sstore(add(0x2050, destLun), add(destTail, 1))
                }

                // Clean up transient LUN 5 broker block cache
                sstore(cacheKey, 0)
                sstore(add(cacheKey, 1), 0)
            }

            function findInode(nameHash) -> inode {
                let count := sload(0x20)
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let base := add(0x2000, mul(i, 100))
                    if eq(sload(base), nameHash) {
                        inode := base
                        break
                    }
                }
            }
            
            function findOrCreateInode(nameHash) -> inode {
                let count := sload(0x20)
                let found := 0
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let base := add(0x2000, mul(i, 100))
                    if eq(sload(base), nameHash) {
                        inode := base
                        found := 1
                        break
                    }
                }
                
                if iszero(found) {
                    inode := add(0x2000, mul(count, 100))
                    sstore(inode, nameHash)
                    sstore(0x20, add(count, 1))
                }
            }
        }
    }
}
