object "BlinkingLightsPanel" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // updateBusState(uint256 addr, uint256 data, uint256 statusFlags) -> returns (uint256)
            // selector: 0x4da31583
            if eq(selector, 0x4da31583) {
                let addr := and(calldataload(4), 0xFFFF)
                let data := and(calldataload(36), 0xFF)
                let statusFlags := and(calldataload(68), 0xFF)
                
                sstore(56000, addr)
                sstore(56001, data)
                sstore(56002, statusFlags)
                
                let count := sload(56003)
                count := add(count, 1)
                sstore(56003, count)
                
                // Track transitions for each of the 16 address bits (for persistence-of-vision intensity)
                for { let i := 0 } lt(i, 16) { i := add(i, 1) } {
                    let bit := and(shr(i, addr), 1)
                    let lastStateSlot := add(56030, i)
                    let lastState := sload(lastStateSlot)
                    
                    if iszero(eq(bit, lastState)) {
                        let transitionSlot := add(56010, i)
                        let transCount := sload(transitionSlot)
                        sstore(transitionSlot, add(transCount, 1))
                        sstore(lastStateSlot, bit)
                    }
                }
                
                mstore(0x00, count)
                return(0x00, 32)
            }
            
            // getPanelLEDs() -> returns (uint256 addressLEDs, uint256 dataLEDs, uint256 statusLEDs, uint256[16] intensity)
            // selector: 0xed7517e9
            if eq(selector, 0xed7517e9) {
                let addr := sload(56000)
                let data := sload(56001)
                let statusFlags := sload(56002)
                let totalUpdates := sload(56003)
                
                mstore(0x00, addr)
                mstore(0x20, data)
                mstore(0x40, statusFlags)
                
                // Calculate intensity (0-255) for each of the 16 LEDs based on transitions relative to total updates
                for { let i := 0 } lt(i, 16) { i := add(i, 1) } {
                    let transitionSlot := add(56010, i)
                    let transCount := sload(transitionSlot)
                    
                    let intensity := 0
                    if totalUpdates {
                        // intensity = (transCount * 255) / totalUpdates
                        intensity := div(mul(transCount, 255), totalUpdates)
                        if gt(intensity, 255) { intensity := 255 }
                    }
                    mstore(add(0x60, mul(i, 32)), intensity)
                }
                
                return(0x00, 608) // 96 (headers) + 512 (16 * 32 bytes array)
            }
            
            revert(0, 0)
        }
    }
}
