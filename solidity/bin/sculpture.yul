object "Sculpture" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
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

            // Helper to get voxel storage slot key from (x, y, z)
            function getVoxelSlot(x, y, z) -> slot {
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, z)
                slot := keccak256(0x00, 96)
            }

            // ----------------------------------------------------------------
            // METHOD 1: setVoxel(uint256 x, uint256 y, uint256 z, uint256 color)
            // Selector: 0x7a22e84c
            // Packs and registers a voxel coordinate and its color.
            // ----------------------------------------------------------------
            if eq(selector, 0xae34fc53) {
                let x := calldataload(4)
                let y := calldataload(36)
                let z := calldataload(68)
                let color := calldataload(100)

                // 1. Store the color directly at the namespaced slot key
                let slotKey := getVoxelSlot(x, y, z)
                sstore(slotKey, color)

                // 2. Add to the list of voxels for rendering
                let count := sload(0)
                
                // Pack coordinates: 
                // Bits 0-79: x
                // Bits 80-159: y
                // Bits 160-239: z
                // Bits 240-255: color
                let packed := add(x, add(shl(80, y), add(shl(160, z), shl(240, and(color, 0xFFFF)))))
                sstore(add(1, count), packed)
                sstore(0, add(count, 1))

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: getVoxel(uint256 x, uint256 y, uint256 z) -> uint256 color
            // Selector: 0x48b61a99
            // Queries color of a voxel coordinate.
            // ----------------------------------------------------------------
            if eq(selector, 0xe177b015) {
                let x := calldataload(4)
                let y := calldataload(36)
                let z := calldataload(68)
                let slotKey := getVoxelSlot(x, y, z)
                mstore(0x00, sload(slotKey))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: project(uint256 cosVal, uint256 sinVal) -> (uint256[] projectedPoints)
            // Selector: 0x9c3d4f12
            // Rotates and projects 3D coordinates onto a 2D orthographic plane.
            // Output format: [length, x1_proj, y1_proj, color1, x2_proj, y2_proj, color2, ...]
            // cosVal and sinVal are scaled by 1000.
            // ----------------------------------------------------------------
            if eq(selector, 0x3c975b4f) {
                let cosVal := calldataload(4)
                let sinVal := calldataload(36)
                let count := sload(0)

                // Write array length prefix to output memory starting at 0x20
                mstore(0x20, mul(count, 3))

                // Offset pointers for data insertion
                let memOffset := 0x40

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let packed := sload(add(1, i))
                    
                    // Unpack x, y, z, color
                    let x := and(packed, 0xFFFFFFFFFFFFFFFFFFFF)
                    let y := and(shr(80, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let z := and(shr(160, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let color := and(shr(240, packed), 0xFFFF)

                    // Fixed-point orthographic projection:
                    // x_proj = (x * cosVal - z * sinVal) / 1000
                    let xProj := sdiv(sub(mul(x, cosVal), mul(z, sinVal)), 1000)
                    let yProj := y

                    mstore(memOffset, xProj)
                    mstore(add(memOffset, 32), yProj)
                    mstore(add(memOffset, 64), color)
                    memOffset := add(memOffset, 96)
                }

                // Return array layout (offset header at 0x00, length at 0x20, values at 0x40)
                mstore(0x00, 32) // points to array header at 0x20
                let totalBytes := add(64, mul(count, 96))
                return(0x00, totalBytes)
            }

            // ----------------------------------------------------------------
            // METHOD 4: stepLife()
            // Selector: 0xd6c83086 (corresponds to stepLife())
            // Mutates the voxel sculpture using 3D Conway's Game of Life rules.
            // Under 3D Life rules: 
            // - Any active voxel survives if it has between 5 and 7 active neighbors.
            // - Any inactive voxel becomes active if it has exactly 6 active neighbors.
            // We search coordinates of current active voxels and all their neighbors.
            // ----------------------------------------------------------------
            if eq(selector, 0xd6c83086) {
                let count := sload(0)
                if iszero(count) {
                    mstore(0x00, 0)
                    return(0x00, 32)
                }

                // Temporary memory layout to collect unique candidate coords
                // Candidates list starts at 0x00 in free memory (let's say 0x1000)
                let candStart := 0x1000
                let candCount := 0

                // Local helper to add a candidate if not already in list
                // To keep it simple and compile-friendly in Yul, we pack candidates as:
                // Bits 0-79: x, Bits 80-159: y, Bits 160-239: z
                function addCandidate(x, y, z, startPtr, candCountPtr) -> newCount {
                    let packed := add(x, add(shl(80, y), shl(160, z)))
                    let curCount := mload(candCountPtr)
                    let found := 0
                    for { let i := 0 } lt(i, curCount) { i := add(i, 1) } {
                        if eq(mload(add(startPtr, mul(i, 32))), packed) {
                            found := 1
                            break
                        }
                    }
                    if iszero(found) {
                        mstore(add(startPtr, mul(curCount, 32)), packed)
                        curCount := add(curCount, 1)
                        mstore(candCountPtr, curCount)
                    }
                    newCount := curCount
                }

                // Populate candidates: all current active voxels and their 26 neighbors
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let packed := sload(add(1, i))
                    let px := and(packed, 0xFFFFFFFFFFFFFFFFFFFF)
                    let py := and(shr(80, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let pz := and(shr(160, packed), 0xFFFFFFFFFFFFFFFFFFFF)

                    // Add center and all neighbors in 3D (-1 to +1 on all axes)
                    // In a Yul loop, we can just expand or loop:
                    for { let dx := sub(px, 1) } lt(dx, add(px, 2)) { dx := add(dx, 1) } {
                        for { let dy := sub(py, 1) } lt(dy, add(py, 2)) { dy := add(dy, 1) } {
                            for { let dz := sub(pz, 1) } lt(dz, add(pz, 2)) { dz := add(dz, 1) } {
                                candCount := addCandidate(dx, dy, dz, candStart, 0x08)
                            }
                        }
                    }
                }

                // Collect the next generation voxels in memory starting at another offset
                let nextStart := add(candStart, mul(candCount, 32))
                let nextCount := 0

                // For each candidate, count active neighbors
                for { let i := 0 } lt(i, candCount) { i := add(i, 1) } {
                    let packed := mload(add(candStart, mul(i, 32)))
                    let cx := and(packed, 0xFFFFFFFFFFFFFFFFFFFF)
                    let cy := and(shr(80, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let cz := and(shr(160, packed), 0xFFFFFFFFFFFFFFFFFFFF)

                    let activeNeighbors := 0
                    for { let dx := sub(cx, 1) } lt(dx, add(cx, 2)) { dx := add(dx, 1) } {
                        for { let dy := sub(cy, 1) } lt(dy, add(cy, 2)) { dy := add(dy, 1) } {
                            for { let dz := sub(cz, 1) } lt(dz, add(cz, 2)) { dz := add(dz, 1) } {
                                if iszero(and(and(eq(dx, cx), eq(dy, cy)), eq(dz, cz))) {
                                    let slot := getVoxelSlot(dx, dy, dz)
                                    if sload(slot) {
                                        activeNeighbors := add(activeNeighbors, 1)
                                    }
                                }
                            }
                        }
                    }

                    // Check if current voxel is active
                    let isCurrentActive := sload(getVoxelSlot(cx, cy, cz))
                    let survives := 0

                    if isCurrentActive {
                        // Survival rules: between 5 and 7 active neighbors
                        if and(gt(activeNeighbors, 4), lt(activeNeighbors, 8)) {
                            survives := 1
                        }
                    }
                    if iszero(isCurrentActive) {
                        // Birth rules: exactly 6 active neighbors
                        if eq(activeNeighbors, 6) {
                            survives := 1
                        }
                    }

                    if survives {
                        // Get color: if new birth, inherit a default color (e.g. '*' = 0x2A), otherwise keep existing
                        let color := isCurrentActive
                        if iszero(color) { color := 0x2A } // Default birth char '*'
                        let nextPacked := add(cx, add(shl(80, cy), add(shl(160, cz), shl(240, color))))
                        mstore(add(nextStart, mul(nextCount, 32)), nextPacked)
                        nextCount := add(nextCount, 1)
                    }
                }

                // Clear old active voxels from storage
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let packed := sload(add(1, i))
                    let px := and(packed, 0xFFFFFFFFFFFFFFFFFFFF)
                    let py := and(shr(80, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let pz := and(shr(160, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    sstore(getVoxelSlot(px, py, pz), 0)
                    sstore(add(1, i), 0)
                }

                // Save new voxels
                sstore(0, nextCount)
                for { let i := 0 } lt(i, nextCount) { i := add(i, 1) } {
                    let packed := mload(add(nextStart, mul(i, 32)))
                    let px := and(packed, 0xFFFFFFFFFFFFFFFFFFFF)
                    let py := and(shr(80, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let pz := and(shr(160, packed), 0xFFFFFFFFFFFFFFFFFFFF)
                    let color := and(shr(240, packed), 0xFFFF)
                    sstore(getVoxelSlot(px, py, pz), color)
                    sstore(add(1, i), packed)
                }

                mstore(0x00, nextCount)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
