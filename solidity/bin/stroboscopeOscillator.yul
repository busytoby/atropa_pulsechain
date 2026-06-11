/*
    Sylvania Stroboscopic Relaxation Oscillator.
    Simulates a neon gas-discharge or stroboscopic tube trigger circuit.
    Charges capacitor towards Vcc and discharges rapidly when striking threshold is reached.
    Produces trigger pulses and relaxation sawtooth LFO waves for synthesizers.
*/
object "StroboscopeOscillator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // processSample(int256 vcc, int256 dummy) -> int256 outputVoltage
            // selector: 0x07a96d8c (matches standard oscillator processSample signature)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let Vcc := calldataload(4)

                // Load state variables
                let V_c := sload(400)       // Capacitor voltage
                let is_discharging := sload(401) // Discharge phase flag

                let V_strike := 8000000000000000000     // 8V strike threshold (scaled 1e18)
                let V_extinguish := 2000000000000000000 // 2V extinguish threshold (scaled 1e18)

                if is_discharging {
                    // Rapid discharge: V_c decays towards V_extinguish
                    // dVc = (V_extinguish - V_c) / 3 (fast time constant)
                    // MUST use signed sdiv since diff can be negative!
                    let diff := sub(V_extinguish, V_c)
                    V_c := add(V_c, sdiv(diff, 3))

                    // If we drop below extinguish threshold, stop discharging
                    if slt(V_c, add(V_extinguish, 100000000000000000)) {
                        is_discharging := 0
                    }
                }
                if iszero(is_discharging) {
                    // Charging phase: V_c charges towards Vcc via resistor
                    // dVc = (Vcc - V_c) / 10 (slower time constant)
                    let diff := sub(Vcc, V_c)
                    V_c := add(V_c, sdiv(diff, 10))

                    // If we exceed strike threshold, ignite discharge phase
                    if sgt(V_c, V_strike) {
                        is_discharging := 1
                    }
                }

                sstore(400, V_c)
                sstore(401, is_discharging)

                mstore(0, V_c)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
