object "VactrolLimiter" {
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

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                // Load stateful LDR illumination
                let l_state := sload(200)

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isClipping := shr(128, statePack)
                    let routingMode := and(shr(64, statePack), 0xffffffffffffffff)

                    let output := input
                    if isClipping {
                        // Symmetrical dual-LED push-pull modeling
                        let L_red := 0
                        if sgt(input, V_FORWARD_RED()) {
                            L_red := sub(input, V_FORWARD_RED())
                        }
                        let L_green := 0
                        let neg_v_green := sub(0, V_FORWARD_GREEN())
                        if slt(input, neg_v_green) {
                            L_green := sub(neg_v_green, input)
                        }
                        let L_target := add(L_red, L_green)

                        // Stateful temporal lag: fast attack (10%), slow release (0.5%)
                        if sgt(L_target, l_state) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 10), 100))
                        }
                        if iszero(sgt(L_target, l_state)) {
                            l_state := add(l_state, sdiv(mul(sub(L_target, l_state), 5), 1000))
                        }

                        // LDR resistance: R_LDR = R_dark / (1 + gamma * L)
                        let denominator := add(scale, sdiv(mul(GAMMA(), l_state), scale))
                        let R_LDR := sdiv(mul(R_DARK(), scale), denominator)

                        // NPN / PNP Routing Mode
                        if iszero(routingMode) {
                            // PNP Shunt Mode: attenuates signal
                            let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                            output := sdiv(mul(input, gain), scale)
                        }
                        if routingMode {
                            // NPN Mode: active boost feedback sweep modulation
                            let gain := add(scale, sdiv(mul(R_SERIES(), scale), R_LDR))
                            output := sdiv(mul(input, gain), scale)
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
                let routingMode := and(shr(64, statePack), 0xffffffffffffffff)

                let l_state := sload(200)
                let output := input

                if isClipping {
                    let L_red := 0
                    if sgt(input, V_FORWARD_RED()) {
                        L_red := sub(input, V_FORWARD_RED())
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

                    if iszero(routingMode) {
                        let gain := sdiv(mul(R_LDR, scale), add(R_SERIES(), R_LDR))
                        output := sdiv(mul(input, gain), scale)
                    }
                    if routingMode {
                        let gain := add(scale, sdiv(mul(R_SERIES(), scale), R_LDR))
                        output := sdiv(mul(input, gain), scale)
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
