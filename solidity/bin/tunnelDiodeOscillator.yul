object "TunnelDiodeOscillator" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers
            function SCALE() -> val { val := 1000000000000000000 }

            // 17-Point Precomputed Esaki Tunnel Diode I-V Curve (0V to 1.6V inputs)
            // Current values Id in Amperes scaled to 1e18 (so 1mA = 1e15)
            function getTunnelLUT(idx) -> id {
                switch idx
                case 0 { id := 0 }
                case 1 { id := 4800000000000000 }    // 4.8mA (near 80mV peak)
                case 2 { id := 3500000000000000 }    // 3.5mA (negative resistance region)
                case 3 { id := 1800000000000000 }    // 1.8mA
                case 4 { id := 600000000000000 }     // 0.6mA (near 350mV valley)
                case 5 { id := 900000000000000 }     // 0.9mA (diffusion starting)
                case 6 { id := 2500000000000000 }    // 2.5mA
                case 7 { id := 7500000000000000 }    // 7.5mA
                case 8 { id := 20000000000000000 }   // 20mA
                case 9 { id := 50000000000000000 }   // 50mA
                case 10 { id := 120000000000000000 } // 120mA
                case 11 { id := 280000000000000000 } // 280mA
                case 12 { id := 600000000000000000 } // 600mA
                case 13 { id := 1200000000000000000 } // 1.2A
                case 14 { id := 2200000000000000000 } // 2.2A
                case 15 { id := 4000000000000000000 } // 4.0A
                case 16 { id := 7000000000000000000 } // 7.0A
                default { id := 7000000000000000000 }
            }

            // Non-linear Tunnel Diode I-V characteristics: Id(V)
            function getDiodeCurrent(V) -> Id {
                let scale := SCALE()
                if slt(V, 0) {
                    // Reverse bias heavy conduction
                    Id := sdiv(mul(V, 100000000000000000), scale) // V * 0.1
                    leave
                }
                
                // Map input scale: 0V to 1.6V over 16 intervals (0.1V step size)
                let delta := 100000000000000000
                let idx := div(V, delta)
                let frac := mod(V, delta)
                
                if sgt(idx, 15) {
                    Id := getTunnelLUT(16)
                    leave
                }
                
                let y0 := getTunnelLUT(idx)
                let y1 := getTunnelLUT(add(idx, 1))
                Id := add(y0, sdiv(mul(sub(y1, y0), frac), delta))
            }

            // processSample(int256 bias, int256 pitch) -> int256 outputVoltage
            // selector: 0x07a96d8c
            if eq(selector, 0x07a96d8c) {
                let bias := calldataload(4)
                let pitch := calldataload(36)
                
                let scale := SCALE()
                
                // State variables stored in slots
                // Slot 200: Vc (capacitor voltage)
                // Slot 201: Il (inductor current)
                let Vc := sload(200)
                let Il := sload(201)
                
                // LC constants: C = 10nF, L = 100uH
                // dt = pitch * 10^-9 sec. We use Euler integration.
                let dt := mul(pitch, 1000000000) // pitch scaled to dt units
                
                // C_inv = 1 / C = 10^8
                // L_inv = 1 / L = 10^4
                let C_inv := 100000000
                let L_inv := 10000
                
                let Id := getDiodeCurrent(Vc)
                
                // dVc = (Ibias - Il - Id) / C
                let Ibias := bias // e.g. 3mA = 3 * 10^15
                let dVc := mul(sub(sub(Ibias, Il), Id), C_inv)
                let Vc_next := add(Vc, sdiv(mul(dVc, dt), scale))
                
                // dIl = (Vc - Rs * Il) / L
                let Rs := 10 // 10 Ohm series resistance
                let dIl := mul(sub(Vc, mul(Il, Rs)), L_inv)
                let Il_next := add(Il, sdiv(mul(dIl, dt), scale))
                
                // Soft boundary clamps to maintain numerical stability
                if sgt(Vc_next, 2000000000000000000) { Vc_next := 2000000000000000000 } // max 2.0V
                if slt(Vc_next, sub(0, 500000000000000000)) { Vc_next := sub(0, 500000000000000000) } // min -0.5V
                
                sstore(200, Vc_next)
                sstore(201, Il_next)
                
                mstore(0x00, Vc_next)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
