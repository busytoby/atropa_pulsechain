object "LunarLander" {
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
                sstore(getSlot(0x100), 15000) // X pos = 150.00
                sstore(getSlot(0x101), 10000) // Y pos = 100.00
                sstore(getSlot(0x102), 300)   // VX = 3.00
                sstore(getSlot(0x103), 0)     // VY = 0.00
                sstore(getSlot(0x104), 1000)  // Fuel = 1000
                sstore(getSlot(0x105), 0)     // Status = 0 (flying)
                sstore(getSlot(0x106), 0)     // Score = 0
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // METHOD 2: updateGame(thrustX, thrustY)
            // Selector: 0xa9e7f536
            if eq(selector, 0xa9e7f536) {
                let status := sload(getSlot(0x105))
                if iszero(status) {
                    let tx := calldataload(4)
                    let ty := calldataload(36)

                    let px := sload(getSlot(0x100))
                    let py := sload(getSlot(0x101))
                    let vx := sload(getSlot(0x102))
                    let vy := sload(getSlot(0x103))
                    let fuel := sload(getSlot(0x104))
                    let score := sload(getSlot(0x106))

                    // Apply gravity (0.08 per frame)
                    vy := add(vy, 8)

                    // Apply thrust if fuel is available
                    if gt(fuel, 0) {
                        if ty {
                            vy := sub(vy, 18) // Thrust up: 0.18 per frame
                            fuel := sub(fuel, 2)
                        }
                        if tx {
                            vx := add(vx, mul(tx, 12)) // Thrust sideways: 0.12 per frame
                            let fuelCost := tx
                            if slt(tx, 0) { fuelCost := sub(0, tx) }
                            fuel := sub(fuel, fuelCost)
                        }
                    }

                    // Force fuel bounds
                    if slt(fuel, 0) { fuel := 0 }

                    // Update pos
                    px := add(px, vx)
                    py := add(py, vy)

                    // Bounds checks (10.0 to 790.0)
                    if slt(px, 1000) {
                        px := 1000
                        vx := 0
                    }
                    if sgt(px, 79000) {
                        px := 79000
                        vx := 0
                    }

                    // Ground contact check (ground is at y = 500.0)
                    if or(eq(py, 50000), sgt(py, 50000)) {
                        py := 50000
                        // Landing pad is located at x range [350.0, 450.0]
                        // VX must be within [-1.5, 1.5]
                        // VY must be less than 2.0 (200)
                        if and(and(sgt(px, 34500), slt(px, 45500)), and(slt(vy, 200), and(sgt(vx, sub(0, 150)), slt(vx, 150)))) {
                            status := 1 // Successful landing!
                            score := add(score, add(div(fuel, 10), 1000))
                        }
                        if iszero(eq(status, 1)) {
                            status := 2 // Crashed!
                        }
                        vx := 0
                        vy := 0
                    }

                    sstore(getSlot(0x100), px)
                    sstore(getSlot(0x101), py)
                    sstore(getSlot(0x102), vx)
                    sstore(getSlot(0x103), vy)
                    sstore(getSlot(0x104), fuel)
                    sstore(getSlot(0x105), status)
                    sstore(getSlot(0x106), score)
                }

                mstore(0x00, sload(getSlot(0x100)))
                mstore(0x20, sload(getSlot(0x101)))
                mstore(0x40, sload(getSlot(0x102)))
                mstore(0x60, sload(getSlot(0x103)))
                mstore(0x80, sload(getSlot(0x104)))
                mstore(0xa0, sload(getSlot(0x105)))
                mstore(0xc0, sload(getSlot(0x106)))
                return(0x00, 224)
            }

            revert(0, 0)
        }
    }
}
