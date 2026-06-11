/*
    OC71 Germanium Common-Emitter Stage Simulator.
    Calculates dynamic collector current to output a distorted audio signal.
*/
object "GermaniumStage" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 }
            function VT() -> val { val := 26000000000000000 }      // 26mV
            function IS() -> val { val := 1000000000000 }          // 1uA Saturation Current
            function BETA() -> val { val := 100 }                  // Current Gain

            // Taylor Series Expansion for exp(x)
            function fixedExp(x) -> val {
                if slt(x, 0) {
                    val := 0
                    leave
                }
                let scale := SCALE()
                let x2 := sdiv(mul(x, x), scale)
                let x3 := sdiv(mul(x2, x), scale)
                let x4 := sdiv(mul(x3, x), scale)
                val := add(add(add(add(scale, x), sdiv(x2, 2)), sdiv(x3, 6)), sdiv(x4, 24))
            }

            // Newton-Raphson loop finding base-emitter voltage
            function solveVbe(vbe_input, Rb) -> Vbe {
                let scale := SCALE()
                let vt := VT()
                let is_val := IS()
                Vbe := 150000000000000000 // 150mV starting point
                
                let coeff := sdiv(mul(Rb, is_val), vt)
                
                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    let x := sdiv(mul(Vbe, scale), vt)
                    
                    // Clamp to keep expTerm within reasonable bounds for Taylor series
                    if sgt(x, 4000000000000000000) { x := 4000000000000000000 } // Clamp to 4.0
                    if slt(x, sub(0, 5000000000000000000)) { x := sub(0, 5000000000000000000) } // Clamp to -5.0

                    let expTerm := fixedExp(x)
                    
                    let Ib := sdiv(mul(is_val, sub(expTerm, scale)), scale)
                    let F := sub(sub(vbe_input, Vbe), sdiv(mul(Ib, Rb), scale))
                    let dF := sub(0, add(scale, sdiv(mul(coeff, expTerm), scale)))
                    
                    Vbe := sub(Vbe, sdiv(mul(F, scale), dF))
                }
            }

            // processSample(int256 inputVoltage) -> int256 outputVoltage
            // selector: 0x96118bcb
            if eq(shr(224, calldataload(0)), 0x96118bcb) {
                let input := calldataload(4)
                let scale := SCALE()
                
                let V_c_in := sload(100) // Input capacitor charge
                let V_c_e  := sload(101) // Emitter capacitor charge
                
                let V_b := sub(input, V_c_in)
                let V_e := V_c_e
                let Vbe_input := sub(V_b, V_e)
                
                let Rb := 100000000000000000000000 // 100k Ohm source resistance
                let Vbe := solveVbe(Vbe_input, Rb)
                
                let x := sdiv(mul(Vbe, scale), VT())
                if sgt(x, 4000000000000000000) { x := 4000000000000000000 }
                if slt(x, sub(0, 5000000000000000000)) { x := sub(0, 5000000000000000000) }
                
                let expTerm := fixedExp(x)
                let Ib := sdiv(mul(IS(), sub(expTerm, scale)), scale)
                let Ic := mul(Ib, BETA())
                
                // Collector output: Vout = Vcc - Ic * Rc (Vcc = 9V, Rc = 4.7k)
                let Vout := sub(9000000000000000000, sdiv(mul(Ic, 4700000000000000000000), scale))
                
                // Update internal capacitor charges
                let dt_div_C := 10000000000000000
                sstore(100, add(V_c_in, sdiv(mul(Ib, dt_div_C), scale)))
                sstore(101, add(V_c_e, sdiv(mul(mul(Ib, add(1, BETA())), dt_div_C), scale)))
                mstore(0, Vout)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
