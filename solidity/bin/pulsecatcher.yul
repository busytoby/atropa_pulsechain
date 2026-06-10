object "TTLPulseCatcher" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // initCircuit(uint256 triggerMode)
            // selector: 0xf664ebcd
            if eq(selector, 0xf664ebcd) {
                let triggerMode := calldataload(4)
                sstore(57000, 0) // LATCH_STATE = 0
                sstore(57001, triggerMode)
                sstore(57002, 0) // PREV_TEST_IN = 0
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            // updateSignal(uint256 signalValue) -> returns (uint256 latched, uint256 pulseLedState)
            // selector: 0x8dacac95
            if eq(selector, 0x8dacac95) {
                let signalValue := and(calldataload(4), 1)
                
                let latchState := sload(57000)
                let triggerMode := sload(57001)
                let prevTestIn := sload(57002)
                
                let transitionOccurred := 0
                
                // Falling Edge (1 -> 0)
                if iszero(triggerMode) {
                    if eq(prevTestIn, 1) {
                        if iszero(signalValue) {
                            transitionOccurred := 1
                        }
                    }
                }
                
                // Rising Edge (0 -> 1)
                if eq(triggerMode, 1) {
                    if iszero(prevTestIn) {
                        if eq(signalValue, 1) {
                            transitionOccurred := 1
                        }
                    }
                }
                
                if transitionOccurred {
                    latchState := 1
                    sstore(57000, 1)
                }
                
                sstore(57002, signalValue)
                
                mstore(0x00, latchState)
                mstore(0x20, latchState) // LED follows the latch state
                return(0x00, 64)
            }
            
            // resetCircuit()
            // selector: 0x9bfdc29f
            if eq(selector, 0x9bfdc29f) {
                sstore(57000, 0) // clear latched pulse
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
