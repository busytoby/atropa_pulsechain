object "AxonalDelay" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                // Load pointers
                let writePtr := sload(112)
                let attenuationLevel := sload(113)
                if iszero(attenuationLevel) {
                    attenuationLevel := scale
                }

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)

                    // Read oldest sample from FIFO (12-step delay buffer slots 100-111)
                    let slot := add(100, writePtr)
                    let oldestSpike := sload(slot)

                    // Store new clipping spike into FIFO
                    sstore(slot, isClipping)

                    // Update pointer
                    writePtr := add(writePtr, 1)
                    if eq(writePtr, 12) {
                        writePtr := 0
                    }

                    if oldestSpike {
                        // Conduction delay trigger: reduce gain by 30%
                        attenuationLevel := 700000000000000000
                    }
                    if iszero(oldestSpike) {
                        // Release attenuation exponentially towards 1.0 (10% step)
                        let diff := sub(scale, attenuationLevel)
                        attenuationLevel := add(attenuationLevel, sdiv(mul(diff, 100000000000000000), scale))
                    }

                    let output := sdiv(mul(input, attenuationLevel), scale)
                    mstore(add(memOffset, mul(i, 32)), output)
                }

                sstore(112, writePtr)
                sstore(113, attenuationLevel)

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFlags) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let isClipping := shr(128, statePack)

                let writePtr := sload(112)
                let attenuationLevel := sload(113)
                if iszero(attenuationLevel) {
                    attenuationLevel := scale
                }

                let slot := add(100, writePtr)
                let oldestSpike := sload(slot)

                sstore(slot, isClipping)

                writePtr := add(writePtr, 1)
                if eq(writePtr, 12) {
                    writePtr := 0
                }

                if oldestSpike {
                    attenuationLevel := 700000000000000000
                }
                if iszero(oldestSpike) {
                    let diff := sub(scale, attenuationLevel)
                    attenuationLevel := add(attenuationLevel, sdiv(mul(diff, 100000000000000000), scale))
                }

                sstore(112, writePtr)
                sstore(113, attenuationLevel)

                let output := sdiv(mul(input, attenuationLevel), scale)
                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
