/*
 * Unix 1 Kernel Core - EVM-Native Yul Implementation
 * 
 * Provides native implementations of core Unix v1 system calls:
 * - sys_open(nameHash, mode) -> fd
 * - sys_write(fd, memOffset, byteCount) -> writtenCount
 * - sys_read(fd, memOffset, byteCount) -> readCount
 * - sys_fork(parentPid) -> childPid
 * - sys_exec(pid, binaryHash) -> success
 * - sys_step(pid) -> executes one instruction step of the active binary
 * - sys_get_state() -> returns system tables (processes, active descriptors)
 */

object "Unix1Kernel" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            // Extract the 4-byte function selector from calldata
            switch shr(224, calldataload(0))
            
            // 0x15a0c0f4: sys_open(nameHash, mode) -> fd
            case 0x15a0c0f4 {
                let nameHash := calldataload(4)
                let mode := calldataload(36)
                
                // Locate or create file inode starting at slot offset 0x2000
                let inode := findOrCreateInode(nameHash)
                
                // Allocate File Descriptor (0 to 15)
                let fd := allocateFD(inode, mode)
                
                mstore(0x00, fd)
                return(0x00, 0x20)
            }
            
            // 0x2e90ef81: sys_write(fd, memOffset, byteCount) -> written
            case 0x2e90ef81 {
                let fd := calldataload(4)
                let memOffset := calldataload(36)
                let count := calldataload(68)
                
                let inode := getInode(fd)
                if iszero(inode) { revert(0, 0) }
                
                // Write memory bytes to storage slots linked to this inode
                for { let i := 0 } lt(i, count) { i := add(i, 32) } {
                    let val := mload(add(memOffset, i))
                    sstore(add(inode, div(i, 32)), val)
                }
                
                mstore(0x00, count)
                return(0x00, 0x20)
            }
            
            // 0xb826f0aa: sys_read(fd, memOffset, byteCount) -> readCount
            case 0xb826f0aa {
                let fd := calldataload(4)
                let memOffset := calldataload(36)
                let count := calldataload(68)
                
                let inode := getInode(fd)
                if iszero(inode) { revert(0, 0) }
                
                // Read from storage slots into memory
                for { let i := 0 } lt(i, count) { i := add(i, 32) } {
                    let val := sload(add(inode, div(i, 32)))
                    mstore(add(memOffset, i), val)
                }
                
                mstore(0x00, count)
                return(0x00, 0x20)
            }
            
            // 0x403d15b0: sys_fork(parentPid) -> childPid
            case 0x403d15b0 {
                let parentPid := calldataload(4)
                let childPid := incrementProcessCounter()
                
                // Copy process registers / state slots (e.g. at 0x100 + pid * 16)
                let parentBase := add(0x100, mul(parentPid, 16))
                let childBase := add(0x100, mul(childPid, 16))
                
                for { let r := 0 } lt(r, 16) { r := add(r, 1) } {
                    sstore(add(childBase, r), sload(add(parentBase, r)))
                }
                
                // Set child pid return code
                mstore(0x00, childPid)
                return(0x00, 0x20)
            }
            
            // 0x5f2f5361: sys_exec(pid, binaryHash) -> success
            case 0x5f2f5361 {
                let pid := calldataload(4)
                let binaryHash := calldataload(36)
                
                // Update IP / Active Binary registry for the target process
                let pStateSlot := add(0x100, mul(pid, 16))
                sstore(add(pStateSlot, 0), binaryHash) // Register 0 stores active binary hash
                sstore(add(pStateSlot, 1), 0)          // Register 1 resets program counter (IP)
                
                mstore(0x00, 1)
                return(0x00, 0x20)
            }

            // 0x00c71a91: sys_step(pid) -> success
            // Executes one instruction of the loaded binary using the Folklore register layout
            case 0x00c71a91 {
                let pid := calldataload(4)
                let pStateSlot := add(0x100, mul(pid, 16))
                let binaryHash := sload(add(pStateSlot, 0))
                let ip := sload(add(pStateSlot, 1))
                
                let inode := findOrCreateInode(binaryHash)
                let instruction := sload(add(inode, ip))
                
                // Decode Folklore CPU instruction [8-bit op][8-bit dst][8-bit src1][8-bit val]
                let op := byte(0, instruction)
                let rDst := byte(1, instruction)
                let rSrc1 := byte(2, instruction)
                let val := byte(3, instruction)
                
                switch op
                // Op 1: LOAD Immediate
                case 1 {
                    sstore(add(pStateSlot, rDst), val)
                    ip := add(ip, 1)
                }
                // Op 2: ADD Registers
                case 2 {
                    let v1 := sload(add(pStateSlot, rSrc1))
                    let v2 := sload(add(pStateSlot, val))
                    sstore(add(pStateSlot, rDst), add(v1, v2))
                    ip := add(ip, 1)
                }
                // Op 4: JUMP if Zero
                case 4 {
                    let cond := sload(add(pStateSlot, rSrc1))
                    switch iszero(cond)
                    case 1 { ip := val }
                    default { ip := add(ip, 1) }
                }
                
                sstore(add(pStateSlot, 1), ip)
                mstore(0x00, 1)
                return(0x00, 0x20)
            }
            
            // 0x2e0f40cf: sys_get_state() -> returns system tables
            case 0x2e0f40cf {
                // Return active process counter & file system registration data
                mstore(0x00, sload(0x10)) // Total active processes
                mstore(0x20, sload(0x20)) // File system inode counter
                return(0x00, 0x40)
            }
            
            default {
                revert(0, 0)
            }
            
            /*
             * Helpers
             */
            function modExp(b, e, m) -> r {
                r := 1
                b := mod(b, m)
                for { } gt(e, 0) { } {
                    if and(e, 1) {
                        r := mulmod(r, b, m)
                    }
                    e := shr(1, e)
                    b := mulmod(b, b, m)
                }
            }
            
            function findOrCreateInode(nameHash) -> inode {
                // Search file table starting at 0x2000
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
                    // Create new inode entry
                    inode := add(0x2000, mul(count, 100))
                    sstore(inode, nameHash)
                    sstore(0x20, add(count, 1))
                }
            }
            
            function allocateFD(inode, mode) -> fd {
                // Search file descriptor table (slots 0x40 to 0x4F)
                for { let i := 0 } lt(i, 16) { i := add(i, 1) } {
                    let slot := add(0x40, i)
                    if iszero(sload(slot)) {
                        sstore(slot, inode)
                        fd := i
                        break
                    }
                }
            }
            
            function getInode(fd) -> inode {
                inode := sload(add(0x40, fd))
            }
            
            function incrementProcessCounter() -> nextPid {
                let current := sload(0x10)
                nextPid := add(current, 1)
                sstore(0x10, nextPid)
            }
        }
    }
}
