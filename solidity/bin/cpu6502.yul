object "CPU6502Emulator" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy runtime code to memory and return it
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    // ========================================================================
    // RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Storage Map for CPU Registers:
            // Slot 0x80: Accumulator A
            // Slot 0x81: X Register
            // Slot 0x82: Y Register
            // Slot 0x83: Stack Pointer (SP)
            // Slot 0x84: Status Register (SR) - Carry (bit 0), Zero (bit 1), Negative (bit 7)
            // Slot 0x85: Program Counter (PC)

            
            // Helper to get non-overlapping memory offset for C64 registers

            // Helper to resolve context user
            function getContextUser() -> user {
                user := mload(0x1F0)
                if iszero(user) {
                    user := caller()
                }
            }

            // Helper to get strictly private storage slot for internal registers (avoiding recursion in getUserSlot)
            function getUserSlotPrivate(addr) -> slot {
                let user := getContextUser()
                mstore(0x00, user)
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }

            // Helper to get namespaced storage slot for a specific user and C64 address
            function getUserSlot(addr) -> slot {
                let user := getContextUser()
                
                // Check if shared screen mode is enabled at register $D50D (54541)
                let sharedMode := and(sload(getUserSlotPrivate(54541)), 0xFF)
                if sharedMode {
                    // Shared ranges: Screen RAM (1024-2047), VIC registers (53248-53311), Color RAM (55296-56295)
                    if or(and(iszero(lt(addr, 1024)), lt(addr, 2048)),
                          or(and(iszero(lt(addr, 53248)), lt(addr, 53312)),
                             and(iszero(lt(addr, 55296)), lt(addr, 56296)))) {
                        user := 0x5555555555555555555555555555555555555555
                    }
                }

                mstore(0x00, user)
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }

            // Helper to resolve double-hashed storage slot for gallery drawings
            function getGallerySlot(user, gallerySlotIndex, offset) -> slot {
                mstore(0x00, user)
                mstore(0x20, gallerySlotIndex)
                let hash1 := keccak256(0x00, 64)
                mstore(0x00, hash1)
                mstore(0x20, offset)
                slot := keccak256(0x00, 64)
            }

            // Helper to query and excise tax via on-chain ERC20 first, fallback to slot 848 thereafter.
            function exciseOnChainTax(taxAmount) -> taxPaidSuccess {
                taxPaidSuccess := 0
                // Target token address: dynamically reconstructed from $D590 - $D5A3 (54672 to 54691)
                let tokenAddress := 0
                for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                    let b := sload(getUserSlot(add(54672, i)))
                    tokenAddress := or(shl(8, tokenAddress), and(b, 0xFF))
                }

                // If tokenAddress is valid (not 0), try on-chain ERC20 logic first
                if tokenAddress {
                    // Let's format call to transferFrom(from, to, amount)
                    // selector: 0x23b872dd
                    // from: getContextUser()
                    // to: 0x1111111111111111111111111111111111111111 (Treasury Address)
                    // amount: taxAmount * 10**18 (standard token mapping)
                    let treasury := 0x1111111111111111111111111111111111111111
                    let weiAmount := mul(taxAmount, 1000000000000000000)

                    // Store call arguments in scratch memory: 0x340 onwards (preserving register caches at 0x80-0x120)
                    mstore(0x340, shl(224, 0x23b872dd))
                    mstore(0x344, getContextUser())
                    mstore(0x364, treasury)
                    mstore(0x384, weiAmount)

                    // Call transferFrom(getContextUser(), treasury, weiAmount)
                    let callSuccess := call(gas(), tokenAddress, 0, 0x340, 100, 0x3A4, 32)
                    if callSuccess {
                        // Check if transferFrom returned true
                        if mload(0x3A4) {
                            taxPaidSuccess := 1
                        }
                    }
                }

                // Fallback to legacy C64 memory-mapped RAM address $0350 (slot 848) if on-chain failed/not present
                if iszero(taxPaidSuccess) {
                    let userBal := sload(getUserSlot(848))
                    if or(gt(userBal, taxAmount), eq(userBal, taxAmount)) {
                        sstore(getUserSlot(848), sub(userBal, taxAmount))
                        
                        mstore(0x300, 0x1111111111111111111111111111111111111111)
                        mstore(0x320, 848)
                        let treasurySlot := keccak256(0x300, 64)
                        let treasuryBal := sload(treasurySlot)
                        sstore(treasurySlot, add(treasuryBal, taxAmount))
                        
                        taxPaidSuccess := 1
                    }
                }
            }
            function regMemOffset(slot) -> offset {
                switch slot
                case 0x80 { offset := 0x80 }
                case 0x81 { offset := 0xA0 }
                case 0x82 { offset := 0xC0 }
                case 0x83 { offset := 0xE0 }
                case 0x84 { offset := 0x100 }
                case 0x85 { offset := 0x120 }
                default { offset := 0 }
            }

            function getReg(slot) -> val {
                val := mload(regMemOffset(slot))
            }

            function setReg(slot, val) {
                mstore(regMemOffset(slot), val)
            }

            // Load registers from storage to memory cache
            function loadRegisterCache() {
                mstore(0x80, sload(getUserSlot(0x80)))
                mstore(0xA0, sload(getUserSlot(0x81)))
                mstore(0xC0, sload(getUserSlot(0x82)))
                mstore(0xE0, sload(getUserSlot(0x83)))
                mstore(0x100, sload(getUserSlot(0x84)))
                mstore(0x120, sload(getUserSlot(0x85)))
            }

            // Write back registers from memory cache to storage
            function saveRegisterCache() {
                sstore(getUserSlot(0x80), mload(0x80))
                sstore(getUserSlot(0x81), mload(0xA0))
                sstore(getUserSlot(0x82), mload(0xC0))
                sstore(getUserSlot(0x83), mload(0xE0))
                sstore(getUserSlot(0x84), mload(0x100))
                sstore(getUserSlot(0x85), mload(0x120))
            }
