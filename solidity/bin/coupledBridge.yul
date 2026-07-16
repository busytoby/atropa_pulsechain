object "CoupledBridge" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_FORWARD_RED() -> val { val := 1800000000000000000 } // 1.8V Red GaP LED
            function V_FORWARD_GREEN() -> val { val := 2100000000000000000 } // 2.1V Green GaP LED
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

                let l_state := sload(200)

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)
                    let tapRatio := and(shr(64, statePack), 0xffffffffffffffff)
                    let mode := and(statePack, 0xffffffffffffffff)

                    let output := input
                    if isClipping {
                        // Multi-tap switching: 0 -> 1:1 (1.0), 1 -> 1:2 (0.5), 2 -> 1:4 (0.25)
                        let tapScale := scale
                        if eq(tapRatio, 1) {
                            tapScale := 500000000000000000 // 0.5
                        }
                        if eq(tapRatio, 2) {
                            tapScale := 250000000000000000 // 0.25
                        }

                        // Symmetrical push-push dual GaP LED rectification
                        let abs_in := input
                        if slt(input, 0) {
                            abs_in := sub(0, input)
                        }

                        let L_red := 0
                        if sgt(abs_in, V_FORWARD_RED()) {
                            L_red := sub(abs_in, V_FORWARD_RED())
                        }
                        let L_green := 0
                        let neg_v_green := sub(0, V_FORWARD_GREEN())
                        if slt(input, neg_v_green) {
                            L_green := sub(neg_v_green, input)
                        }
                        let L_target := add(L_red, L_green)

                        // Stateful temporal decay lag (10% attack, 0.5% release)
                        if sgt(L_target, l_state) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 10), 100))
                        }
                        if iszero(sgt(L_target, l_state)) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 5), 1000))
                        }

                        // LDR resistance calculation
                        let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                        // Dynamic Mutual Inductance (core saturation coupling): k = k0 * R_LDR / (R_series + R_LDR)
                        let k := sdiv(mul(K0(), R_LDR), add(R_SERIES(), R_LDR))

                        // Output generation based on PNP (shunt) or NPN (sweep feedback) mode
                        if iszero(mode) {
                            // PNP Shunt Mode: dynamic coupling reduction
                            output := sdiv(mul(sdiv(mul(input, k), scale), tapScale), scale)
                        }
                        if mode {
                            // NPN Mode: active boost range sweep
                            let activeGain := add(scale, sub(scale, k))
                            output := sdiv(mul(sdiv(mul(input, activeGain), scale), tapScale), scale)
                        }
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
                let tapRatio := and(shr(64, statePack), 0xffffffffffffffff)
                let mode := and(statePack, 0xffffffffffffffff)

                let l_state := sload(200)
                let output := input

                if isClipping {
                    let tapScale := scale
                    if eq(tapRatio, 1) {
                        tapScale := 500000000000000000
                    }
                    if eq(tapRatio, 2) {
                        tapScale := 250000000000000000
                    }

                    let abs_in := input
                    if slt(input, 0) {
                        abs_in := sub(0, input)
                    }

                    let L_red := 0
                    if sgt(abs_in, V_FORWARD_RED()) {
                        L_red := sub(abs_in, V_FORWARD_RED())
                    }
                    let L_green := 0
                    let neg_v_green := sub(0, V_FORWARD_GREEN())
                    if slt(input, neg_v_green) {
                        L_green := sub(neg_v_green, input)
                    }
                    let L_target := add(L_red, L_green)

                    if sgt(L_target, l_state) {
                        l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 10), 100))
                    }
                    if iszero(sgt(L_target, l_state)) {
                        l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 5), 1000))
                    }

                    let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                    let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                    let k := sdiv(mul(K0(), R_LDR), add(R_SERIES(), R_LDR))

                    if iszero(mode) {
                        output := sdiv(mul(sdiv(mul(input, k), scale), tapScale), scale)
                    }
                    if mode {
                        let activeGain := add(scale, sub(scale, k))
                        output := sdiv(mul(sdiv(mul(input, activeGain), scale), tapScale), scale)
                    }
                }

                sstore(200, l_state)
                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
