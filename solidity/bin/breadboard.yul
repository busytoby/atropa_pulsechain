object "Breadboard" {
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

            // Helper to get connection mapping storage key
            // pin: unique pin index (e.g. 0 to 255)
            function getConnectionSlot(pin) -> slot {
                mstore(0x00, pin)
                mstore(0x20, 0x1c1c1c1c) // Namespace salt
                slot := keccak256(0x00, 64)
            }

            // Helper to get logic gate definition key
            // gateId: unique identifier for the gate instance
            function getGateSlot(gateId) -> slot {
                mstore(0x00, gateId)
                mstore(0x20, 0x9a9a9a9a) // Namespace salt
                slot := keccak256(0x00, 64)
            }

            // ----------------------------------------------------------------
            // METHOD 1: setGate(uint256 gateId, uint256 gateType, uint256 inPin1, uint256 inPin2, uint256 outPin)
            // Selector: 0x82b4dc25
            // Deploys ("solders") a logical IC gate onto the breadboard.
            // Gate Types:
            // 1: AND, 2: OR, 3: XOR, 4: NAND, 5: NOR, 6: NOT
            // ----------------------------------------------------------------
            if eq(selector, 0xf2c44b12) {
                let gateId := calldataload(4)
                let gateType := calldataload(36)
                let inPin1 := calldataload(68)
                let inPin2 := calldataload(100)
                let outPin := calldataload(132)

                // Pack gate: 
                // Bits 0-7: gateType
                // Bits 8-15: inPin1
                // Bits 16-23: inPin2
                // Bits 24-31: outPin
                // Bits 32: active flag (1)
                let packed := add(gateType, add(shl(8, inPin1), add(shl(16, inPin2), add(shl(24, outPin), shl(32, 1)))))
                
                sstore(getGateSlot(gateId), packed)

                // Record gate count if it's new
                let count := sload(0)
                // Search if gate already in array to avoid duplicate index
                let found := 0
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    if eq(sload(add(1, i)), gateId) {
                        found := 1
                        break
                    }
                }
                if iszero(found) {
                    sstore(add(1, count), gateId)
                    sstore(0, add(count, 1))
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: setPinValue(uint256 pin, uint256 value)
            // Selector: 0x56c6e63e
            // Set logic level (0 or 1) of a source/input pin directly.
            // ----------------------------------------------------------------
            if eq(selector, 0x56c6e63e) {
                let pin := calldataload(4)
                let val := calldataload(36)
                sstore(getConnectionSlot(pin), val)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: getPinValue(uint256 pin) -> uint256
            // Selector: 0x4860f445
            // Returns current state of a pin.
            // ----------------------------------------------------------------
            if eq(selector, 0x4860f445) {
                let pin := calldataload(4)
                mstore(0x00, sload(getConnectionSlot(pin)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: propagate()
            // Selector: 0xc5460912
            // Resolves the logical states by executing logic for all active gates.
            // We run a propagation loop to update output pins based on gate inputs.
            // ----------------------------------------------------------------
            if eq(selector, 0xc5460912) {
                let count := sload(0)
                
                // Perform multiple passes (e.g. 3) to allow signals to flow through serial connections
                for { let pass := 0 } lt(pass, 3) { pass := add(pass, 1) } {
                    for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                        let gateId := sload(add(1, i))
                        let packed := sload(getGateSlot(gateId))
                        
                        // If gate is active (Bit 32 is set)
                        if and(shr(32, packed), 1) {
                            let gateType := and(packed, 0xFF)
                            let inPin1 := and(shr(8, packed), 0xFF)
                            let inPin2 := and(shr(16, packed), 0xFF)
                            let outPin := and(shr(24, packed), 0xFF)

                            let val1 := sload(getConnectionSlot(inPin1))
                            let val2 := sload(getConnectionSlot(inPin2))
                            let outVal := 0

                            switch gateType
                            case 1 { // AND
                                outVal := and(val1, val2)
                            }
                            case 2 { // OR
                                outVal := or(val1, val2)
                            }
                            case 3 { // XOR
                                outVal := xor(val1, val2)
                            }
                            case 4 { // NAND
                                outVal := iszero(and(val1, val2))
                            }
                            case 5 { // NOR
                                outVal := iszero(or(val1, val2))
                            }
                            case 6 { // NOT (unary on inPin1)
                                outVal := iszero(val1)
                            }

                            sstore(getConnectionSlot(outPin), and(outVal, 1))
                        }
                    }
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: recycleGate(uint256 gateId)
            // Selector: 0xdecfe519
            // Desolders/reclaims a gate instance, resetting its active state.
            // ----------------------------------------------------------------
            if eq(selector, 0xdecfe519) {
                let gateId := calldataload(4)
                let slot := getGateSlot(gateId)
                let packed := sload(slot)
                
                // Clear active flag (Bit 32) and save
                let recycled := and(packed, 0xFFFFFFFF)
                sstore(slot, recycled)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
