object "GyratorFilter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 inputSignal, int256 pitchAndQ) -> int256 outputSignal
            // selector: 0x07a96d8c (standard 2-parameter processSample)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let packedVal := calldataload(36)
                
                // Unpack pitchF (lower 128 bits) and dampingQ (upper 128 bits)
                let pitchF := and(packedVal, 0xffffffffffffffffffffffffffffffff) // tuning coefficient f
                let dampingQ := shr(128, packedVal) // damping coefficient q = 1/Q
                let scale := SCALE()

                // Load filter state (bandpass = slot 100, lowpass = slot 101)
                let bp := sload(100)
                let lp := sload(101)

                // SVF state calculations:
                // hp = input - lp - (dampingQ * bp) / scale
                let dampingForce := sdiv(mul(dampingQ, bp), scale)
                let hp := sub(sub(input, lp), dampingForce)

                // bp = bp + (pitchF * hp) / scale
                let bp_delta := sdiv(mul(pitchF, hp), scale)
                let bp_new := add(bp, bp_delta)

                // lp = lp + (pitchF * bp_new) / scale
                let lp_delta := sdiv(mul(pitchF, bp_new), scale)
                let lp_new := add(lp, lp_delta)

                // Save states
                sstore(100, bp_new)
                sstore(101, lp_new)

                // Return bandpass output
                mstore(0, bp_new)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
