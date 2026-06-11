/*
    Sylvania Stroboscopic Relaxation Oscillator.
    Simulates a neon gas-discharge or stroboscopic tube trigger circuit.
    Charges capacitor towards Vcc and discharges rapidly when striking threshold is reached.
    Supports adjustable charge (T_on) and discharge (T_off) time constants.
*/
object "StroboscopeOscillator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // processSample(int256 vcc, int256 packedParams) -> int256 outputVoltage
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let Vcc := calldataload(4)
                let packedParams := calldataload(36)
                
                let chargeTC := and(packedParams, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
                let dischargeTC := shr(128, packedParams)
                
                if iszero(chargeTC) { chargeTC := 10 }
                if iszero(dischargeTC) { dischargeTC := 3 }

                // Load state variables
                let V_c := sload(400)       // Capacitor voltage
                let is_discharging := sload(401) // Discharge phase flag

                let V_strike := 8000000000000000000     // 8V strike threshold (scaled 1e18)
                let V_extinguish := 2000000000000000000 // 2V extinguish threshold (scaled 1e18)

                if is_discharging {
                    // Rapid discharge: V_c decays towards V_extinguish
                    let diff := sub(V_extinguish, V_c)
                    V_c := add(V_c, sdiv(diff, dischargeTC))

                    // If we drop below extinguish threshold, stop discharging
                    if slt(V_c, add(V_extinguish, 100000000000000000)) {
                        is_discharging := 0
                    }
                }
                if iszero(is_discharging) {
                    // Charging phase: V_c charges towards Vcc via resistor
                    let diff := sub(Vcc, V_c)
                    V_c := add(V_c, sdiv(diff, chargeTC))

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
