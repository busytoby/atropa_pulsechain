object "BionikaReflex" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_THRESHOLD() -> val { val := 800000000000000000 } // 800mV firing threshold

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                // Load neural state history
                let vm := sload(100)
                let refractorySteps := sload(101)
                let attenuationLevel := sload(102)
                if iszero(attenuationLevel) {
                    attenuationLevel := scale
                }

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)

                    if refractorySteps {
                        refractorySteps := sub(refractorySteps, 1)
                        vm := 0
                        // Decay attenuation level back towards 1.0 (release phase)
                        attenuationLevel := sdiv(add(mul(attenuationLevel, 90), mul(scale, 10)), 100)
                    }
                    if iszero(refractorySteps) {
                        let condition := isClipping
                        if condition {
                            // Inject stimulus current (potential increase)
                            vm := add(vm, 300000000000000000) // +300mV
                        }
                        if iszero(condition) {
                            // Leak integration (decay membrane potential)
                            vm := sdiv(mul(vm, 80), 100) // 80% leak decay
                        }

                        if sgt(vm, V_THRESHOLD()) {
                            // Firing! Reset potential, trigger attenuation and enter refractory period
                            vm := 0
                            refractorySteps := 3
                            attenuationLevel := 700000000000000000 // 30% reduction (0.7)
                        }
                    }

                    let output := sdiv(mul(input, attenuationLevel), scale)
                    mstore(add(memOffset, mul(i, 32)), output)
                }

                sstore(100, vm)
                sstore(101, refractorySteps)
                sstore(102, attenuationLevel)

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndStimulus) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let isClipping := shr(128, statePack)

                let vm := sload(100)
                let refractorySteps := sload(101)
                let attenuationLevel := sload(102)
                if iszero(attenuationLevel) {
                    attenuationLevel := scale
                }

                if refractorySteps {
                    refractorySteps := sub(refractorySteps, 1)
                    vm := 0
                    attenuationLevel := sdiv(add(mul(attenuationLevel, 90), mul(scale, 10)), 100)
                }
                if iszero(refractorySteps) {
                    let condition := isClipping
                    if condition {
                        vm := add(vm, 300000000000000000)
                    }
                    if iszero(condition) {
                        vm := sdiv(mul(vm, 80), 100)
                    }

                    if sgt(vm, V_THRESHOLD()) {
                        vm := 0
                        refractorySteps := 3
                        attenuationLevel := 700000000000000000
                    }
                }

                sstore(100, vm)
                sstore(101, refractorySteps)
                sstore(102, attenuationLevel)

                let output := sdiv(mul(input, attenuationLevel), scale)
                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
