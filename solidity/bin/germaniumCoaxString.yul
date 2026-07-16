object "GermaniumCoaxString" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_GE_THRESHOLD() -> val { val := 200000000000000000 } // 0.2V Germanium junction barrier
            function R_DARK() -> val { val := 1000000 } // 1M ohms
            function R_SERIES() -> val { val := 100000 } // 100k ohms
            function GAMMA() -> val { val := 8000000000000000000 } // gamma = 8.0 (high sensitivity)

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                let l_state := sload(200)

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)
                    let pickupDistance := and(statePack, 0xffffffffffffffff) // dynamic coupling distance (scaled by 1e18)

                    let output := input
                    if isClipping {
                        // 1. Germanium pre-amplifier stage on the coaxial "string":
                        // Symmetrical soft-knee saturation using 0.2V threshold.
                        let abs_in := input
                        if slt(input, 0) {
                            abs_in := sub(0, input)
                        }

                        let saturatedString := input
                        if sgt(abs_in, V_GE_THRESHOLD()) {
                            let excess := sub(abs_in, V_GE_THRESHOLD())
                            // Compress string signal: output = sign * (0.2V + excess * 0.15)
                            let compressedExcess := sdiv(mul(excess, 150000000000000000), scale)
                            let mag := add(V_GE_THRESHOLD(), compressedExcess)
                            saturatedString := mag
                            if slt(input, 0) {
                                saturatedString := sub(0, mag)
                            }
                        }

                        // 2. Coaxial pickup induction coupling:
                        // Induced signal is inversely proportional to pickupDistance
                        if iszero(pickupDistance) {
                            pickupDistance := scale
                        }
                        let couplingFactor := sdiv(mul(scale, scale), pickupDistance)
                        let inducedSignal := sdiv(mul(saturatedString, couplingFactor), scale)

                        // 3. Germanium diode bridge detection on the vactrol coil:
                        // High sensitivity full-wave rectifier using 0.2V Germanium diodes to drive the LED.
                        let abs_induced := inducedSignal
                        if slt(inducedSignal, 0) {
                            abs_induced := sub(0, inducedSignal)
                        }

                        let L_ge := 0
                        if sgt(abs_induced, V_GE_THRESHOLD()) {
                            L_ge := sub(abs_induced, V_GE_THRESHOLD())
                        }

                        // Asymmetric LDR lag integration (10% attack, 0.5% release)
                        if sgt(L_ge, l_state) {
                            l_state := add(l_state, sdiv(mul(sub(L_ge, l_state), 10), 100))
                        }
                        if iszero(sgt(L_ge, l_state)) {
                            l_state := add(l_state, sdiv(mul(sub(L_ge, l_state), 5), 1000))
                        }

                        // Calculate gain reduction
                        let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)
                        let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))

                        // Output is the saturated string signal modulated by the vactrol LDR shunt
                        output := sdiv(mul(saturatedString, gain), scale)
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                sstore(200, l_state)
                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFlags) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let isClipping := shr(128, statePack)
                let pickupDistance := and(statePack, 0xffffffffffffffff)

                let l_state := sload(200)
                let output := input

                if isClipping {
                    let abs_in := input
                    if slt(input, 0) {
                        abs_in := sub(0, input)
                    }

                    let saturatedString := input
                    if sgt(abs_in, V_GE_THRESHOLD()) {
                        let excess := sub(abs_in, V_GE_THRESHOLD())
                        let compressedExcess := sdiv(mul(excess, 150000000000000000), scale)
                        let mag := add(V_GE_THRESHOLD(), compressedExcess)
                        saturatedString := mag
                        if slt(input, 0) {
                            saturatedString := sub(0, mag)
                        }
                    }

                    if iszero(pickupDistance) {
                        pickupDistance := scale
                    }
                    let couplingFactor := sdiv(mul(scale, scale), pickupDistance)
                    let inducedSignal := sdiv(mul(saturatedString, couplingFactor), scale)

                    let abs_induced := inducedSignal
                    if slt(inducedSignal, 0) {
                        abs_induced := sub(0, inducedSignal)
                    }

                    let L_ge := 0
                    if sgt(abs_induced, V_GE_THRESHOLD()) {
                        L_ge := sub(abs_induced, V_GE_THRESHOLD())
                    }

                    if sgt(L_ge, l_state) {
                        l_state := add(l_state, sdiv(mul(sub(L_ge, l_state), 10), 100))
                    }
                    if iszero(sgt(L_ge, l_state)) {
                        l_state := add(l_state, sdiv(mul(sub(L_ge, l_state), 5), 1000))
                    }

                    let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                    let R_LDR := sdiv(mul(R_DARK(), scale), denominator)
                    let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))

                    output := sdiv(mul(saturatedString, gain), scale)
                }

                sstore(200, l_state)
                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
