/*
    OC71 Germanium Common-Emitter Stage Simulator.
    Uses pre-computed tables for base-emitter voltage drops and shunting currents,
    modeling the dynamic saturation curve without expensive Newton-Raphson loop iterations.
*/
object "GermaniumStage" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 }
            function BETA() -> val { val := 100 } // Current Gain

            // 17-Point Precomputed Germanium Base-Emitter Voltage Table (0V to 1.6V inputs)
            function getGermaniumLUT(idx) -> v {
                switch idx
                case 0 { v := 0 }
                case 1 { v := 40000000000000000 }    // 0.04V
                case 2 { v := 80000000000000000 }    // 0.08V
                case 3 { v := 115000000000000000 }   // 0.115V
                case 4 { v := 140000000000000000 }   // 0.14V
                case 5 { v := 160000000000000000 }   // 0.16V
                case 6 { v := 175000000000000000 }   // 0.175V
                case 7 { v := 185000000000000000 }   // 0.185V
                case 8 { v := 195000000000000000 }   // 0.195V
                case 9 { v := 202000000000000000 }   // 0.202V
                case 10 { v := 208000000000000000 }  // 0.208V
                case 11 { v := 213000000000000000 }  // 0.213V
                case 12 { v := 218000000000000000 }  // 0.218V
                case 13 { v := 222000000000000000 }  // 0.222V
                case 14 { v := 226000000000000000 }  // 0.226V
                case 15 { v := 230000000000000000 }  // 0.230V
                case 16 { v := 235000000000000000 }  // 0.235V
                default { v := 235000000000000000 }
            }

            // Interpolates Vbe voltage drop from input base voltage
            function solveVbe(vbe_input) -> Vbe {
                if slt(vbe_input, 0) {
                    Vbe := 0
                    leave
                }
                
                // Map input scale: 0V to 1.6V (1.6e18) over 16 intervals (0.1V step size)
                let delta := 100000000000000000
                let idx := div(vbe_input, delta)
                let frac := mod(vbe_input, delta)
                
                if sgt(idx, 15) {
                    Vbe := getGermaniumLUT(16)
                    leave
                }
                
                let y0 := getGermaniumLUT(idx)
                let y1 := getGermaniumLUT(add(idx, 1))
                Vbe := add(y0, sdiv(mul(sub(y1, y0), frac), delta))
            }

            // processSample(int256 inputVoltage, int256 extraParam) -> int256 outputVoltage
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let scale := SCALE()
                
                let V_c_in := sload(100) // Input capacitor charge
                let V_c_e  := sload(101) // Emitter capacitor charge
                
                let V_b := sub(input, V_c_in)
                let V_e := V_c_e
                let Vbe_input := sub(V_b, V_e)
                
                let Rb := 100000000000000000000000 // 100k Ohm source resistance
                let Vbe := solveVbe(Vbe_input)
                
                // Base current shunting equation: Ib = (Vbe_input - Vbe) / Rb
                let Ib := 0
                if sgt(Vbe_input, Vbe) {
                    Ib := sdiv(mul(sub(Vbe_input, Vbe), scale), Rb)
                }
                
                let Ic := mul(Ib, BETA())
                
                // Collector output: Vout = Vcc - Ic * Rc (Vcc = 9V, Rc = 4.7k)
                let Vout := sub(9000000000000000000, sdiv(mul(Ic, 4700000000000000000000), scale))
                
                // Update internal capacitor charges
                let dt_div_C := 10000000000000000
                
                let charging_b := sdiv(mul(Ib, dt_div_C), scale)
                let discharge_b := sdiv(mul(V_c_in, 1000000000000000), scale) // 1e15 Base Discharge Rate
                sstore(100, sub(add(V_c_in, charging_b), discharge_b))

                let charging_e := sdiv(mul(mul(Ib, add(1, BETA())), dt_div_C), scale)
                let discharge_e := sdiv(mul(V_c_e, 5000000000000000), scale) // 5e15 Emitter Discharge Rate
                sstore(101, sub(add(V_c_e, charging_e), discharge_e))
                
                mstore(0, Vout)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
