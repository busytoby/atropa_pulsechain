/*
    Raytheon Piezoresistive Transducer Transistor Stage.
    Simulates a stress-sensitive transistor microphone.
    Incoming acoustic pressure (deflection) directly modulates the base-emitter conduction offset,
    altering base current and output collector voltage.
*/
object "TransducerStage" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function VT() -> val { val := 26000000000000000 }      // 26mV
            function BASE_OFFSET() -> val { val := 600000000000000000 } // 600mV Silicon base barrier
            function IS() -> val { val := 1000000000000 }          // 1uA Saturation
            function BETA() -> val { val := 150 }                  // Current Gain

            // processSample(int256 soundPressure, int256 electricalBias) -> int256 outputVoltage
            // selector: 0x07a96d8c (standard processSample signature)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let pressure := calldataload(4)
                let bias := calldataload(36)
                let scale := SCALE()

                // Modulate base conduction offset by sound pressure
                // Gamma sensitivity = 0.05. Offset decreases as pressure increases.
                // MUST use signed sdiv since pressure can be negative!
                let gamma := 50000000000000000 // 0.05
                let modulated_offset := sub(BASE_OFFSET(), sdiv(mul(pressure, gamma), scale))

                // Safeguard modulated offset bounds
                if slt(modulated_offset, 100000000000000000) { modulated_offset := 100000000000000000 } // min 100mV
                if sgt(modulated_offset, 1000000000000000000) { modulated_offset := 1000000000000000000 } // max 1V

                // Base-emitter input voltage: bias - modulated_offset
                let Vbe_input := sub(bias, modulated_offset)

                // Compute base current using linear approximation of diode conduction
                let Ib := 0
                if sgt(Vbe_input, 0) {
                    // Linear approximation of diode current above offset: Ib = Vbe_input / R_internal
                    // R_internal = 10k Ohm (10000)
                    Ib := sdiv(Vbe_input, 10000)
                }

                let Ic := mul(Ib, BETA())

                // Collector output: Vout = Vcc - Ic * Rc (Vcc = 9V, Rc = 2.2k)
                // Vcc = 9e18
                let Rc := 2200
                let drop := mul(Ic, Rc)
                let Vout := sub(9000000000000000000, drop)

                // Safety bounds clamp
                if slt(Vout, 0) { Vout := 0 }
                if sgt(Vout, 9000000000000000000) { Vout := 9000000000000000000 }

                mstore(0, Vout)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
