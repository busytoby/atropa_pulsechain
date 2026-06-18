object "WMQ_Scheduler" {
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
            // METHOD 1: registerCardProcess(uint256 cardId, uint256 entryPoint) -> void
            // Selector: 0x8a1b9c7e
            // ----------------------------------------------------------------
            if eq(selector, 0x8a1b9c7e) {
                let cardId := calldataload(4)
                if gt(cardId, 5) { revert(0, 0) } // Cap Card ID at 5 (0 to 5)
                let entryPoint := calldataload(36)
                
                // Initialize process control block (PCB) slots for the Card
                let pcbBase := getPcbOffset(cardId)
                sstore(add(pcbBase, 0), entryPoint) // Reg IP (Program Counter)
                sstore(add(pcbBase, 1), 0xFD)       // Reg SP (Stack Pointer)
                sstore(add(pcbBase, 2), 0)          // Reg A (Accumulator)
                sstore(add(pcbBase, 3), 0)          // Reg X
                sstore(add(pcbBase, 4), 0)          // Reg Y
                sstore(add(pcbBase, 5), 0)          // Reg Status flags (SR)
                sstore(add(pcbBase, 6), 1)          // Active status (1 = Active, 0 = Suspended)
                
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 2: runNextSlice(uint256 stepLimit) -> uint256 executedSteps
            // Selector: 0x52756e53
            // ----------------------------------------------------------------
            if eq(selector, 0x52756e53) {
                let stepLimit := calldataload(4)
                let executed := schedulerDispatch(stepLimit)
                mstore(0x00, executed)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: getCardRegs(uint256 cardId) -> (uint256 IP, uint256 SP, uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 Status)
            // Selector: 0x47657267
            // ----------------------------------------------------------------
            if eq(selector, 0x47657267) {
                let cardId := calldataload(4)
                if gt(cardId, 5) { revert(0, 0) } // Cap query Card ID at 5 (0 to 5)
                let pcbBase := getPcbOffset(cardId)
                
                let ptr := mload(0x40)
                mstore(ptr, sload(add(pcbBase, 0)))
                mstore(add(ptr, 32), sload(add(pcbBase, 1)))
                mstore(add(ptr, 64), sload(add(pcbBase, 2)))
                mstore(add(ptr, 96), sload(add(pcbBase, 3)))
                mstore(add(ptr, 128), sload(add(pcbBase, 4)))
                mstore(add(ptr, 160), sload(add(pcbBase, 5)))
                mstore(add(ptr, 192), sload(add(pcbBase, 6)))
                
                return(ptr, 224)
            }

            revert(0, 0)

            // ================================================================
            // CORE TASK DISPATCHER LOOP
            // ================================================================
            function schedulerDispatch(stepLimit) -> executedSteps {
                // Before scheduling next slice, poll for process launch requests
                pollProcessLaunches()

                let nextCard := findReadyCard()
                if eq(nextCard, 0xFFFFFFFF) {
                    executedSteps := 0
                    leave
                }

                // Hydrate transient registers of the selected Card
                let pcbBase := getPcbOffset(nextCard)
                let ip := sload(add(pcbBase, 0))
                let sp := sload(add(pcbBase, 1))
                let a := sload(add(pcbBase, 2))
                let x := sload(add(pcbBase, 3))
                let y := sload(add(pcbBase, 4))
                let sr := sload(add(pcbBase, 5))

                // Check if we need to inject a WinchesterMQ hardware interrupt (IRQ)
                let mqHead := sload(add(0x2000, nextCard))
                let mqTail := sload(add(0x2050, nextCard))
                if lt(mqHead, mqTail) {
                    let interruptDisable := and(sr, 0x04)
                    if iszero(interruptDisable) {
                        let stackPage := 0x0100
                        sstore(add(stackPage, sp), shr(8, and(ip, 0xFF00)))
                        sp := sub(sp, 1)
                        sstore(add(stackPage, sp), and(ip, 0x00FF))
                        sp := sub(sp, 1)

                        sstore(add(stackPage, sp), sr)
                        sp := sub(sp, 1)

                        let irqVector := sload(0xFFFE)
                        if iszero(irqVector) { irqVector := 0xFF00 } // fallback
                        ip := irqVector

                        sr := or(sr, 0x04)
                    }
                }

                // Simulate execution slice inside the ZMM Guest Emulator thunk
                let exitReason, steps := executeGuestVmSlice(nextCard, ip, sp, a, x, y, sr, stepLimit)
                executedSteps := steps

                // Check exit state
                switch exitReason
                case 1 { // Yielded
                    saveCardContext(nextCard, pcbBase)
                }
                case 2 { // Halted
                    saveCardContext(nextCard, pcbBase)
                    sstore(add(pcbBase, 6), 0) // Suspended
                }
                default {
                    saveCardContext(nextCard, pcbBase)
                }
                
                sstore(0x10, mod(add(nextCard, 1), 6))
            }

            // Checks the scheduler's inbox for pending LAUN (0x4c41554e) spawn request blocks
            function pollProcessLaunches() {
                // LUN 5 is the broker. Let's inspect scheduler launch queue (mapped to LUN 5, LBA index 0xFF)
                let head := sload(0x2005) // Scheduler head for LAUN topic (0x2000 + 5)
                let tail := sload(0x2055) // Scheduler tail for LAUN topic (0x2050 + 5)
                
                if lt(head, tail) {
                    let cacheKey := keccak256(add(0x1000, head), 32)
                    let word0 := sload(cacheKey)
                    
                    // Validate LAUN magic header (bytes 0-3)
                    let magic := shr(224, word0)
                    if eq(magic, 0x4c41554e) {
                        // Extract target Card ID (bytes 4-7)
                        let targetCard := and(shr(192, word0), 0xFFFFFFFF)
                        
                        // Extract 20-byte target binary contract address (bytes 8-27)
                        let binaryAddr := and(word0, 0x000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
                        
                        // Execute process spawner loading sequence
                        spawnProcess(targetCard, binaryAddr)
                    }
                    
                    // Consume launch message block
                    sstore(0x2005, add(head, 1))
                }
            }

            // Performs raw bytecode copy and initializes guest process context
            function spawnProcess(cardId, binaryAddr) {
                if gt(cardId, 5) { leave } // Cap Card ID at 5 (6 slots: 0 to 5)
                if iszero(extcodesize(binaryAddr)) { leave }
                
                // Copy guest binary into RAM memory window for this Card (represented at 0x8000 + cardId * 0x1000)
                let destOffset := add(0x8000, mul(cardId, 0x1000))
                let codeSize := extcodesize(binaryAddr)
                if gt(codeSize, 4096) { codeSize := 4096 } // Cap at 4KB RAM page size limit
                
                let ptr := mload(0x40)
                extcodecopy(binaryAddr, ptr, 0, codeSize)
                
                // Copy loaded code directly to the card's transient RAM storage slots
                for { let offset := 0 } lt(offset, codeSize) { offset := add(offset, 32) } {
                    sstore(add(destOffset, offset), mload(add(ptr, offset)))
                }

                // Initialize control block registers for fresh start
                let pcbBase := getPcbOffset(cardId)
                sstore(add(pcbBase, 0), 0)          // PC (IP) = 0 (Start vector)
                sstore(add(pcbBase, 1), 0xFD)       // SP
                sstore(add(pcbBase, 2), 0)          // A
                sstore(add(pcbBase, 3), 0)          // X
                sstore(add(pcbBase, 4), 0)          // Y
                sstore(add(pcbBase, 5), 0)          // SR
                sstore(add(pcbBase, 6), 1)          // Status = Active
                sstore(add(pcbBase, 7), 0)          // Idle state = 0 (Ready)
            }

            // Scans the 6 card slots to find a runnable/ready process context
            function findReadyCard() -> cardId {
                cardId := 0xFFFFFFFF
                let currentHead := sload(0x10) // Read scheduler queue pointer
                
                for { let i := 0 } lt(i, 6) { i := add(i, 1) } {
                    let candidate := mod(add(currentHead, i), 6)
                    let pcbBase := getPcbOffset(candidate)
                    let status := sload(add(pcbBase, 6)) // Status flag

                    // Process is active
                    if eq(status, 1) {
                        // Check if its Winchester LUN inbox queue has pending messages to wake it
                        let mqHead := sload(add(0x2000, candidate)) // Inbox Head register index
                        let mqTail := sload(add(0x2050, candidate)) // Inbox Tail register index
                        
                        if or(lt(mqHead, mqTail), iszero(sload(add(pcbBase, 7)))) {
                            cardId := candidate
                            break
                        }
                    }
                }
            }

            function getPcbOffset(cardId) -> offset {
                // Pre-allocates a 16-slot control block region per Card starting at 0x6000
                offset := add(0x6000, mul(cardId, 16))
            }

            function saveCardContext(cardId, pcbBase) {
                // Host VM emulator updates transient slots on execution exit. Latch and save.
                sstore(add(pcbBase, 0), sload(0x100)) // IP
                sstore(add(pcbBase, 1), sload(0x101)) // SP
                sstore(add(pcbBase, 2), sload(0x102)) // A
                sstore(add(pcbBase, 3), sload(0x103)) // X
                sstore(add(pcbBase, 4), sload(0x104)) // Y
                sstore(add(pcbBase, 5), sload(0x105)) // SR
                sstore(add(pcbBase, 7), sload(0x106)) // Idle status state
            }

            // Interface stub to trigger Guest CPU execution thunk
            function executeGuestVmSlice(cardId, ip, sp, a, x, y, sr, stepLimit) -> exitReason, steps {
                // Place context registers into global ZMM CPU thunk registers
                sstore(0x100, ip)
                sstore(0x101, sp)
                sstore(0x102, a)
                sstore(0x103, x)
                sstore(0x104, y)
                sstore(0x105, sr)
                
                // Emulated execution slice loop thunk. Returns step counts and yield signals.
                // For test verification purposes, dry-runs execute mock steps.
                steps := stepLimit
                exitReason := 1 // Yielded
            }
        }
    }
}
