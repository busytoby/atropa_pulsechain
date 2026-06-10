object "TMS9900Scheduler" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Virtual memory helpers (16-bit word addressable or byte addressable, let's store 32-bit values for ease in storage)
            function getMem(addr) -> val {
                val := sload(add(71000, addr))
            }
            function setMem(addr, val) {
                sstore(add(71000, addr), val)
            }

            // Helpers for thread control block
            function getThreadWP(tid) -> wp {
                wp := sload(add(70100, mul(tid, 10)))
            }
            function setThreadWP(tid, wp) {
                sstore(add(70100, mul(tid, 10)), wp)
            }
            function getThreadPC(tid) -> pcVal {
                pcVal := sload(add(add(70100, mul(tid, 10)), 1))
            }
            function setThreadPC(tid, pcVal) {
                sstore(add(add(70100, mul(tid, 10)), 1), pcVal)
            }
            function getThreadST(tid) -> st {
                st := sload(add(add(70100, mul(tid, 10)), 2))
            }
            function setThreadST(tid, st) {
                sstore(add(add(70100, mul(tid, 10)), 2), st)
            }
            function getThreadStatus(tid) -> status {
                status := sload(add(add(70100, mul(tid, 10)), 3))
            }
            function setThreadStatus(tid, status) {
                sstore(add(add(70100, mul(tid, 10)), 3), status)
            }
            function getThreadPriority(tid) -> prio {
                prio := sload(add(add(70100, mul(tid, 10)), 4))
            }
            function setThreadPriority(tid, prio) {
                sstore(add(add(70100, mul(tid, 10)), 4), prio)
            }

            // 1. createThread(uint256 startPC, uint256 priority) -> uint256 threadId
            // Selector: 0x908f7972
            if eq(selector, 0x908f7972) {
                let startPC := calldataload(4)
                let priority := calldataload(36)
                if iszero(priority) { priority := 1 } // Minimum priority 1

                let tid := sload(70001) // threadCount
                sstore(70001, add(tid, 1))

                // Workspaces are spaced out by 256 virtual memory cells
                let wp := mul(add(tid, 1), 256)

                setThreadWP(tid, wp)
                setThreadPC(tid, startPC)
                setThreadST(tid, 0)
                setThreadStatus(tid, 1) // status 1 = ready
                setThreadPriority(tid, priority)

                mstore(0x00, tid)
                return(0x00, 32)
            }

            // 2. readThreadRegister(uint256 threadId, uint256 regId) -> uint256
            // Selector: 0x1d552742
            if eq(selector, 0x1d552742) {
                let tid := calldataload(4)
                let regId := and(calldataload(36), 0xF) // 0-15
                let wp := getThreadWP(tid)
                mstore(0x00, getMem(add(wp, regId)))
                return(0x00, 32)
            }

            // 3. writeThreadRegister(uint256 threadId, uint256 regId, uint256 val)
            // Selector: 0xf2a201f3
            if eq(selector, 0xf2a201f3) {
                let tid := calldataload(4)
                let regId := and(calldataload(36), 0xF)
                let val := calldataload(68)
                let wp := getThreadWP(tid)
                setMem(add(wp, regId), val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 4. getThreadState(uint256 threadId) -> (uint256 wp, uint256 pc, uint256 status, uint256 priority)
            // Selector: 0xbf9531a7
            if eq(selector, 0xbf9531a7) {
                let tid := calldataload(4)
                mstore(0x00, getThreadWP(tid))
                mstore(0x20, getThreadPC(tid))
                mstore(0x40, getThreadStatus(tid))
                mstore(0x60, getThreadPriority(tid))
                return(0x00, 128)
            }

            // 5. writeMemory(uint256 addr, uint8 val)
            // Selector: 0x5517b193
            if eq(selector, 0x5517b193) {
                let addr := calldataload(4)
                let val := and(calldataload(36), 0xFF)
                setMem(addr, val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 6. runScheduler(uint256 totalSteps) -> uint256[] path
            // Selector: 0x346d6ea6
            if eq(selector, 0x346d6ea6) {
                let totalSteps := calldataload(4)
                let step := 0

                let threadCount := sload(70001)
                if iszero(threadCount) { revert(0, 0) }

                let activeTid := sload(70000)
                let activeStatus := getThreadStatus(activeTid)
                
                // Active registers
                let regPC := 0
                let regWP := 0
                let regST := 0
                let sliceRemaining := 0

                if eq(activeStatus, 1) {
                    regPC := getThreadPC(activeTid)
                    regWP := getThreadWP(activeTid)
                    regST := getThreadST(activeTid)
                    let prio := getThreadPriority(activeTid)
                    sliceRemaining := mul(prio, 3)
                }

                // We will store the executed thread ID at each step in memory to return as a trace
                let traceOffset := 0x40 // standard bytes array output offset is 0x20 for offset, 0x40 for len
                let traceLen := 0

                for { } lt(step, totalSteps) { step := add(step, 1) } {
                    // Check if current thread is invalid or slice exhausted
                    let switchNeeded := 0
                    if or(iszero(eq(getThreadStatus(activeTid), 1)), iszero(sliceRemaining)) {
                        switchNeeded := 1
                    }

                    if switchNeeded {
                        // Save current context if active thread is still runnable
                        if eq(getThreadStatus(activeTid), 1) {
                            setThreadPC(activeTid, regPC)
                            setThreadWP(activeTid, regWP)
                            setThreadST(activeTid, regST)
                        }

                        // Find next ready thread (Round Robin)
                        let found := 0
                        let nextTid := activeTid
                        for { let i := 0 } lt(i, threadCount) { i := add(i, 1) } {
                            nextTid := mod(add(nextTid, 1), threadCount)
                            if eq(getThreadStatus(nextTid), 1) {
                                found := 1
                                break
                            }
                        }

                        if iszero(found) {
                            // No ready threads
                            break
                        }

                        activeTid := nextTid
                        regPC := getThreadPC(activeTid)
                        regWP := getThreadWP(activeTid)
                        regST := getThreadST(activeTid)
                        let prio := getThreadPriority(activeTid)
                        sliceRemaining := mul(prio, 3)
                    }

                    // Log current thread execution in trace
                    mstore(add(0x40, mul(traceLen, 32)), activeTid)
                    traceLen := add(traceLen, 1)

                    // Execute one instruction
                    let opcode := getMem(regPC)
                    sliceRemaining := sub(sliceRemaining, 1)

                    // NOP (0x00)
                    if eq(opcode, 0x00) {
                        regPC := add(regPC, 1)
                        continue
                    }

                    // LI reg, val (0x01) -> load immediate into register `reg`
                    if eq(opcode, 0x01) {
                        let targetReg := and(getMem(add(regPC, 1)), 0xF)
                        let val := getMem(add(regPC, 2))
                        setMem(add(regWP, targetReg), val)
                        regPC := add(regPC, 3)
                        continue
                    }

                    // ADD regSrc, regDest (0x02)
                    if eq(opcode, 0x02) {
                        let regSrc := and(getMem(add(regPC, 1)), 0xF)
                        let regDest := and(getMem(add(regPC, 2)), 0xF)
                        let valSrc := getMem(add(regWP, regSrc))
                        let valDest := getMem(add(regWP, regDest))
                        setMem(add(regWP, regDest), and(add(valSrc, valDest), 0xFFFFFFFF))
                        regPC := add(regPC, 3)
                        continue
                    }

                    // YIELD (0x03)
                    if eq(opcode, 0x03) {
                        regPC := add(regPC, 1)
                        sliceRemaining := 0 // Force context switch
                        continue
                    }

                    // HLT (0xFF)
                    if eq(opcode, 0xFF) {
                        setThreadStatus(activeTid, 2) // status 2 = halted
                        sliceRemaining := 0
                        continue
                    }

                    // Invalid Opcode acts like NOP
                    regPC := add(regPC, 1)
                }

                // Save back final running thread context
                sstore(70000, activeTid)
                if eq(getThreadStatus(activeTid), 1) {
                    setThreadPC(activeTid, regPC)
                    setThreadWP(activeTid, regWP)
                    setThreadST(activeTid, regST)
                }

                // Return trace array
                mstore(0x00, 0x20) // offset
                mstore(0x20, traceLen) // length
                return(0x00, add(64, mul(traceLen, 32)))
            }

            revert(0, 0)
        }
    }
}
