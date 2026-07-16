object "PiezoLimiter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_BASE_OFFSET() -> val { val := 600000000000000000 } // 600mV base offset
            function GAMMA() -> val { val := 100000000000000000 } // 0.1 modulation factor
            function MIN_OFFSET() -> val { val := 50000000000000000 } // 50mV minimum threshold
            function BETA() -> val { val := 100000000000000000 } // 0.1 (10% slope for soft clipping)

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let stress := calldataload(add(68, mul(i, 64)))

                    // Modulate offset: V_offset = V_base_offset - gamma * stress
                    let mod_val := sdiv(mul(GAMMA(), stress), scale)
                    let v_offset := V_BASE_OFFSET()
                    let is_greater := sgt(v_offset, mod_val)
                    if is_greater {
                        v_offset := sub(v_offset, mod_val)
                    }
                    if iszero(is_greater) {
                        v_offset := 0
                    }
                    if lt(v_offset, MIN_OFFSET()) {
                        v_offset := MIN_OFFSET()
                    }

                    let output := input
                    // Soft-clipping threshold limits
                    if sgt(input, v_offset) {
                        let excess := sub(input, v_offset)
                        output := add(v_offset, sdiv(mul(excess, BETA()), scale))
                    }
                    let neg_offset := sub(0, v_offset)
                    if slt(input, neg_offset) {
                        let excess := add(input, v_offset)
                        output := add(neg_offset, sdiv(mul(excess, BETA()), scale))
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stressLevel) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let stress := calldataload(36)
                let scale := SCALE()

                let mod_val := sdiv(mul(GAMMA(), stress), scale)
                let v_offset := V_BASE_OFFSET()
                let is_greater := sgt(v_offset, mod_val)
                if is_greater {
                    v_offset := sub(v_offset, mod_val)
                }
                if iszero(is_greater) {
                    v_offset := 0
                }
                if lt(v_offset, MIN_OFFSET()) {
                    v_offset := MIN_OFFSET()
                }

                let output := input
                if sgt(input, v_offset) {
                    let excess := sub(input, v_offset)
                    output := add(v_offset, sdiv(mul(excess, BETA()), scale))
                }
                let neg_offset := sub(0, v_offset)
                if slt(input, neg_offset) {
                    let excess := add(input, v_offset)
                    output := add(neg_offset, sdiv(mul(excess, BETA()), scale))
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
