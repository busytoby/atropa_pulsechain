object "FolkloreCPU" {
    // ========================================================================
    // DEPLOYMENT CODE (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    // ========================================================================
    // RUNTIME CODE
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for namespaced user storage mapping (EVM style)
            function getContextUser() -> user {
                user := mload(0x1F0)
                if iszero(user) {
                    user := caller()
                }
            }

            function getUserSlot(addr) -> slot {
                let user := getContextUser()
                mstore(0x00, user)
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }

            // Native EVM Physics and Game loop tick
            function processPhysics() {
                let px := sload(getUserSlot(55027))
                let py := sload(getUserSlot(55028))
                let pvy := sload(getUserSlot(55029))
                let jumping := sload(getUserSlot(55030))
                let energy := sload(getUserSlot(55031))
                let score := sload(getUserSlot(55032))
                let screen := sload(getUserSlot(55033))
                let moveDir := sload(getUserSlot(55025))
                let jumpTrigger := sload(getUserSlot(55026))

                // Sign conversion for vertical velocity
                if gt(pvy, 127) { pvy := sub(pvy, 256) }

                // Horizontal movement input processing
                if eq(moveDir, 1) { // Left
                    if gt(px, 15) { px := sub(px, 3) }
                }
                if eq(moveDir, 2) { // Right
                    px := add(px, 3)
                    if gt(px, 785) { // Screen transition
                        if lt(screen, 3) {
                            screen := add(screen, 1)
                            px := 20
                            // Setup screen-specific entity coordinates
                            if eq(screen, 2) {
                                sstore(getUserSlot(55037), 600) // crow_x
                                sstore(getUserSlot(55038), 240) // crow_y
                                sstore(getUserSlot(55039), 254) // crow_vx (-2)
                            }
                            if eq(screen, 3) {
                                sstore(getUserSlot(55040), 600) // gargamel_x
                                sstore(getUserSlot(55041), 253) // gargamel_vx (-3)
                            }
                        }
                        if eq(screen, 3) {
                            if gt(px, 780) { px := 780 }
                        }
                    }
                }

                // Jump mechanics (EVM float-free gravity)
                if jumpTrigger {
                    if iszero(jumping) {
                        pvy := sub(0, 12)
                        jumping := 1
                        sstore(getUserSlot(55036), 1) // Sound Strobe: Jump
                    }
                }

                // Apply gravity step
                if jumping {
                    pvy := add(pvy, 1)
                    py := add(py, pvy)
                }

                // Ground collisions
                if gt(py, 500) {
                    py := 500
                    pvy := 0
                    jumping := 0
                }
                if eq(py, 500) {
                    pvy := 0
                    jumping := 0
                }

                // Obstacle and hazard interactions
                if eq(screen, 1) {
                    // Fence obstacle
                    if and(and(gt(px, 340), lt(px, 380)), gt(py, 485)) {
                        if gt(energy, 15) { energy := sub(energy, 15) }
                        if iszero(gt(energy, 15)) { energy := 0 }
                        if gt(px, 30) { px := sub(px, 30) }
                        sstore(getUserSlot(55036), 2) // Sound Strobe: Hit
                    }
                    // Boulder obstacle
                    if and(and(gt(px, 580), lt(px, 620)), gt(py, 495)) {
                        if gt(energy, 10) { energy := sub(energy, 10) }
                        if iszero(gt(energy, 10)) { energy := 0 }
                        if gt(px, 30) { px := sub(px, 30) }
                        sstore(getUserSlot(55036), 2) // Sound Strobe: Hit
                    }
                }

                if eq(screen, 2) {
                    // Stalagmite spikes
                    if and(and(gt(px, 380), lt(px, 420)), or(gt(py, 490), eq(py, 490))) {
                        if gt(energy, 20) { energy := sub(energy, 20) }
                        if iszero(gt(energy, 20)) { energy := 0 }
                        if gt(px, 30) { px := sub(px, 30) }
                        sstore(getUserSlot(55036), 2) // Sound Strobe: Hit
                    }
                    
                    // Shadow Crow flight patterns
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

                    // Crow proximity check
                    let dx := sub(px, bat_x)
                    let dy := sub(sub(py, 20), bat_y)
                    let dist_sq := add(mul(dx, dx), mul(dy, dy))
                    if lt(dist_sq, 784) {
                        if gt(energy, 30) { energy := sub(energy, 30) }
                        if iszero(gt(energy, 30)) { energy := 0 }
                        if gt(px, 30) { px := sub(px, 30) }
                        sstore(getUserSlot(55036), 2) // Sound Strobe: Hit
                    }
                }

                if eq(screen, 3) {
                    // Gargamel patrol physics
                    let gargamel_x := sload(getUserSlot(55040))
                    let gargamel_vx := sload(getUserSlot(55041))
                    if gt(gargamel_vx, 127) { gargamel_vx := sub(gargamel_vx, 256) }

                    gargamel_x := add(gargamel_x, gargamel_vx)
                    if or(lt(gargamel_x, 300), gt(gargamel_x, 650)) {
                        gargamel_vx := sub(0, gargamel_vx)
                    }
                    sstore(getUserSlot(55040), gargamel_x)
                    sstore(getUserSlot(55041), and(gargamel_vx, 0xFF))

                    // Gargamel proximity check
                    let dx := sub(px, gargamel_x)
                    let dy := sub(py, 500)
                    let dist_sq := add(mul(dx, dx), mul(dy, dy))
                    if lt(dist_sq, 1600) {
                        if gt(energy, 40) { energy := sub(energy, 40) }
                        if iszero(gt(energy, 40)) { energy := 0 }
                        if gt(px, 40) { px := sub(px, 40) }
                        sstore(getUserSlot(55036), 2) // Sound Strobe: Hit
                    }

                    // Win Condition (reach the cage)
                    if gt(px, 640) {
                        sstore(getUserSlot(55035), 1)
                    }
                }

                // Check Game Over
                if iszero(energy) {
                    sstore(getUserSlot(55034), 1)
                }

                // Write states back to user mapping slots
                sstore(getUserSlot(55027), px)
                sstore(getUserSlot(55028), py)
                sstore(getUserSlot(55029), and(pvy, 0xFF))
                sstore(getUserSlot(55030), jumping)
                sstore(getUserSlot(55031), energy)
                sstore(getUserSlot(55032), score)
                sstore(getUserSlot(55033), screen)
                sstore(getUserSlot(55026), 0)
            }

            // ----------------------------------------------------------------
            // METHOD: poke(uint256 addr, uint256 val)
            // Selector: 0x8029e7c0
            // ----------------------------------------------------------------
            if eq(selector, 0x8029e7c0) {
                let addr := calldataload(4)
                let val := calldataload(36)
                
                sstore(getUserSlot(addr), val)
                if eq(addr, 55024) {
                    if val {
                        processPhysics()
                        sstore(getUserSlot(55024), 0)
                    }
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: peek(uint256 addr)
            // Selector: 0x7861d269
            // ----------------------------------------------------------------
            if eq(selector, 0x7861d269) {
                let addr := calldataload(4)
                mstore(0x00, sload(getUserSlot(addr)))
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