// Helper function to update Zero and Negative flags based on value
            function updateFlags(val) {
                let sr := getReg(0x84)
                
                // Clear Zero (bit 1) and Negative (bit 7) flags
                sr := and(sr, 0x7D)

                // Set Zero flag if val is 0
                if iszero(val) {
                    sr := or(sr, 0x02)
                }

                // Set Negative flag if bit 7 of val is set
                if and(val, 0x80) {
                    sr := or(sr, 0x80)
                }

                setReg(0x84, sr)
            }

            // Helper function to get Carry flag
            function getCarry() -> carry {
                carry := and(getReg(0x84), 0x01)
            }

            // Helper function to set Carry flag
            function setCarry(val) {
                let sr := getReg(0x84)
                sr := and(sr, 0xFE) // Clear Carry
                if val {
                    sr := or(sr, 0x01)
                }
                setReg(0x84, sr)
            }

            // Helper function to push to stack
            function pushStack(val) {
                let sp := getReg(0x83)
                // C64 Stack mapped to 0x100 - 0x1FF
                sstore(getUserSlot(add(0x100, sp)), and(val, 0xFF))
                setReg(0x83, and(sub(sp, 1), 0xFF))
            }

            // Helper function to pull from stack
            function pullStack() -> val {
                let sp := and(add(getReg(0x83), 1), 0xFF)
                val := sload(getUserSlot(add(0x100, sp)))
                setReg(0x83, sp)
            }

            // Helper function to fetch the current 6509 data bank
            function getDataBank() -> bank {
                bank := 15
                let cpuMode := sload(getUserSlot(54540))
                if eq(cpuMode, 1) {
                    bank := sload(getUserSlot(1))
                    if iszero(bank) { bank := 15 }
                }
            }

            // Helper function to read from memory/registers (handling memory-mapped I/O and bank-switching)
            function readMemoryBanked(addr, bank) -> val {
                // Apply cartridge bank-switching for range $8000-$9FFF (32768-40959) using bank register at $D500 (54528)
                if and(iszero(lt(addr, 32768)), lt(addr, 40960)) {
                    let bankVal := sload(getUserSlot(54528))
                    addr := add(addr, mul(bankVal, 8192))
                }

                switch addr
                case 54552 { // AABB Collision Detection coprocessor output ($D518)
                    let x1 := sload(getUserSlot(54544))
                    let y1 := sload(getUserSlot(54545))
                    let w1 := sload(getUserSlot(54546))
                    let h1 := sload(getUserSlot(54547))
                    let x2 := sload(getUserSlot(54548))
                    let y2 := sload(getUserSlot(54549))
                    let w2 := sload(getUserSlot(54550))
                    let h2 := sload(getUserSlot(54551))
                    let cond1 := lt(x1, add(x2, w2))
                    let cond2 := lt(x2, add(x1, w1))
                    let cond3 := lt(y1, add(y2, h2))
                    let cond4 := lt(y2, add(y1, h1))
                    if and(and(cond1, cond2), and(cond3, cond4)) {
                        val := 1
                    }
                }
                case 54562 { // Ring Buffer Size ($D522)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    val := and(sub(add(tail, 32), head), 31)
                }
                case 54564 { // Last Popped Value ($D524)
                    val := sload(getUserSlot(54564))
                }
                case 54794 { // $D60A (TMS5220 Status register)
                    val := sload(getUserSlot(54794))
                }
                case 54795 { // $D60B (TMS5220 FIFO length)
                    val := sload(getUserSlot(54795))
                }
                case 53770 { // $D20A (POKEY Random Register - LFSR)
                    let lfsr := sload(getUserSlotPrivate(53770))
                    if iszero(lfsr) { lfsr := 0x1F } // Seed
                    let bit := xor(and(lfsr, 1), and(shr(4, lfsr), 1))
                    lfsr := and(or(shr(1, lfsr), shl(8, bit)), 0x1FF)
                    sstore(getUserSlotPrivate(53770), lfsr)
                    val := and(lfsr, 0xFF)
                }
                case 56321 { // $DC01 (CIA 1 Data Port B - Keyboard row read)
                    let colMask := sload(getUserSlot(56320))
                    let rowMask := 0xFF
                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        // Shift 1 by i bits to check active column
                        if iszero(and(colMask, shl(i, 1))) {
                            let rowVal := sload(getUserSlot(add(56350, i)))
                            if iszero(rowVal) {
                                rowVal := 0xFF
                            }
                            rowMask := and(rowMask, rowVal)
                        }
                    }
                    val := rowMask
                }
                default {
                    let targetAddr := addr
                    let cpuMode := sload(getUserSlot(54540))
                    if and(and(eq(cpuMode, 1), iszero(lt(addr, 512))), and(lt(addr, 65536), or(lt(addr, 53248), gt(addr, 57343)))) {
                        targetAddr := add(addr, mul(bank, 65536))
                    }
                    val := sload(getUserSlot(targetAddr))
                }
            }

            // Wrapper to read using default/instruction bank (Code Bank)
            function readMemory(addr) -> val {
                let codeBank := 15
                let cpuMode := sload(getUserSlot(54540))
                if eq(cpuMode, 1) {
                    codeBank := sload(getUserSlot(0))
                    if iszero(codeBank) { codeBank := 15 }
                }
                val := readMemoryBanked(addr, codeBank)
            }

            // Helper function to write to memory/registers (handling memory-mapped I/O and bank-switching)
            function writeMemoryBanked(addr, val, bank) {
                // Apply cartridge bank-switching for range $8000-$9FFF (32768-40959) using bank register at $D500 (54528)
                if and(iszero(lt(addr, 32768)), lt(addr, 40960)) {
                    let c64bank := sload(getUserSlot(54528))
                    addr := add(addr, mul(c64bank, 8192))
                }

                // --- Doodle Graphics Coprocessor Helpers ---
                function getSin(angle) -> s {
                    angle := mod(angle, 16)
                    switch angle
                    case 0 { s := 0 }
                    case 1 { s := 38 }
                    case 2 { s := 70 }
                    case 3 { s := 92 }
                    case 4 { s := 100 }
                    case 5 { s := 92 }
                    case 6 { s := 70 }
                    case 7 { s := 38 }
                    case 8 { s := 0 }
                    case 9 { s := sub(0, 38) }
                    case 10 { s := sub(0, 70) }
                    case 11 { s := sub(0, 92) }
                    case 12 { s := sub(0, 100) }
                    case 13 { s := sub(0, 92) }
                    case 14 { s := sub(0, 70) }
                    case 15 { s := sub(0, 38) }
                }

                function getCos(angle) -> c {
                    angle := mod(angle, 16)
                    switch angle
                    case 0 { c := 100 }
                    case 1 { c := 92 }
                    case 2 { c := 70 }
                    case 3 { c := 38 }
                    case 4 { c := 0 }
                    case 5 { c := sub(0, 38) }
                    case 6 { c := sub(0, 70) }
                    case 7 { c := sub(0, 92) }
                    case 8 { c := sub(0, 100) }
                    case 9 { c := sub(0, 92) }
                    case 10 { c := sub(0, 70) }
                    case 11 { c := sub(0, 38) }
                    case 12 { c := 0 }
                    case 13 { c := 38 }
                    case 14 { c := 70 }
                    case 15 { c := 92 }
                }

                function getSignedCoord(slot) -> coord {
                    let raw := and(sload(getUserSlot(slot)), 0xFF)
                    coord := raw
                    if gt(raw, 127) {
                        coord := sub(raw, 256)
                    }
                }

                function plotPixel(x, y, color) {
                    if or(gt(x, 319), gt(y, 199)) { leave }
                    let cellX := div(x, 8)
                    let cellY := div(y, 8)
                    let lineY := mod(y, 8)
                    let bitIdx := sub(7, mod(x, 8))
                    let byteAddr := add(8192, add(add(mul(cellY, 320), mul(cellX, 8)), lineY))
                    let currentByte := and(sload(getUserSlot(byteAddr)), 0xFF)
                    let newByte := 0
                    if color {
                        newByte := or(currentByte, shl(bitIdx, 1))
                    }
                    if iszero(color) {
                        newByte := and(currentByte, not(shl(bitIdx, 1)))
                    }
                    sstore(getUserSlot(byteAddr), and(newByte, 0xFF))
                }

                function drawLine(x0, y0, x1, y1, color) {
                    let absDx := 0
                    let sx := 1
                    if gt(x1, x0) { absDx := sub(x1, x0) }
                    if iszero(gt(x1, x0)) {
                        absDx := sub(x0, x1)
                        sx := sub(0, 1)
                    }

                    let absDy := 0
                    let sy := 1
                    if gt(y1, y0) { absDy := sub(y1, y0) }
                    if iszero(gt(y1, y0)) {
                        absDy := sub(y0, y1)
                        sy := sub(0, 1)
                    }

                    let err := sub(absDx, absDy)
                    let x := x0
                    let y := y0

                    for { let steps := 0 } lt(steps, 1000) { steps := add(steps, 1) } {
                        plotPixel(x, y, color)
                        if and(eq(x, x1), eq(y, y1)) { break }
                        let e2 := mul(err, 2)
                        let negAbsDy := sub(0, absDy)
                        if sgt(e2, negAbsDy) {
                            err := sub(err, absDy)
                            x := add(x, sx)
                        }
                        if slt(e2, absDx) {
                            err := add(err, absDx)
                            y := add(y, sy)
                        }
                    }
                }

                function plotCirclePoints(xc, yc, px, py, col) {
                    plotPixel(add(xc, px), add(yc, py), col)
                    plotPixel(sub(xc, px), add(yc, py), col)
                    plotPixel(add(xc, px), sub(yc, py), col)
                    plotPixel(sub(xc, px), sub(yc, py), col)
                    plotPixel(add(xc, py), add(yc, px), col)
                    plotPixel(sub(xc, py), add(yc, px), col)
                    plotPixel(add(xc, py), sub(yc, px), col)
                    plotPixel(sub(xc, py), sub(yc, px), col)
                }

                function drawCircle(x0, y0, r, color) {
                    let x := 0
                    let y := r
                    let d := sub(3, mul(2, r))

                    plotCirclePoints(x0, y0, x, y, color)
                    for { } sgt(y, x) { } {
                        x := add(x, 1)
                        if sgt(d, 0) {
                            y := sub(y, 1)
                            d := add(d, sub(mul(4, sub(x, y)), 10))
                        }
                        if iszero(sgt(d, 0)) {
                            d := add(d, add(mul(4, x), 6))
                        }
                        plotCirclePoints(x0, y0, x, y, color)
                    }
                }

                switch addr
                case 54560 { // Ring Buffer Push ($D520)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    let nextTail := and(add(tail, 1), 31)
                    if iszero(eq(nextTail, head)) {
                        sstore(getUserSlot(add(54567, tail)), val)
                        sstore(getUserSlot(54566), nextTail)
                    }
                }
                case 54561 { // Trigger Ring Buffer Pop transaction ($D521)
                    let head := sload(getUserSlot(54565))
                    let tail := sload(getUserSlot(54566))
                    if iszero(eq(head, tail)) {
                        let poppedVal := sload(getUserSlot(add(54567, head)))
                        sstore(getUserSlot(54564), poppedVal) // Save to last popped value
                        head := and(add(head, 1), 31)
                        sstore(getUserSlot(54565), head) // Advance head
                    }
                }
                case 54563 { // Ring Buffer Clear ($D523)
                    sstore(getUserSlot(54565), 0)
                    sstore(getUserSlot(54566), 0)
                }
                case 53265 { // $D011 (VIC-II Control Register 1)
                    // Bit 7 is the high bit of the raster compare target
                    sstore(getUserSlot(53301), and(val, 0x80))
                    sstore(getUserSlot(53265), val)
                }
                case 53266 { // $D012 (VIC-II Raster Line compare / read raster)
                    // Low 8 bits of the raster compare target
                    sstore(getUserSlot(53300), and(val, 0xFF))
                }
                case 53273 { // $D019 (Interrupt Flag Register - Write 1 to clear)
                    let currentVal := sload(getUserSlot(53273))
                    let newVal := and(and(currentVal, not(val)), 0x7F) // Clear bit 7 first
                    let d01aVal := sload(getUserSlot(53274))
                    if and(newVal, d01aVal) {
                        newVal := or(newVal, 0x80)
                    }
                    sstore(getUserSlot(53273), newVal)
                }
                case 54784 { // $D600 (Votrax Phoneme register)
                    // Excise 1 OTRT unit Diyat Speech Tax per phoneme
                    let speechTax := 1
                    if exciseOnChainTax(speechTax) {
                        log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), speechTax)
                    }
                    sstore(getUserSlot(54784), val)
                    sstore(getUserSlot(54786), 1) // Set Busy to 1
                    let cnt := sload(getUserSlot(54787))
                    sstore(getUserSlot(54787), add(cnt, 1))
                }
                case 54792 { // $D608 (TMS5220 Command register)
                    sstore(getUserSlot(54792), val)
                    let cmdType := and(val, 0xF0)
                    if eq(cmdType, 0x50) { // Speak External
                        sstore(getUserSlot(54794), 0xE0) // Status: TS=1, BL=1, BE=1
                        sstore(getUserSlot(54795), 0)    // Clear FIFO length
                    }
                    if eq(cmdType, 0x90) { // Reset
                        sstore(getUserSlot(54794), 0x00) // Clear Status
                        sstore(getUserSlot(54795), 0)    // Clear FIFO length
                    }
                }
                case 54793 { // $D609 (TMS5220 Data register / FIFO)
                    let fifoLen := sload(getUserSlot(54795))
                    if lt(fifoLen, 16) {
                        sstore(getUserSlot(add(55936, fifoLen)), val)
                        fifoLen := add(fifoLen, 1)
                        sstore(getUserSlot(54795), fifoLen)
                    }
                    let status := 0x80 // TS=1
                    if lt(fifoLen, 4) {
                        status := or(status, 0x40) // BL=1
                    }
                    sstore(getUserSlot(54794), status)
                }
                case 54927 { // Cannonball Blitz Physics Trigger ($D68F)
                    sstore(getUserSlot(54927), val)
                    if val {
                        let px := sload(getUserSlot(54912))
                        let py := sload(getUserSlot(54913))
                        let pvx := sload(getUserSlot(54914))
                        let pvy := sload(getUserSlot(54915))
                        
                        // Treat velocities as signed
                        if gt(pvx, 127) { pvx := sub(pvx, 256) }
                        if gt(pvy, 127) { pvy := sub(pvy, 256) }
                        
                        // Apply movement
                        px := add(px, pvx)
                        py := add(py, pvy)
                        
                        // Keep within boundaries
                        if gt(px, 39) { px := 39 }
                        if gt(py, 24) { py := 24 }
                        
                        // Simple Gravity: if player is not on scaffold row (8, 16, 24), pull down
                        let onScaffold := 0
                        if eq(py, 8) { onScaffold := 1 }
                        if eq(py, 16) { onScaffold := 1 }
                        if eq(py, 24) { onScaffold := 1 }
                        if iszero(onScaffold) {
                            py := add(py, 1) // Fall down
                        }
                        
                        // Save Player coordinates
                        sstore(getUserSlot(54912), px)
                        sstore(getUserSlot(54913), py)
                        
                        // Update Rolling Cannonball 1
                        let ox := sload(getUserSlot(54916))
                        let oy := sload(getUserSlot(54917))
                        let active := sload(getUserSlot(54918))
                        let collided := 0
                        
                        if active {
                            ox := add(ox, 1)
                            if gt(ox, 39) {
                                ox := 0
                            }
                            sstore(getUserSlot(54916), ox)
                            
                            // Check Collision
                            if and(eq(px, ox), eq(py, oy)) {
                                collided := 1
                                sstore(getUserSlot(54928), 1) // Set collision flag
                                // Reset player position to start (0, 24)
                                sstore(getUserSlot(54912), 0)
                                sstore(getUserSlot(54913), 24)
                            }
                        }
                        
                        // Update score if no collision
                        if iszero(collided) {
                            let score := sload(getUserSlot(54925))
                            sstore(getUserSlot(54925), add(score, 10))
                        }
                        
                        // Charge 1 OTRT Arcade Play Tax
                        let playTax := 1
                        if exciseOnChainTax(playTax) {
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), playTax)
                        }
                        
                        // Emit CannonballStep event log
                        let currentScore := sload(getUserSlot(54925))
                        mstore(0x300, currentScore)
                        mstore(0x320, collided)
                        log2(0x300, 64, 0xbc8ad72f2ba16a9205578bc41b22e18413b63200ff9dfe8bc9c8be7ce6823908, getContextUser())
                        
                        sstore(getUserSlot(54927), 0) // Clear trigger
                    }
                }
                case 54949 { // Hometax Calculation Trigger ($D6A5)
                    sstore(getUserSlot(54949), val)
                    if val {
                        // Read 32-bit gross income from slots 54944-54947
                        let gross := 0
                        for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54944, i)))
                            gross := or(shl(8, gross), and(b, 0xFF))
                        }

                        // Apply deductions
                        let dedCategory := sload(getUserSlot(54948))
                        let ded := 0
                        if eq(dedCategory, 1) { ded := 1000 } // Single
                        if eq(dedCategory, 2) { ded := 2000 } // Joint
                        if eq(dedCategory, 3) { ded := 5000 } // Corporation

                        let net := 0
                        if gt(gross, ded) {
                            net := sub(gross, ded)
                        }

                        // Bracket tax rate
                        let tax := 0
                        if net {
                            let rate := 10 // 10%
                            if gt(net, 10000) {
                                rate := 15 // 15%
                            }
                            tax := div(mul(net, rate), 100)
                        }

                        // Store 32-bit tax result across 54950-54953
                        sstore(getUserSlot(54950), and(shr(24, tax), 0xFF))
                        sstore(getUserSlot(54951), and(shr(16, tax), 0xFF))
                        sstore(getUserSlot(54952), and(shr(8, tax), 0xFF))
                        sstore(getUserSlot(54953), and(tax, 0xFF))

                        sstore(getUserSlot(54949), 0) // Clear trigger
                    }
                }
                case 55024 { // Smurfs Physics Trigger ($D6B0)
                    sstore(getUserSlot(55024), val)
                    if val {
                        let isGameOver := sload(getUserSlot(55034))
                        let isGameWin := sload(getUserSlot(55035))
                        if and(iszero(isGameOver), iszero(isGameWin)) {
                            // Read Player pos & velocity
                            let px := sload(getUserSlot(55027))
                            let py := sload(getUserSlot(55028))
                            let pvy := sload(getUserSlot(55029))
                            let jumping := sload(getUserSlot(55030))
                            let energy := sload(getUserSlot(55031))
                            let score := sload(getUserSlot(55032))
                            let screen := sload(getUserSlot(55033))
                            
                            // Apply signed coordinate conversion for velocity
                            if gt(pvy, 127) { pvy := sub(pvy, 256) }

                            // Read Inputs
                            let moveDir := sload(getUserSlot(55025))
                            let jumpTrigger := sload(getUserSlot(55026))

                            // Horizontal movement
                            if eq(moveDir, 1) { // Left
                                if gt(px, 15) { px := sub(px, 3) }
                            }
                            if eq(moveDir, 2) { // Right
                                px := add(px, 3)
                                if gt(px, 785) { // Transition screen
                                    if lt(screen, 3) {
                                        screen := add(screen, 1)
                                        px := 20
                                        score := add(score, 500)
                                    }
                                    if eq(screen, 3) {
                                        if gt(px, 785) { px := 785 }
                                    }
                                }
                            }

                            // Jump physics (Evolved: Double Jump support)
                            if jumpTrigger {
                                let canJump := 0
                                if iszero(jumping) {
                                    jumping := 1
                                    canJump := 1
                                }
                                // Double Jump logic: slot 55042 holds jump counter
                                let jumpCount := sload(getUserSlot(55042))
                                if and(jumping, lt(jumpCount, 2)) {
                                    jumpCount := add(jumpCount, 1)
                                    sstore(getUserSlot(55042), jumpCount)
                                    canJump := 1
                                }
                                
                                if canJump {
                                    pvy := sub(0, 11) // Slighly lower impulse for double jump control
                                    sstore(getUserSlot(55036), 1) // Sound Strobe: 1 (jump)
                                }
                                sstore(getUserSlot(55026), 0) // Clear trigger
                            }
                            
                            if jumping {
                                py := add(py, pvy)
                                pvy := add(pvy, 1) // Gravity
                                if or(gt(py, 520), eq(py, 520)) {
                                    py := 520
                                    pvy := 0
                                    jumping := 0
                                    sstore(getUserSlot(55042), 0) // Reset jump count on ground
                                }
                            }

                            // Obstacle & collision check per screen
                            if eq(screen, 1) {
                                // Fence at x = 360, height 25 (y=495)
                                if and(and(gt(px, 345), lt(px, 375)), or(gt(py, 495), eq(py, 495))) {
                                    if gt(energy, 20) { energy := sub(energy, 20) }
                                    if iszero(gt(energy, 20)) { energy := 0 }
                                    if gt(px, 30) { px := sub(px, 30) }
                                    sstore(getUserSlot(55036), 2) // Hit Sound
                                }
                                // Rock at x = 600, height 15 (y=505)
                                if and(and(gt(px, 580), lt(px, 620)), or(gt(py, 505), eq(py, 505))) {
                                    if gt(energy, 25) { energy := sub(energy, 25) }
                                    if iszero(gt(energy, 25)) { energy := 0 }
                                    if gt(px, 30) { px := sub(px, 30) }
                                    sstore(getUserSlot(55036), 2) // Hit Sound
                                }
                            }

                            if eq(screen, 2) {
                                // Stalagmite at x = 400, height 30 (y=490)
                                if and(and(gt(px, 380), lt(px, 420)), or(gt(py, 490), eq(py, 490))) {
                                    if gt(energy, 20) { energy := sub(energy, 20) }
                                    if iszero(gt(energy, 20)) { energy := 0 }
                                    if gt(px, 30) { px := sub(px, 30) }
                                    sstore(getUserSlot(55036), 2) // Hit Sound
                                }
                                
                                // Bat movement
                                let bat_x := sload(getUserSlot(55037))
                                let bat_y := sload(getUserSlot(55038))
                                let bat_vx := sload(getUserSlot(55039))
                                if gt(bat_vx, 127) { bat_vx := sub(bat_vx, 256) }
                                
                                bat_x := add(bat_x, bat_vx)
                                if or(lt(bat_x, 150), gt(bat_x, 650)) {
                                    bat_vx := sub(0, bat_vx)
                                }
                                sstore(getUserSlot(55037), bat_x)
                                sstore(getUserSlot(55039), and(bat_vx, 0xFF))

                                // Bat collision (distance squared check)
                                let dx := sub(px, bat_x)
                                let dy := sub(sub(py, 20), bat_y)
                                let dist_sq := add(mul(dx, dx), mul(dy, dy))
                                if lt(dist_sq, 784) { // 28^2
                                    if gt(energy, 30) { energy := sub(energy, 30) }
                                    if iszero(gt(energy, 30)) { energy := 0 }
                                    if gt(px, 30) { px := sub(px, 30) }
                                    sstore(getUserSlot(55036), 2) // Hit Sound
                                }
                            }

                            if eq(screen, 3) {
                                // Gargamel movement
                                let gargamel_x := sload(getUserSlot(55040))
                                let gargamel_vx := sload(getUserSlot(55041))
                                if gt(gargamel_vx, 127) { gargamel_vx := sub(gargamel_vx, 256) }

                                gargamel_x := add(gargamel_x, gargamel_vx)
                                if or(lt(gargamel_x, 250), gt(gargamel_x, 600)) {
                                    gargamel_vx := sub(0, gargamel_vx)
                                }
                                sstore(getUserSlot(55040), gargamel_x)
                                sstore(getUserSlot(55041), and(gargamel_vx, 0xFF))

                                // Gargamel collision
                                if and(and(gt(px, sub(gargamel_x, 20)), lt(px, add(gargamel_x, 20))), or(gt(py, 480), eq(py, 480))) {
                                    if gt(energy, 40) { energy := sub(energy, 40) }
                                    if iszero(gt(energy, 40)) { energy := 0 }
                                    if gt(px, 40) { px := sub(px, 40) }
                                    sstore(getUserSlot(55036), 2) // Hit Sound
                                }

                                // Win Check (Cage x = 680)
                                if or(gt(px, 665), eq(px, 665)) {
                                    isGameWin := 1
                                    score := add(score, 5000)
                                    sstore(getUserSlot(55036), 3) // Win Sound
                                }
                            }

                            // Update energy/gameover
                            if iszero(energy) {
                                isGameOver := 1
                                sstore(getUserSlot(55036), 4) // GameOver Sound
                            }

                            // Save positions
                            sstore(getUserSlot(55027), px)
                            sstore(getUserSlot(55028), py)
                            sstore(getUserSlot(55029), and(pvy, 0xFF))
                            sstore(getUserSlot(55030), jumping)
                            sstore(getUserSlot(55031), energy)
                            sstore(getUserSlot(55032), score)
                            sstore(getUserSlot(55033), screen)
                            sstore(getUserSlot(55034), isGameOver)
                            sstore(getUserSlot(55035), isGameWin)
                        }
                        
                        sstore(getUserSlot(55024), 0) // Clear trigger
                    }
                }
                case 54954 { // Hometax Payment Release Trigger ($D6AA)
                    sstore(getUserSlot(54954), val)
                    if val {
                        // Read 32-bit tax from 54950-54953
                        let tax := 0
                        for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54950, i)))
                            tax := or(shl(8, tax), and(b, 0xFF))
                        }

                        if tax {
                            // Charge tax using exciseOnChainTax
                            let taxPaidSuccess := exciseOnChainTax(tax)
                            if taxPaidSuccess {
                                // Read gross income for logging
                                let gross := 0
                                for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                                    let b := sload(getUserSlot(add(54944, i)))
                                    gross := or(shl(8, gross), and(b, 0xFF))
                                }
                                
                                // Emit HometaxSettled event log
                                mstore(0x300, gross)
                                mstore(0x320, tax)
                                log2(0x300, 64, 0x32e57424af98ac3cd87222f4bb36060bb284d1bdd8f9dfe1b49b71f9ee25e999, getContextUser())
                                
                                // Reset Net taxable registers
                                sstore(getUserSlot(54950), 0)
                                sstore(getUserSlot(54951), 0)
                                sstore(getUserSlot(54952), 0)
                                sstore(getUserSlot(54953), 0)
                            }
                        }
                        sstore(getUserSlot(54954), 0) // Clear strobe
                    }
                }
                case 54979 { // Sierra AGI Command Selector ($D6C3)
                    sstore(getUserSlot(54979), val)
                    if val {
                        // 1. Room Transition Check
                        if eq(val, 1) {
                            let x := sload(getUserSlot(54976))
                            let y := sload(getUserSlot(54977))
                            let room := sload(getUserSlot(54978))
                            
                            let newRoom := room
                            let changed := 0
                            
                            // Exit West
                            if lt(x, 5) {
                                newRoom := sub(room, 1)
                                sstore(getUserSlot(54976), 150) // Warp to East edge
                                changed := 1
                            }
                            // Exit East
                            if gt(x, 155) {
                                newRoom := add(room, 1)
                                sstore(getUserSlot(54976), 10) // Warp to West edge
                                changed := 1
                            }
                            
                            if changed {
                                // Excise 1 OTRT Diyat Game Tax on room transition
                                let playTax := 1
                                if exciseOnChainTax(playTax) {
                                    log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), playTax)
                                }
                                sstore(getUserSlot(54978), newRoom)
                                let flags := sload(getUserSlot(54983))
                                sstore(getUserSlot(54983), or(flags, 4))
                            }
                        }
                        
                        // 2. Parser / Interaction Check
                        if eq(val, 2) {
                            let itemID := sload(getUserSlot(54980))
                            let room := sload(getUserSlot(54978))
                            let success := 0
                            
                            // Room 1 (Forest Cleave) -> Gold Key (ID: 10) at position X: [70, 90], Y: [90, 110]
                            if eq(room, 1) {
                                if eq(itemID, 10) {
                                    let x := sload(getUserSlot(54976))
                                    let y := sload(getUserSlot(54977))
                                    if and(and(gt(x, 69), lt(x, 91)), and(gt(y, 89), lt(y, 111))) {
                                        let inv := sload(getUserSlot(54981))
                                        sstore(getUserSlot(54981), or(inv, 1)) // Get Gold Key
                                        success := 1
                                    }
                                }
                            }
                            
                            // Room 2 (Castle Gate) -> Castle Door (ID: 20) at position X: [50, 70], Y: [120, 140]
                            if eq(room, 2) {
                                if eq(itemID, 20) {
                                    let x := sload(getUserSlot(54976))
                                    let y := sload(getUserSlot(54977))
                                    if and(and(gt(x, 49), lt(x, 71)), and(gt(y, 119), lt(y, 141))) {
                                        let inv := sload(getUserSlot(54981))
                                        if and(inv, 1) { // Requires Gold Key (Bit 0)
                                            sstore(getUserSlot(54981), or(inv, 2)) // Castle Unlocked
                                            success := 1
                                        }
                                    }
                                }
                            }
                            
                            let flags := sload(getUserSlot(54983))
                            if success {
                                sstore(getUserSlot(54983), or(flags, 2))
                                
                                // Emit SierraAgiSettled event log (Topic: 0xe11ca69da21867c2d5885c07342880a18413b63200ff9dfe8bc9c8be7ce68239)
                                mstore(0x300, room)
                                mstore(0x320, itemID)
                                log2(0x300, 64, 0xe11ca69da21867c2d5885c07342880a18413b63200ff9dfe8bc9c8be7ce68239, getContextUser())
                            }
                            if iszero(success) {
                                sstore(getUserSlot(54983), and(flags, not(2)))
                            }
                        }
                        sstore(getUserSlot(54979), 0)
                    }
                }
                case 55000 { // Data-Pac Flat-File Search Strobe ($D6D8)
                    sstore(getUserSlot(55000), val)
                    if val {
                        // 1. Read Record Start Address from 54992-54993
                        let startAddr := or(shl(8, and(sload(getUserSlot(54992)), 0xFF)), and(sload(getUserSlot(54993)), 0xFF))
                        
                        // 2. Read Record Count (54994) and Size (54995)
                        let recordCount := and(sload(getUserSlot(54994)), 0xFF)
                        let recordSize := and(sload(getUserSlot(54995)), 0xFF)

                        // 3. Read 4-byte Query Key from 54996-54999
                        let queryKey := 0
                        for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54996, i)))
                            queryKey := or(shl(8, queryKey), and(b, 0xFF))
                        }

                        let matchAddr := 0xFFFF // Default: Not Found
                        let found := 0

                        // 4. Scan records
                        for { let i := 0 } and(lt(i, recordCount), iszero(found)) { i := add(i, 1) } {
                            let recAddr := add(startAddr, mul(i, recordSize))
                            
                            // Load first 4 bytes of record to construct the key
                            let recordKey := 0
                            for { let k := 0 } lt(k, 4) { k := add(k, 1) } {
                                let b := readMemory(add(recAddr, k))
                                recordKey := or(shl(8, recordKey), and(b, 0xFF))
                            }

                            // Compare
                            if eq(recordKey, queryKey) {
                                matchAddr := recAddr
                                found := 1
                            }
                        }

                        // 5. Store 16-bit match address in 55001-55002
                        sstore(getUserSlot(55001), and(shr(8, matchAddr), 0xFF))
                        sstore(getUserSlot(55002), and(matchAddr, 0xFF))

                        sstore(getUserSlot(55000), 0) // Clear strobe
                    }
                }
                case 54966 { // Wireframe Projection Trigger ($D6B6)
                    sstore(getUserSlot(54966), val)
                    if val {
                        let pitch := sload(getUserSlot(54960))
                        let yaw := sload(getUserSlot(54961))
                        let roll := sload(getUserSlot(54962))
                        let scale := sload(getUserSlot(54963))
                        let vertexCount := sload(getUserSlot(54964))
                        let lineCount := sload(getUserSlot(54965))

                        if iszero(scale) { scale := 100 }

                        // 1. Calculate projected 2D coordinates for all vertices and cache in scratch memory
                        // We store coordinates in scratch memory at 0x500 + i * 8
                        for { let i := 0 } lt(i, vertexCount) { i := add(i, 1) } {
                            let x := getSignedCoord(add(55280, mul(i, 3)))
                            let y := getSignedCoord(add(55281, mul(i, 3)))
                            let z := getSignedCoord(add(55282, mul(i, 3)))

                            // Rotate Roll (Z axis)
                            let sinR := getSin(roll)
                            let cosR := getCos(roll)
                            let x1 := sdiv(sub(mul(x, cosR), mul(y, sinR)), 100)
                            let y1 := sdiv(add(mul(x, sinR), mul(y, cosR)), 100)

                            // Rotate Yaw (Y axis)
                            let sinY := getSin(yaw)
                            let cosY := getCos(yaw)
                            let x2 := sdiv(add(mul(x1, cosY), mul(z, sinY)), 100)
                            let z1 := sdiv(sub(mul(z, cosY), mul(x1, sinY)), 100)

                            // Rotate Pitch (X axis)
                            let sinP := getSin(pitch)
                            let cosP := getCos(pitch)
                            let y2 := sdiv(sub(mul(y1, cosP), mul(z1, sinP)), 100)
                            let z2 := sdiv(add(mul(y1, sinP), mul(z1, cosP)), 100)

                            // Perspective projection
                            let zDist := add(z2, 150)
                            if iszero(gt(zDist, 0)) { zDist := 1 }
                            let projX := add(160, sdiv(mul(x2, scale), zDist))
                            let projY := add(100, sdiv(mul(y2, scale), zDist))

                            mstore(add(0x500, mul(i, 64)), projX)
                            mstore(add(0x520, mul(i, 64)), projY)

                            sstore(getUserSlot(add(55800, mul(i, 2))), projX)
                            sstore(getUserSlot(add(55801, mul(i, 2))), projY)
                        }

                        // 2. Clear Doodle Graphics Framebuffer
                        for { let idx := 0 } lt(idx, 8000) { idx := add(idx, 1) } {
                            sstore(getUserSlot(add(8192, idx)), 0)
                        }

                        // 3. Connect vertices with lines
                        for { let j := 0 } lt(j, lineCount) { j := add(j, 1) } {
                            let fromIdx := and(sload(getUserSlot(add(55536, mul(j, 2)))), 0xFF)
                            let toIdx := and(sload(getUserSlot(add(55537, mul(j, 2)))), 0xFF)

                            if and(lt(fromIdx, vertexCount), lt(toIdx, vertexCount)) {
                                let x0 := mload(add(0x500, mul(fromIdx, 64)))
                                let y0 := mload(add(0x520, mul(fromIdx, 64)))
                                let x1 := mload(add(0x500, mul(toIdx, 64)))
                                let y1 := mload(add(0x520, mul(toIdx, 64)))

                                drawLine(x0, y0, x1, y1, 1)
                            }
                        }

                        // 4. Charge 1 OTRT wireframe tax
                        let tax := 1
                        if exciseOnChainTax(tax) {
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), tax)
                        }

                        // Emit WireframeRendered event log
                        mstore(0x300, lineCount)
                        log2(0x300, 32, 0xee6c1cb417ad0a8b94df8c8a14b0368b634882dfba51912bc5527e0291f24d1a, getContextUser())

                        sstore(getUserSlot(54967), 1) // Status: Success
                        sstore(getUserSlot(54966), 0) // Clear trigger
                    }
                }
                case 55013 { // Homeword Render Trigger ($D6E5)
                    sstore(getUserSlot(55013), val)
                    if val {
                        // 1. Clear Screen RAM (1024 to 2023)
                        for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                            sstore(getUserSlot(add(1024, i)), 32)
                        }
                        
                        // 2. Draw Page Border
                        // Top & Bottom Border (rows 2 and 22, columns 10 to 30)
                        for { let col := 10 } lt(col, 31) { col := add(col, 1) } {
                            sstore(getUserSlot(add(1024, add(mul(2, 40), col))), 160)
                            sstore(getUserSlot(add(1024, add(mul(22, 40), col))), 160)
                        }
                        // Left & Right Border (rows 3 to 21, columns 10 and 30)
                        for { let row := 3 } lt(row, 22) { row := add(row, 1) } {
                            sstore(getUserSlot(add(1024, add(mul(row, 40), 10))), 160)
                            sstore(getUserSlot(add(1024, add(mul(row, 40), 30))), 160)
                        }

                        // 3. Format Spooled Text
                        let left := sload(getUserSlot(55008))
                        let right := sload(getUserSlot(55009))
                        let just := sload(getUserSlot(55010))
                        let spacing := sload(getUserSlot(55011))
                        let len := sload(getUserSlot(55012))
                        
                        if iszero(spacing) { spacing := 1 }
                        if iszero(right) { right := 70 }
                        
                        // Scale margins to 18 columns wide page boundary (columns 11 to 28)
                        let pageLeft := div(mul(left, 18), 80)
                        let pageRight := div(mul(right, 18), 80)
                        if gt(pageLeft, 17) { pageLeft := 0 }
                        if gt(pageRight, 18) { pageRight := 18 }
                        if lt(pageRight, add(pageLeft, 2)) { pageRight := add(pageLeft, 2) }
                        
                        let lineWidth := sub(pageRight, pageLeft)
                        let startRow := 3
                        let currentRow := startRow
                        let currentCol := 0
                        
                        let textIdx := 0
                        for { } lt(textIdx, len) { } {
                            if gt(currentRow, 21) { break }
                            
                            // Read character from buffer
                            let char := and(sload(getUserSlot(add(55024, textIdx))), 0xFF)
                            textIdx := add(textIdx, 1)
                            
                            // Convert to PETSCII character line representation
                            let previewChar := 45 // Hyphen
                            if eq(char, 32) { previewChar := 32 } // Space remains space
                            
                            let colOffset := add(11, pageLeft)
                            sstore(getUserSlot(add(1024, add(mul(currentRow, 40), add(colOffset, currentCol)))), previewChar)
                            
                            currentCol := add(currentCol, 1)
                            if eq(currentCol, lineWidth) {
                                currentCol := 0
                                currentRow := add(currentRow, spacing)
                            }
                        }

                        // 4. Charge 2 OTRT Spooling Tax
                        let tax := 2
                        if exciseOnChainTax(tax) {
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), tax)
                        }
                        
                        // Emit HomewordRendered event log
                        mstore(0x300, len)
                        log2(0x300, 32, 0x50fdb39d7be5a0438cf1dfba392019ff88d8b634882dfba51912bc5527e0291f, getContextUser())
                        
                        sstore(getUserSlot(55014), 1) // Status: Success
                        sstore(getUserSlot(55013), 0) // Clear strobe
                    }
                }
                case 54624 { // Trigger EVM Contract Call ($D560)
                    sstore(getUserSlot(54624), val)
                    if val {
                         let targetContract := 0
                         for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                             let b := sload(getUserSlot(add(54576, i)))
                             targetContract := or(shl(8, targetContract), and(b, 0xFF))
                         }

                         let methodSel := 0
                         for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                             let b := sload(getUserSlot(add(54596, i)))
                             methodSel := or(shl(8, methodSel), and(b, 0xFF))
                         }

                         let arg1Addr := 0
                         for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                             let b := sload(getUserSlot(add(54600, i)))
                             arg1Addr := or(shl(8, arg1Addr), and(b, 0xFF))
                         }

                         let arg2Val := 0
                         for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                             let b := sload(getUserSlot(add(54620, i)))
                             arg2Val := or(shl(8, arg2Val), and(b, 0xFF))
                         }

                          // Prepare call in memory at 0x300
                          mstore(0x300, shl(224, methodSel))
                          mstore(0x304, arg1Addr)
                          mstore(0x324, arg2Val)

                          // Debug: Save variables
                          sstore(getUserSlot(54630), 12345)
                          sstore(getUserSlot(54631), methodSel)
                          sstore(getUserSlot(54632), arg1Addr)
                          sstore(getUserSlot(54633), arg2Val)
                          sstore(getUserSlot(54634), caller())

                          // Execute Call
                          let success := call(gas(), targetContract, 0, 0x300, 68, 0x344, 32)
                          sstore(getUserSlot(54625), success)

                          if success {
                              let retVal := mload(0x344)
                              sstore(getUserSlot(54626), and(shr(24, retVal), 0xFF))
                              sstore(getUserSlot(54627), and(shr(16, retVal), 0xFF))
                              sstore(getUserSlot(54628), and(shr(8, retVal), 0xFF))
                              sstore(getUserSlot(54629), and(retVal, 0xFF))
                          }
                     }
                 }
                case 54648 { // Trigger Doodle Graphics Coprocessor ($D578)
                    sstore(getUserSlot(54648), val)
                    if val {
                        let cmd := and(sload(getUserSlot(54640)), 0xFF)
                        let x0 := or(shl(8, and(sload(getUserSlot(54642)), 0xFF)), and(sload(getUserSlot(54641)), 0xFF))
                        let y0 := and(sload(getUserSlot(54643)), 0xFF)
                        let x1 := or(shl(8, and(sload(getUserSlot(54645)), 0xFF)), and(sload(getUserSlot(54644)), 0xFF))
                        let y1 := and(sload(getUserSlot(54646)), 0xFF)
                        let color := and(sload(getUserSlot(54647)), 0xFF)

                        switch cmd
                        case 1 { // Draw Pixel
                            plotPixel(x0, y0, color)
                        }
                        case 2 { // Draw Line
                            drawLine(x0, y0, x1, y1, color)
                        }
                        case 3 { // Draw Circle (Radius is x1)
                            drawCircle(x0, y0, x1, color)
                        }
                        case 4 { // Clear Canvas
                            for { let i := 0 } lt(i, 8192) { i := add(i, 1) } {
                                sstore(getUserSlot(add(8192, i)), 0)
                            }
                        }
                        case 5 { // Copy Framebuffer (Fill Screen & Color RAM)
                            for { let i := 0 } lt(i, 200) { i := add(i, 1) } {
                                sstore(getUserSlot(add(1024, i)), color)
                                sstore(getUserSlot(add(55296, i)), color)
                            }
                        }
                    }
                }
                case 54700 { // Trigger COMTAX Coprocessor ($D5AC)
                    if val {
                        let tokenAddress := 0
                        for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54672, i)))
                            tokenAddress := or(shl(8, tokenAddress), and(b, 0xFF))
                        }

                        let blockStart := 0
                        for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54692, i)))
                            blockStart := or(shl(8, blockStart), and(b, 0xFF))
                        }

                        let blockEnd := 0
                        for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54696, i)))
                            blockEnd := or(shl(8, blockEnd), and(b, 0xFF))
                        }

                        let rate := and(sload(getUserSlot(54705)), 0xFF)

                        // Call balanceOf(getContextUser()) to get balance
                        mstore(0x300, shl(224, 0x70a08231))
                        mstore(0x304, getContextUser())
                        let success := staticcall(gas(), tokenAddress, 0x300, 36, 0x344, 32)
                        
                        sstore(getUserSlot(54720), tokenAddress)
                        sstore(getUserSlot(54721), success)
                        
                        let tokenBal := 0
                        if success {
                            tokenBal := mload(0x344)
                        }
                        // Gains = rawBal * (blockEnd - blockStart) / 10000
                        let blockDiff := 0
                        if gt(blockEnd, blockStart) {
                            blockDiff := sub(blockEnd, blockStart)
                        }

                        sstore(getUserSlot(54722), tokenBal)
                        sstore(getUserSlot(54723), getContextUser())
                        sstore(getUserSlot(54724), blockDiff)

                        // Normalize 18 decimals back to raw unit
                        let rawBal := div(tokenBal, 1000000000000000000)

                        let gains := div(mul(rawBal, blockDiff), 10000)
                        let taxDue := div(mul(gains, rate), 100)

                        // Save results
                        sstore(getUserSlot(54701), and(shr(24, gains), 0xFF))
                        sstore(getUserSlot(54702), and(shr(16, gains), 0xFF))
                        sstore(getUserSlot(54703), and(shr(8, gains), 0xFF))
                        sstore(getUserSlot(54704), and(gains, 0xFF))

                        sstore(getUserSlot(54706), and(shr(24, taxDue), 0xFF))
                        sstore(getUserSlot(54707), and(shr(16, taxDue), 0xFF))
                        sstore(getUserSlot(54708), and(shr(8, taxDue), 0xFF))
                        sstore(getUserSlot(54709), and(taxDue, 0xFF))

                        // Dynamic Diyat: Automatically excise the calculated tax due
                        if gt(taxDue, 0) {
                            if exciseOnChainTax(taxDue) {
                                sstore(getUserSlot(54735), 1)
                                log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), taxDue)
                            }
                        }
                    }
                }
                case 54725 { // Trigger Matrix Math Coprocessor ($D5C5)
                    sstore(getUserSlot(54725), val)
                    if val {
                        let matrixPtr := or(shl(8, and(sload(getUserSlot(54721)), 0xFF)), and(sload(getUserSlot(54720)), 0xFF))
                        let rows := and(sload(getUserSlot(54722)), 0xFF)
                        let cols := and(sload(getUserSlot(54723)), 0xFF)
                        let op := and(sload(getUserSlot(54724)), 0xFF)
                        
                        let total := 0
                        let size := mul(rows, cols)
                        
                        switch op
                        case 1 { // Sum
                            for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                                let cellVal := and(sload(getUserSlot(add(matrixPtr, i))), 0xFF)
                                total := add(total, cellVal)
                            }
                        }
                        case 2 { // Average
                            for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                                let cellVal := and(sload(getUserSlot(add(matrixPtr, i))), 0xFF)
                                total := add(total, cellVal)
                            }
                            if gt(size, 0) {
                                total := div(total, size)
                            }
                        }
                        case 3 { // Dot Product
                            for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                                let valA := and(sload(getUserSlot(add(matrixPtr, i))), 0xFF)
                                let valB := and(sload(getUserSlot(add(add(matrixPtr, size), i))), 0xFF)
                                total := add(total, mul(valA, valB))
                            }
                        }
                        
                        // Store 32-bit result to $D5C6 - $D5C9 (54726 to 54729)
                        sstore(getUserSlot(54726), and(shr(24, total), 0xFF))
                        sstore(getUserSlot(54727), and(shr(16, total), 0xFF))
                        sstore(getUserSlot(54728), and(shr(8, total), 0xFF))
                        sstore(getUserSlot(54729), and(total, 0xFF))
                    }
                }
                case 54711 { // Trigger Parallel Bus / IEEE-488 ($D5B7)
                    sstore(getUserSlot(54711), val)
                    if val {
                        let ramPtr := or(shl(8, and(sload(getUserSlot(54704)), 0xFF)), and(sload(getUserSlot(54705)), 0xFF))
                        let length := or(shl(8, and(sload(getUserSlot(54706)), 0xFF)), and(sload(getUserSlot(54707)), 0xFF))
                        let filePtr := or(shl(8, and(sload(getUserSlot(54708)), 0xFF)), and(sload(getUserSlot(54709)), 0xFF))
                        let opCode := and(sload(getUserSlot(54710)), 0xFF)
                        
                        let diskAddress := 0
                        for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54712, i)))
                            diskAddress := or(shl(8, diskAddress), and(b, 0xFF))
                        }
                        
                        // Parallel Bus Diyat Tax: 1 unit per 64 bytes transferred (min 1 unit)
                        let busTax := add(div(length, 64), 1)
                        if iszero(exciseOnChainTax(busTax)) { revert(0, 0) }
                        
                        // Log payment of Parallel Bus tax
                        log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), busTax)
                        
                        mstore(0x300, shl(224, 0x9812a4df))
                        mstore(0x304, 32)
                        let payloadPtr := 0x344
                        
                        switch opCode
                        case 1 { // Read
                            mstore8(payloadPtr, 0x52)
                            mstore8(add(payloadPtr, 1), 0x30)
                            mstore8(add(payloadPtr, 2), 0x3a)
                            let nameLen := 0
                            for {} lt(nameLen, 32) { nameLen := add(nameLen, 1) } {
                                let char := and(sload(getUserSlot(add(filePtr, nameLen))), 0xFF)
                                mstore8(add(add(payloadPtr, 3), nameLen), char)
                                if iszero(char) { break }
                            }
                            let totalCmdLen := add(3, add(nameLen, 1))
                            mstore(0x324, totalCmdLen)
                            let success := call(gas(), diskAddress, 0, 0x300, add(68, totalCmdLen), 0x300, 1000)
                            if iszero(success) { revert(0, 0) }
                            if success {
                                let retLen := mload(0x320)
                                if gt(retLen, length) { retLen := length }
                                for { let i := 0 } lt(i, retLen) { i := add(i, 1) } {
                                    let b := byte(0, mload(add(0x340, i)))
                                    sstore(getUserSlot(add(ramPtr, i)), b)
                                }
                            }
                        }
                        case 2 { // Write
                            mstore8(payloadPtr, 0x57)
                            mstore8(add(payloadPtr, 1), 0x30)
                            mstore8(add(payloadPtr, 2), 0x3a)
                            let nameLen := 0
                            for {} lt(nameLen, 32) { nameLen := add(nameLen, 1) } {
                                let char := and(sload(getUserSlot(add(filePtr, nameLen))), 0xFF)
                                mstore8(add(add(payloadPtr, 3), nameLen), char)
                                if iszero(char) { break }
                            }
                            let dataStartOffset := add(3, add(nameLen, 1))
                            for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                                let b := and(sload(getUserSlot(add(ramPtr, i))), 0xFF)
                                mstore8(add(add(payloadPtr, dataStartOffset), i), b)
                            }
                            let totalCmdLen := add(dataStartOffset, length)
                            mstore(0x324, totalCmdLen)
                            let success := call(gas(), diskAddress, 0, 0x300, add(68, totalCmdLen), 0x300, 32)
                            if iszero(success) { revert(0, 0) }
                        }
                    }
                }
                case 54735 { // Tax Payment Trigger ($D5CF)
                    sstore(getUserSlot(54735), val)
                    if val {
                        let taxDue := 0
                        taxDue := or(shl(24, and(sload(getUserSlot(54706)), 0xFF)), taxDue)
                        taxDue := or(shl(16, and(sload(getUserSlot(54707)), 0xFF)), taxDue)
                        taxDue := or(shl(8, and(sload(getUserSlot(54708)), 0xFF)), taxDue)
                        taxDue := or(and(sload(getUserSlot(54709)), 0xFF), taxDue)
                        
                        if gt(taxDue, 0) {
                            if exciseOnChainTax(taxDue) {
                                sstore(getUserSlot(54706), 0)
                                sstore(getUserSlot(54707), 0)
                                sstore(getUserSlot(54708), 0)
                                sstore(getUserSlot(54709), 0)
                                
                                sstore(getUserSlot(54735), 1)
                                
                                // Emit TaxPaid event: log3(memOffset, memSize, topic1, topic2, topic3)
                                // Topic1: keccak256("TaxPaid(address,uint256)") = 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925
                                log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), taxDue)
                            }
                        }
                    }
                }
                case 54736 { // Active K-KEYS function key index ($D5D0)
                    if and(gt(val, 0), lt(val, 9)) {
                        sstore(getUserSlot(54736), sub(val, 1)) // Save as 0-7
                    }
                }
                case 54737 { // Macro string length ($D5D1)
                    let activeKey := and(sload(getUserSlot(54736)), 0xFF)
                    sstore(getUserSlot(add(54752, mul(activeKey, 16))), and(val, 0xFF))
                }
                case 54752 { // Nonce Generator Trigger ($D5E0)
                    sstore(getUserSlot(54752), val)
                    if val {
                        let nonceCounter := add(sload(getUserSlot(54800)), 1)
                        sstore(getUserSlot(54800), nonceCounter)
                        
                        mstore(0x300, number())
                        mstore(0x320, timestamp())
                        mstore(0x340, blockhash(sub(number(), 1)))
                        mstore(0x360, getContextUser())
                        mstore(0x380, nonceCounter)
                        let hashVal := keccak256(0x300, 160)
                        
                        for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                            let b := and(byte(i, hashVal), 0xFF)
                            sstore(getUserSlot(add(54752, i)), b)
                        }
                    }
                }
                case 54892 { // Protecto Order desk Strobe ($D66C)
                    sstore(getUserSlot(54892), val)
                    if val {
                        let itemId := sload(getUserSlot(54880)) // Get active catalog selection
                        let price := 0
                        
                        // Resolve prices
                        if eq(itemId, 1) { price := 12 } // B128 Computer
                        if eq(itemId, 2) { price := 8 }  // SFD-1001 Disk Drive
                        if eq(itemId, 3) { price := 6 }  // 1902 Monitor
                        if eq(itemId, 4) { price := 1 }  // Centronics Cable
                        if eq(itemId, 5) { price := 2 }  // Crush, Crumble and Chomp! (Epyx Game)
                        if eq(itemId, 6) { price := 3 }  // Temple of Apshai (Epyx Game)
                        
                        if iszero(price) { revert(0, 0) } // Revert if item invalid
                        
                        // Excise payment + 1 OTRT shipping/ledger tax
                        let totalCost := add(price, 1)
                        let userBal := sload(getUserSlot(848))
                        if lt(userBal, totalCost) { revert(0, 0) }
                        
                        // Deduct balance and credit treasury
                        sstore(getUserSlot(848), sub(userBal, totalCost))
                        
                        mstore(0x300, 0x1111111111111111111111111111111111111111)
                        mstore(0x320, 848)
                        let treasurySlot := keccak256(0x300, 64)
                        sstore(treasurySlot, add(sload(treasurySlot), totalCost))
                        
                        // Log transaction to block index
                        // Topic 1: keccak256("ProtectoOrder(address,uint256,uint256)") = 0x9bcbf7ea2838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b999
                        log3(0, 0, 0x9bcbf7ea2838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b999, getContextUser(), itemId)
                        
                        sstore(getUserSlot(54892), 0) // Clear strobe
                    }
                }
                case 54897 { // Gallery Command Trigger ($D671)
                    sstore(getUserSlot(54897), val)
                    if val {
                        let gSlot := and(sload(getUserSlot(54896)), 0x07) // Selected slot 0-7
                        let user := getContextUser()
                        
                        // Command 1: Save current Doodle Canvas (8192 to 16383 -> 8192 bytes packed as 256 words)
                        if eq(val, 1) {
                            for { let i := 0 } lt(i, 256) { i := add(i, 1) } {
                                let packedVal := 0
                                for { let j := 0 } lt(j, 32) { j := add(j, 1) } {
                                    let pixelVal := and(sload(getUserSlot(add(8192, add(mul(i, 32), j)))), 0xFF)
                                    packedVal := or(shl(8, packedVal), pixelVal)
                                }
                                sstore(getGallerySlot(user, gSlot, i), packedVal)
                            }
                        }
                        // Command 2: Load selected Gallery slot into Doodle Canvas
                        if eq(val, 2) {
                            for { let i := 0 } lt(i, 256) { i := add(i, 1) } {
                                let packedVal := sload(getGallerySlot(user, gSlot, i))
                                for { let j := 0 } lt(j, 32) { j := add(j, 1) } {
                                    let pixelVal := and(shr(mul(sub(31, j), 8), packedVal), 0xFF)
                                    sstore(getUserSlot(add(8192, add(mul(i, 32), j))), pixelVal)
                                }
                            }
                        }
                        // Command 3: Clear selected Gallery slot
                        if eq(val, 3) {
                            for { let i := 0 } lt(i, 256) { i := add(i, 1) } {
                                sstore(getGallerySlot(user, gSlot, i), 0)
                            }
                        }
                        
                        sstore(getUserSlot(54897), 0) // Clear trigger strobe
                    }
                }
                default {
                    // Check if writing to macro string character registers $D5D2 - $D5DF (54738 to 54751)
                    if and(iszero(lt(addr, 54738)), lt(addr, 54752)) {
                        let activeKey := and(sload(getUserSlot(54736)), 0xFF)
                        let charIdx := sub(addr, 54738)
                        sstore(getUserSlot(add(add(54753, mul(activeKey, 16)), charIdx)), and(val, 0xFF))
                    }
                    let targetAddr := addr
                    let cpuMode := sload(getUserSlot(54540))
                    if and(and(eq(cpuMode, 1), iszero(lt(addr, 512))), and(lt(addr, 65536), or(lt(addr, 53248), gt(addr, 57343)))) {
                        targetAddr := add(addr, mul(bank, 65536))
                    }
                    sstore(getUserSlot(targetAddr), val)
                }
            }

            // Wrapper to write using default/instruction bank (Code Bank)
            function writeMemory(addr, val) {
                let codeBank := 15
                let cpuMode := sload(getUserSlot(54540))
                if eq(cpuMode, 1) {
                    codeBank := sload(getUserSlot(0))
                    if iszero(codeBank) { codeBank := 15 }
                }
                writeMemoryBanked(addr, val, codeBank)
            }

            // Helper function to update flags for comparison instructions
            function updateCompareFlags(regVal, operandVal) {
                let sr := getReg(0x84)
                sr := and(sr, 0x7C) // Clear N, Z, C
                if or(gt(regVal, operandVal), eq(regVal, operandVal)) {
                    sr := or(sr, 0x01)
                }
                if eq(regVal, operandVal) {
                    sr := or(sr, 0x02)
                }
                let diff := and(sub(regVal, operandVal), 0xFF)
                if and(diff, 0x80) {
                    sr := or(sr, 0x80)
                }
                setReg(0x84, sr)
            }

            // Core Execution Dispatcher
            function executeInternal(opcode, operand) -> branchTaken {
                branchTaken := 0
                
                switch opcode
                // LDA # (Load Accumulator Immediate)
                case 0xA9 {
                    let val := and(operand, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LDA absolute (Load Accumulator from memory)
                case 0xAD {
                    let val := and(readMemory(operand), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA absolute (Store Accumulator to address)
                case 0x8D {
                    writeMemory(operand, getReg(0x80))
                }
                // AND # (Logical AND with Accumulator Immediate)
                case 0x29 {
                    let val := and(getReg(0x80), and(operand, 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // AND absolute (Logical AND with Accumulator from memory)
                case 0x2D {
                    let val := and(getReg(0x80), and(readMemory(operand), 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // CMP # (Compare Accumulator Immediate)
                case 0xC9 {
                    updateCompareFlags(getReg(0x80), and(operand, 0xFF))
                }
                // CMP absolute (Compare Accumulator from memory)
                case 0xCD {
                    updateCompareFlags(getReg(0x80), and(readMemory(operand), 0xFF))
                }
                // CMP zero-page (Compare Accumulator from Zero Page)
                case 0xC5 {
                    updateCompareFlags(getReg(0x80), and(sload(getUserSlot(and(operand, 0xFF))), 0xFF))
                }
                // CPX # (Compare X Register Immediate)
                case 0xE0 {
                    updateCompareFlags(getReg(0x81), and(operand, 0xFF))
                }
                // CPY # (Compare Y Register Immediate)
                case 0xC0 {
                    updateCompareFlags(getReg(0x82), and(operand, 0xFF))
                }
                // INC absolute (Increment memory address)
                case 0xEE {
                    let val := and(add(readMemory(operand), 1), 0xFF)
                    writeMemory(operand, val)
                    updateFlags(val)
                }
                // DEC absolute (Decrement memory address)
                case 0xCE {
                    let val := and(sub(readMemory(operand), 1), 0xFF)
                    writeMemory(operand, val)
                    updateFlags(val)
                }
                // DEC zero-page (Decrement zero page address)
                case 0xC6 {
                    let addr := and(operand, 0xFF)
                    let val := and(sub(readMemory(addr), 1), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // INC zero-page (Increment zero page address)
                case 0xE6 {
                    let addr := and(operand, 0xFF)
                    let val := and(add(readMemory(addr), 1), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // LDA zero-page (Load Accumulator from Zero Page)
                case 0xA5 {
                    let val := and(sload(getUserSlot(and(operand, 0xFF))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA zero-page (Store Accumulator to Zero Page)
                case 0x85 {
                    sstore(getUserSlot(and(operand, 0xFF)), getReg(0x80))
                }
                // LDX # (Load X Register Immediate)
                case 0xA2 {
                    let val := and(operand, 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LDX absolute (Load X Register from memory)
                case 0xAE {
                    let val := and(readMemory(operand), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // STX absolute (Store X Register to address)
                case 0x8E {
                    writeMemory(operand, getReg(0x81))
                }
                // LDY # (Load Y Register Immediate)
                case 0xA0 {
                    let val := and(operand, 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // STY absolute (Store Y Register to address)
                case 0x8C {
                    writeMemory(operand, getReg(0x82))
                }
                // CLC (Clear Carry Flag)
                case 0x18 {
                    setCarry(0)
                }
                // SEC (Set Carry Flag)
                case 0x38 {
                    setCarry(1)
                }
                // ADC # (Add with Carry)
                case 0x69 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let sum := add(add(a, and(operand, 0xFF)), carry)
                    setCarry(gt(sum, 255))
                    let val := and(sum, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // SBC # (Subtract with Carry)
                case 0xE9 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let diff := sub(sub(a, and(operand, 0xFF)), sub(1, carry))
                    let noBorrow := iszero(and(diff, 0x100))
                    setCarry(noBorrow)
                    let val := and(diff, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // ADC zero-page (Add with Carry)
                case 0x65 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let zpVal := and(sload(getUserSlot(and(operand, 0xFF))), 0xFF)
                    let sum := add(add(a, zpVal), carry)
                    setCarry(gt(sum, 255))
                    let val := and(sum, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // SBC zero-page (Subtract with Carry)
                case 0xE5 {
                    let a := getReg(0x80)
                    let carry := getCarry()
                    let zpVal := and(sload(getUserSlot(and(operand, 0xFF))), 0xFF)
                    let diff := sub(sub(a, zpVal), sub(1, carry))
                    let noBorrow := iszero(and(diff, 0x100))
                    setCarry(noBorrow)
                    let val := and(diff, 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // INX (Increment X)
                case 0xE8 {
                    let val := and(add(getReg(0x81), 1), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // DEX (Decrement X)
                case 0xCA {
                    let val := and(sub(getReg(0x81), 1), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // INY (Increment Y)
                case 0xC8 {
                    let val := and(add(getReg(0x82), 1), 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // DEY (Decrement Y)
                case 0x88 {
                    let val := and(sub(getReg(0x82), 1), 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // TAX (Transfer A to X)
                case 0xAA {
                    let val := getReg(0x80)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // TXA (Transfer X to A)
                case 0x8A {
                    let val := getReg(0x81)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // TAY (Transfer A to Y)
                case 0xA8 {
                    let val := getReg(0x80)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // TYA (Transfer Y to A)
                case 0x98 {
                    let val := getReg(0x82)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // PHA (Push Accumulator)
                case 0x48 {
                    pushStack(getReg(0x80))
                }
                // PLA (Pull Accumulator)
                case 0x68 {
                    let val := pullStack()
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // PHP (Push Processor Status)
                case 0x08 {
                    pushStack(getReg(0x84))
                }
                // PLP (Pull Processor Status)
                case 0x28 {
                    let val := pullStack()
                    setReg(0x84, val)
                }
                // JMP absolute
                case 0x4C {
                    setReg(0x85, operand)
                    branchTaken := 1
                }
                // JSR (Jump to Subroutine)
                case 0x20 {
                    let currentPC := getReg(0x85)
                    let handled := 0
                    
                    switch operand
                    case 65490 { // $FFD2 - CHROUT (Output character in A to screen)
                        let char := getReg(0x80)
                        let cursor := sload(getUserSlot(211)) // $D3 - cursor column
                        let row := sload(getUserSlot(214))    // $D6 - cursor row
                        // Calculate screen RAM address (1024 + row * 40 + cursor)
                        let screenAddr := add(1024, add(mul(row, 40), cursor))
                        
                        // Handle special characters
                        switch char
                        case 13 { // Return key
                            row := add(row, 1)
                            cursor := 0
                        }
                        case 147 { // Clear Screen (CLR)
                            row := 0
                            cursor := 0
                        }
                        default {
                            let screenCode := char
                            if and(iszero(lt(char, 65)), iszero(gt(char, 90))) {
                                screenCode := sub(char, 64) // A-Z -> 1-26
                            }
                            if and(iszero(lt(char, 97)), iszero(gt(char, 122))) {
                                screenCode := sub(char, 96) // a-z -> 1-26
                            }
                            writeMemory(screenAddr, screenCode)
                            // Also write white color (1) to color RAM
                            writeMemory(add(55296, add(mul(row, 40), cursor)), 1)
                            
                            cursor := add(cursor, 1)
                            if iszero(lt(cursor, 40)) {
                                cursor := 0
                               row := add(row, 1)
                            }
                        }
                        
                        if iszero(lt(row, 25)) {
                            row := 0
                        }
                        
                        sstore(getUserSlot(211), cursor)
                        sstore(getUserSlot(214), row)
                        handled := 1
                    }
                    case 65508 { // $FFE4 - GETIN (Get character from keyboard buffer into A)
                        let bufLen := sload(getUserSlot(198))
                        let char := 0
                        if gt(bufLen, 0) {
                            char := sload(getUserSlot(637))
                            
                            // Check if the character is a function key F1-F8 (133 to 140)
                            if and(iszero(lt(char, 133)), lt(char, 141)) {
                                let keyIdx := sub(char, 133) // 0 to 7
                                let macroLen := and(sload(getUserSlot(add(54752, mul(keyIdx, 16)))), 0xFF)
                                if gt(macroLen, 0) {
                                    // Shift existing buffer elements to the right to make space for the macro string
                                    let shiftAmt := sub(macroLen, 1)
                                    let newLen := add(bufLen, shiftAmt)
                                    if gt(newLen, 10) { newLen := 10 }
                                    
                                    for { let j := sub(bufLen, 1) } gt(j, 0) { j := sub(j, 1) } {
                                        let targetIdx := add(j, shiftAmt)
                                        if lt(targetIdx, 10) {
                                            sstore(getUserSlot(add(637, targetIdx)), sload(getUserSlot(add(637, j))))
                                        }
                                    }
                                    
                                    for { let j := 0 } lt(j, macroLen) { j := add(j, 1) } {
                                        let mChar := and(sload(getUserSlot(add(add(54753, mul(keyIdx, 16)), j))), 0xFF)
                                        if lt(j, 10) {
                                            sstore(getUserSlot(add(637, j)), mChar)
                                        }
                                    }
                                    
                                    sstore(getUserSlot(198), newLen)
                                    char := sload(getUserSlot(637))
                                }
                            }
                            
                            // Normal buffer shift
                            bufLen := sload(getUserSlot(198))
                            if gt(bufLen, 0) {
                                for { let i := 0 } lt(i, sub(bufLen, 1)) { i := add(i, 1) } {
                                    sstore(getUserSlot(add(637, i)), sload(getUserSlot(add(638, i))))
                                }
                                sstore(getUserSlot(198), sub(bufLen, 1))
                            }
                        }
                        setReg(0x80, char)
                        updateFlags(char)
                        handled := 1
                    }
                    case 65520 { // $FFF0 - PLOT (Read/set cursor position)
                        let sr := getReg(0x84)
                        let carry := and(sr, 0x01)
                        if iszero(carry) {
                            // Carry is clear: SET cursor position
                            // Y register (0x82) contains row (Y)
                            // X register (0x81) contains column (X)
                            let colVal := getReg(0x81)
                            let rowVal := getReg(0x82)
                            sstore(getUserSlot(211), colVal)
                            sstore(getUserSlot(214), rowVal)
                        }
                        if carry {
                            // Carry is set: READ cursor position
                            // Return row in Y (0x82), column in X (0x81)
                            let colVal := and(sload(getUserSlot(211)), 0xFF)
                            let rowVal := and(sload(getUserSlot(214)), 0xFF)
                            setReg(0x81, colVal)
                            setReg(0x82, rowVal)
                        }
                        handled := 1
                    }
                    
                    if iszero(handled) {
                        let retAddr := add(currentPC, 2)
                        pushStack(shr(8, retAddr))
                        pushStack(and(retAddr, 0xFF))
                        setReg(0x85, operand)
                        branchTaken := 1
                    }
                }
                // RTS (Return from Subroutine)
                case 0x60 {
                    let low := pullStack()
                    let high := pullStack()
                    let target := or(shl(8, high), low)
                    setReg(0x85, add(target, 1))
                    branchTaken := 1
                }
                // BPL (Branch on Plus / Negative flag clear)
                case 0x10 {
                    if iszero(and(getReg(0x84), 0x80)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BMI (Branch on Minus / Negative flag set)
                case 0x30 {
                    if and(getReg(0x84), 0x80) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BVC (Branch on Overflow clear)
                case 0x50 {
                    if iszero(and(getReg(0x84), 0x40)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BVS (Branch on Overflow set)
                case 0x70 {
                    if and(getReg(0x84), 0x40) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BCC (Branch on Carry clear)
                case 0x90 {
                    if iszero(getCarry()) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BCS (Branch on Carry set)
                case 0xB0 {
                    if getCarry() {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BNE (Branch on Not Equal / Zero flag clear)
                case 0xD0 {
                    if iszero(and(getReg(0x84), 0x02)) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // BEQ (Branch on Equal / Zero flag set)
                case 0xF0 {
                    if and(getReg(0x84), 0x02) {
                        branchTaken := executeBranch(operand)
                    }
                }
                // CLI (Clear Interrupt Disable)
                case 0x58 {
                    setReg(0x84, and(getReg(0x84), 0xFB))
                }
                // SEI (Set Interrupt Disable)
                case 0x78 {
                    setReg(0x84, or(getReg(0x84), 0x04))
                }
                // RTI (Return from Interrupt)
                case 0x40 {
                    let sr := pullStack()
                    setReg(0x84, sr)
                    let low := pullStack()
                    let high := pullStack()
                    setReg(0x85, or(shl(8, high), low))
                    branchTaken := 1
                }
                // LDA abs,X
                case 0xBD {
                    let val := and(readMemory(add(operand, getReg(0x81))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LDA abs,Y
                case 0xB9 {
                    let val := and(readMemory(add(operand, getReg(0x82))), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA abs,X
                case 0x9D {
                    writeMemory(add(operand, getReg(0x81)), getReg(0x80))
                }
                // STA abs,Y
                case 0x99 {
                    writeMemory(add(operand, getReg(0x82)), getReg(0x80))
                }
                // LDA zp,X
                case 0xB5 {
                    let addr := and(add(operand, getReg(0x81)), 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA zp,X
                case 0x95 {
                    let addr := and(add(operand, getReg(0x81)), 0xFF)
                    writeMemory(addr, getReg(0x80))
                }
                // LDA (zp),Y
                case 0xB1 {
                    let zp := and(operand, 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemoryBanked(add(base, getReg(0x82)), getDataBank()), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA (zp),Y
                case 0x91 {
                    let zp := and(operand, 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    writeMemoryBanked(add(base, getReg(0x82)), getReg(0x80), getDataBank())
                }
                // LDA (zp,X)
                case 0xA1 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemoryBanked(base, getDataBank()), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // STA (zp,X)
                case 0x81 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    writeMemoryBanked(base, getReg(0x80), getDataBank())
                }
                // LAX zp (0xA7)
                case 0xA7 {
                    let val := and(readMemory(and(operand, 0xFF)), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LAX zp,Y (0xB7)
                case 0xB7 {
                    let addr := and(add(operand, getReg(0x82)), 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LAX abs (0xAF)
                case 0xAF {
                    let val := and(readMemory(operand), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LAX abs,Y (0xBF)
                case 0xBF {
                    let val := and(readMemory(add(operand, getReg(0x82))), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LAX (zp,X) (0xA3)
                case 0xA3 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemoryBanked(base, getDataBank()), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LAX (zp),Y (0xB3)
                case 0xB3 {
                    let zp := and(operand, 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(readMemoryBanked(add(base, getReg(0x82)), getDataBank()), 0xFF)
                    setReg(0x80, val)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // SAX zp (0x87)
                case 0x87 {
                    let val := and(getReg(0x80), getReg(0x81))
                    writeMemory(and(operand, 0xFF), val)
                }
                // SAX zp,Y (0x97)
                case 0x97 {
                    let addr := and(add(operand, getReg(0x82)), 0xFF)
                    let val := and(getReg(0x80), getReg(0x81))
                    writeMemory(addr, val)
                }
                // SAX abs (0x8F)
                case 0x8F {
                    let val := and(getReg(0x80), getReg(0x81))
                    writeMemory(operand, val)
                }
                // SAX (zp,X) (0x83)
                case 0x83 {
                    let zp := and(add(operand, getReg(0x81)), 0xFF)
                    let low := and(readMemory(zp), 0xFF)
                    let high := and(readMemory(and(add(zp, 1), 0xFF)), 0xFF)
                    let base := or(shl(8, high), low)
                    let val := and(getReg(0x80), getReg(0x81))
                    writeMemoryBanked(base, val, getDataBank())
                }
                // ASL Accumulator (0x0A)
                case 0x0A {
                    let a := getReg(0x80)
                    setCarry(and(shr(7, a), 0x01))
                    let val := and(shl(1, a), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // ASL Zero Page (0x06)
                case 0x06 {
                    let addr := and(operand, 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    setCarry(and(shr(7, val), 0x01))
                    val := and(shl(1, val), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // LSR Accumulator (0x4A)
                case 0x4A {
                    let a := getReg(0x80)
                    setCarry(and(a, 0x01))
                    let val := shr(1, a)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LSR Zero Page (0x46)
                case 0x46 {
                    let addr := and(operand, 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    setCarry(and(val, 0x01))
                    val := shr(1, val)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // ROL Accumulator (0x2A)
                case 0x2A {
                    let a := getReg(0x80)
                    let oldCarry := getCarry()
                    setCarry(and(shr(7, a), 0x01))
                    let val := and(or(shl(1, a), oldCarry), 0xFF)
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // ROL Zero Page (0x26)
                case 0x26 {
                    let addr := and(operand, 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    let oldCarry := getCarry()
                    setCarry(and(shr(7, val), 0x01))
                    val := and(or(shl(1, val), oldCarry), 0xFF)
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // ROR Accumulator (0x6A)
                case 0x6A {
                    let a := getReg(0x80)
                    let oldCarry := getCarry()
                    setCarry(and(a, 0x01))
                    let val := or(shr(1, a), shl(7, oldCarry))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // ROR Zero Page (0x66)
                case 0x66 {
                    let addr := and(operand, 0xFF)
                    let val := and(readMemory(addr), 0xFF)
                    let oldCarry := getCarry()
                    setCarry(and(val, 0x01))
                    val := or(shr(1, val), shl(7, oldCarry))
                    writeMemory(addr, val)
                    updateFlags(val)
                }
                // ORA Zero Page (0x05)
                case 0x05 {
                    let val := or(getReg(0x80), and(sload(getUserSlot(and(operand, 0xFF))), 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // ORA Immediate (0x09)
                case 0x09 {
                    let val := or(getReg(0x80), and(operand, 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // EOR Zero Page (0x45)
                case 0x45 {
                    let val := xor(getReg(0x80), and(sload(getUserSlot(and(operand, 0xFF))), 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // EOR Immediate (0x49)
                case 0x49 {
                    let val := xor(getReg(0x80), and(operand, 0xFF))
                    setReg(0x80, val)
                    updateFlags(val)
                }
                // LDX Zero Page (0xA6)
                case 0xA6 {
                    let val := and(readMemory(and(operand, 0xFF)), 0xFF)
                    setReg(0x81, val)
                    updateFlags(val)
                }
                // LDY Zero Page (0xA4)
                case 0xA4 {
                    let val := and(readMemory(and(operand, 0xFF)), 0xFF)
                    setReg(0x82, val)
                    updateFlags(val)
                }
                // Undocumented and Standard NOPs
                case 0xEA {}
                case 0x04 {} case 0x14 {} case 0x34 {} case 0x44 {} case 0x54 {} case 0x64 {} case 0x74 {}
                case 0x80 {} case 0x82 {} case 0x89 {} case 0xC2 {} case 0xD4 {} case 0xE2 {} case 0xF4 {}
                case 0x0C {} case 0x1C {} case 0x3C {} case 0x5C {} case 0x7C {} case 0xDC {} case 0xFC {}
                case 0x1A {} case 0x3A {} case 0x5A {} case 0x7A {} case 0xDA {} case 0xFA {}
                default {
                    revert(0, 0)
                }
            }

            // Helper to compute and apply relative branch offset
            function executeBranch(operand) -> taken {
                let offset := and(operand, 0xFF)
                if and(offset, 0x80) {
                    offset := sub(offset, 256)
                }
                let currentPC := getReg(0x85)
                setReg(0x85, add(add(currentPC, 2), offset))
                taken := 1
            }

            // Helper to get instruction length
            function getOpLength(op) -> len {
                len := 1 // default implicit/accumulator/stack/1-byte NOPs
                
                // 2-byte opcodes
                let is2Byte := or(
                    or(or(or(eq(op, 0xA9), eq(op, 0xA2)), or(eq(op, 0xA0), eq(op, 0x69))),
                       or(or(eq(op, 0xE9), eq(op, 0x10)), or(eq(op, 0x30), or(eq(op, 0x50), or(eq(op, 0x70), or(eq(op, 0x29), or(eq(op, 0xC9), or(eq(op, 0xC5), or(eq(op, 0xE0), or(eq(op, 0xC0), or(eq(op, 0xA5), or(eq(op, 0x85), or(eq(op, 0xC6), eq(op, 0xE6)))))))))))))),
                    or(
                       or(or(eq(op, 0x90), eq(op, 0xB0)), or(eq(op, 0xD0), eq(op, 0xF0))),
                       or(or(or(eq(op, 0xB1), eq(op, 0x91)), or(eq(op, 0xA1), eq(op, 0x81))), or(eq(op, 0xB5), eq(op, 0x95)))
                    )
                )
                
                // Add undocumented LAX/SAX 2-byte opcodes
                is2Byte := or(is2Byte, or(or(or(eq(op, 0xA7), eq(op, 0xB7)), or(eq(op, 0xA3), eq(op, 0xB3))), or(or(eq(op, 0x87), eq(op, 0x97)), eq(op, 0x83))))
                
                // Add new shift/bitwise 2-byte opcodes
                is2Byte := or(is2Byte, or(
                    or(or(or(eq(op, 0x06), eq(op, 0x46)), or(eq(op, 0x26), eq(op, 0x66))),
                       or(or(eq(op, 0x05), eq(op, 0x09)), or(eq(op, 0x45), eq(op, 0x49)))),
                    or(or(or(eq(op, 0xA6), eq(op, 0xA4)), or(eq(op, 0x65), eq(op, 0xE5))), 0)
                ))

                // Add undocumented NOP 2-byte opcodes
                is2Byte := or(is2Byte, or(or(or(or(eq(op, 0x04), eq(op, 0x14)), or(eq(op, 0x34), eq(op, 0x44))), or(or(eq(op, 0x54), eq(op, 0x64)), or(eq(op, 0x74), eq(op, 0x80)))), or(or(or(eq(op, 0x82), eq(op, 0x89)), or(eq(op, 0xC2), eq(op, 0xD4))), or(eq(op, 0xE2), eq(op, 0xF4)))))

                if is2Byte {
                    len := 2
                }

                // 3-byte opcodes
                let is3Byte := or(
                    or(or(or(eq(op, 0xAD), eq(op, 0x8D)), or(eq(op, 0xAE), eq(op, 0x8E))),
                       or(or(eq(op, 0x8C), eq(op, 0x4C)), or(eq(op, 0x20), or(eq(op, 0x2D), or(eq(op, 0xCD), or(eq(op, 0xEE), eq(op, 0xCE))))))),
                    or(or(eq(op, 0xBD), eq(op, 0xB9)), or(eq(op, 0x9D), eq(op, 0x99)))
                )
                
                // Add undocumented LAX/SAX 3-byte opcodes
                is3Byte := or(is3Byte, or(or(eq(op, 0xAF), eq(op, 0xBF)), eq(op, 0x8F)))
                
                // Add undocumented NOP 3-byte opcodes
                is3Byte := or(is3Byte, or(or(or(eq(op, 0x0C), eq(op, 0x1C)), or(eq(op, 0x3C), eq(op, 0x5C))), or(or(eq(op, 0x7C), eq(op, 0xDC)), eq(op, 0xFC))))

                if is3Byte {
                    len := 3
                }
            }

            // ----------------------------------------------------------------
            // Method 1: executeOp(uint8 opcode, uint256 operand) -> uint256
            // Selector: 0x2a9cd5d1
            // ----------------------------------------------------------------
            if eq(selector, 0x2a9cd5d1) {
                loadRegisterCache()
                let opcode := shr(248, calldataload(4))
                let operand := calldataload(36)
                
                let branchTaken := executeInternal(opcode, operand)
                
                if iszero(branchTaken) {
                    let len := getOpLength(opcode)
                    let currentPC := getReg(0x85)
                    setReg(0x85, add(currentPC, len))
                }
                
                saveRegisterCache()
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 2: getCPUState() -> (uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 SP, uint256 PC)
            // Selector: 0xcbe94d1f (actually 0xe0f77c57)
            // ----------------------------------------------------------------
            if eq(selector, 0xe0f77c57) {
                loadRegisterCache()
                mstore(0x00, getReg(0x80))
                mstore(0x20, getReg(0x81))
                mstore(0x40, getReg(0x82))
                mstore(0x60, getReg(0x84))
                mstore(0x80, getReg(0x83))
                mstore(0xA0, getReg(0x85))
                return(0x00, 192)
            }

            // ----------------------------------------------------------------
            // Method 3: runSteps(uint256 maxSteps) -> uint256
            // Selector: 0xc45b1808
            // ----------------------------------------------------------------
            if eq(selector, 0xc45b1808) {
                loadRegisterCache()
                let maxSteps := calldataload(4)
                let stepsExecuted := 0
                let halted := 0

                for { } and(lt(stepsExecuted, maxSteps), iszero(halted)) { stepsExecuted := add(stepsExecuted, 1) } {
                    // Check for pending IRQ interrupt
                    let irqPending := and(sload(getUserSlot(53273)), 0x80)
                    let irqDisable := and(getReg(0x84), 0x04)
                    if and(iszero(iszero(irqPending)), iszero(irqDisable)) {
                        let currentPC := getReg(0x85)
                        // Push PC high and low
                        pushStack(shr(8, currentPC))
                        pushStack(and(currentPC, 0xFF))
                        // Push SR status
                        pushStack(getReg(0x84))
                        // Set I flag
                        setReg(0x84, or(getReg(0x84), 0x04))
                        // Load IRQ vector from $FFFE / $FFFF (65534 / 65535)
                        let lowVector := and(sload(getUserSlot(65534)), 0xFF)
                        let highVector := and(sload(getUserSlot(65535)), 0xFF)
                        setReg(0x85, or(shl(8, highVector), lowVector))
                    }

                    let currentPC := getReg(0x85)
                    
                    let handledHook := 0
                    if eq(currentPC, 65493) { // LOAD ($FFD5)
                        let diskAddress := 0
                        for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54712, i)))
                            diskAddress := or(shl(8, diskAddress), and(b, 0xFF))
                        }
                        if diskAddress {
                            // JiffyDOS KERNAL Hook Tax: 5 OTRT units
                            let hookTax := 5
                            if iszero(exciseOnChainTax(hookTax)) { revert(0, 0) }
                            
                            // Log payment of JiffyDOS hook tax
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), hookTax)
                            let fnPtr := or(shl(8, and(sload(getUserSlot(188)), 0xFF)), and(sload(getUserSlot(187)), 0xFF))
                            let fnLen := and(sload(getUserSlot(183)), 0xFF)
                            let destAddr := or(shl(8, getReg(0x82)), getReg(0x81))
                            
                            mstore(0x300, shl(224, 0x9812a4df))
                            mstore(0x304, 32)
                            let payloadPtr := 0x344
                            mstore8(payloadPtr, 0x52)
                            mstore8(add(payloadPtr, 1), 0x30)
                            mstore8(add(payloadPtr, 2), 0x3a)
                            for { let i := 0 } lt(i, fnLen) { i := add(i, 1) } {
                                let char := and(sload(getUserSlot(add(fnPtr, i))), 0xFF)
                                mstore8(add(add(payloadPtr, 3), i), char)
                            }
                            mstore8(add(add(payloadPtr, 3), fnLen), 0x00)
                            
                            let totalCmdLen := add(4, fnLen)
                            mstore(0x324, totalCmdLen)
                            let success := call(gas(), diskAddress, 0, 0x300, add(68, totalCmdLen), 0x300, 1000)
                            if iszero(success) { revert(0, 0) }
                            if success {
                                let retLen := mload(0x320)
                                for { let i := 0 } lt(i, retLen) { i := add(i, 1) } {
                                    let b := byte(0, mload(add(0x340, i)))
                                    sstore(getUserSlot(add(destAddr, i)), b)
                                }
                                setReg(0x84, and(getReg(0x84), 0xFE)) // Clear Carry
                                let lowPC := pullStack()
                                let highPC := pullStack()
                                let retPC := add(or(shl(8, highPC), lowPC), 1)
                                setReg(0x85, retPC)
                                handledHook := 1
                            }
                        }
                    }
                    if eq(currentPC, 65496) { // SAVE ($FFD8)
                        let diskAddress := 0
                        for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                            let b := sload(getUserSlot(add(54712, i)))
                            diskAddress := or(shl(8, diskAddress), and(b, 0xFF))
                        }
                        if diskAddress {
                            // JiffyDOS KERNAL Hook Tax: 5 OTRT units
                            let hookTax := 5
                            if iszero(exciseOnChainTax(hookTax)) { revert(0, 0) }
                            
                            // Log payment of JiffyDOS hook tax
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), hookTax)
                            let fnPtr := or(shl(8, and(sload(getUserSlot(188)), 0xFF)), and(sload(getUserSlot(187)), 0xFF))
                            let fnLen := and(sload(getUserSlot(183)), 0xFF)
                            let startAddr := or(shl(8, and(sload(getUserSlot(68)), 0xFF)), and(sload(getUserSlot(67)), 0xFF))
                            let endAddr := or(shl(8, getReg(0x82)), getReg(0x81))
                            let saveLen := 0
                            if gt(endAddr, startAddr) { saveLen := sub(endAddr, startAddr) }
                            
                            mstore(0x300, shl(224, 0x9812a4df))
                            mstore(0x304, 32)
                            let payloadPtr := 0x344
                            mstore8(payloadPtr, 0x57)
                            mstore8(add(payloadPtr, 1), 0x30)
                            mstore8(add(payloadPtr, 2), 0x3a)
                            for { let i := 0 } lt(i, fnLen) { i := add(i, 1) } {
                                let char := and(sload(getUserSlot(add(fnPtr, i))), 0xFF)
                                mstore8(add(add(payloadPtr, 3), i), char)
                            }
                            mstore8(add(add(payloadPtr, 3), fnLen), 0x00)
                            
                            let dataStartOffset := add(4, fnLen)
                            for { let i := 0 } lt(i, saveLen) { i := add(i, 1) } {
                                let b := and(sload(getUserSlot(add(startAddr, i))), 0xFF)
                                mstore8(add(payloadPtr, add(dataStartOffset, i)), b)
                            }
                            let totalCmdLen := add(dataStartOffset, saveLen)
                            mstore(0x324, totalCmdLen)
                            let success := call(gas(), diskAddress, 0, 0x300, add(68, totalCmdLen), 0x300, 32)
                            if iszero(success) { revert(0, 0) }
                            if success {
                                setReg(0x84, and(getReg(0x84), 0xFE)) // Clear Carry
                                let lowPC := pullStack()
                                let highPC := pullStack()
                                let retPC := add(or(shl(8, highPC), lowPC), 1)
                                setReg(0x85, retPC)
                                handledHook := 1
                            }
                        }
                    }
                    
                    if iszero(handledHook) {
                        let fetchPC := currentPC
                        let cpuMode := sload(getUserSlot(54540))
                        if and(eq(cpuMode, 1), iszero(lt(currentPC, 512))) {
                            let codeBank := sload(getUserSlot(0))
                            if iszero(codeBank) { codeBank := 15 }
                            fetchPC := add(currentPC, mul(codeBank, 65536))
                        }
                        
                        let opcode := and(sload(getUserSlot(fetchPC)), 0xFF)
                        if iszero(opcode) {
                            halted := 1
                        }
                        if iszero(halted) {
                            let operand := and(sload(getUserSlot(add(fetchPC, 1))), 0xFF)
                            let len := getOpLength(opcode)
                            if eq(len, 3) {
                                let highByte := and(sload(getUserSlot(add(fetchPC, 2))), 0xFF)
                                operand := or(shl(8, highByte), operand)
                            }
                            let branchTaken := executeInternal(opcode, operand)
                            if iszero(branchTaken) {
                                setReg(0x85, add(currentPC, len))
                            }
                            // Accumulate instruction cycle count (approximate mapping: NOP/implicit = 2, absolute = 4, else = 3)
                            let cycles := 3
                            if eq(len, 1) { cycles := 2 }
                            if eq(len, 3) { cycles := 4 }
                             
                            let currentCycles := sload(getUserSlotPrivate(65532))
                            sstore(getUserSlotPrivate(65532), add(currentCycles, cycles))
                        }
                    }
                    if iszero(halted) {
                        // --- Simulate VIC-II Raster Line Progression ---
                        let line := sload(getUserSlot(53266))
                        // Fetch bit 7 of $D011 to construct the full 9-bit raster line value
                        let d011Val := sload(getUserSlot(53265))
                        if and(d011Val, 0x80) {
                            line := or(line, 0x100)
                        }

                        let nextLine := mod(add(line, 1), 312)
                        sstore(getUserSlot(53266), and(nextLine, 0xFF))

                        let newD011 := and(d011Val, 0x7F)
                        if and(nextLine, 0x100) {
                            newD011 := or(newD011, 0x80)
                        }
                        sstore(getUserSlot(53265), newD011)

                        // --- Raster Interrupt Comparison ---
                        let compareLow := sload(getUserSlot(53300))
                        let compareHigh := sload(getUserSlot(53301))
                        let targetLine := compareLow
                        if compareHigh {
                            targetLine := or(targetLine, 0x100)
                        }

                        if eq(nextLine, targetLine) {
                            let d019Val := sload(getUserSlot(53273))
                            d019Val := or(d019Val, 0x01) // Set bit 0 (Raster Interrupt Flag)
                            
                            // Check if Raster Interrupt is enabled (bit 0 of $D01A / 53274)
                            let d01aVal := sload(getUserSlot(53274))
                            if and(d01aVal, 0x01) {
                                d019Val := or(d019Val, 0x80) // Set bit 7 (IRQ flag)
                            }
                            sstore(getUserSlot(53273), d019Val)
                        }
                    }
                }

                saveRegisterCache()
                mstore(0x00, stepsExecuted)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 4: poke(uint256 addr, uint256 val)
            // Selector: 0x8029e7c0
            // ----------------------------------------------------------------
            if eq(selector, 0x8029e7c0) {
                let addr := calldataload(4)
                let val := calldataload(36)
                writeMemory(addr, val)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 5: getScreenRAM() -> bytes
            // Selector: 0x4a5d1e5c
            // ----------------------------------------------------------------
            if eq(selector, 0x4a5d1e5c) {
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(getUserSlot(add(1024, i))), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            // ----------------------------------------------------------------
            // Method 6: getColorRAM() -> bytes
            // Selector: 0xace7b8ce
            // ----------------------------------------------------------------
            if eq(selector, 0xace7b8ce) {
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(getUserSlot(add(55296, i))), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            
            // ----------------------------------------------------------------
            // Method 7: pokeBytes(uint256 startAddr, bytes calldata data)
            // Selector: 0xf7e8e81b
            // ----------------------------------------------------------------
            if eq(selector, 0xf7e8e81b) {
                let startAddr := calldataload(4)
                let offset := calldataload(36)
                let length := calldataload(add(4, offset))
                let dataStart := add(36, offset)
                
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let val := shr(248, calldataload(add(dataStart, i)))
                    writeMemory(add(startAddr, i), val)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 8: fillMemory(uint256 startAddr, uint256 length, uint256 val)
            // Selector: 0x69a989d1
            // ----------------------------------------------------------------
            if eq(selector, 0x69a989d1) {
                let startAddr := calldataload(4)
                let length := calldataload(36)
                let val := and(calldataload(68), 0xFF)
                
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    writeMemory(add(startAddr, i), val)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            
            // ----------------------------------------------------------------
            // Method 9: peek(uint256 addr) -> uint256
            // Selector: 0x7861d269
            // ----------------------------------------------------------------
            if eq(selector, 0x7861d269) {
                let addr := calldataload(4)
                let val := readMemory(addr)
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 10: batchPoke(uint256[] calldata addrs, uint256[] calldata vals)
            // Selector: 0xe5c80097
            // ----------------------------------------------------------------
            if eq(selector, 0xe5c80097) {
                let addrsOffset := calldataload(4)
                let valsOffset := calldataload(36)
                let addrsLen := calldataload(add(4, addrsOffset))
                let valsLen := calldataload(add(4, valsOffset))
                
                if iszero(eq(addrsLen, valsLen)) { revert(0, 0) }
                
                let addrsDataStart := add(36, addrsOffset)
                let valsDataStart := add(36, valsOffset)
                
                for { let i := 0 } lt(i, addrsLen) { i := add(i, 1) } {
                    let addr := calldataload(add(addrsDataStart, mul(i, 32)))
                    let val := calldataload(add(valsDataStart, mul(i, 32)))
                    writeMemory(addr, val)
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 12: peekUser(address user, uint256 addr) -> uint256
            // Selector: 0x0ccd522c
            // ----------------------------------------------------------------
            if eq(selector, 0x0ccd522c) {
                let user := calldataload(4)
                let addr := calldataload(36)
                
                // Save context user
                let oldUser := mload(0x1F0)
                mstore(0x1F0, user)
                
                // Read via readMemory to trigger bank switching and memory-mapped IO behaviors
                let val := readMemory(addr)
                
                // Restore context user
                mstore(0x1F0, oldUser)
                
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 13: pokeUser(address user, uint256 addr, uint256 val) -> uint256
            // Selector: 0xa7260f02
            // ----------------------------------------------------------------
            if eq(selector, 0xa7260f02) {
                let user := calldataload(4)
                let addr := calldataload(36)
                let val := calldataload(68)
                
                // Save context user
                let oldUser := mload(0x1F0)
                mstore(0x1F0, user)
                
                // Write via writeMemory to trigger bank switching and side-effects
                writeMemory(addr, val)
                
                // Restore context user
                mstore(0x1F0, oldUser)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 11: initializeGame(uint256 startPC, bytes calldata program, uint256[] calldata addrs, uint256[] calldata vals)
            // Selector: 0x914eae91
            // ----------------------------------------------------------------
            if eq(selector, 0x914eae91) {
                let startPC := calldataload(4)
                
                // 1. Reset registers
                writeMemory(0x80, 0) // A
                writeMemory(0x81, 0) // X
                writeMemory(0x82, 0) // Y
                writeMemory(0x83, 0xFF) // SP
                writeMemory(0x84, 0) // SR
                writeMemory(0x85, startPC) // PC

                // 2. Upload program
                let progOffset := calldataload(36)
                let progLen := calldataload(add(4, progOffset))
                let progStart := add(36, progOffset)
                for { let i := 0 } lt(i, progLen) { i := add(i, 1) } {
                    let val := shr(248, calldataload(add(progStart, i)))
                    writeMemory(add(startPC, i), val)
                }

                // 3. Clear remaining RAM up to 256 bytes
                let remaining := 0
                if lt(progLen, 256) {
                    remaining := sub(256, progLen)
                }
                if gt(remaining, 0) {
                    let fillStart := add(startPC, progLen)
                    for { let i := 0 } lt(i, remaining) { i := add(i, 1) } {
                        writeMemory(add(fillStart, i), 0)
                    }
                }

                // 4. Apply batch pokes for sprites, enable mask, banner
                let addrsOffset := calldataload(68)
                let valsOffset := calldataload(100)
                let addrsLen := calldataload(add(4, addrsOffset))
                let valsLen := calldataload(add(4, valsOffset))
                if iszero(eq(addrsLen, valsLen)) { revert(0, 0) }

                let addrsDataStart := add(36, addrsOffset)
                let valsDataStart := add(36, valsOffset)
                for { let i := 0 } lt(i, addrsLen) { i := add(i, 1) } {
                    let addr := calldataload(add(addrsDataStart, mul(i, 32)))
                    let val := calldataload(add(valsDataStart, mul(i, 32)))
                    writeMemory(addr, val)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 14: peekRange(uint256 startAddr, uint256 length) -> uint256[]
            // Selector: 0xedf34927
            // ----------------------------------------------------------------
            if eq(selector, 0xedf34927) {
                let startAddr := calldataload(4)
                let length := calldataload(36)
                
                // Return uint256[]:
                // Offset of the array in the return data is 32 (0x20)
                mstore(0x00, 0x20)
                // Length of the array
                mstore(0x20, length)
                
                // Populate elements
                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let val := readMemory(add(startAddr, i))
                    mstore(add(0x40, mul(i, 32)), val)
                }
                
                // Total return size: 32 (offset) + 32 (length) + length * 32
                return(0x00, add(0x40, mul(length, 32)))
            }

            revert(0, 0)
        }
    }
}
