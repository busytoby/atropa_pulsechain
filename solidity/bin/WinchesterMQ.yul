object "WinchesterMQ" {
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
            // METHOD 1: writeSignalsOut(uint8 signals) -> void
            // Selector: 0x485301a0 (Simulates writing to $DF01)
            // ----------------------------------------------------------------
            if eq(selector, 0x485301a0) {
                let sigs := and(calldataload(4), 0xFF)
                let sel := and(sigs, 0x01)
                let rst := and(shr(1, sigs), 0x01)
                let ack := and(shr(2, sigs), 0x01)

                let prevAck := sload(5) // Get last stored ACK

                // Reset handling
                if rst {
                    performReset()
                    return(0, 0)
                }

                // Selection Phase: SEL goes High and BSY is low
                if and(sel, iszero(sload(0))) {
                    sstore(0, 1)    // Assert BSY = 1
                    sstore(2, 1)    // Assert C/D = 1 (Command phase)
                    sstore(3, 0)    // Assert I/O = 0 (Host to Controller)
                    sstore(4, 0)    // Assert MSG = 0
                    sstore(6, 0)    // Clear CDB_BYTE_COUNT
                    sstore(13, 0)   // Clear DATA_BYTE_COUNT
                    sstore(1, 1)    // Assert REQ = 1 (Request first CDB byte)
                }

                // If ACK transitions from Low to High (Asserted by guest)
                if and(ack, iszero(prevAck)) {
                    sstore(5, 1) // Save new ACK state
                    processAckHigh()
                }

                // If ACK transitions from High to Low (Deasserted by guest)
                if and(iszero(ack), prevAck) {
                    sstore(5, 0) // Save new ACK state
                    processAckLow()
                }

                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 2: readSignalsIn() -> uint8 signals
            // Selector: 0x525302b0 (Simulates reading from $DF02)
            // ----------------------------------------------------------------
            if eq(selector, 0x525302b0) {
                let sigs := 0
                if sload(0) { sigs := or(sigs, 0x01) } // BSY
                if sload(1) { sigs := or(sigs, 0x02) } // REQ
                if sload(2) { sigs := or(sigs, 0x04) } // C/D
                if sload(3) { sigs := or(sigs, 0x08) } // I/O
                if sload(4) { sigs := or(sigs, 0x10) } // MSG
                mstore(0x00, sigs)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: writeDataPort(uint8 val) -> void
            // Selector: 0x98d400c0 (Simulates writing to $DF00)
            // ----------------------------------------------------------------
            if eq(selector, 0x98d400c0) {
                let val := and(calldataload(4), 0xFF)
                sstore(0x20, val) // Store in transient Data Port register
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 4: readDataPort() -> uint8 val
            // Selector: 0x52d400d0 (Simulates reading from $DF00)
            // ----------------------------------------------------------------
            if eq(selector, 0x52d400d0) {
                let val := sload(0x20)
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: mountDisk(uint256 lun, address diskContract) -> void
            // Selector: 0x4d6f756e (Mounts a VirtualDisk contract to a LUN)
            // ----------------------------------------------------------------
            if eq(selector, 0x4d6f756e) {
                let lun := calldataload(4)
                let diskContract := calldataload(36)
                sstore(add(0x5000, lun), diskContract)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 6: getMountedDisk(uint256 lun) -> address diskContract
            // Selector: 0x476f756e
            // ----------------------------------------------------------------
            if eq(selector, 0x476f756e) {
                let lun := calldataload(4)
                mstore(0x00, sload(add(0x5000, lun)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 7: flushDiskCache(uint256 lun) -> void
            // Selector: 0x466c7573 (Force flushes the memory cache to the cartridge)
            // ----------------------------------------------------------------
            if eq(selector, 0x466c7573) {
                let lun := calldataload(4)
                flushDisk(lun)
                return(0, 0)
            }

            revert(0, 0)

            // ================================================================
            // HELPER LOGIC FOR SASI EMULATOR
            // ================================================================
            function performReset() {
                sstore(0, 0) // BSY = 0
                sstore(1, 0) // REQ = 0
                sstore(2, 0) // C/D = 0
                sstore(3, 0) // I/O = 0
                sstore(4, 0) // MSG = 0
                sstore(5, 0) // ACK = 0
                sstore(6, 0) // CDB count = 0
                sstore(13, 0) // Data count = 0
            }

            // Invoked when ACK rises: latch data or prepare output byte
            function processAckHigh() {
                let cd := sload(2)
                let io := sload(3)

                // 1. Command Phase: Guest is sending CDB command bytes (C/D = 1, I/O = 0)
                if and(cd, iszero(io)) {
                    let byteIndex := sload(6)
                    let inputByte := sload(0x20) // Read current transient data port
                    sstore(add(7, byteIndex), inputByte) // Store in CDB buffer slots 7-12
                    
                    let nextIndex := add(byteIndex, 1)
                    sstore(6, nextIndex)

                    sstore(1, 0) // Drop REQ = 0 to signify byte captured
                    leave
                }

                // 2. Data Write Phase: Guest is sending payload data bytes (C/D = 0, I/O = 0)
                if and(iszero(cd), iszero(io)) {
                    let byteIndex := sload(13)
                    let inputByte := sload(0x20)
                    
                    let blockId := getCdbLba()
                    let lun := getCdbLun()
                    
                    // Lazy-load sector from cartridge if memory cache slot is not initialized
                    ensureSectorLoaded(lun, blockId)
                    
                    // Write data byte directly to dynamic storage array at matching offset
                    let wordOffset := div(byteIndex, 32)
                    let byteOffset := mod(byteIndex, 32)
                    let cacheKey := keccak256(add(0x1000, blockId), 32)
                    let currentWord := sload(add(cacheKey, wordOffset))
                    
                    // Inject byte into correct position of 256-bit word
                    let shift := mul(sub(31, byteOffset), 8)
                    let mask := not(shl(shift, 0xFF))
                    let newWord := or(and(currentWord, mask), shl(shift, inputByte))
                    
                    sstore(add(cacheKey, wordOffset), newWord)
                    
                    // Mark sector as dirty (bitmask)
                    markSectorDirty(blockId)
                    
                    sstore(13, add(byteIndex, 1))
                    sstore(1, 0) // Drop REQ = 0
                    leave
                }

                // 3. Data Read Phase: Guest is reading bytes (C/D = 0, I/O = 1)
                // Just drop REQ = 0 to acknowledge that the guest read the byte
                if and(iszero(cd), io) {
                    sstore(1, 0) // Drop REQ = 0
                    leave
                }
            }

            // Invoked when ACK drops: setup next byte or transition phases
            function processAckLow() {
                let cd := sload(2)
                let io := sload(3)

                // 1. Command Phase: Continue gathering command bytes or transition
                if and(cd, iszero(io)) {
                    let count := sload(6)
                    if lt(count, 6) {
                        sstore(1, 1) // Assert REQ = 1 for next command byte
                        leave
                    }
                    
                    // 6-byte CDB complete. Parse command type.
                    let opcode := sload(7) // Opcode stored in CDB index 0 (slot 7)
                    
                    // Opcode 0x08 = Read Block / MQGET (Updated for 2-phase commit)
                    if eq(opcode, 0x08) {
                        sstore(2, 0) // C/D = 0 (Data phase)
                        sstore(3, 1) // I/O = 1 (Read from controller to guest)
                        sstore(13, 0) // Clear DATA_BYTE_COUNT
                        
                        // Set the currently read block as pending acknowledgment lease (slot 0x30)
                        let blockId := getCdbLba()
                        sstore(0x30, blockId)

                        // Load first read byte to data port and assert REQ
                        setupNextReadByte(0)
                        leave
                    }

                    // Opcode 0x0A = Write Block / MQPUT
                    if eq(opcode, 0x0A) {
                        sstore(2, 0) // C/D = 0 (Data phase)
                        sstore(3, 0) // I/O = 0 (Write from guest to controller)
                        sstore(13, 0) // Clear DATA_BYTE_COUNT
                        sstore(1, 1)  // Assert REQ = 1 to request first byte
                        leave
                    }

                    // Opcode 0x0C = Read Specific Block by Correlation ID
                    if eq(opcode, 0x0C) {
                        sstore(2, 0) // C/D = 0 (Data phase)
                        sstore(3, 0) // I/O = 0 (Host will write 32-byte CorrelId first)
                        sstore(13, 0) // Clear data count
                        sstore(1, 1)  // Assert REQ = 1
                        leave
                    }

                    // Opcode 0x1E = Acknowledge Message (Commit MQGET)
                    if eq(opcode, 0x1E) {
                        let blockId := getCdbLba()
                        let pending := sload(0x30)
                        
                        // Verify block is indeed leased for ACK
                        if eq(blockId, pending) {
                            let currentHead := sload(14)
                            if eq(blockId, currentHead) {
                                sstore(14, add(currentHead, 1)) // Permanently consume block
                            }
                            sstore(0x30, 0xFFFFFFFF) // Clear lease
                            transitionToStatus(0x00) // Good status
                            leave
                        }
                        
                        transitionToStatus(0x02) // Error status
                        leave
                    }

                    // Unknown or unhandled commands: immediately transition to status phase
                    transitionToStatus(0x02) // 0x02 = Check Condition (Error)
                    leave
                }

                // 2. Data Write Phase: Wait for bytes to be written
                if and(iszero(cd), iszero(io)) {
                    let opcode := sload(7)

                    // Special case: Opcode 0x0C (Writing the 32-byte target Correlation ID)
                    if eq(opcode, 0x0C) {
                        let byteIndex := sload(13)
                        let inputByte := sload(0x20)
                        
                        // Store the 32-byte target Correlation ID in slots 0x50-0x6F
                        sstore(add(0x50, byteIndex), inputByte)
                        let nextCount := add(byteIndex, 1)
                        sstore(13, nextCount)

                        if lt(nextCount, 32) {
                            sstore(1, 1) // Request next byte
                            leave
                        }

                        // 32-byte CorrelId received. Search for matching block.
                        let matchedBlock := findBlockByCorrelation()
                        if eq(matchedBlock, 0xFFFFFFFF) {
                            transitionToStatus(0x02) // Not Found
                            leave
                        }

                        // Match found! Transition to Data Read Phase
                        sstore(8, and(shr(16, matchedBlock), 0x1F)) // Inject matched LBA
                        sstore(9, and(shr(8, matchedBlock), 0xFF))
                        sstore(10, and(matchedBlock, 0xFF))

                        sstore(0x30, matchedBlock) // Set pending lease

                        sstore(2, 0) // C/D = 0
                        sstore(3, 1) // I/O = 1 (Read phase)
                        sstore(13, 0) // Reset read data counter
                        setupNextReadByte(0)
                        leave
                    }

                    let count := sload(13)
                    if lt(count, 256) {
                        sstore(1, 1) // Request next byte
                        leave
                    }
                    
                    // Block write finished successfully. Determine priority routing.
                    let blockId := getCdbLba()
                    let cacheKey := keccak256(add(0x1000, blockId), 32)
                    let word0 := sload(cacheKey)
                    
                    // Extract Priority byte (Byte 6 of sector, low byte of high 16-bit word)
                    let priority := and(shr(200, word0), 0xFF)
                    
                    // If high priority (Priority > 4), perform head insertion
                    if gt(priority, 4) {
                        let currentHead := sload(14)
                        let newHead := sub(currentHead, 1)
                        sstore(14, newHead) // Decrement head pointer
                        
                        // Remap block data cache to the new head position
                        let newHeadKey := keccak256(add(0x1000, newHead), 32)
                        for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                            sstore(add(newHeadKey, j), sload(add(cacheKey, j)))
                            sstore(add(cacheKey, j), 0) // Clean up old LBA cache slot
                        }
                        
                        transitionToStatus(0x00)
                        leave
                    }

                    // Standard priority: advance tail index
                    let currentTail := sload(15)
                    if eq(blockId, currentTail) {
                        sstore(15, add(currentTail, 1)) // Grow queue tail index
                    }

                    transitionToStatus(0x00) // Good status
                    leave
                }

                // 3. Data Read Phase: Send next byte until 256 bytes transferred
                if and(iszero(cd), io) {
                    let count := sload(13)
                    let nextCount := add(count, 1)
                    sstore(13, nextCount)
                    
                    if lt(nextCount, 256) {
                        setupNextReadByte(nextCount)
                        leave
                    }

                    // Read complete. (Queue pointer is NOT advanced yet, waiting for Opcode 0x1E)
                    transitionToStatus(0x00) // Good status
                    leave
                }

                // 4. Status Phase completion: release bus
                if and(cd, io) {
                    performReset() // Release bus by clearing all registers
                }
            }

            function setupNextReadByte(index) {
                let blockId := getCdbLba()
                let lun := getCdbLun()
                
                // Lazy-load sector from cartridge if memory cache slot is not initialized
                ensureSectorLoaded(lun, blockId)

                // On first byte read (index 0), perform TTL validation check
                if iszero(index) {
                    let isExpired := checkBlockExpired(blockId)
                    if isExpired {
                        // Skip this block by advancing the read queue head and moving to status
                        let currentHead := sload(14)
                        if eq(blockId, currentHead) {
                            sstore(14, add(currentHead, 1))
                        }
                        transitionToStatus(0x04) // Status 0x04: Block Skipped/Expired
                        leave
                    }
                }
                
                let wordOffset := div(index, 32)
                let byteOffset := mod(index, 32)
                let cacheKey := keccak256(add(0x1000, blockId), 32)
                let word := sload(add(cacheKey, wordOffset))
                
                let shift := mul(sub(31, byteOffset), 8)
                let byteVal := and(shr(shift, word), 0xFF)
                
                sstore(0x20, byteVal) // Place byte on data port register
                sstore(1, 1)          // Assert REQ = 1
            }

            // Validates if the message block contains an active TTL expiry
            function checkBlockExpired(blockId) -> expired {
                expired := 0
                let cacheKey := keccak256(add(0x1000, blockId), 32)
                
                // ExpiryJiffies is stored in bytes 8-15 (word 0, low-mid bytes)
                let word0 := sload(cacheKey)
                let expiry := and(shr(128, word0), 0xFFFFFFFFFFFFFFFF)
                
                // If ExpiryJiffies is non-zero, validate against current system Jiffies
                if expiry {
                    let currentJiffies := mul(number(), 60)
                    if gt(currentJiffies, expiry) {
                        expired := 1
                    }
                }
            }

            // Scans queue sectors to match the requested 32-byte Correlation ID
            function findBlockByCorrelation() -> matchedLba {
                matchedLba := 0xFFFFFFFF
                let currentHead := sload(14)
                let currentTail := sload(15)
                
                // Pack 32 bytes of queried Correlation ID from slots 0x50-0x6F into a single 256-bit word
                let targetWord := 0
                for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                    targetWord := or(shl(8, targetWord), and(sload(add(0x50, i)), 0xFF))
                }

                let lun := getCdbLun()

                // Scan through queue indices
                for { let lba := currentHead } lt(lba, currentTail) { lba := add(lba, 1) } {
                    ensureSectorLoaded(lun, lba)
                    
                    let cacheKey := keccak256(add(0x1000, lba), 32)
                    
                    // Correlation ID is stored in bytes 16-47 (spanning word 0 and word 1 of the sector)
                    // Offset 16-31 is the lower 16 bytes of word 0
                    // Offset 32-47 is the upper 16 bytes of word 1
                    let word0 := sload(cacheKey)
                    let word1 := sload(add(cacheKey, 1))
                    
                    let part1 := and(word0, 0x00000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
                    let part2 := shr(128, word1)
                    let sectorCorrel := or(shl(128, part1), part2)

                    if eq(sectorCorrel, targetWord) {
                        matchedLba := lba
                        break
                    }
                }
            }

            function transitionToStatus(statusCode) {
                sstore(2, 1) // C/D = 1
                sstore(3, 1) // I/O = 1
                sstore(4, 1) // MSG = 1
                sstore(0x20, statusCode) // Place status on data port
                sstore(1, 1) // Assert REQ = 1
            }

            function getCdbLba() -> lba {
                // Parse LBA from CDB (bytes 1, 2, and 3 stored at slots 8, 9, 10)
                let b1 := sload(8)
                let b2 := sload(9)
                let b3 := sload(10)
                lba := or(or(shl(16, and(b1, 0x1F)), shl(8, and(b2, 0xFF))), and(b3, 0xFF))
            }

            function getCdbLun() -> lun {
                // LUN is stored in the high bits of byte 1 (CDB index 1, slot 8)
                let b1 := sload(8)
                lun := shr(5, and(b1, 0xE0))
            }

            // ================================================================
            // CARTRIDGE LOADING / PERSISTENCE PORTING LOGIC
            // ================================================================
            
            // Checks if the memory cache page has been hydrated from VirtualDisk
            function ensureSectorLoaded(lun, lba) {
                let initKey := keccak256(add(0x3000, lba), 32)
                let initialized := sload(initKey)
                if initialized { leave }

                let diskContract := sload(add(0x5000, lun))
                if iszero(diskContract) {
                    sstore(initKey, 1) // Assume blank disk if unmounted
                    leave
                }

                // Call VirtualDisk.readSector(lba) to read 8 words of data
                let ptr := mload(0x40)
                mstore(ptr, 0x573b95d200000000000000000000000000000000000000000000000000000000) // readSector selector
                mstore(add(ptr, 4), lba)

                let success := staticcall(gas(), diskContract, ptr, 36, ptr, 256)
                if success {
                    let cacheKey := keccak256(add(0x1000, lba), 32)
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        sstore(add(cacheKey, i), mload(add(ptr, mul(i, 32))))
                    }
                }
                sstore(initKey, 1) // Mark as loaded in memory
            }

            function markSectorDirty(lba) {
                let maskWordIdx := div(lba, 256)
                let bitIdx := mod(lba, 256)
                let maskKey := keccak256(add(0x4000, maskWordIdx), 32)
                let mask := sload(maskKey)
                sstore(maskKey, or(mask, shl(bitIdx, 1)))
            }

            function flushDisk(lun) {
                let diskContract := sload(add(0x5000, lun))
                if iszero(diskContract) { leave }

                // Scan dirty masks (currently supporting up to 256 sectors)
                let maskKey := keccak256(0x4000, 32)
                let mask := sload(maskKey)
                if iszero(mask) { leave }

                let ptr := mload(0x40)

                for { let lba := 0 } lt(lba, 256) { lba := add(lba, 1) } {
                    if and(mask, shl(lba, 1)) {
                        // Hydrate calldata parameters to writeSector(lba, sectorData)
                        mstore(ptr, 0x8aef890d00000000000000000000000000000000000000000000000000000000) // selector
                        mstore(add(ptr, 4), lba)
                        
                        let cacheKey := keccak256(add(0x1000, lba), 32)
                        for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                            mstore(add(ptr, add(36, mul(j, 32))), sload(add(cacheKey, j)))
                        }

                        // Call external disk cartridge write
                        let success := call(gas(), diskContract, 0, ptr, 292, 0, 0)
                    }
                }

                // Clear dirty mask
                sstore(maskKey, 0)
            }
        }
    }
}
