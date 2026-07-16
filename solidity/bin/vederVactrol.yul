object "VederVactrol" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_FORWARD_RED() -> val { val := 1800000000000000000 } // 1.8V Red GaP LED
            function R_DARK() -> val { val := 1000000 } // 1M ohms dark resistance
            function R_SERIES() -> val { val := 100000 } // 100k ohms series resistance
            function GAMMA() -> val { val := 5000000000000000000 } // gamma = 5.0

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
                    let L_bias := and(statePack, 0xffffffffffffffffffffffffffffffff)

                    let output := input
                    if isClipping {
                        let abs_in := input
                        if slt(input, 0) {
                            abs_in := sub(0, input)
                        }

                        let L_red := 0
                        if sgt(abs_in, V_FORWARD_RED()) {
                            L_red := sub(abs_in, V_FORWARD_RED())
                        }

                        // Apply constant background Veder-Bias to pre-fill shallow traps
                        let L_target := add(L_bias, L_red)

                        // Stateful temporal lag: fast attack (10%), slow release (0.5%)
                        if sgt(L_target, l_state) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 10), 100))
                        }
                        if iszero(sgt(L_target, l_state)) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 5), 1000))
                        }

                        // LDR resistance calculation
                        let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                        // PNP Shunt Gain Attenuation
                        let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                        output := sdiv(mul(input, gain), scale)
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
                let L_bias := and(statePack, 0xffffffffffffffffffffffffffffffff)

                let l_state := sload(200)
                let output := input

                if isClipping {
                    let abs_in := input
                    if slt(input, 0) {
                        abs_in := sub(0, input)
                    }

                    let L_red := 0
                    if sgt(abs_in, V_FORWARD_RED()) {
                        L_red := sub(abs_in, V_FORWARD_RED())
                    }

                    let L_target := add(L_bias, L_red)

                    if sgt(L_target, l_state) {
                        l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 10), 100))
                    }
                    if iszero(sgt(L_target, l_state)) {
                        l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 5), 1000))
                    }

                    let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                    let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                    let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                    output := sdiv(mul(input, gain), scale)
                }

                sstore(200, l_state)
                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
