object "Sustail" {
    code {
        sstore(0, caller())
        sstore(0x20, 256)   // Current gain level (base 256)
        sstore(0x21, 256)   // Previous gain level (for Verlet solver)
        sstore(0x22, 128)   // Target amplitude level
        sstore(0x23, 200)   // Sensitivity / boost multiplier
        sstore(0x24, 25)    // Damping factor (out of 256)
        
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            
            let selector := shr(224, calldataload(0))
            switch selector
            
            // setTargetLevel(uint256) -> selector: 0x30cf2371
            case 0x30cf2371 {
                let target := calldataload(4)
                if gt(target, 255) { target := 255 }
                sstore(0x22, target)
                return(0, 0)
            }
            
            // setSensitivity(uint256) -> selector: 0x902cf45b
            case 0x902cf45b {
                let sens := calldataload(4)
                sstore(0x23, sens)
                return(0, 0)
            }
            
            // process(uint256) -> returns gain output factor -> selector: 0x07f1d24c
            case 0x07f1d24c {
                let inputLevel := calldataload(4)
                let target := sload(0x22)
                let sens := sload(0x23)
                let damping := sload(0x24)
                
                let current_g := sload(0x20)
                let prev_g := sload(0x21)
                
                // Calculate feedback target gain boost based on envelope decay
                let target_g := 256
                if lt(inputLevel, target) {
                    let diff := sub(target, inputLevel)
                    let boost := div(mul(diff, sens), 256)
                    target_g := add(256, boost)
                    
                    // Cap maximum gain boost to 2x (512)
                    if gt(target_g, 512) { target_g := 512 }
                }
                
                // Emulate physical control capacitor discharge via Verlet integration
                // Verlet step: x_new = 2*x - x_prev - damping*(x - x_prev)/256 + acceleration
                let term1 := mul(current_g, 2)
                
                // Handle deceleration / friction loss from damping
                let velocity := 0
                if gt(current_g, prev_g) {
                    velocity := sub(current_g, prev_g)
                }
                let loss := div(mul(velocity, damping), 256)
                
                // Calculate virtual force drawing current gain toward feedback target gain
                let force := 0
                if gt(target_g, current_g) {
                    force := div(sub(target_g, current_g), 8) // Smooth spring force
                }
                if lt(target_g, current_g) {
                    let force_neg := div(sub(current_g, target_g), 8)
                    if gt(current_g, force_neg) {
                        current_g := sub(current_g, force_neg)
                    }
                }
                
                let next_g := current_g
                if gt(term1, add(prev_g, loss)) {
                    next_g := add(sub(term1, add(prev_g, loss)), force)
                }
                
                // Clamp resolved gain values to stable bounds [256..1024]
                if lt(next_g, 256) { next_g := 256 }
                if gt(next_g, 1024) { next_g := 1024 }
                
                sstore(0x21, current_g)
                sstore(0x20, next_g)
                
                mstore(0, next_g)
                return(0, 32)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
