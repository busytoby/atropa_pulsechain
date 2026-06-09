object "YarsRevenge" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            function getSlot(offset) -> slot {
                slot := offset
            }

            // METHOD 1: initializeGame()
            // Selector: 0xd6b046a2
            if eq(selector, 0xd6b046a2) {
                sstore(getSlot(0x100), 10000) // Yar X = 100.00
                sstore(getSlot(0x101), 27000) // Yar Y = 270.00
                sstore(getSlot(0x102), 70000) // Qotile X = 700.00
                sstore(getSlot(0x103), 27000) // Qotile Y = 270.00
                sstore(getSlot(0x104), 0)     // Cannon X = 0
                sstore(getSlot(0x105), 0)     // Cannon Y = 0
                sstore(getSlot(0x106), 0)     // Swirl X = 0
                sstore(getSlot(0x107), 0)     // Swirl Y = 0
                
                sstore(getSlot(0x108), 3)     // Lives = 3
                sstore(getSlot(0x109), 0)     // Score = 0
                sstore(getSlot(0x10a), 0)     // Status = 0 (playing)
                
                sstore(getSlot(0x10b), 0)     // Cannon Active = 0 (0: inactive, 1: ready, 2: fired)
                sstore(getSlot(0x10c), 0)     // Swirl Active = 0 (0: inactive, 1: spinning, 2: fired)
                sstore(getSlot(0x10d), 40000) // Neutral zone center X = 400.00
                sstore(getSlot(0x10e), 0)     // Tick / Frame counter = 0

                // Initialize Shield Columns (5 columns, 32 blocks each)
                // 32 bits set to 1 -> 0xffffffff
                sstore(getSlot(0x200), 0xffffffff)
                sstore(getSlot(0x201), 0xffffffff)
                sstore(getSlot(0x202), 0xffffffff)
                sstore(getSlot(0x203), 0xffffffff)
                sstore(getSlot(0x204), 0xffffffff)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // METHOD 2: updateGame(tx, ty, fire, action)
            // Selector: 0xa9e7f536
            if eq(selector, 0xa9e7f536) {
                let status := sload(getSlot(0x10a))
                let tick := add(sload(getSlot(0x10e)), 1)
                sstore(getSlot(0x10e), tick)

                if eq(status, 0) {
                    let tx := calldataload(4)
                    let ty := calldataload(36)
                    let fire := calldataload(68)
                    let action := calldataload(100)

                    let yarX := sload(getSlot(0x100))
                    let yarY := sload(getSlot(0x101))
                    let qotileX := sload(getSlot(0x102))
                    let qotileY := sload(getSlot(0x103))
                    let cannonX := sload(getSlot(0x104))
                    let cannonY := sload(getSlot(0x105))
                    let swirlX := sload(getSlot(0x106))
                    let swirlY := sload(getSlot(0x107))
                    let lives := sload(getSlot(0x108))
                    let score := sload(getSlot(0x109))
                    
                    let cannonActive := sload(getSlot(0x10b))
                    let swirlActive := sload(getSlot(0x10c))

                    // 1. Move Yar (0.04 scaled)
                    yarX := add(yarX, mul(tx, 400))
                    yarY := add(yarY, mul(ty, 400))

                    // Keep Yar in bounds (10.0 to 790.0 X, 50.0 to 490.0 Y)
                    if slt(yarX, 1000) { yarX := 1000 }
                    if sgt(yarX, 79000) { yarX := 79000 }
                    if slt(yarY, 5000) { yarY := 5000 }
                    if sgt(yarY, 49000) { yarY := 49000 }

                    // 2. Move Qotile (up and down slowly)
                    let qDirection := 1
                    if and(div(tick, 100), 1) { qDirection := sub(0, 1) }
                    qotileY := add(qotileY, mul(qDirection, 80)) // speed 0.8
                    if slt(qotileY, 10000) { qotileY := 10000 }
                    if sgt(qotileY, 44000) { qotileY := 44000 }

                    // 3. Bite shield block if action is pressed (contact check)
                    // Shield Columns: 0 to 4 are located at X: 600, 610, 620, 630, 640
                    // Rows Y: 110 to 430 (step 10)
                    // If Yar is overlapping a block and bites/touches:
                    let yarCol := div(sub(yarX, 60000), 1000) // 0 to 4
                    if and(sgt(yarX, 59500), slt(yarX, 65500)) {
                        if and(sgt(yarY, 10500), slt(yarY, 43500)) {
                            let yarRow := div(sub(yarY, 11000), 1000) // 0 to 31
                            if and(and(sgt(yarCol, sub(0, 1)), slt(yarCol, 5)), and(sgt(yarRow, sub(0, 1)), slt(yarRow, 32))) {
                                let colVal := sload(getSlot(add(0x200, yarCol)))
                                let bitMask := shl(yarRow, 1)
                                if and(colVal, bitMask) {
                                    // Clear block
                                    sstore(getSlot(add(0x200, yarCol)), xor(colVal, bitMask))
                                    score := add(score, 10)
                                    // Touching/eating shield primes Zorlon Cannon
                                    if iszero(cannonActive) {
                                        cannonActive := 1 // ready
                                    }
                                }
                            }
                        }
                    }

                    // 4. Handle Zorlon Cannon (Fires left to right or targets Qotile)
                    if eq(cannonActive, 1) {
                        // Spawn Cannon at Yar position when fire is clicked
                        if fire {
                            cannonActive := 2 // Fired
                            cannonX := yarX
                            cannonY := yarY
                        }
                    }
                    if eq(cannonActive, 2) {
                        cannonX := add(cannonX, 800) // speed 8.0
                        if sgt(cannonX, 80000) {
                            cannonActive := 0 // Reset
                            cannonX := 0
                            cannonY := 0
                        }
                        // Collision check between Cannon and Qotile
                        if and(sgt(cannonX, sub(qotileX, 2000)), slt(cannonX, add(qotileX, 2000))) {
                            if and(sgt(cannonY, sub(qotileY, 3000)), slt(cannonY, add(qotileY, 3000))) {
                                status := 1 // Victory!
                                score := add(score, 1000)
                                cannonActive := 0
                            }
                        }
                    }

                    // 5. Handle Qotile Swirl / Missile launcher
                    if iszero(swirlActive) {
                        // Periodically launch swirl
                        if eq(mod(tick, 200), 0) {
                            swirlActive := 1 // Spin/Warning
                            swirlX := qotileX
                            swirlY := qotileY
                        }
                    }
                    if eq(swirlActive, 1) {
                        // Wait/Spin for 60 ticks, then launch
                        if eq(mod(tick, 250), 0) {
                            swirlActive := 2 // Launched
                        }
                    }
                    if eq(swirlActive, 2) {
                        // Swirl flies left towards Yar
                        swirlX := sub(swirlX, 600) // speed 6.0
                        if slt(swirlX, 0) {
                            swirlActive := 0
                        }
                        
                        // Collision check: Swirl hits Yar
                        // Yar is safe if in the Neutral Zone (X: 350 to 450) and not shooting
                        let inNeutral := and(sgt(yarX, 35000), slt(yarX, 45000))
                        if iszero(inNeutral) {
                            if and(sgt(swirlX, sub(yarX, 1500)), slt(swirlX, add(yarX, 1500))) {
                                if and(sgt(swirlY, sub(yarY, 1500)), slt(swirlY, add(yarY, 1500))) {
                                    lives := sub(lives, 1)
                                    swirlActive := 0
                                    if slt(lives, 1) {
                                        status := 3 // Game Over
                                    }
                                    if sgt(lives, 0) {
                                        status := 2 // Reset round
                                    }
                                }
                            }
                        }
                    }

                    sstore(getSlot(0x100), yarX)
                    sstore(getSlot(0x101), yarY)
                    sstore(getSlot(0x102), qotileX)
                    sstore(getSlot(0x103), qotileY)
                    sstore(getSlot(0x104), cannonX)
                    sstore(getSlot(0x105), cannonY)
                    sstore(getSlot(0x106), swirlX)
                    sstore(getSlot(0x107), swirlY)
                    sstore(getSlot(0x108), lives)
                    sstore(getSlot(0x109), score)
                    sstore(getSlot(0x10a), status)
                    sstore(getSlot(0x10b), cannonActive)
                    sstore(getSlot(0x10c), swirlActive)
                }

                mstore(0x00, sload(getSlot(0x100)))
                mstore(0x20, sload(getSlot(0x101)))
                mstore(0x40, sload(getSlot(0x102)))
                mstore(0x60, sload(getSlot(0x103)))
                mstore(0x80, sload(getSlot(0x104)))
                mstore(0xa0, sload(getSlot(0x105)))
                mstore(0xc0, sload(getSlot(0x106)))
                mstore(0xe0, sload(getSlot(0x107)))
                mstore(0x100, sload(getSlot(0x108)))
                mstore(0x120, sload(getSlot(0x109)))
                mstore(0x140, sload(getSlot(0x10a)))
                mstore(0x160, sload(getSlot(0x10b)))
                mstore(0x180, sload(getSlot(0x10c)))
                mstore(0x1a0, sload(getSlot(0x10d)))
                mstore(0x1c0, sload(getSlot(0x200)))
                mstore(0x1e0, sload(getSlot(0x201)))
                mstore(0x200, sload(getSlot(0x202)))
                mstore(0x220, sload(getSlot(0x203)))
                mstore(0x240, sload(getSlot(0x204)))
                return(0x00, 608)
            }

            revert(0, 0)
        }
    }
}
