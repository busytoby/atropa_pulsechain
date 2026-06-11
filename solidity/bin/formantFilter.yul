object "FormantFilter" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 inputSignal, int256 pitchAndQ) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let packedVal := calldataload(36)
                
                // Unpack pitchF (lower 128 bits) and dampingQ (upper 128 bits)
                let pitchF := and(packedVal, 0xffffffffffffffffffffffffffffffff)
                let dampingQ := shr(128, packedVal)
                let scale := SCALE()

                // Load filter state
                let bp := sload(100)
                let lp := sload(101)

                // Compute quadrature envelope power: P = (BP^2 + LP^2) / SCALE
                let power := add(sdiv(mul(bp, bp), scale), sdiv(mul(lp, lp), scale))

                // Dynamic Damping (Anti-Ringing Feedback):
                // If power exceeds 0.3V (300000000000000000), dynamically scale up dampingQ (reduce resonance)
                let dynamicDamping := dampingQ
                let threshold := 300000000000000000
                if sgt(power, threshold) {
                    let excess := sub(power, threshold)
                    // Increase damping proportionally to prevent metallic ringing under clipping
                    dynamicDamping := add(dampingQ, sdiv(mul(excess, 4), scale))
                }

                // Chamberlin SVF state update
                let dampingForce := sdiv(mul(dynamicDamping, bp), scale)
                let hp := sub(sub(input, lp), dampingForce)
                let bp_delta := sdiv(mul(pitchF, hp), scale)
                let bp_new := add(bp, bp_delta)
                let lp_delta := sdiv(mul(pitchF, bp_new), scale)
                let lp_new := add(lp, lp_delta)

                // Save states
                sstore(100, bp_new)
                sstore(101, lp_new)

                // Symmetrical quadratic saturation transfer function:
                // y_sat = bp_new - 0.2 * bp_new^2 * sgn(bp_new) / SCALE
                let output := bp_new
                let absOut := bp_new
                let sgn := 1
                if slt(bp_new, 0) {
                    absOut := sub(0, bp_new)
                    sgn := sub(0, 1)
                }

                // Only apply quadratic saturation if output is large to generate warm even harmonics
                if sgt(absOut, 100000000000000000) {
                    let squared := sdiv(mul(absOut, absOut), scale)
                    let correction := sdiv(mul(squared, 200000000000000000), scale) // 0.2 multiplier
                    output := sub(bp_new, mul(correction, sgn))
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
