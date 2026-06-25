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

                // 1. Chaotic Jerk Oscillator (Sprott system integration):
                // State coordinates: x=sload(102), y=sload(103), z=sload(104)
                let jx := sload(102)
                let jy := sload(103)
                let jz := sload(104)
                if and(eq(jx, 0), eq(jy, 0)) {
                    jx := 100000000000000000 // 0.1 init
                    jy := 100000000000000000
                }
                
                // Jerk ODE: dz/dt = -0.6*z - y + sgn(x)
                let sgn_jx := scale
                if slt(jx, 0) { sgn_jx := sub(0, scale) }
                let dt_j := 5000000000000000 // dt = 0.005
                let jz_delta := sdiv(mul(sub(sgn_jx, add(jy, sdiv(mul(600000000000000000, jz), scale))), dt_j), scale)
                let jz_new := add(jz, jz_delta)
                let jy_new := add(jy, sdiv(mul(jz_new, dt_j), scale))
                let jx_new := add(jx, sdiv(mul(jy_new, dt_j), scale))
                
                sstore(102, jx_new)
                sstore(103, jy_new)
                sstore(104, jz_new)

                // 2. Dynamic Warpfolding Saturation stage:
                // Folding threshold deforms dynamically driven by the chaotic jerk amplitude
                let fold_threshold := add(300000000000000000, sdiv(mul(jx_new, 2), 10)) // base 0.3V + modulated jerk
                let output := bp_new
                
                // Wrap and fold signal if it exceeds the dynamic threshold
                if sgt(output, fold_threshold) {
                    let excess := sub(output, fold_threshold)
                    output := sub(fold_threshold, excess)
                }
                if slt(output, sub(0, fold_threshold)) {
                    let excess := add(output, fold_threshold)
                    output := add(sub(0, fold_threshold), sub(0, excess))
                }

                // Absolute Scale Wrap-Around: If output exceeds 1e18 limits, wrap it modulo SCALE() back to 0
                let absOut := output
                let sgn := 1
                if slt(output, 0) {
                    absOut := sub(0, output)
                    sgn := sub(0, 1)
                }
                if ugt(absOut, scale) {
                    output := mul(mod(absOut, scale), sgn)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
