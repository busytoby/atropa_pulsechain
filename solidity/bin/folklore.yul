object "cpu6502" {
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

            // Helper to query and excise tax via standalone on-chain Diyat contract (never subordinate)
            function exciseOnChainTax(taxAmount) -> taxPaidSuccess {
                taxPaidSuccess := 0
                // Standalone Diyat contract address: retrieved from slot 54695
                let diyatAddress := sload(getUserSlot(54695))

                if diyatAddress {
                    // Call exciseTax(address,uint256) -> 0x904a4bc3
                    mstore(0x340, shl(224, 0x904a4bc3))
                    mstore(0x344, getContextUser())
                    mstore(0x364, taxAmount)

                    let callSuccess := call(gas(), diyatAddress, 0, 0x340, 68, 0x3A4, 32)
                    if callSuccess {
                        if mload(0x3A4) {
                            taxPaidSuccess := 1
                        }
                    }
                }
            }

            // Helper to update GTIA PMG registers
            function updatePmgState(pmgAddr, isMissile, index, hpos, size, color) {
                if pmgAddr {
                    mstore(0x340, shl(224, 0xaeed2db8))
                    mstore(0x344, isMissile)
                    mstore(0x364, index)
                    mstore(0x384, hpos)
                    mstore(0x3A4, size)
                    mstore(0x3C4, color)
                    let success := delegatecall(gas(), pmgAddr, 0x340, 164, 0x00, 0)
                }
            }

            // Helper to check GTIA hardware collisions
            function checkPmgCollisions(pmgAddr) -> mask {
                mask := 0
                if pmgAddr {
                    mstore(0x340, shl(224, 0xb966a078))
                    let success := delegatecall(gas(), pmgAddr, 0x340, 4, 0x3E4, 32)
                    if success {
                        mask := mload(0x3E4)
                    }
                }
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

                // ============================================================
                // GAUNTLET GAME ENGINE EXTENSION
                // ============================================================
                // Storage Map mapping:
                // 55050: isGauntletActive (Boolean)
                // 55051: gauntletPlayerX (16-bit)
                // 55052: gauntletPlayerY (16-bit)
                // 55053: gauntletHealth (uint256, starts at 2000)
                // 55054: gauntletKeys (uint256)
                // 55055: gauntletGhostX (16-bit)
                // 55056: gauntletGhostY (16-bit)
                // 55057: gauntletSpawnerX (16-bit)
                // 55058: gauntletSpawnerY (16-bit)
                // 55059: gauntletInputDirection (1=Up, 2=Down, 3=Left, 4=Right)
                // 55060: gauntletFireAxisTrigger (Boolean)
                // 55061: gauntletWeapon (1=Battle Axe, 2=Sword, 3=Magic Wand, 4=Bow)
                // 55062: gauntletArmor (1=Leather Armor, 2=Chain Mail, 3=Plate Mail, 4=Magic Robes)
                
                let gauntletActive := sload(getUserSlot(55050))
                if gauntletActive {
                    let gpx := sload(getUserSlot(55051))
                    let gpy := sload(getUserSlot(55052))
                    let ghealth := sload(getUserSlot(55053))
                    let ggx := sload(getUserSlot(55055))
                    let ggy := sload(getUserSlot(55056))
                    let gsx := sload(getUserSlot(55057))
                    let gsy := sload(getUserSlot(55058))

                    // 1 & 2. Slow health drain and Movement Physics
                    {
                        if gt(ghealth, 0) {
                            ghealth := sub(ghealth, 1)
                        }

                        let gdir := sload(getUserSlot(55059))
                        switch gdir
                        case 1 { // Up
                            if gt(gpy, 10) { gpy := sub(gpy, 4) }
                        }
                        case 2 { // Down
                            if lt(gpy, 470) { gpy := add(gpy, 4) }
                        }
                        case 3 { // Left
                            if gt(gpx, 10) { gpx := sub(gpx, 4) }
                        }
                        case 4 { // Right
                            if lt(gpx, 710) { gpx := add(gpx, 4) }
                        }
                    }

                    // Update Player 0 PMG state: isMissile = 0, index = 0, hpos = gpx/3, size = 0, color = 0x1E (Blue)
                    updatePmgState(sload(getUserSlot(54696)), 0, 0, div(gpx, 3), 0, 0x1E)

                    // 3. Spawner Spawn Logic & Ghost Movement
                    {
                        if and(and(iszero(ggx), iszero(ggy)), and(gt(gsx, 0), gt(gsy, 0))) {
                            let spawnTimer := sload(getUserSlot(55061))
                            spawnTimer := add(spawnTimer, 1)
                            if gt(spawnTimer, 150) {
                                ggx := gsx
                                ggy := gsy
                                spawnTimer := 0
                                let gen := sload(getUserSlot(55062))
                                sstore(getUserSlot(55062), add(gen, 1))
                            }
                            sstore(getUserSlot(55061), spawnTimer)
                        }

                        if and(gt(ggx, 0), gt(ggy, 0)) {
                            let dx := sub(gpx, ggx)
                            let dy := sub(gpy, ggy)
                            if gt(dx, 0) { ggx := add(ggx, 1) }
                            if lt(dx, 0) { ggx := sub(ggx, 1) }
                            if gt(dy, 0) { ggy := add(ggy, 1) }
                            if lt(dy, 0) { ggy := sub(ggy, 1) }

                            let dist_sq := add(mul(dx, dx), mul(dy, dy))
                            if lt(dist_sq, 400) {
                                if gt(ghealth, 100) { ghealth := sub(ghealth, 100) }
                                ggx := 0
                                ggy := 0
                            }
                        }
                    }

                    // 4. Projectile Physics & Firing (Self-contained scope)
                    {
                        let projActive := sload(getUserSlot(55074))
                        let projX := sload(getUserSlot(55070))
                        let projY := sload(getUserSlot(55071))
                        let projVX := sload(getUserSlot(55072))
                        if gt(projVX, 127) { projVX := sub(projVX, 256) }
                        let projVY := sload(getUserSlot(55073))
                        if gt(projVY, 127) { projVY := sub(projVY, 256) }

                        let gfire := sload(getUserSlot(55060))
                        if gfire {
                            if iszero(projActive) {
                                projActive := 1
                                projX := gpx
                                projY := gpy
                                projVX := 8
                                projVY := 0

                                let gdir := sload(getUserSlot(55059))
                                if eq(gdir, 1) { // Up
                                    projVX := 0
                                    projVY := sub(0, 8)
                                }
                                if eq(gdir, 2) { // Down
                                    projVX := 0
                                    projVY := 8
                                }
                                if eq(gdir, 3) { // Left
                                    projVX := sub(0, 8)
                                    projVY := 0
                                }
                                if eq(gdir, 4) { // Right
                                    projVX := 8
                                    projVY := 0
                                }
                            }
                            sstore(getUserSlot(55060), 0)
                        }

                        if projActive {
                            projX := add(projX, projVX)
                            projY := add(projY, projVY)

                            if or(or(gt(projX, 800), lt(projX, 0)), or(gt(projY, 500), lt(projY, 0))) {
                                projActive := 0
                            }

                            if and(gt(gsx, 0), gt(gsy, 0)) {
                                let dx := sub(projX, gsx)
                                let dy := sub(projY, gsy)
                                let dist_sq := add(mul(dx, dx), mul(dy, dy))
                                if lt(dist_sq, 900) {
                                    if exciseOnChainTax(1) {
                                        gsx := 0
                                        gsy := 0
                                        score := add(score, 500)
                                        projActive := 0
                                    }
                                }
                            }

                            if and(gt(ggx, 0), gt(ggy, 0)) {
                                let gdx := sub(projX, ggx)
                                let gdy := sub(projY, ggy)
                                let gdist_sq := add(mul(gdx, gdx), mul(gdy, gdy))
                                if lt(gdist_sq, 900) {
                                    if exciseOnChainTax(1) {
                                        ggx := 0
                                        ggy := 0
                                        score := add(score, 100)
                                        projActive := 0
                                    }
                                }
                            }

                            // Update Missile 0 register
                            updatePmgState(sload(getUserSlot(54696)), 1, 0, div(projX, 3), 0, 0x3A)
                        }
                        if iszero(projActive) {
                            updatePmgState(sload(getUserSlot(54696)), 1, 0, 0, 0, 0)
                        }

                        // Check simulated GTIA hardware collisions
                        let collisionMask := checkPmgCollisions(sload(getUserSlot(54696)))
                        sstore(getUserSlot(55075), collisionMask)

                        sstore(getUserSlot(55070), projX)
                        sstore(getUserSlot(55071), projY)
                        sstore(getUserSlot(55072), and(projVX, 0xFF))
                        sstore(getUserSlot(55073), and(projVY, 0xFF))
                        sstore(getUserSlot(55074), projActive)
                    }

                    // Save state back
                    sstore(getUserSlot(55051), gpx)
                    sstore(getUserSlot(55052), gpy)
                    sstore(getUserSlot(55053), ghealth)
                    sstore(getUserSlot(55055), ggx)
                    sstore(getUserSlot(55056), ggy)
                    sstore(getUserSlot(55057), gsx)
                    sstore(getUserSlot(55058), gsy)
                    sstore(getUserSlot(55059), 0) // reset movement
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
            // METHOD: calculateVisibility(uint256 observerX, uint256 observerY, uint256 targetX, uint256 targetY) -> uint256 visible
            // Selector: 0x8b3260cc
            // ----------------------------------------------------------------
            if eq(selector, 0x8b3260cc) {
                let ox := calldataload(4)
                let oy := calldataload(36)
                let tx := calldataload(68)
                let ty := calldataload(100)

                // Simple Euclidean distance and height-difference visibility threshold check
                let dx := sub(tx, ox)
                let dy := sub(ty, oy)
                let dist_sq := add(mul(dx, dx), mul(dy, dy))

                let targetHeight := sload(getUserSlot(add(56000, add(mul(ty, 8), tx))))
                let observerHeight := sload(getUserSlot(add(56000, add(mul(oy, 8), ox))))

                let visible := 1
                // If target is too far or significantly lower than the observer without height advantage
                if and(gt(dist_sq, 25), lt(targetHeight, observerHeight)) {
                    visible := 0
                }

                mstore(0x00, visible)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: checkLineOfSight(uint256 ox, uint256 oy, uint256 tx, uint256 ty) -> uint256 blocked
            // Selector: 0x4f128be1
            // ----------------------------------------------------------------
            if eq(selector, 0x4f128be1) {
                let ox := calldataload(4)
                let oy := calldataload(36)
                let tx := calldataload(68)
                let ty := calldataload(100)

                let blocked := 0
                let steps := 5
                
                // Perform a simple 5-step raycast interpolation to check intermediate height blocks
                for { let step := 1 } lt(step, steps) { step := add(step, 1) } {
                    // Interpolated grid coordinates: ix = ox + (tx - ox) * step / steps
                    let ix := add(ox, div(mul(sub(tx, ox), step), steps))
                    let iy := add(oy, div(mul(sub(ty, oy), step), steps))

                    // Skip start/end boundary tiles
                    if and(iszero(and(eq(ix, ox), eq(iy, oy))), iszero(and(eq(ix, tx), eq(iy, ty)))) {
                        let cellHeight := sload(getUserSlot(add(56000, add(mul(iy, 8), ix))))
                        let observerHeight := sload(getUserSlot(add(56000, add(mul(oy, 8), ox))))
                        
                        // If intermediate cell is higher or equal to observer, the ray is blocked
                        if or(gt(cellHeight, observerHeight), eq(cellHeight, observerHeight)) {
                            blocked := 1
                        }
                    }
                }

                mstore(0x00, blocked)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: mutateHeight(uint256 x, uint256 y, uint256 height)
            // Selector: 0x9330ba02
            // ----------------------------------------------------------------
            if eq(selector, 0x9330ba02) {
                let x := calldataload(4)
                let y := calldataload(36)
                let height := calldataload(68)
                
                sstore(getUserSlot(add(56000, add(mul(y, 8), x))), height)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: teleportObserver(uint256 ox, uint256 oy)
            // Selector: 0xf1df9a0c
            // ----------------------------------------------------------------
            if eq(selector, 0xf1df9a0c) {
                let ox := calldataload(4)
                let oy := calldataload(36)
                
                sstore(getUserSlot(55100), ox)
                sstore(getUserSlot(55101), oy)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            // ----------------------------------------------------------------
            // METHOD: checkWinCondition(uint256 ox, uint256 oy) -> uint256 win
            // Selector: 0x933cc4bf
            // ----------------------------------------------------------------
            if eq(selector, 0x933cc4bf) {
                let ox := calldataload(4)
                let oy := calldataload(36)

                let win := 0
                // Pedestal Tower coordinates are x: 4, y: 3 (Index 28)
                if and(eq(ox, 4), eq(oy, 3)) {
                    let towerHeight := sload(getUserSlot(56028)) // Index 28 height
                    if or(gt(towerHeight, 3), eq(towerHeight, 4)) {
                        win := 1
                    }
                }
                mstore(0x00, win)
                return(0x00, 32)
            }
                
            // ----------------------------------------------------------------
            // METHOD: registerHighScore(uint256 score)
            // Selector: 0xf552b9ac
            // ----------------------------------------------------------------
            if eq(selector, 0xf552b9ac) {
                let score := calldataload(4)
                let user := getContextUser()
                
                // Write high score mapping to user account slot offset 57000
                sstore(getUserSlot(57000), score)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: getHighScore(address player) -> uint256 score
            // Selector: 0x939ad4fe
            // ----------------------------------------------------------------
            if eq(selector, 0x939ad4fe) {
                let player := calldataload(4)
                
                // Temporarily swap context user to fetch mapping slot
                let oldUser := mload(0x1F0)
                mstore(0x1F0, player)
                let score := sload(getUserSlot(57000))
                mstore(0x1F0, oldUser)
                
                mstore(0x00, score)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: peekUser(address player, uint256 addr) -> uint256 val
            // Selector: 0xc2e22c95
            // ----------------------------------------------------------------
            if eq(selector, 0xc2e22c95) {
                let player := calldataload(4)
                let addr := calldataload(36)
                
                let oldUser := mload(0x1F0)
                mstore(0x1F0, player)
                let val := sload(getUserSlot(addr))
                mstore(0x1F0, oldUser)
                
                mstore(0x00, val)
                return(0x00, 32)
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
            // METHOD: pushUnicodeKey(uint256 codePoint)
            // Selector: 0x7684ddbd
            // Pushes a Unicode code point to the keyboard FIFO buffer.
            // ----------------------------------------------------------------
            if eq(selector, 0x7684ddbd) {
                let codePoint := calldataload(4)
                let head := sload(getUserSlot(55200))
                let tail := sload(getUserSlot(55201))
                
                // Initialize size limit to 64 slots if not set
                let limit := sload(getUserSlot(55202))
                if iszero(limit) {
                    limit := 64
                    sstore(getUserSlot(55202), limit)
                }

                // Check if buffer is full: (tail + 1) % limit == head
                let nextTail := mod(add(tail, 1), limit)
                if iszero(eq(nextTail, head)) {
                    // Write to FIFO slot (starting at 55203)
                    sstore(getUserSlot(add(55203, tail)), codePoint)
                    sstore(getUserSlot(55201), nextTail)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: popUnicodeKey() -> uint256 codePoint
            // Selector: 0x42555906
            // Pops a Unicode code point from the keyboard FIFO buffer.
            // ----------------------------------------------------------------
            if eq(selector, 0x42555906) {
                let head := sload(getUserSlot(55200))
                let tail := sload(getUserSlot(55201))
                let codePoint := 0

                // If buffer is not empty: head != tail
                if iszero(eq(head, tail)) {
                    codePoint := sload(getUserSlot(add(55203, head)))
                    
                    let limit := sload(getUserSlot(55202))
                    if iszero(limit) { limit := 64 }

                    let nextHead := mod(add(head, 1), limit)
                    sstore(getUserSlot(55200), nextHead)
                }

                mstore(0x00, codePoint)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: recordTransition(uint256 state, uint256 action)
            // Selector: 0xc05000f7
            // Records a step's state and action during the active episode.
            // ----------------------------------------------------------------
            if eq(selector, 0xc05000f7) {
                let stateVal := calldataload(4)
                let actionVal := calldataload(36)
                
                let stepCount := sload(getUserSlot(55500))
                // Limit episode history to 100 steps to control storage overhead
                if lt(stepCount, 100) {
                    // Pack state-action: state (bits 0-127), action (bits 128-255)
                    let packed := add(stateVal, shl(128, actionVal))
                    sstore(getUserSlot(add(55501, stepCount)), packed)
                    sstore(getUserSlot(55500), add(stepCount, 1))
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: applyMonteCarloScore(int256 score)
            // Selector: 0xcc396490
            // Evaluates performance and updates all recorded transition weights.
            // ----------------------------------------------------------------
            if eq(selector, 0xcc396490) {
                let score := calldataload(4)
                let stepCount := sload(getUserSlot(55500))

                for { let i := 0 } lt(i, stepCount) { i := add(i, 1) } {
                    let packed := sload(getUserSlot(add(55501, i)))
                    let stateVal := and(packed, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
                    let actionVal := shr(128, packed)

                    // Helper to resolve policy weight slot key: keccak256(state, action, salt)
                    mstore(0x00, stateVal)
                    mstore(0x20, actionVal)
                    mstore(0x40, 0x88888888)
                    let weightSlot := keccak256(0x00, 96)

                    let weight := sload(weightSlot)
                    // Convert signed values appropriately in EVM
                    weight := add(weight, score)
                    sstore(weightSlot, weight)

                    // Clear episode steps
                    sstore(getUserSlot(add(55501, i)), 0)
                }

                // Reset episode step count
                sstore(getUserSlot(55500), 0)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: getPolicyWeight(uint256 state, uint256 action) -> int256 weight
            // Selector: 0xa25b60c0
            // Returns the recorded policy utility weight for a state-action pair.
            // ----------------------------------------------------------------
            if eq(selector, 0xa25b60c0) {
                let stateVal := calldataload(4)
                let actionVal := calldataload(36)

                mstore(0x00, stateVal)
                mstore(0x20, actionVal)
                mstore(0x40, 0x88888888)
                let weightSlot := keccak256(0x00, 96)

                mstore(0x00, sload(weightSlot))
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

            // ----------------------------------------------------------------
            // METHOD: autotuneSpeech(uint256 eventId) -> bytes
            // Selector: 0x51c726a8
            // ----------------------------------------------------------------
            if eq(selector, 0x51c726a8) {
                let eventId := calldataload(4)
                let speechSynth := 0x8ff9a7885e7CB3C01D556e1bC4f9fb2A6e7Ac167

                // 1. Register optimized speaker coefficients for 'ana'
                // registerSpeaker(bytes32 name, bytes32[8] embedding) -> 0x5f60cdf4
                mstore(0x100, shl(224, 0x5f60cdf4))
                mstore(0x104, 0x616e610000000000000000000000000000000000000000000000000000000000) // "ana"
                mstore(0x124, 20)   // c0 (optimized from 15)
                mstore(0x144, sub(0, 25)) // c1
                mstore(0x164, 40)   // c2
                mstore(0x184, sub(0, 10)) // c3
                mstore(0x1A4, 5)    // c4
                mstore(0x1C4, sub(0, 8))  // c5
                mstore(0x1E4, 12)   // c6
                mstore(0x204, sub(0, 10))  // c7 (optimized from -5)
                
                let successReg := call(gas(), speechSynth, 0, 0x100, 292, 0, 0)
                
                // 2. Perform predictMelQuantized
                // predictMelQuantized(bytes32[] phonemes, bytes32 name) -> 0x18c1ab9a
                mstore(0x240, shl(224, 0x18c1ab9a))
                mstore(0x244, 64) // Offset
                mstore(0x264, 0x616e610000000000000000000000000000000000000000000000000000000000) // "ana"
                
                let numPhonemes := 3
                mstore(0x284, numPhonemes)
                
                if eq(eventId, 1) { // Jump kiai: "ee", "aa", "p"
                    mstore(0x2A4, 0x6565000000000000000000000000000000000000000000000000000000000000)
                    mstore(0x2C4, 0x6161000000000000000000000000000000000000000000000000000000000000)
                    mstore(0x2E4, 0x7000000000000000000000000000000000000000000000000000000000000000)
                }
                if iszero(eq(eventId, 1)) { // Damage grunt: "oo", "m", "p"
                    mstore(0x2A4, 0x6f6f000000000000000000000000000000000000000000000000000000000000)
                    mstore(0x2C4, 0x6d00000000000000000000000000000000000000000000000000000000000000)
                    mstore(0x2E4, 0x7000000000000000000000000000000000000000000000000000000000000000)
                }
                
                let successPred := call(gas(), speechSynth, 0, 0x240, 196, 0x300, 512)
                
                // Extract Mel prediction
                let melOffset := mload(0x300)
                let melSize := mload(add(0x300, melOffset))
                let melDataPtr := add(add(0x300, melOffset), 32)
                
                // 3. Synthesize voice using synthesizeNeuralWav
                // synthesizeNeuralWav(bytes melFrames, uint256 upsampleFactor) -> 0x20c4433b
                mstore(0x500, shl(224, 0x20c4433b))
                mstore(0x504, 96) // Offset of bytes array
                mstore(0x524, 2560) // upsampleFactor (2560)
                mstore(0x544, 0x616e610000000000000000000000000000000000000000000000000000000000) // speaker name "ana"
                mstore(0x564, melSize) // melFrames size
                
                // Copy melFrames data
                let words := div(add(melSize, 31), 32)
                for { let w := 0 } lt(w, words) { w := add(w, 1) } {
                    mstore(add(0x584, mul(w, 32)), mload(add(melDataPtr, mul(w, 32))))
                }
                
                let totalSynthInputSize := add(132, mul(words, 32))
                let successSynth := call(gas(), speechSynth, 0, 0x500, totalSynthInputSize, 0x6000, 4096)
                
                if successSynth {
                    // Return the synthesized WAV bytes buffer
                    let outputOffset := mload(0x6000)
                    let outputSize := mload(add(0x6000, outputOffset))
                    return(add(0x6000, outputOffset), add(32, outputSize))
                }
                
                revert(0, 0)
            }

            revert(0, 0)
        }
    }
}
