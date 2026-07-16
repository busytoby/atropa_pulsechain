object "Compander" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 inputSignal, int256 modeAndThreshold) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let packedVal := calldataload(36)

                // Unpack:
                // threshold: bits 0..127 (scaled by 1e18)
                // mode: bits 128..255 (0 = Compressor, 1 = Expander)
                let threshold := and(packedVal, 0xffffffffffffffffffffffffffffffff)
                let mode := shr(128, packedVal)
                let scale := SCALE()

                // Load envelope state (storage slot 100)
                let env := sload(100)

                // Rectify input
                let absVal := input
                if slt(input, 0) {
                    absVal := sub(0, input)
                }

                // Envelope tracking: attack = 0.5, release = 0.01 (99% decay)
                if sgt(absVal, env) {
                    env := sdiv(add(env, absVal), 2)
                } {
                    env := sdiv(mul(env, 990000000000000000), scale)
                }
                sstore(100, env)

                let output := input

                // Prevent division by zero
                if iszero(threshold) {
                    threshold := 100000000000000000 // 0.1V default
                }

                if eq(mode, 0) {
                    // COMPRESSOR MODE
                    let gain := scale
                    if sgt(env, threshold) {
                        gain := sdiv(mul(threshold, scale), env)
                    }
                    output := sdiv(mul(input, gain), scale)
                }
                if eq(mode, 1) {
                    // EXPANDER MODE (reciprocal of compressor gain)
                    let gain := scale
                    if sgt(env, threshold) {
                        gain := sdiv(mul(env, scale), threshold)
                    }
                    output := sdiv(mul(input, gain), scale)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
