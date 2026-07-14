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
            if eq(selector, 0x3c130093) {
                sstore(0xF120, 0)
                sstore(0xF121, 0)
                return(0, 0)
            }
            if eq(selector, 0x0ff11000) {
                let f1 := calldataload(4)
                let f2 := calldataload(36)
                sstore(0xF150, f1)
                sstore(0xF151, f2)
                return(0, 0)
            }
            if eq(selector, 0x0ff22000) {
                if sload(0xF303) { revert(0, 0) }
                sstore(0xF303, 1)
                let priority := calldataload(4)
                let ev_type := calldataload(36)
                let ev_timestamp := calldataload(68)
                let ev_data := calldataload(100)
                let head := sload(0xF300)
                let tail := sload(0xF301)
                let size := sload(0xF302)
                if lt(size, 16) {
                    let slot := add(0xF310, mul(tail, 4))
                    sstore(slot, priority)
                    sstore(add(slot, 1), ev_type)
                    sstore(add(slot, 2), ev_timestamp)
                    sstore(add(slot, 3), ev_data)
                    tail := mod(add(tail, 1), 16)
                    sstore(0xF301, tail)
                    sstore(0xF302, add(size, 1))
                }
                sstore(0xF303, 0)
                return(0, 0)
            }
            if eq(selector, 0x0ff23000) {
                if sload(0xF303) { revert(0, 0) }
                sstore(0xF303, 1)
                let head := sload(0xF300)
                let tail := sload(0xF301)
                let size := sload(0xF302)
                if gt(size, 0) {
                    let slot := add(0xF310, mul(head, 4))
                    mstore(0x00, sload(slot))
                    mstore(0x20, sload(add(slot, 1)))
                    mstore(0x40, sload(add(slot, 2)))
                    mstore(0x60, sload(add(slot, 3)))
                    head := mod(add(head, 1), 16)
                    sstore(0xF300, head)
                    sstore(0xF302, sub(size, 1))
                    sstore(0xF303, 0)
                    return(0x00, 128)
                }
                sstore(0xF303, 0)
                return(0, 0)
            }
            if eq(selector, 0xe399f0e0) {
                let v_in := calldataload(4)
                sstore(0xF199, v_in)
                let v_out := 0
                let diode_drop := 700 // 0.7V bias drop in milli-units
                
                if gt(v_in, diode_drop) {
                    v_out := sub(v_in, diode_drop)
                }
                let neg_limit := sub(0, diode_drop)
                if slt(v_in, neg_limit) {
                    v_out := add(v_in, diode_drop)
                }
                
                // 1. AGC-Driven Firewall: threshold safety check (limit 2500)
                let threat_count := sload(0xF120)
                if gt(v_out, 2500) {
                    threat_count := add(threat_count, 1)
                    sstore(0xF120, threat_count)
                }
                if gt(threat_count, 3) {
                    sstore(0xF121, 1) // Set firewall block active
                }
                if sload(0xF121) {
                    v_out := 0 // Mute signal
                }
                
                // Store virtual hardware NPN (pos_drive) / PNP (neg_drive) registers
                if gt(v_out, 0) {
                    sstore(0xF100, v_out)
                    sstore(0xF101, 0)
                }
                if lt(v_out, 0) {
                    sstore(0xF100, 0)
                    sstore(0xF101, sub(0, v_out))
                }
                if eq(v_out, 0) {
                    sstore(0xF100, 0)
                    sstore(0xF101, 0)
                }
                
                // Pure Yul Goertzel recursion step (coeff = 1.854 -> scaled by 10000)
                let s_prev := sload(0xF102)
                let s_prev2 := sload(0xF103)
                let coeff := 18540
                let s_term := div(mul(coeff, s_prev), 10000)
                let s := sub(add(v_out, s_term), s_prev2)
                
                sstore(0xF103, s_prev)
                sstore(0xF102, s)
                
                // 2. PLL Phase-Lock tracking (deviation = diff of input vs last step)
                let pll_diff := sub(v_in, s_prev)
                sstore(0xF125, pll_diff)
                
                mstore(0x00, v_out)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 1: writeSignalsOut(uint8 signals) -> void
            // Selector: 0x485301a0 (Simulates writing to $DF01)
            // ----------------------------------------------------------------
            if eq(selector, 0x485301a0) {
                let sigs := and(calldataload(4), 0xFF)
                let sel := and(sigs, 0x01)
                let rst := and(shr(1, sigs), 0x01)
                let ack := and(shr(2, sigs), 0x01)

                let prevAck := loadTransient(5) // Get last stored ACK

                // Reset handling
                if rst {
                    performReset()
                    return(0, 0)
                }

                // Selection Phase: SEL goes High and BSY is low
                if and(sel, iszero(loadTransient(0))) {
                    storeTransient(0, 1)    // Assert BSY = 1
                    storeTransient(2, 1)    // Assert C/D = 1 (Command phase)
                    storeTransient(3, 0)    // Assert I/O = 0 (Host to Controller)
                    storeTransient(4, 0)    // Assert MSG = 0
                    storeTransient(6, 0)    // Clear CDB_BYTE_COUNT
                    storeTransient(13, 0)   // Clear DATA_BYTE_COUNT
                    storeTransient(1, 1)    // Assert REQ = 1 (Request first CDB byte)
                }

                // If ACK transitions from Low to High (Asserted by guest)
                if and(ack, iszero(prevAck)) {
                    storeTransient(5, 1) // Save new ACK state
                    processAckHigh()
                }

                // If ACK transitions from High to Low (Deasserted by guest)
                if and(iszero(ack), prevAck) {
                    storeTransient(5, 0) // Save new ACK state
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
                if loadTransient(0) { sigs := or(sigs, 0x01) } // BSY
                if loadTransient(1) { sigs := or(sigs, 0x02) } // REQ
                if loadTransient(2) { sigs := or(sigs, 0x04) } // C/D
                if loadTransient(3) { sigs := or(sigs, 0x08) } // I/O
                if loadTransient(4) { sigs := or(sigs, 0x10) } // MSG
                mstore(0x00, sigs)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: writeDataPort(uint8 val) -> void
            // Selector: 0x98d400c0 (Simulates writing to $DF00)
            // ----------------------------------------------------------------
            if eq(selector, 0x98d400c0) {
                let val := and(calldataload(4), 0xFF)
                storeTransient(0x20, val) // Store in transient Data Port register
                
                // Track SCSI hardware stats in EVM storage slots
                let txCount := sload(0xF304)
                sstore(0xF304, add(txCount, 1))
                
                // Emulate SCSI Parity check: trigger parity error if byte is exactly 0xFF
                if eq(val, 0xFF) {
                    let errCount := sload(0xF305)
                    sstore(0xF305, add(errCount, 1))
                }
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 4: readDataPort() -> uint8 val
            // Selector: 0x52d400d0 (Simulates reading from $DF00)
            // ----------------------------------------------------------------
            if eq(selector, 0x52d400d0) {
                let val := loadTransient(0x20)
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

            // ----------------------------------------------------------------
            // METHOD 8: postEvent(bytes32 val) -> void
            // Selector: 0xccb077a0
            // ----------------------------------------------------------------
            if eq(selector, 0xccb077a0) {
                let val := calldataload(4)
                mstore(0x00, val)
                log1(0x00, 32, 0xe1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1da)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 9: readTransient(uint256 index) -> uint256
            // Selector: 0xe9d601b0
            // ----------------------------------------------------------------
            if eq(selector, 0xe9d601b0) {
                let index := calldataload(4)
                let val := loadTransient(index)
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 10: writeTransient(uint256 index, uint256 val) -> void
            // Selector: 0x19a402c0
            // ----------------------------------------------------------------
            if eq(selector, 0x19a402c0) {
                let index := calldataload(4)
                let val := calldataload(36)
                storeTransient(index, val)
                return(0, 0)
            }

            revert(0, 0)

            // ================================================================
            // HELPER LOGIC FOR SASI EMULATOR
            // ================================================================
            function performReset() {
                storeTransient(0, 0) // BSY = 0
                storeTransient(1, 0) // REQ = 0
                storeTransient(2, 0) // C/D = 0
                storeTransient(3, 0) // I/O = 0
                storeTransient(4, 0) // MSG = 0
                storeTransient(5, 0) // ACK = 0
                storeTransient(6, 0) // CDB count = 0
                storeTransient(13, 0) // Data count = 0
            }

            // Invoked when ACK rises: latch data or prepare output byte
            function processAckHigh() {
                let cd := loadTransient(2)
                let io := loadTransient(3)

                // 1. Command Phase: Guest is sending CDB command bytes (C/D = 1, I/O = 0)
                if and(cd, iszero(io)) {
                    let byteIndex := loadTransient(6)
                    let inputByte := loadTransient(0x20) // Read current transient data port
                    storeTransient(add(7, byteIndex), inputByte) // Store in CDB buffer slots 7-12
                    
                    let nextIndex := add(byteIndex, 1)
                    storeTransient(6, nextIndex)

                    storeTransient(1, 0) // Drop REQ = 0 to signify byte captured
                    leave
                }

                // 2. Data Write Phase: Guest is sending payload data bytes (C/D = 0, I/O = 0)
                if and(iszero(cd), iszero(io)) {
                    let byteIndex := loadTransient(13)
                    let inputByte := loadTransient(0x20)
                    
                    let blockId := getCdbLba()
                    let lun := getCdbLun()
                    
                    // Lazy-load sector from cartridge if memory cache slot is not initialized
                    ensureSectorLoaded(lun, blockId)
                    
                    // Write data byte directly to dynamic storage array at matching offset
                    let wordOffset := div(byteIndex, 32)
                    let byteOffset := mod(byteIndex, 32)
                    let cacheKey := hashKey(0x1000, blockId)
                    let currentWord := sload(add(cacheKey, wordOffset))
                    
                    // Inject byte into correct position of 256-bit word
                    let shift := mul(sub(31, byteOffset), 8)
                    let mask := not(shl(shift, 0xFF))
                    let newWord := or(and(currentWord, mask), shl(shift, inputByte))
                    
                    sstore(add(cacheKey, wordOffset), newWord)
                    
                    // Mark sector as dirty (bitmask)
                    markSectorDirty(blockId)
                    
                    storeTransient(13, add(byteIndex, 1))
                    storeTransient(1, 0) // Drop REQ = 0
                    leave
                }

                // 3. Data Read Phase: Guest is reading bytes (C/D = 0, I/O = 1)
                // Just drop REQ = 0 to acknowledge that the guest read the byte
                if and(iszero(cd), io) {
                    storeTransient(1, 0) // Drop REQ = 0
                    leave
                }
            }

            // Invoked when ACK drops: setup next byte or transition phases
            function processAckLow() {
                let cd := loadTransient(2)
                let io := loadTransient(3)

                // 1. Command Phase: Continue gathering command bytes or transition
                if and(cd, iszero(io)) {
                    let count := loadTransient(6)
                    if lt(count, 6) {
                        storeTransient(1, 1) // Assert REQ = 1 for next command byte
                        leave
                    }
                    
                    // 6-byte CDB complete. Parse command type.
                    let opcode := loadTransient(7) // Opcode stored in CDB index 0 (slot 7)
                    
                    // Opcode 0x08 = Read Block / MQGET (Updated for 2-phase commit)
                    if eq(opcode, 0x08) {
                        storeTransient(2, 0) // C/D = 0 (Data phase)
                        storeTransient(3, 1) // I/O = 1 (Read from controller to guest)
                        storeTransient(13, 0) // Clear DATA_BYTE_COUNT
                        
                        // Set the currently read block as pending acknowledgment lease (slot 0x30)
                        let blockId := getCdbLba()
                        storeTransient(0x30, blockId)

                        // Load first read byte to data port and assert REQ
                        setupNextReadByte(0)
                        leave
                    }

                    // Opcode 0x0A = Write Block / MQPUT
                    if eq(opcode, 0x0A) {
                        let blockId := getCdbLba()
                        let lun := getCdbLun()
                        
                        // Flow Control: check if queue length exceeds 16 blocks
                        let currentHead := sload(add(0x2000, lun))
                        let currentTail := sload(add(0x2050, lun))
                        let size := 0
                        if lt(currentHead, currentTail) { size := sub(currentTail, currentHead) }
                        
                        if gt(size, 15) {
                            transitionToStatus(0x08) // Status 0x08: Queue Full / Busy
                            leave
                        }

                        storeTransient(2, 0) // C/D = 0 (Data phase)
                        storeTransient(3, 0) // I/O = 0 (Write from guest to controller)
                        storeTransient(13, 0) // Clear DATA_BYTE_COUNT
                        storeTransient(1, 1)  // Assert REQ = 1 to request first byte
                        leave
                    }

                    // Opcode 0x0C = Read Specific Block by Correlation ID
                    if eq(opcode, 0x0C) {
                        storeTransient(2, 0) // C/D = 0 (Data phase)
                        storeTransient(3, 0) // I/O = 0 (Host will write 32-byte CorrelId first)
                        storeTransient(13, 0) // Clear data count
                        storeTransient(1, 1)  // Assert REQ = 1
                        leave
                    }

                    // Opcode 0x1E = Acknowledge Message (Commit MQGET)
                    if eq(opcode, 0x1E) {
                        let blockId := getCdbLba()
                        let pending := loadTransient(0x30)
                        
                        // Verify block is indeed leased for ACK
                        if eq(blockId, pending) {
                            let currentHead := sload(14)
                            if eq(blockId, currentHead) {
                                sstore(14, add(currentHead, 1)) // Permanently consume block
                            }
                            storeTransient(0x30, 0xFFFFFFFF) // Clear lease
                            
                            // Journaling Log: LogAck(lun, blockId)
                            mstore(0x00, getCdbLun())
                            mstore(0x20, blockId)
                            log1(0x00, 0x40, 0xb8b6a3efb8b6a3efb8b6a3efb8b6a3efb8b6a3efb8b6a3efb8b6a3efb8b6a3ef)

                            transitionToStatus(0x00) // Good status
                            leave
                        }
                        
                        transitionToStatus(0x02) // Error status
                        leave
                    }

                    // Opcode 0x1F = Subscribe to Topic
                    if eq(opcode, 0x1F) {
                        let topicId := getCdbLba() // Extract Topic ID from LBA fields
                        let subscriberLun := getCdbLun()
                        
                        // Register subscriber in storage: mapping(topicId => array of (LUN, LBA))
                        let subscriberCountKey := hashKey(0x7000, topicId)
                        let subCount := sload(subscriberCountKey)
                        
                        // Store the destination LUN/LBA target for this subscriber index
                        let targetKey := hashKey(add(0x7100, topicId), subCount)
                        sstore(targetKey, subscriberLun)
                        
                        // Increment subscriber count
                        sstore(subscriberCountKey, add(subCount, 1))
                        
                        transitionToStatus(0x00) // Good status
                        leave
                    }

                    // Unknown or unhandled commands: immediately transition to status phase
                    transitionToStatus(0x02) // 0x02 = Check Condition (Error)
                    leave
                }

                // 2. Data Write Phase: Wait for bytes to be written
                if and(iszero(cd), iszero(io)) {
                    let opcode := loadTransient(7)

                    // Special case: Opcode 0x0C (Writing the 32-byte target Correlation ID)
                    if eq(opcode, 0x0C) {
                        let byteIndex := loadTransient(13)
                        let inputByte := loadTransient(0x20)
                        
                        // Store the 32-byte target Correlation ID in slots 0x50-0x6F
                        storeTransient(add(0x50, byteIndex), inputByte)
                        let nextCount := add(byteIndex, 1)
                        storeTransient(13, nextCount)

                        if lt(nextCount, 32) {
                            storeTransient(1, 1) // Request next byte
                            leave
                        }

                        // 32-byte CorrelId received. Search for matching block.
                        let matchedBlock := findBlockByCorrelation()
                        if eq(matchedBlock, 0xFFFFFFFF) {
                            transitionToStatus(0x02) // Not Found
                            leave
                        }

                        // Match found! Transition to Data Read Phase
                        storeTransient(8, and(shr(16, matchedBlock), 0x1F)) // Inject matched LBA
                        storeTransient(9, and(shr(8, matchedBlock), 0xFF))
                        storeTransient(10, and(matchedBlock, 0xFF))

                        storeTransient(0x30, matchedBlock) // Set pending lease

                        storeTransient(2, 0) // C/D = 0
                        storeTransient(3, 1) // I/O = 1 (Read phase)
                        storeTransient(13, 0) // Reset read data counter
                        setupNextReadByte(0)
                        leave
                    }

                    let count := loadTransient(13)
                    if lt(count, 256) {
                        storeTransient(1, 1) // Request next byte
                        leave
                    }
                    
                    // Block write finished successfully. Determine priority routing.
                    let blockId := getCdbLba()
                    let lun := getCdbLun()
                    
                    // Special case: LUN 5 is the Topic Publish Broker
                    if eq(lun, 5) {
                        performTopicFanOut(blockId)
                        
                        // Journaling Log: LogPut(lun, blockId, topicId)
                        mstore(0x00, lun)
                        mstore(0x20, blockId)
                        let cacheKey := hashKey(0x1000, blockId)
                        mstore(0x40, shr(224, sload(cacheKey))) // Topic ID from magic header
                        log1(0x00, 0x60, 0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29a1bee1dae9af77dac73aa045)

                        transitionToStatus(0x00)
                        leave
                    }

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
                        let newHeadKey := hashKey(0x1000, newHead)
                        for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                            sstore(add(newHeadKey, j), sload(add(cacheKey, j)))
                            sstore(add(cacheKey, j), 0) // Clean up old LBA cache slot
                        }
                        
                        // Journaling Log: LogPut(lun, newHead, 0)
                        mstore(0x00, lun)
                        mstore(0x20, newHead)
                        mstore(0x40, 0)
                        log1(0x00, 0x60, 0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29a1bee1dae9af77dac73aa045)

                        transitionToStatus(0x00)
                        leave
                    }

                    // Standard priority: advance tail index
                    let currentTail := sload(15)
                    if eq(blockId, currentTail) {
                        sstore(15, add(currentTail, 1)) // Grow queue tail index
                    }

                    // Journaling Log: LogPut(lun, blockId, 0)
                    mstore(0x00, lun)
                    mstore(0x20, blockId)
                    mstore(0x40, 0)
                    log1(0x00, 0x60, 0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29a1bee1dae9af77dac73aa045)

                    transitionToStatus(0x00) // Good status
                    leave
                }

                // 3. Data Read Phase: Send next byte until 256 bytes transferred
                if and(iszero(cd), io) {
                    let count := loadTransient(13)
                    let nextCount := add(count, 1)
                    storeTransient(13, nextCount)
                    
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
                let cacheKey := hashKey(0x1000, blockId)
                let word := sload(add(cacheKey, wordOffset))
                
                let shift := mul(sub(31, byteOffset), 8)
                let byteVal := and(shr(shift, word), 0xFF)
                
                storeTransient(0x20, byteVal) // Place byte on data port register
                storeTransient(1, 1)          // Assert REQ = 1
            }

            // Validates if the message block contains an active TTL expiry
            function checkBlockExpired(blockId) -> expired {
                expired := 0
                let cacheKey := hashKey(0x1000, blockId)
                
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
                    targetWord := or(shl(8, targetWord), and(loadTransient(add(0x50, i)), 0xFF))
                }

                let lun := getCdbLun()

                // Scan through queue indices
                for { let lba := currentHead } lt(lba, currentTail) { lba := add(lba, 1) } {
                    ensureSectorLoaded(lun, lba)
                    
                    let cacheKey := hashKey(0x1000, lba)
                    
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

            // Replicates published messages to all subscribed inbox queues
            function performTopicFanOut(blockId) {
                let cacheKey := hashKey(0x1000, blockId)
                let word0 := sload(cacheKey)
                let word1 := sload(add(cacheKey, 1))

                // Topic ID is parsed from bytes 0-3 of the payload (the Magic topic header)
                let topicId := shr(224, word0)

                let subscriberCountKey := hashKey(0x7000, topicId)
                let count := sload(subscriberCountKey)
                if iszero(count) { leave }

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let targetKey := hashKey(add(0x7100, topicId), i)
                    let destLun := sload(targetKey)

                    // Write message block to subscriber's inbox queue tail
                    let destTail := sload(add(0x2050, destLun))
                    let destKey := hashKey(0x1000, destTail)
                    
                    sstore(destKey, word0)
                    sstore(add(destKey, 1), word1)

                    // Increment the subscriber's tail index
                    sstore(add(0x2050, destLun), add(destTail, 1))
                }

                // Clean up transient LUN 5 broker block cache
                sstore(cacheKey, 0)
                sstore(add(cacheKey, 1), 0)
            }

            function transitionToStatus(statusCode) {
                storeTransient(2, 1) // C/D = 1
                storeTransient(3, 1) // I/O = 1
                storeTransient(4, 1) // MSG = 1
                storeTransient(0x20, statusCode) // Place status on data port
                storeTransient(1, 1) // Assert REQ = 1
            }

            function getCdbLba() -> lba {
                // Parse LBA from CDB (bytes 1, 2, and 3 stored at slots 8, 9, 10)
                let b1 := loadTransient(8)
                let b2 := loadTransient(9)
                let b3 := loadTransient(10)
                lba := or(or(shl(16, and(b1, 0x1F)), shl(8, and(b2, 0xFF))), and(b3, 0xFF))
            }

            function getCdbLun() -> lun {
                // LUN is stored in the high bits of byte 1 (CDB index 1, slot 8)
                let b1 := loadTransient(8)
                lun := shr(5, and(b1, 0xE0))
            }

            // ================================================================
            // CARTRIDGE LOADING / PERSISTENCE PORTING LOGIC
            // ================================================================
            
            // Checks if the memory cache page has been hydrated from VirtualDisk
            function ensureSectorLoaded(lun, lba) {
                let initKey := hashKey(0x3000, lba)
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
                    let cacheKey := hashKey(0x1000, lba)
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        sstore(add(cacheKey, i), mload(add(ptr, mul(i, 32))))
                    }
                }
                sstore(initKey, 1) // Mark as loaded in memory
            }

            function markSectorDirty(lba) {
                let maskWordIdx := div(lba, 256)
                let bitIdx := mod(lba, 256)
                let maskKey := hashKey(0x4000, maskWordIdx)
                let mask := sload(maskKey)
                sstore(maskKey, or(mask, shl(bitIdx, 1)))
            }

            function flushDisk(lun) {
                let diskContract := sload(add(0x5000, lun))
                if iszero(diskContract) { leave }

                // Scan dirty masks (currently supporting up to 256 sectors)
                let maskKey := hashKey(0x4000, 0)
                let mask := sload(maskKey)
                if iszero(mask) { leave }

                let ptr := mload(0x40)

                for { let lba := 0 } lt(lba, 256) { lba := add(lba, 1) } {
                    if and(mask, shl(lba, 1)) {
                        // Hydrate calldata parameters to writeSector(lba, sectorData)
                        mstore(ptr, 0x8aef890d00000000000000000000000000000000000000000000000000000000) // selector
                        mstore(add(ptr, 4), lba)
                        
                        let cacheKey := hashKey(0x1000, lba)
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

            function hashKey(prefix, val) -> key {
                mstore(0x200, add(prefix, val))
                key := keccak256(0x200, 32)
            }

            // Namespaced Transient State Mappings for Public Chains
            function getTransientSlot(index) -> slot {
                mstore(0x280, caller())
                mstore(0x2A0, index)
                slot := keccak256(0x280, 64)
            }

            function loadTransient(idx) -> val {
                val := sload(getTransientSlot(idx))
            }

            function storeTransient(idx, val) {
                sstore(getTransientSlot(idx), val)
            }
        }
    }
}
