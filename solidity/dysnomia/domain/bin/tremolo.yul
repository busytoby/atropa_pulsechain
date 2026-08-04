object "Tremolo" {
    code {
        sstore(0, caller()) // Owner
        sstore(0x10, 5)     // Default Rate (LFO Timer reload)
        sstore(0x11, 128)   // Default Depth (out of 256)
        sstore(0x12, 5)     // Initial Timer Counter
        sstore(0x13, 1)     // LFO Direction (1 = Up, 0 = Down)
        sstore(0x14, 0)     // Current LFO Output Value
        
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            
            let selector := shr(224, calldataload(0))
            switch selector
            
            // setRate(uint256) -> selector: 0x7b6a4821
            case 0x7b6a4821 {
                let rate := calldataload(4)
                if iszero(rate) { rate := 1 }
                sstore(0x10, rate)
                return(0, 0)
            }
            
            // setDepth(uint256) -> selector: 0x9e12bf4a
            case 0x9e12bf4a {
                let depth := calldataload(4)
                if gt(depth, 256) { depth := 256 }
                sstore(0x11, depth)
                return(0, 0)
            }
            
            // tick() -> returns current gain factor (0..256) -> selector: 0x99a23412
            case 0x99a23412 {
                let rate := sload(0x10)
                let depth := sload(0x11)
                let timer := sload(0x12)
                let direction := sload(0x13)
                let lfo := sload(0x14)
                
                // Decrement Timer 1 Counter
                if gt(timer, 0) {
                    timer := sub(timer, 1)
                }
                
                // On Timer 1 Underflow, advance the LFO phase
                if iszero(timer) {
                    timer := rate // Reload Timer
                    
                    switch direction
                    case 1 { // Incrementing
                        lfo := add(lfo, 8)
                        if gt(lfo, 255) {
                            lfo := 255
                            direction := 0 // Change direction to Down
                        }
                    }
                    case 0 { // Decrementing
                        if lt(lfo, 8) {
                            lfo := 0
                            direction := 1 // Change direction to Up
                        }
                        if gt(lfo, 0) {
                            lfo := sub(lfo, 8)
                        }
                    }
                    
                    sstore(0x13, direction)
                    sstore(0x14, lfo)
                }
                
                sstore(0x12, timer)
                
                // Calculate gain modulation: gain = 256 - (LFO * depth / 256)
                let attenuation := div(mul(lfo, depth), 256)
                let gain := sub(256, attenuation)
                
                mstore(0, gain)
                return(0, 32)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
