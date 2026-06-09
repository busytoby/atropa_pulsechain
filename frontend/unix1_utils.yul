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
            
            default {
                revert(0, 0)
            }
            
            /*
             * Helpers
             */
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
