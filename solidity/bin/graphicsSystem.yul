object "GraphicsSystem" {
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

            // VIC-II Sprite Registers Map:
            // Storage slots mimic standard Commodore 64 VIC-II I/O registers:
            // 53248 to 53263 (0xD000 to 0xD00F): Sprite 0-7 X/Y coordinates
            // 53269 (0xD015): Sprite Enable Register (8-bit mask)
            // 53278 (0xD01E): Sprite-to-Sprite Collision Register (8-bit mask)

            // Helper to write coordinate byte
            function setReg(addr, val) {
                sstore(addr, val)
            }

            // Helper to get coordinate byte
            function getReg(addr) -> val {
                val := sload(addr)
            }

            // ----------------------------------------------------------------
            // Method 1: updateSprite(uint8 index, uint16 x, uint8 y)
            // Selector: 0xb9e48d3c
            // ----------------------------------------------------------------
            if or(eq(selector, 0xa2f72f27), eq(selector, 0xb9e48d3c)) {
                let index := 0
                let x := 0
                let y := 0
                
                switch selector
                case 0xb9e48d3c {
                    index := shr(248, calldataload(4))
                    x := shr(240, calldataload(36))
                    y := shr(248, calldataload(68))
                }
                default {
                    index := and(calldataload(4), 0xFF)
                    x := and(calldataload(36), 0xFFFF)
                    y := and(calldataload(68), 0xFF)
                }

                if gt(index, 7) { revert(0, 0) }

                // Sprite X registers: 53248 + index * 2
                // Sprite Y registers: 53249 + index * 2
                let baseAddr := add(53248, mul(index, 2))
                
                // Store low X coordinate
                setReg(baseAddr, x)
                // Store Y coordinate
                setReg(add(baseAddr, 1), y)

                // Sprite Enable Register (53269) - set the bit for this sprite to active
                let enabledMask := sload(53269)
                sstore(53269, or(enabledMask, shl(index, 1)))

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 2: checkCollisions() -> uint8 (Returns Sprite Collision Mask)
            // Selector: 0x5a18a994 or 0x954e455a
            // ----------------------------------------------------------------
            if or(eq(selector, 0x5a18a994), eq(selector, 0x954e455a)) {
                let collisionMask := 0
                let enabled := sload(53269)

                // Compare active sprites (0-7) to check for overlapping 24x21 bounding boxes
                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    // Check if sprite i is enabled
                    if and(enabled, shl(i, 1)) {
                        let iAddr := add(53248, mul(i, 2))
                        let ix := getReg(iAddr)
                        let iy := getReg(add(iAddr, 1))

                        for { let j := add(i, 1) } lt(j, 8) { j := add(j, 1) } {
                            // Check if sprite j is enabled
                            if and(enabled, shl(j, 1)) {
                                let jAddr := add(53248, mul(j, 2))
                                let jx := getReg(jAddr)
                                let jy := getReg(add(jAddr, 1))

                                // Bounding box overlap logic (24px width, 21px height)
                                // |ix - jx| < 24 AND |iy - jy| < 21
                                let xDiff := 0
                                if gt(ix, jx) { xDiff := sub(ix, jx) }
                                if iszero(gt(ix, jx)) { xDiff := sub(jx, ix) }

                                let yDiff := 0
                                if gt(iy, jy) { yDiff := sub(iy, jy) }
                                if iszero(gt(iy, jy)) { yDiff := sub(jy, iy) }

                                if and(lt(xDiff, 24), lt(yDiff, 21)) {
                                    // Mark collision flag for both sprites
                                    collisionMask := or(collisionMask, shl(i, 1))
                                    collisionMask := or(collisionMask, shl(j, 1))
                                }
                            }
                        }
                    }
                }

                // Update collision register (53278)
                sstore(53278, collisionMask)

                mstore(0x00, collisionMask)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 3: setSpritePattern(uint8 index, bytes32 part1, bytes32 part2) -> uint256
            // Selector: 0x7d4aa477
            // ----------------------------------------------------------------
            if eq(selector, 0x7d4aa477) {
                let index := and(calldataload(4), 0xFF)
                if gt(index, 7) { revert(0, 0) }

                let part1 := calldataload(36)
                let part2 := calldataload(68)

                let slotBase := add(54000, mul(index, 2))
                sstore(slotBase, part1)
                sstore(add(slotBase, 1), part2)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 4: setSpriteRow(uint8 index, uint8 row, uint256 val) -> uint256
            // Selector: 0x178e36ef
            // ----------------------------------------------------------------
            if eq(selector, 0x178e36ef) {
                let index := and(calldataload(4), 0xFF)
                let row := and(calldataload(36), 0xFF)
                let val := calldataload(68)

                if or(gt(index, 7), gt(row, 20)) { revert(0, 0) }

                // Store at slot: 54000 + index * 32 + row
                let slot := add(54000, add(mul(index, 32), row))
                sstore(slot, val)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 5: setSpritePatternFull(uint8 index, uint256[21] rows) -> uint256
            // Selector: 0x2c1da841
            // ----------------------------------------------------------------
            if eq(selector, 0x2c1da841) {
                let index := and(calldataload(4), 0xFF)
                if gt(index, 7) { revert(0, 0) }

                let slotBase := add(54000, mul(index, 32))
                for { let r := 0 } lt(r, 21) { r := add(r, 1) } {
                    let val := calldataload(add(36, mul(r, 32)))
                    sstore(add(slotBase, r), val)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 6: getScreenRAM() -> bytes
            // Selector: 0x4a5d1e5c
            // ----------------------------------------------------------------
            if eq(selector, 0x4a5d1e5c) {
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(add(1024, i)), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            // ----------------------------------------------------------------
            // Method 7: getColorRAM() -> bytes
            // Selector: 0xace7b8ce
            // ----------------------------------------------------------------
            if eq(selector, 0xace7b8ce) {
                mstore(0x00, 0x20) // Offset
                mstore(0x20, 1000) // Length: 1000 bytes
                
                for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
                    let val := and(sload(add(55296, i)), 0xFF)
                    mstore8(add(0x40, i), val)
                }
                
                return(0x00, 1088) // 64 (headers) + 1024 (padded data size)
            }

            // ----------------------------------------------------------------
            // Method 8: calculateGolfTrajectory(uint256 power, uint256 angle, int256 wind, int256 spin)
            // Selector: 0x22924bb6
            // ----------------------------------------------------------------
            if eq(selector, 0x22924bb6) {
                let power := calldataload(4)
                let angle := calldataload(36)
                let wind := calldataload(68)
                let spin := calldataload(100)

                // Retrieve sin and cos
                let c := 0
                if lt(angle, 15) { c := 96 }
                if and(iszero(lt(angle, 15)), lt(angle, 30)) { c := 91 }
                if and(iszero(lt(angle, 30)), lt(angle, 45)) { c := 81 }
                if and(iszero(lt(angle, 45)), lt(angle, 60)) { c := 70 }
                if and(iszero(lt(angle, 60)), lt(angle, 75)) { c := 50 }
                if and(iszero(lt(angle, 75)), lt(angle, 90)) { c := 25 }
                if iszero(lt(angle, 90)) { c := 0 }

                let s := 0
                if lt(angle, 15) { s := 25 }
                if and(iszero(lt(angle, 15)), lt(angle, 30)) { s := 50 }
                if and(iszero(lt(angle, 30)), lt(angle, 45)) { s := 70 }
                if and(iszero(lt(angle, 45)), lt(angle, 60)) { s := 86 }
                if and(iszero(lt(angle, 60)), lt(angle, 75)) { s := 96 }
                if iszero(lt(angle, 75)) { s := 100 }

                // Initial velocities scaled by 10
                let vx := div(mul(power, c), 10)
                let vy := div(mul(power, s), 10)

                // Track variables
                let px := 0
                let py := 0
                
                // Write arrays to memory starting at 0x80
                // Array 1: X coordinates
                // Array 2: Y coordinates
                
                let maxSteps := 30
                let stepCount := 0
                
                // Loop to calculate trajectory
                for { let i := 0 } lt(i, maxSteps) { i := add(i, 1) } {
                    // Update positions
                    px := add(px, div(vx, 10))
                    
                    // Add wind speed effect to X position
                    px := add(px, wind)
                    // Add hook/slice spin drift to X position
                    px := add(px, spin)
                    
                    py := add(py, div(vy, 10))
                    
                    // Store in memory
                    mstore(add(0x100, mul(i, 32)), px)
                    mstore(add(0x500, mul(i, 32)), py)
                    
                    stepCount := add(stepCount, 1)
                    
                    // Update vertical velocity due to gravity
                    vy := sub(vy, 8)
                    
                    // Apply drag to velocities
                    vx := div(mul(vx, 98), 100)
                    vy := div(mul(vy, 98), 100)
                    
                    // Stop if we hit the ground (y < 0, but since unsigned, check if overflowed or 0)
                    if gt(py, 0xF000000000000000000000000000000000000000000000000000000000000000) {
                        break
                    }
                    if and(eq(py, 0), gt(i, 0)) {
                        break
                    }
                }
                
                // Construct return structure:
                // 0x00: Offset to xCoords (0x40)
                // 0x20: Offset to yCoords (0x40 + 0x20 + stepCount * 32)
                // 0x40: Length of xCoords (stepCount)
                // 0x60: xCoords data...
                // ...
                // 0x60 + stepCount * 32: Length of yCoords (stepCount)
                // 0x80 + stepCount * 32: yCoords data...
                
                mstore(0x00, 0x40)
                let yOffset := add(0x60, mul(stepCount, 32))
                mstore(0x20, yOffset)
                
                mstore(0x40, stepCount)
                for { let j := 0 } lt(j, stepCount) { j := add(j, 1) } {
                    let val := mload(add(0x100, mul(j, 32)))
                    mstore(add(0x60, mul(j, 32)), val)
                }
                
                mstore(yOffset, stepCount)
                for { let j := 0 } lt(j, stepCount) { j := add(j, 1) } {
                    let val := mload(add(0x500, mul(j, 32)))
                    mstore(add(add(yOffset, 32), mul(j, 32)), val)
                }
                
                let totalSize := add(yOffset, add(32, mul(stepCount, 32)))
                return(0x00, totalSize)
            }

            // ----------------------------------------------------------------
            // Method 9: getMercenaryGeometry(uint256 objId) -> int256[]
            // Selector: 0xbc9cd173
            // ----------------------------------------------------------------
            if eq(selector, 0xbc9cd173) {
                let objId := calldataload(4)
                let mStart := 0x40
                let mAddr := mStart
                
                if eq(objId, 1) {
                    // Central Tower (9 vertices, 16 lines)
                    mstore(mAddr, 9) mAddr := add(mAddr, 32) // V
                    // X
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    // Y
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 80) mAddr := add(mAddr, 32)
                    mstore(mAddr, 80) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 80) mAddr := add(mAddr, 32)
                    mstore(mAddr, 80) mAddr := add(mAddr, 32)
                    mstore(mAddr, 120) mAddr := add(mAddr, 32)
                    // Z
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    
                    mstore(mAddr, 16) mAddr := add(mAddr, 32) // L
                    // Lines
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 1) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 3) mAddr := add(mAddr, 32) mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 5) mAddr := add(mAddr, 32) mstore(mAddr, 6) mAddr := add(mAddr, 32)
                    mstore(mAddr, 6) mAddr := add(mAddr, 32) mstore(mAddr, 7) mAddr := add(mAddr, 32)
                    mstore(mAddr, 7) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 6) mAddr := add(mAddr, 32)
                    mstore(mAddr, 3) mAddr := add(mAddr, 32) mstore(mAddr, 7) mAddr := add(mAddr, 32)
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) mstore(mAddr, 6) mAddr := add(mAddr, 32)
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) mstore(mAddr, 7) mAddr := add(mAddr, 32)
                }
                
                if eq(objId, 2) {
                    // Novagen Hangar (8 vertices, 12 lines)
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) // V
                    // X
                    mstore(mAddr, sub(0, 50)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 50) mAddr := add(mAddr, 32)
                    mstore(mAddr, 50) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 50)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 50)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 50) mAddr := add(mAddr, 32)
                    mstore(mAddr, 50) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 50)) mAddr := add(mAddr, 32)
                    // Y
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 35) mAddr := add(mAddr, 32)
                    mstore(mAddr, 35) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 35) mAddr := add(mAddr, 32)
                    mstore(mAddr, 35) mAddr := add(mAddr, 32)
                    // Z
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 30)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    mstore(mAddr, 30) mAddr := add(mAddr, 32)
                    
                    mstore(mAddr, 12) mAddr := add(mAddr, 32) // L
                    // Lines
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 1) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 3) mAddr := add(mAddr, 32) mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 5) mAddr := add(mAddr, 32) mstore(mAddr, 6) mAddr := add(mAddr, 32)
                    mstore(mAddr, 6) mAddr := add(mAddr, 32) mstore(mAddr, 7) mAddr := add(mAddr, 32)
                    mstore(mAddr, 7) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 6) mAddr := add(mAddr, 32)
                    mstore(mAddr, 3) mAddr := add(mAddr, 32) mstore(mAddr, 7) mAddr := add(mAddr, 32)
                }
                
                if eq(objId, 3) {
                    // Spaceship / Interstellar Cruiser (6 vertices, 10 lines)
                    mstore(mAddr, 6) mAddr := add(mAddr, 32) // V
                    // X
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 20) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 10)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 10) mAddr := add(mAddr, 32)
                    // Y
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 15) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 5)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 5)) mAddr := add(mAddr, 32)
                    // Z
                    mstore(mAddr, 50) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 20)) mAddr := add(mAddr, 32)
                    
                    mstore(mAddr, 10) mAddr := add(mAddr, 32) // L
                    // Lines
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 1) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 4) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 5) mAddr := add(mAddr, 32)
                }
                
                if eq(objId, 4) {
                    // Defense Pylon (5 vertices, 8 lines)
                    mstore(mAddr, 5) mAddr := add(mAddr, 32) // V
                    // X
                    mstore(mAddr, sub(0, 10)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 10) mAddr := add(mAddr, 32)
                    mstore(mAddr, 10) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 10)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    // Y
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 100) mAddr := add(mAddr, 32)
                    // Z
                    mstore(mAddr, sub(0, 10)) mAddr := add(mAddr, 32)
                    mstore(mAddr, sub(0, 10)) mAddr := add(mAddr, 32)
                    mstore(mAddr, 10) mAddr := add(mAddr, 32)
                    mstore(mAddr, 10) mAddr := add(mAddr, 32)
                    mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    
                    mstore(mAddr, 8) mAddr := add(mAddr, 32) // L
                    // Lines
                    mstore(mAddr, 0) mAddr := add(mAddr, 32) mstore(mAddr, 1) mAddr := add(mAddr, 32)
                    mstore(mAddr, 1) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 2) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                    mstore(mAddr, 3) mAddr := add(mAddr, 32) mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 0) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 1) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 2) mAddr := add(mAddr, 32)
                    mstore(mAddr, 4) mAddr := add(mAddr, 32) mstore(mAddr, 3) mAddr := add(mAddr, 32)
                }
                
                mstore(0x00, 0x20)
                let totalElements := div(sub(mAddr, 0x40), 32)
                mstore(0x20, totalElements)
                let totalSize := add(0x40, mul(totalElements, 32))
                return(0x00, totalSize)
            }

            // ----------------------------------------------------------------
            // Method 10: updateFighterState(uint256 x, uint256 y, int256 vx, int256 vy, uint256 action, uint256 targetX, uint256 targetY) -> (uint256, uint256, int256, int256, uint256)
            // Selector: 0xd6c09b2e
            // ----------------------------------------------------------------
            if eq(selector, 0xd6c09b2e) {
                let x := calldataload(4)
                let y := calldataload(36)
                let vx := calldataload(68)
                let vy := calldataload(100)
                let action := calldataload(132)
                let targetX := calldataload(164)
                let targetY := calldataload(196)

                // 1. Process Actions
                if eq(action, 1) { // Jump
                    if eq(y, 0) {
                        vy := 15
                    }
                }
                if eq(action, 2) { // Left
                    vx := sub(0, 5)
                }
                if eq(action, 3) { // Right
                    vx := 5
                }
                if eq(action, 0) { // Idle/Stop horizontal
                    vx := 0
                }

                // 2. Integrate Physics
                x := add(x, vx)
                y := add(y, vy)
                
                // Apply Gravity
                vy := sub(vy, 1)

                // Floor boundary check (y < 0 is large in unsigned)
                if gt(y, 200) {
                    y := 0
                    vy := 0
                }

                // Clamp X boundaries
                if gt(x, 320) {
                    if lt(x, 1000) { x := 320 }
                    if iszero(lt(x, 1000)) { x := 0 }
                }

                // 3. Collision Bounding Box Checks
                let width := 16
                let height := 24
                if eq(action, 4) { // Attack / Flying Kick
                    width := 24
                    height := 16
                }
                if eq(action, 5) { // Chuck Norris Punch
                    width := 20
                    height := 22
                }
                if eq(action, 6) { // Chuck Norris Kick
                    width := 22
                    height := 14
                }
                if eq(action, 7) { // Chuck Norris Block
                    width := 0
                    height := 0
                }
                if eq(action, 8) { // Chuck Norris Somersault Superkick
                    width := 32
                    height := 18
                }

                let xDiff := 0
                if gt(x, targetX) { xDiff := sub(x, targetX) }
                if iszero(gt(x, targetX)) { xDiff := sub(targetX, x) }

                let yDiff := 0
                if gt(y, targetY) { yDiff := sub(y, targetY) }
                if iszero(gt(y, targetY)) { yDiff := sub(targetY, y) }

                let hit := 0
                if and(lt(xDiff, width), lt(yDiff, height)) {
                    hit := 1
                }

                // 4. Return new state
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, vx)
                mstore(0x60, vy)
                mstore(0x80, hit)
                return(0x00, 160)
            }

            // ----------------------------------------------------------------
            // Method 11: simulateXonoxDoubleEnder(uint256 side, uint256 input1, uint256 input2, ...)
            // Selector: 0xe577232a
            // ----------------------------------------------------------------
            if eq(selector, 0xe577232a) {
                let side := calldataload(4)
                let input1 := calldataload(36)
                let input2 := calldataload(68)
                let input3 := calldataload(100)
                let input4 := calldataload(132)
                let input5 := calldataload(164)
                let input6 := calldataload(196)

                // Side 0: Artillery Duel Ballistic Simulator
                if eq(side, 0) {
                    let t := input1
                    let angleDeg := input2
                    let velocity := input3
                    let wind := input4
                    let targetX := input5
                    let targetY := input6

                    let scale := 1000000000000000000

                    // ROM Sine/Cosine Helper logic
                    let cosVal := 1000000000000000000
                    let sinVal := 0

                    if eq(angleDeg, 15) {
                        sinVal := 258819045102520762
                        cosVal := 965925826289068286
                    }
                    if eq(angleDeg, 30) {
                        sinVal := 500000000000000000
                        cosVal := 866025403784438646
                    }
                    if eq(angleDeg, 45) {
                        sinVal := 707106781186547524
                        cosVal := 707106781186547524
                    }
                    if eq(angleDeg, 60) {
                        sinVal := 866025403784438646
                        cosVal := 500000000000000000
                    }
                    if eq(angleDeg, 75) {
                        sinVal := 965925826289068286
                        cosVal := 258819045102520762
                    }
                    if eq(angleDeg, 90) {
                        sinVal := 1000000000000000000
                        cosVal := 0
                    }
                    if and(gt(angleDeg, 0), lt(angleDeg, 90)) {
                        if iszero(eq(angleDeg, 15)) {
                            if iszero(eq(angleDeg, 30)) {
                                if iszero(eq(angleDeg, 45)) {
                                    if iszero(eq(angleDeg, 60)) {
                                        if iszero(eq(angleDeg, 75)) {
                                            // Fallback Linear
                                            sinVal := mul(angleDeg, 17453292519943295)
                                            cosVal := mul(sub(90, angleDeg), 17453292519943295)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // x = (v0 * cos(theta) + wind) * t
                    let termX := add(div(mul(velocity, cosVal), scale), wind)
                    let x := div(mul(termX, t), scale)

                    // y = v0 * sin(theta) * t - (0.5 * g * t^2)
                    let gHalf := 4900000000000000000
                    let termY1 := div(mul(velocity, sinVal), scale)
                    let termY1_t := div(mul(termY1, t), scale)
                    let termY2 := div(mul(gHalf, mul(t, t)), mul(scale, scale))
                    let y := sub(termY1_t, termY2)

                    let hitTarget := 0
                    if iszero(lt(x, targetX)) {
                        if iszero(lt(y, targetY)) {
                            hitTarget := 1
                        }
                    }

                    let hitTerrain := 0
                    if iszero(gt(y, 0)) {
                        hitTerrain := 1
                    }

                    mstore(0x00, x)
                    mstore(0x20, y)
                    mstore(0x40, hitTarget)
                    mstore(0x60, hitTerrain)
                    mstore(0x80, 0)
                    return(0x00, 160)
                }

                // Side 1: Chuck Norris Combat step forwarding
                if eq(side, 1) {
                    let x := input1
                    let y := input2
                    let vx := input3
                    let vy := input4
                    let action := input5
                    let targetX := input6
                    let targetY := 0 // default 0 for ground level combat

                    // Process Jump Action
                    if eq(action, 1) {
                        if eq(y, 0) {
                            vy := 15
                        }
                    }
                    if eq(action, 2) { vx := sub(0, 5) }
                    if eq(action, 3) { vx := 5 }
                    if eq(action, 0) { vx := 0 }

                    x := add(x, vx)
                    y := add(y, vy)
                    vy := sub(vy, 1)

                    if gt(y, 200) {
                        y := 0
                        vy := 0
                    }

                    if gt(x, 320) {
                        if lt(x, 1000) { x := 320 }
                        if iszero(lt(x, 1000)) { x := 0 }
                    }

                    let width := 16
                    let height := 24
                    if eq(action, 4) { width := 24 height := 16 }
                    if eq(action, 5) { width := 20 height := 22 }
                    if eq(action, 6) { width := 22 height := 14 }
                    if eq(action, 7) { width := 0 height := 0 }
                    if eq(action, 8) { width := 32 height := 18 }

                    let xDiff := 0
                    if gt(x, targetX) { xDiff := sub(x, targetX) }
                    if iszero(gt(x, targetX)) { xDiff := sub(targetX, x) }

                    let yDiff := 0
                    if gt(y, targetY) { yDiff := sub(y, targetY) }
                    if iszero(gt(y, targetY)) { yDiff := sub(targetY, y) }

                    let hit := 0
                    if and(lt(xDiff, width), lt(yDiff, height)) {
                        hit := 1
                    }

                    mstore(0x00, x)
                    mstore(0x20, y)
                    mstore(0x40, vx)
                    mstore(0x60, vy)
                    mstore(0x80, hit)
                    return(0x00, 160)
                }

                revert(0, 0)
            }

            // ----------------------------------------------------------------
            // Method 12: animateSkeletalJoint(parentX, parentY, parentZ, localOffsetX, localOffsetY, localOffsetZ, angleStart, angleEnd, t)
            // Selector: 0x8bfb5a7b
            // ----------------------------------------------------------------
            if eq(selector, 0x8bfb5a7b) {
                let parentX := calldataload(4)
                let parentY := calldataload(36)
                let parentZ := calldataload(68)
                let localOffsetX := calldataload(100)
                let localOffsetY := calldataload(132)
                let localOffsetZ := calldataload(164)
                let angleStart := calldataload(196)
                let angleEnd := calldataload(228)
                let t := calldataload(260) // 0 to 1000 representing LERP (0.0 to 1.0)

                // 1. Keyframe Angle Interpolation (LERP)
                let angleDiff := 0
                let isNegativeAngle := lt(angleEnd, angleStart)
                if isNegativeAngle {
                    angleDiff := sub(angleStart, angleEnd)
                }
                if iszero(isNegativeAngle) {
                    angleDiff := sub(angleEnd, angleStart)
                }

                let angleLerp := div(mul(angleDiff, t), 1000)
                let angleDeg := angleStart
                if isNegativeAngle {
                    angleDeg := sub(angleStart, angleLerp)
                }
                if iszero(isNegativeAngle) {
                    angleDeg := add(angleStart, angleLerp)
                }

                // Ensure angle is bounded within 0-360
                angleDeg := mod(angleDeg, 360)

                // 2. Trigonometric Lookup for angleDeg
                let scale := 1000000000000000000
                let cosVal := 1000000000000000000
                let sinVal := 0

                // Map angle to first quadrant (0-90) for high precision lookup
                let quadrant := div(angleDeg, 90)
                let quadAngle := mod(angleDeg, 90)
                
                if eq(quadAngle, 15) {
                    sinVal := 258819045102520762
                    cosVal := 965925826289068286
                }
                if eq(quadAngle, 30) {
                    sinVal := 500000000000000000
                    cosVal := 866025403784438646
                }
                if eq(quadAngle, 45) {
                    sinVal := 707106781186547524
                    cosVal := 707106781186547524
                }
                if eq(quadAngle, 60) {
                    sinVal := 866025403784438646
                    cosVal := 500000000000000000
                }
                if eq(quadAngle, 75) {
                    sinVal := 965925826289068286
                    cosVal := 258819045102520762
                }
                if eq(quadAngle, 90) {
                    sinVal := 1000000000000000000
                    cosVal := 0
                }
                if and(gt(quadAngle, 0), lt(quadAngle, 90)) {
                    if iszero(eq(quadAngle, 15)) {
                        if iszero(eq(quadAngle, 30)) {
                            if iszero(eq(quadAngle, 45)) {
                                if iszero(eq(quadAngle, 60)) {
                                    if iszero(eq(quadAngle, 75)) {
                                        sinVal := mul(quadAngle, 17453292519943295)
                                        cosVal := mul(sub(90, quadAngle), 17453292519943295)
                                    }
                                }
                            }
                        }
                    }
                }

                // Adjust sin/cos sign based on quadrant
                if eq(quadrant, 1) { // 90 to 180 degrees
                    let temp := sinVal
                    sinVal := cosVal
                    cosVal := sub(0, temp)
                }
                if eq(quadrant, 2) { // 180 to 270 degrees
                    sinVal := sub(0, sinVal)
                    cosVal := sub(0, cosVal)
                }
                if eq(quadrant, 3) { // 270 to 360 degrees
                    let temp := sinVal
                    sinVal := sub(0, cosVal)
                    cosVal := temp
                }

                // 3. 2D Rotation on XY plane
                // xRot = (localOffsetX * cosVal - localOffsetY * sinVal) / scale
                // yRot = (localOffsetX * sinVal + localOffsetY * cosVal) / scale
                let xRot := div(sub(mul(localOffsetX, cosVal), mul(localOffsetY, sinVal)), scale)
                let yRot := div(add(mul(localOffsetX, sinVal), mul(localOffsetY, cosVal)), scale)

                // 4. Translate by parent coordinate
                let childX := add(parentX, xRot)
                let childY := add(parentY, yRot)
                let childZ := add(parentZ, localOffsetZ)

                mstore(0x00, childX)
                mstore(0x20, childY)
                mstore(0x40, childZ)
                return(0x00, 96)
            }

            // ----------------------------------------------------------------
            // Method 13: decodeAudioPWM(uint256 pcmSample, uint256 carrierTime) -> uint256
            // Selector: 0xf5d914ab
            // ----------------------------------------------------------------
            if eq(selector, 0xf5d914ab) {
                let pcmSample := calldataload(4)
                let carrierTime := calldataload(36)

                let carrierThreshold := mod(carrierTime, 256)
                let speakerState := 0
                if gt(pcmSample, carrierThreshold) {
                    speakerState := 1
                }

                mstore(0x00, speakerState)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 14: decodeAudioBit(uint256 packedByte, uint256 sampleIndex) -> uint256
            // Selector: 0xc3a39e8a
            // ----------------------------------------------------------------
            if eq(selector, 0xc3a39e8a) {
                let packedByte := calldataload(4)
                let sampleIndex := calldataload(36)

                let shiftAmount := sub(7, and(sampleIndex, 0x07))
                let bitVal := and(shr(shiftAmount, packedByte), 1)

                mstore(0x00, bitVal)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
