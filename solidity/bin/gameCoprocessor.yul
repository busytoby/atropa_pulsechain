object "GameCoprocessor" {
    code {
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // Helpers for User Storage mapping
            function getContextUser() -> user {
                user := caller()
                let txOrigin := origin()
                if eq(user, txOrigin) {
                    let bypass := sload(0xfa4ba7802838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925)
                    if bypass {
                        user := bypass
                    }
                }
            }

            function getUserSlotPrivate(addr) -> slot {
                mstore(0x00, getContextUser())
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }

            function getUserSlot(addr) -> slot {
                let user := getContextUser()
                let sharedMode := and(sload(getUserSlotPrivate(54541)), 0xFF)
                if sharedMode {
                    if or(or(and(iszero(lt(addr, 1024)), lt(addr, 2048)),
                             or(and(iszero(lt(addr, 53248)), lt(addr, 53312)),
                                and(iszero(lt(addr, 55296)), lt(addr, 56296)))),
                          and(iszero(lt(addr, 36864)), lt(addr, 36880))) {
                        user := 0x5555555555555555555555555555555555555555
                    }
                }
                let targetAddr := addr
                if and(iszero(lt(addr, 0x80)), lt(addr, 0x86)) {
                    let bankId := sload(getUserSlotPrivate(54539))
                    targetAddr := add(addr, mul(bankId, 0x100))
                }
                mstore(0x00, user)
                mstore(0x20, targetAddr)
                slot := keccak256(0x00, 64)
            }

            // On-chain Tax Helpers
            function exciseOnChainTax(taxAmount) -> taxPaidSuccess {
                if sload(getUserSlot(54700)) {
                    taxPaidSuccess := 1
                    leave
                }
                taxPaidSuccess := 0
                let diyatAddress := sload(getUserSlot(54695))
                if diyatAddress {
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

            // Game Helper functions
            function checkStrikeCollision(strikeType, dist, targetStance) -> hit, dmg {
                hit := 0
                dmg := 0
                if iszero(targetStance) {
                    if eq(strikeType, 1) {
                        if or(lt(dist, 15), eq(dist, 15)) {
                            hit := 1
                            dmg := 100
                        }
                    }
                    if eq(strikeType, 2) {
                        if or(lt(dist, 25), eq(dist, 25)) {
                            hit := 1
                            dmg := 100
                        }
                    }
                    leave
                }
                if eq(strikeType, 1) {
                    if and(or(lt(dist, 15), eq(dist, 15)), iszero(eq(targetStance, 2))) {
                        hit := 1
                        dmg := 10
                    }
                }
                if eq(strikeType, 2) {
                    if and(or(lt(dist, 25), eq(dist, 25)), iszero(eq(targetStance, 3))) {
                        hit := 1
                        dmg := 15
                    }
                }
            }

            function checkOnScaffold(py) -> onScaffold {
                onScaffold := 0
                if eq(py, 8) { onScaffold := 1 }
                if eq(py, 16) { onScaffold := 1 }
                if eq(py, 24) { onScaffold := 1 }
            }

            function applyAirTime(py, jumpTimerSlot, actionSlot, onScaffold, pvy) -> new_py {
                let jumpTimer := sload(getUserSlot(jumpTimerSlot))
                let action := sload(getUserSlot(actionSlot))
                if and(eq(action, 1), onScaffold) {
                    jumpTimer := 4
                    sstore(getUserSlot(actionSlot), 0)
                }
                new_py := py
                if gt(jumpTimer, 0) {
                    if gt(new_py, 1) { new_py := sub(new_py, 2) }
                    if iszero(gt(new_py, 1)) { new_py := 0 }
                    jumpTimer := sub(jumpTimer, 1)
                    sstore(getUserSlot(jumpTimerSlot), jumpTimer)
                }
                if iszero(gt(jumpTimer, 0)) {
                    new_py := add(new_py, pvy)
                    let postMoveOnScaffold := checkOnScaffold(new_py)
                    if iszero(postMoveOnScaffold) {
                        new_py := add(new_py, 1)
                    }
                }
            }

            // executeGame(uint256 register, uint256 val) -> bool
            // Selector: 0x69e5ee0b
            if eq(selector, 0x69e5ee0b) {
                let addr := calldataload(4)
                let val := calldataload(36)

                switch addr
                case 55020 { // Lode Runner Action Trigger ($D6EC)
                    sstore(getUserSlot(55020), val)
                    if val {
                        let px := sload(getUserSlot(55018))
                        let py := sload(getUserSlot(55019))
                        if eq(val, 1) {
                            if and(gt(px, 0), lt(py, 24)) {
                                let tx := sub(px, 1)
                                let ty := add(py, 1)
                                let t_addr := add(1024, add(mul(ty, 40), tx))
                                let tile := sload(getUserSlot(t_addr))
                                if eq(tile, 1) { sstore(getUserSlot(t_addr), 6) }
                            }
                        }
                        if eq(val, 2) {
                            if and(lt(px, 39), lt(py, 24)) {
                                let tx := add(px, 1)
                                let ty := add(py, 1)
                                let t_addr := add(1024, add(mul(ty, 40), tx))
                                let tile := sload(getUserSlot(t_addr))
                                if eq(tile, 1) { sstore(getUserSlot(t_addr), 6) }
                            }
                        }
                        if eq(val, 3) {
                            if and(lt(px, 40), lt(py, 25)) {
                                let t_addr := add(1024, add(mul(py, 40), px))
                                let tile := sload(getUserSlot(t_addr))
                                if eq(tile, 5) {
                                    sstore(getUserSlot(t_addr), 0)
                                    let score := sload(getUserSlot(55021))
                                    sstore(getUserSlot(55021), add(score, 1))
                                }
                            }
                        }
                        sstore(getUserSlot(55020), 0)
                    }
                }
                case 55024 { // Smurfs Physics Trigger ($D6B0)
                    sstore(getUserSlot(55024), val)
                    if val {
                        let isGameOver := sload(getUserSlot(55034))
                        let isGameWin := sload(getUserSlot(55035))
                        if and(iszero(isGameOver), iszero(isGameWin)) {
                            let px := sload(getUserSlot(55027))
                            let py := sload(getUserSlot(55028))
                            let pvy := sload(getUserSlot(55029))
                            let jumping := sload(getUserSlot(55030))
                            let energy := sload(getUserSlot(55031))
                            let score := sload(getUserSlot(55032))
                            let screen := sload(getUserSlot(55033))
                            if gt(pvy, 127) { pvy := sub(pvy, 256) }
                            let moveDir := sload(getUserSlot(55025))
                            let jumpTrigger := sload(getUserSlot(55026))
                            if eq(moveDir, 1) {
                                if gt(px, 15) { px := sub(px, 3) }
                            }
                            if eq(moveDir, 2) {
                                px := add(px, 3)
                                if gt(px, 785) {
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
                            if jumpTrigger {
                                if iszero(jumping) {
                                    pvy := sub(0, 10)
                                    jumping := 1
                                    sstore(getUserSlot(55030), 1)
                                }
                                sstore(getUserSlot(55026), 0)
                            }
                            py := add(py, pvy)
                            pvy := add(pvy, 1)
                            if or(gt(py, 175), eq(py, 175)) {
                                py := 175
                                pvy := 0
                                jumping := 0
                                sstore(getUserSlot(55030), 0)
                            }
                            let ex := sload(getUserSlot(55036))
                            let ey := sload(getUserSlot(55037))
                            let active := sload(getUserSlot(55038))
                            if active {
                                let dx := 0
                                if gt(px, ex) { dx := sub(px, ex) }
                                if iszero(gt(px, ex)) { dx := sub(ex, px) }
                                let dy := 0
                                if gt(py, ey) { dy := sub(py, ey) }
                                if iszero(gt(py, ey)) { dy := sub(ey, py) }
                                if and(lt(dx, 20), lt(dy, 20)) {
                                    if gt(energy, 20) {
                                        energy := sub(energy, 20)
                                    }
                                    if iszero(gt(energy, 20)) {
                                        energy := 0
                                        sstore(getUserSlot(55034), 1)
                                    }
                                }
                            }
                            let hx := sload(getUserSlot(55039))
                            if and(eq(screen, 3), lt(px, hx)) {
                                sstore(getUserSlot(55035), 1)
                                score := add(score, 1000)
                            }
                            sstore(getUserSlot(55027), px)
                            sstore(getUserSlot(55028), py)
                            sstore(getUserSlot(55029), pvy)
                            sstore(getUserSlot(55031), energy)
                            sstore(getUserSlot(55032), score)
                            sstore(getUserSlot(55033), screen)
                        }
                        let playTax := 1
                        if exciseOnChainTax(playTax) {
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), playTax)
                        }
                        sstore(getUserSlot(55024), 0)
                    }
                }
                case 54927 { // Cannonball Blitz Physics Trigger ($D68F)
                    sstore(getUserSlot(54927), val)
                    if val {
                        let px := sload(getUserSlot(54912))
                        let py := sload(getUserSlot(54913))
                        let pvx := sload(getUserSlot(54914))
                        let pvy := sload(getUserSlot(54915))
                        if gt(pvx, 127) { pvx := sub(pvx, 256) }
                        if gt(pvy, 127) { pvy := sub(pvy, 256) }
                        px := add(px, pvx)
                        let onScaffold := checkOnScaffold(py)
                        py := applyAirTime(py, 54926, 54929, onScaffold, pvy)
                        if gt(px, 39) { px := 39 }
                        if gt(py, 24) { py := 24 }
                        sstore(getUserSlot(54912), px)
                        sstore(getUserSlot(54913), py)
                        let ox := sload(getUserSlot(54916))
                        let oy := sload(getUserSlot(54917))
                        let active := sload(getUserSlot(54918))
                        let collided := 0
                        if active {
                            ox := add(ox, 1)
                            if gt(ox, 39) { ox := 0 }
                            sstore(getUserSlot(54916), ox)
                            if and(eq(px, ox), eq(py, oy)) {
                                collided := 1
                                sstore(getUserSlot(54928), 1)
                                sstore(getUserSlot(54912), 0)
                                sstore(getUserSlot(54913), 24)
                            }
                        }
                        let ox2 := sload(getUserSlot(54919))
                        let oy2 := sload(getUserSlot(54920))
                        let active2 := sload(getUserSlot(54921))
                        if active2 {
                            if gt(ox2, 1) { ox2 := sub(ox2, 2) }
                            if iszero(gt(ox2, 1)) { ox2 := 39 }
                            sstore(getUserSlot(54919), ox2)
                            if and(eq(px, ox2), eq(py, oy2)) {
                                collided := 1
                                sstore(getUserSlot(54928), 1)
                                sstore(getUserSlot(54912), 0)
                                sstore(getUserSlot(54913), 24)
                            }
                        }
                        if iszero(collided) {
                            let score := sload(getUserSlot(54925))
                            sstore(getUserSlot(54925), add(score, 10))
                        }
                        let playTax := 1
                        if exciseOnChainTax(playTax) {
                            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), playTax)
                        }
                        let currentScore := sload(getUserSlot(54925))
                        mstore(0x300, currentScore)
                        mstore(0x320, collided)
                        log2(0x300, 64, 0xbc8ad72f2ba16a9205578bc41b22e18413b63200ff9dfe8bc9c8be7ce6823908, getContextUser())
                        sstore(getUserSlot(54927), 0)
                    }
                }
                case 55046 { // Karateka Combat Tick Trigger ($D706)
                    sstore(getUserSlot(55046), val)
                    if val {
                        let p_strike := sload(getUserSlot(55041))
                        let e_stance := sload(getUserSlot(55042))
                        let dist := sload(getUserSlot(55043))
                        let e_health := sload(getUserSlot(55045))
                        if iszero(e_health) { e_health := 100 }
                        if p_strike {
                            let hit, dmg := checkStrikeCollision(p_strike, dist, e_stance)
                            if hit {
                                if gt(dmg, e_health) { dmg := e_health }
                                sstore(getUserSlot(55045), sub(e_health, dmg))
                            }
                        }
                        sstore(getUserSlot(55041), 0)
                        sstore(getUserSlot(55046), 0)
                    }
                }
                case 55062 { // Choplifter Physics Tick Trigger ($D716)
                    sstore(getUserSlot(55062), val)
                    if val {
                        let px := sload(getUserSlot(55056))
                        let py := sload(getUserSlot(55057))
                        let flags := sload(getUserSlot(55058))
                        let fuel := sload(getUserSlot(55059))
                        if gt(fuel, 0) {
                            fuel := sub(fuel, 1)
                            sstore(getUserSlot(55059), fuel)
                            let dir := and(flags, 1)
                            let speed := and(shr(1, flags), 3)
                            if eq(dir, 1) {
                                px := add(px, speed)
                                if gt(px, 319) { px := 319 }
                            }
                            if iszero(dir) {
                                if gt(px, speed) { px := sub(px, speed) }
                                if iszero(gt(px, speed)) { px := 0 }
                            }
                            sstore(getUserSlot(55056), px)
                        }
                        sstore(getUserSlot(55062), 0)
                    }
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
