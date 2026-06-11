object "EquaAmplifier" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 inputSignal, int256 loadImpedance) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let loadRL := calldataload(36) // Load resistance RL (scaled by 1e18)
                let scale := SCALE()

                // Equa-Amplifier protection limits:
                // Re = 0.33 ohms, Vbe_prot = 0.65V
                // I_limit = Vbe_prot / Re = 1.9697A
                let I_limit := 1969696969696969697 // 1.97A

                // Dynamic voltage limit: V_max = I_limit * loadRL / scale
                let V_max := sdiv(mul(I_limit, loadRL), scale)

                // Enforce minimum voltage limit to prevent division by zero or full mute at 0 RL
                if slt(V_max, 100000000000000000) {
                    V_max := 100000000000000000 // 0.1V minimum threshold
                }

                // Load envelope state (storage slot 100)
                let env := sload(100)

                // Rectify input
                let absInput := input
                if slt(input, 0) {
                    absInput := sub(0, input)
                }

                // Dynamic envelope tracking
                // attack = 0.5, release = 0.01
                if sgt(absInput, env) {
                    env := sdiv(add(env, absInput), 2)
                } {
                    env := sdiv(mul(env, 990000000000000000), scale)
                }

                // Save envelope state
                sstore(100, env)

                // Gain calculation: gain = V_max / env if env > V_max
                let gain := scale
                if sgt(env, V_max) {
                    gain := sdiv(mul(V_max, scale), env)
                }

                // Apply gain to input
                let output := sdiv(mul(input, gain), scale)

                // Symmetrical transistor soft clipping (SOAR protection triggering)
                let softLimit := sdiv(mul(V_max, 950000000000000000), scale)
                if sgt(output, softLimit) {
                    output := softLimit
                }
                let negSoftLimit := sub(0, softLimit)
                if slt(output, negSoftLimit) {
                    output := negSoftLimit
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
