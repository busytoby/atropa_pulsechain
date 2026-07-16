object "HysteresisLimiter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_STRIKE() -> val { val := 8000000000000000000 } // 8V strike
            function V_EXTINGUISH() -> val { val := 5000000000000000000 } // 5V extinguish

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let state := sload(100)
                let env := sload(101)

                let memOffset := 0x80

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 32)))

                    let absVal := input
                    if slt(input, 0) {
                        absVal := sub(0, input)
                    }

                    let is_greater := sgt(absVal, env)
                    if is_greater {
                        env := sdiv(add(env, absVal), 2)
                    }
                    if iszero(is_greater) {
                        env := sdiv(mul(env, 750000000000000000), scale)
                    }

                    let next_state := state
                    if iszero(state) {
                        if sgt(env, V_STRIKE()) {
                            next_state := 1
                        }
                    }
                    if eq(state, 1) {
                        if lt(env, V_EXTINGUISH()) {
                            next_state := 0
                        }
                    }
                    state := next_state

                    let output := input
                    if state {
                        let gain := sdiv(mul(V_EXTINGUISH(), scale), env)
                        output := sdiv(mul(input, gain), scale)
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                sstore(100, state)
                sstore(101, env)

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 extraParam) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let scale := SCALE()

                let state := sload(100)
                let env := sload(101)

                let absVal := input
                if slt(input, 0) {
                    absVal := sub(0, input)
                }

                let is_greater := sgt(absVal, env)
                if is_greater {
                    env := sdiv(add(env, absVal), 2)
                }
                if iszero(is_greater) {
                    env := sdiv(mul(env, 750000000000000000), scale)
                }
                sstore(101, env)

                let next_state := state
                if iszero(state) {
                    if sgt(env, V_STRIKE()) {
                        next_state := 1
                    }
                }
                if eq(state, 1) {
                    if lt(env, V_EXTINGUISH()) {
                        next_state := 0
                    }
                }
                state := next_state
                sstore(100, state)

                let output := input
                if state {
                    let gain := sdiv(mul(V_EXTINGUISH(), scale), env)
                    output := sdiv(mul(input, gain), scale)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
