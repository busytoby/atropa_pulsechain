/*
    Philips Philicorda Neon Master Oscillator & Frequency Divider.
    Simulates a neon gas-discharge relaxation oscillator coupled with 
    a bistable multivibrator (flip-flop) dividing stage for octave generation.
*/
object "PhilicordaGenerator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_STRIKE() -> val { val := mul(85, SCALE()) } // 85V strike potential
            function V_EXTINGUISH() -> val { val := mul(60, SCALE()) } // 60V extinguish potential
            function V_CC() -> val { val := mul(115, SCALE()) } // 115V supply rail

            // processSample(int256 pitchCoefficient, int256 rcRate) -> (int256 masterSaw, int256 dividedSquare)
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let pitch := calldataload(4)
                let rate := calldataload(36)
                let scale := SCALE()

                // Load internal states
                let Vc := sload(100)
                let is_discharging := sload(101)
                let divider_state := sload(102)
                let last_state := sload(103)

                // Initialize Vc to extinguish level if uninitialized
                if eq(Vc, 0) {
                    Vc := V_EXTINGUISH()
                }

                // Compute charging rate based on rate parameter & pitch coefficient
                // rate is represented as fractional charging increment per sample
                let dv := 0
                if eq(is_discharging, 0) {
                    // Charging: Vc increases towards V_CC
                    let diff := sub(V_CC(), Vc)
                    dv := div(mul(mul(diff, rate), pitch), mul(scale, scale))
                    Vc := add(Vc, dv)

                    // Ignite neon tube if strike voltage reached
                    if sgt(Vc, V_STRIKE()) {
                        is_discharging := 1
                    }
                } {
                    // Discharging: Rapid decay towards V_EXTINGUISH
                    let diff := sub(Vc, V_EXTINGUISH())
                    // Rapid discharge factor (constant multiplier for fast discharge)
                    let discharge_rate := mul(rate, 8)
                    dv := div(mul(diff, discharge_rate), scale)
                    Vc := sub(Vc, dv)

                    // Extinguish when voltage drops below limit
                    if slt(Vc, add(V_EXTINGUISH(), 1000000000000000000)) {
                        Vc := V_EXTINGUISH()
                        is_discharging := 0
                    }
                }

                // Bistable flip-flop octave divider:
                // Trigger flip-flop on transition from charging to discharging (falling edge of master saw)
                let current_state := is_discharging
                if and(eq(current_state, 1), eq(last_state, 0)) {
                    divider_state := xor(divider_state, 1)
                }

                // Save states
                sstore(100, Vc)
                sstore(101, is_discharging)
                sstore(102, divider_state)
                sstore(103, current_state)

                // Map outputs to 1e18 scale (centered and normalized)
                // Master Saw: scale Vc from [60V, 85V] to [-1V, 1V]
                let span := sub(V_STRIKE(), V_EXTINGUISH())
                let mid := add(V_EXTINGUISH(), sdiv(span, 2))
                let masterOut := sdiv(mul(sub(Vc, mid), scale), sdiv(span, 2))

                // Divided Square: output -1V (when divider_state is 0) or 1V (when divider_state is 1)
                let divOut := sub(0, scale)
                if eq(divider_state, 1) {
                    divOut := scale
                }

                mstore(0, V_CC())
                mstore(32, Vc)
                return(0, 64)
            }
            revert(0, 0)
        }
    }
}
