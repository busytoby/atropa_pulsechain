/*
    OA85 Germanium Diode Envelope Detector & Signal Tracker.
    Simulates a discrete Germanium diode half-wave rectifier and RC filter stage.
    Used for envelope tracking, compression, and visual telemetry in synthesizers.
*/
object "EnvelopeDetector" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // processSample(int256 inputVoltage) -> int256 envelopeOut
            // selector: 0x96118bcb
            if eq(shr(224, calldataload(0)), 0x96118bcb) {
                let input := calldataload(4)

                // Load persistent capacitor charge (envelope voltage)
                let V_c := sload(300)

                // Rectify: conduction offset of Germanium diode (150mV = 1.5e17)
                let V_rect := 0
                if sgt(input, 150000000000000000) {
                    V_rect := sub(input, 150000000000000000)
                }

                // Attack and Release envelope tracking
                if sgt(V_rect, V_c) {
                    // Fast attack: V_c = V_c + (V_rect - V_c) / 2
                    let diff := sub(V_rect, V_c)
                    V_c := add(V_c, div(diff, 2))
                }
                if iszero(sgt(V_rect, V_c)) {
                    // Release decay: V_c = V_c - V_c / 4 (decay 25% per sample)
                    V_c := sub(V_c, div(V_c, 4))
                }

                // Clamp to zero
                if slt(V_c, 0) { V_c := 0 }

                sstore(300, V_c)
                mstore(0, V_c)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
