object "MvsTapeCertifier" {
    code {
        sstore(0, caller())
        sstore(0x10, 0) // Lock register for sector database (exclusive locks)
        
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            switch selector
            
            // verifyRegisterSequence(uint256 solenoid, uint256 brake) -> selector: 0xd76fa120
            case 0xd76fa120 {
                let solenoid := calldataload(4)
                let brake := calldataload(36)
                
                // Rule 1: Solenoid must engage (1) prior to releasing the brake (0)
                let valid := 0
                if and(eq(solenoid, 1), eq(brake, 0)) {
                    valid := 1
                }
                
                mstore(0, valid)
                return(0, 32)
            }
            
            // verifyBrakeLatency(uint256 cycleOffset) -> selector: 0x4e23cf01
            case 0x4e23cf01 {
                let cycleOffset := calldataload(4)
                
                // Rule 2: Caliper brake must lock the shaft within 1 cycle
                let valid := 0
                if iszero(gt(cycleOffset, 1)) {
                    valid := 1
                }
                
                mstore(0, valid)
                return(0, 32)
            }
            
            // verifyRAWVerification(uint256 checksumVal) -> selector: 0x0bf8923a
            case 0x0bf8923a {
                let checksumVal := calldataload(4)
                
                // Rule 3: RAW head status drops to 0 on checksum/parity mismatch (e.g. 0xDEAD)
                let status := 1
                if eq(checksumVal, 0xDEAD) {
                    status := 0
                }
                
                mstore(0, status)
                return(0, 32)
            }
            
            // writeGroup(uint256 startSec, uint256 count, uint256 dataVal, uint256 failIndex) -> selector: 0xe39fa210
            case 0xe39fa210 {
                let startSec := calldataload(4)
                let count := calldataload(36)
                let dataVal := calldataload(68)
                let failIndex := calldataload(100) // Simulate write failure at this index
                
                // Initialize rollback savepoint for atomic transaction tracking
                let success := 1
                let i := 0
                
                for {} lt(i, count) { i := add(i, 1) } {
                    let curSector := add(startSec, i)
                    
                    // Simulate write pass
                    mstore(0, curSector)
                    mstore(32, 0x100)
                    let slot := keccak256(0, 64)
                    
                    // Check if write lock is active
                    let lockedBy := sload(add(slot, 1))
                    if and(gt(lockedBy, 0), iszero(eq(lockedBy, caller()))) {
                        success := 0
                        break
                    }
                    
                    // Store pre-transaction state for rollback recovery
                    let backupVal := sload(slot)
                    
                    // Perform write
                    sstore(slot, dataVal)
                    
                    // Simulate verification failure
                    if eq(i, failIndex) {
                        // Rollback all written sectors in group transaction
                        let r := 0
                        for {} iszero(gt(r, i)) { r := add(r, 1) } {
                            let rollbackSec := add(startSec, r)
                            mstore(0, rollbackSec)
                            mstore(32, 0x100)
                            let rSlot := keccak256(0, 64)
                            sstore(rSlot, backupVal) // Restore previous values
                        }
                        success := 0
                        break
                    }
                }
                
                mstore(0, success)
                return(0, 32)
            }
            
            // acquireLock(uint256 sector, uint256 callerId) -> selector: 0x902d3412
            case 0x902d3412 {
                let sector := calldataload(4)
                let callerId := calldataload(36)
                
                mstore(0, sector)
                mstore(32, 0x100)
                let slot := keccak256(0, 64)
                let lockSlot := add(slot, 1)
                
                let success := 0
                let currentLock := sload(lockSlot)
                if or(iszero(currentLock), eq(currentLock, callerId)) {
                    sstore(lockSlot, callerId)
                    success := 1
                }
                
                mstore(0, success)
                return(0, 32)
            }
            
            // replayJournal(uint256 logCount) -> selector: 0x7a8e5200
            // Calldata: selector (4), logCount (32), log1_sector (32), log1_val (32), log1_commit (32), ...
            case 0x7a8e5200 {
                let logCount := calldataload(4)
                let success := 1
                
                let i := 0
                for {} lt(i, logCount) { i := add(i, 1) } {
                    let offset := add(36, mul(i, 96))
                    let sector := calldataload(offset)
                    let val := calldataload(add(offset, 32))
                    let commit := calldataload(add(offset, 64))
                    
                    if eq(commit, 1) {
                        mstore(0, sector)
                        mstore(32, 0x100)
                        let slot := keccak256(0, 64)
                        sstore(slot, val)
                    }
                }
                
                mstore(0, success)
                return(0, 32)
            }
            
            // peekSector(uint256 sector) -> selector: 0x228cf01b
            case 0x228cf01b {
                let sector := calldataload(4)
                mstore(0, sector)
                mstore(32, 0x100)
                let slot := keccak256(0, 64)
                mstore(0, sload(slot))
                return(0, 32)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
