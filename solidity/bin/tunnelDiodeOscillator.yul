object "TunnelDiodeOscillator" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers
            function SCALE() -> val { val := 1000000000000000000 }
            function VT() -> val { val := 26000000000000000 }      // 26mV thermal voltage
            function IP() -> val { val := 5000000000000000 }       // 5mA Peak Current
            function VP() -> val { val := 80000000000000000 }      // 80mV Peak Voltage
            function IV() -> val { val := 500000000000000 }        // 0.5mA Valley Current
            function VV() -> val { val := 350000000000000000 }     // 350mV Valley Voltage

            // Exponential approximation: exp(x)
            function fixedExp(x) -> val {
                let scale := SCALE()
                let x2 := sdiv(mul(x, x), scale)
                let x3 := sdiv(mul(x2, x), scale)
                let x4 := sdiv(mul(x3, x), scale)
                val := add(add(add(add(scale, x), sdiv(x2, 2)), sdiv(x3, 6)), sdiv(x4, 24))
            }

            // Exponential approximation: exp(-x) for positive x
            function fixedExpNeg(x) -> val {
                let scale := SCALE()
                let ex := fixedExp(x)
                if iszero(ex) { ex := 1 }
                val := sdiv(mul(scale, scale), ex)
            }

            // Non-linear Tunnel Diode I-V characteristics: Id(V)
            function getDiodeCurrent(V) -> Id {
                let scale := SCALE()
                let vp := VP()
                let ip := IP()
                
                // If V is negative, it conducts heavily in reverse bias
                if slt(V, 0) {
                    Id := sdiv(mul(V, 100000000000000000), scale) // simple slope
                    leave
                }
                
                // Tunneling Current component: It = Ip * (V/Vp) * e^(1 - V/Vp)
                let ratio := sdiv(mul(V, scale), vp)
                let It := 0
                if iszero(sgt(V, vp)) {
                    let expTerm := fixedExp(sub(scale, ratio))
                    It := sdiv(mul(mul(ip, ratio), expTerm), mul(scale, scale))
                }
                if sgt(V, vp) {
                    let expTerm := fixedExpNeg(sub(ratio, scale))
                    It := sdiv(mul(mul(ip, ratio), expTerm), mul(scale, scale))
                }
                
                // Excess & Diffusion Current component: Idiff = Iv * e^((V - Vv) / Vt)
                let Idiff := 0
                if sgt(V, 300000000000000000) { // starting around 300mV
                    let diffRatio := sdiv(mul(sub(V, 300000000000000000), scale), VT())
                    Idiff := sdiv(mul(500000000000000, fixedExp(diffRatio)), scale)
                }
                
                Id := add(It, Idiff)
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
                
                // Return output voltage scaled up for audio output
                mstore(0x00, Vc_next)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
