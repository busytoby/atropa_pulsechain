object "VactrolLimiter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_FORWARD() -> val { val := 1800000000000000000 } // 1.8V forward bias
            function R_DARK() -> val { val := 1000000 } // 1M ohms dark resistance
            function R_SERIES() -> val { val := 100000 } // 100k ohms series resistance
            function GAMMA() -> val { val := 5000000000000000000 } // gamma = 5.0

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

                    let output := input
                    if isClipping {
                        let abs_in := input
                        if slt(input, 0) {
                            abs_in := sub(0, input)
                        }

                        let L := 0
                        if sgt(abs_in, V_FORWARD()) {
                            // L = 0.5 * (abs(Vin) - V_forward)
                            let L_raw := sub(abs_in, V_FORWARD())
                            L := sdiv(mul(L_raw, 500000000000000000), scale)
                        }

                        // R_LDR = R_dark / (1 + gamma * L)
                        let denominator := add(scale, sdiv(mul(GAMMA(), L), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                        // Gain = R_LDR / (R_series + R_LDR)
                        let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                        output := sdiv(mul(input, gain), scale)
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

                let output := input
                if isClipping {
                    let abs_in := input
                    if slt(input, 0) {
                        abs_in := sub(0, input)
                    }

                    let L := 0
                    if sgt(abs_in, V_FORWARD()) {
                        let L_raw := sub(abs_in, V_FORWARD())
                        L := sdiv(mul(L_raw, 500000000000000000), scale)
                    }

                    let denominator := add(scale, sdiv(mul(GAMMA(), L), scale))
                    let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                    let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                    output := sdiv(mul(input, gain), scale)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
