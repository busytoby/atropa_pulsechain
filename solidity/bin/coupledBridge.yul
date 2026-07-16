object "CoupledBridge" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_GE_THRESHOLD() -> val { val := 200000000000000000 } // 0.2V Germanium junction barrier
            function R_DARK() -> val { val := 1000000 } // 1M ohms dark resistance
            function R_SERIES() -> val { val := 100000 } // 100k ohms series resistance
            function GAMMA() -> val { val := 5000000000000000000 } // gamma = 5.0
            function K0() -> val { val := 980000000000000000 } // base coupling k = 0.98

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)
                    let control := and(shr(64, statePack), 0xffffffffffffffff) // External accumulator value
                    let tapRatio := and(shr(32, statePack), 0xffffffff)
                    let mode := and(statePack, 0xffffffff)

                    let output := input
                    if isClipping {
                        // 1. Germanium pre-amplifier saturation on the string
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

                        // 2. Stateless LDR resistance driven by external control accumulator
                        let denominator := add(scale, sdiv(mul(GAMMA(), control), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                        // 3. Dynamic Mutual Inductance (core saturation coupling)
                        let k := sdiv(mul(K0(), R_LDR), add(R_SERIES(), R_LDR))

                        // 4. Multi-tap switching
                        let tapScale := scale
                        if eq(tapRatio, 1) {
                            tapScale := 500000000000000000
                        }
                        if eq(tapRatio, 2) {
                            tapScale := 250000000000000000
                        }

                        // 5. Mode routing: 0 -> PNP Shunt, 1 -> NPN Sweep Feedback
                        if iszero(mode) {
                            output := sdiv(mul(sdiv(mul(saturatedString, k), scale), tapScale), scale)
                        }
                        if mode {
                            let activeGain := add(scale, sub(scale, k))
                            output := sdiv(mul(sdiv(mul(saturatedString, activeGain), scale), tapScale), scale)
                        }
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFlags) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let isClipping := shr(128, statePack)
                let control := and(shr(64, statePack), 0xffffffffffffffff)
                let tapRatio := and(shr(32, statePack), 0xffffffff)
                let mode := and(statePack, 0xffffffff)

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

                    let denominator := add(scale, sdiv(mul(GAMMA(), control), scale))
                    let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                    let k := sdiv(mul(K0(), R_LDR), add(R_SERIES(), R_LDR))

                    let tapScale := scale
                    if eq(tapRatio, 1) {
                        tapScale := 500000000000000000
                    }
                    if eq(tapRatio, 2) {
                        tapScale := 250000000000000000
                    }

                    if iszero(mode) {
                        output := sdiv(mul(sdiv(mul(saturatedString, k), scale), tapScale), scale)
                    }
                    if mode {
                        let activeGain := add(scale, sub(scale, k))
                        output := sdiv(mul(sdiv(mul(saturatedString, activeGain), scale), tapScale), scale)
                    }
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
