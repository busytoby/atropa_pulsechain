object "WaylandCompositor" {
    code {
        // Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // Function selector dispatcher
            let selector := shr(224, calldataload(0))
            
            switch selector
            // createSurface(id, x, y, w, h, layer, tex_id)
            case 0xa3b5c001 {
                let id := calldataload(4)
                let x := calldataload(36)
                let y := calldataload(68)
                let w := calldataload(100)
                let h := calldataload(132)
                let layer := calldataload(164)
                let tex_id := calldataload(196)
                
                // Save surface data to storage (indexed by id)
                sstore(add(shl(8, id), 0), x)
                sstore(add(shl(8, id), 1), y)
                sstore(add(shl(8, id), 2), w)
                sstore(add(shl(8, id), 3), h)
                sstore(add(shl(8, id), 4), layer)
                sstore(add(shl(8, id), 5), tex_id)
                sstore(add(shl(8, id), 6), 1) // active status
                
                return(0, 0)
            }
            // updatePosition(id, x, y, layer)
            case 0xb2c4d002 {
                let id := calldataload(4)
                let x := calldataload(36)
                let y := calldataload(68)
                let layer := calldataload(100)
                
                sstore(add(shl(8, id), 0), x)
                sstore(add(shl(8, id), 1), y)
                sstore(add(shl(8, id), 4), layer)
                
                return(0, 0)
            }
            // composite() -> triggers Vulkan compositing host thunk
            case 0xc1d3e003 {
                // Read compositor layout state from storage and assemble scene graph in memory
                let memStart := 0x80
                let activeCount := 0
                
                // Collect active surfaces (id from 1 to 16 for simplicity)
                for { let id := 1 } lt(id, 17) { id := add(id, 1) } {
                    let active := sload(add(shl(8, id), 6))
                    if active {
                        let offset := add(memStart, mul(activeCount, 192))
                        mstore(add(offset, 0), id)
                        mstore(add(offset, 32), sload(add(shl(8, id), 0))) // x
                        mstore(add(offset, 64), sload(add(shl(8, id), 1))) // y
                        mstore(add(offset, 96), sload(add(shl(8, id), 2))) // w
                        mstore(add(offset, 128), sload(add(shl(8, id), 3))) // h
                        mstore(add(offset, 160), sload(add(shl(8, id), 4))) // layer
                        mstore(add(offset, 192), sload(add(shl(8, id), 5))) // tex_id
                        activeCount := add(activeCount, 1)
                    }
                }
                
                // Write active count at beginning of memory buffer
                mstore(0x60, activeCount)
                
                // Trigger composition via call to address 0x103 (Vulkan composition thunk)
                let success := call(gas(), 0x103, 0, 0x60, add(32, mul(activeCount, 192)), 0, 0)
                
                mstore(0, success)
                return(0, 32)
            }
            default {
                revert(0, 0)
            }
        }
    }
}
