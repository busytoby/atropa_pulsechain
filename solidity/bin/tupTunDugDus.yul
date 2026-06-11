object "TupTunDugDus" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 }
            function VT() -> val { val := 26000000000000000 }      // 26mV
            
            // Accurate fixed-point exp(x) for all positive x using 2^u * exp(r)
            function fixedExp(x) -> val {
                if slt(x, 0) {
                    val := 0
                    leave
                }
                let scale := 1000000000000000000
                let ln2 := 693147180559945309
                let u := sdiv(x, ln2)
                let r := smod(x, ln2)
                
                let r2 := sdiv(mul(r, r), scale)
                let r3 := sdiv(mul(r2, r), scale)
                let r4 := sdiv(mul(r3, r), scale)
                let er := add(add(add(add(scale, r), sdiv(r2, 2)), sdiv(r3, 6)), sdiv(r4, 24))
                
                if sgt(u, 80) { u := 80 }
                val := shl(u, er)
            }

            // process(uint8 componentType, int256 inputVal) -> int256
            // selector: 0xfdd3c4d9
            if eq(shr(224, calldataload(0)), 0xfdd3c4d9) {
                let compType := calldataload(4)
                let input := calldataload(36)
                let scale := SCALE()
                let vt := VT()

                let outVal := 0

                switch compType
                case 0 {
                    // TUN: Silicon NPN Transistor (Vbe ~ 0.65V)
                    let Is := 10000 // 10fA in 1e18 scale
                    let Rb := 100000000000000000000000 // 100k
                    let Rc := 1000000000000000000000   // 1k
                    let beta := 250
                    
                    let Vbe := 0
                    if sgt(input, 0) { Vbe := 500000000000000000 } // 500mV start
                    
                    let coeff := sdiv(mul(Rb, Is), vt)

                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        let x := sdiv(mul(Vbe, scale), vt)
                        if sgt(x, 30000000000000000000) { x := 30000000000000000000 }
                        if slt(x, 0) { x := 0 }

                        let expTerm := fixedExp(x)
                        let Ib := sdiv(mul(Is, sub(expTerm, scale)), scale)
                        let F := sub(sub(input, Vbe), sdiv(mul(Ib, Rb), scale))
                        let dF := sub(0, add(scale, sdiv(mul(coeff, expTerm), scale)))

                        Vbe := sub(Vbe, sdiv(mul(F, scale), dF))
                    }
                    
                    let finalX := sdiv(mul(Vbe, scale), vt)
                    if sgt(finalX, 30000000000000000000) { finalX := 30000000000000000000 }
                    if slt(finalX, 0) { finalX := 0 }

                    let finalIb := sdiv(mul(Is, sub(fixedExp(finalX), scale)), scale)
                    let Ic := mul(finalIb, beta)
                    outVal := sub(9000000000000000000, sdiv(mul(Ic, Rc), scale))
                }
                case 1 {
                    // TUP: Silicon PNP Transistor (complementary polarities)
                    let Is := 10000
                    let Rb := 100000000000000000000000
                    let Rc := 1000000000000000000000
                    let beta := 250
                    let invInput := sub(0, input)
                    
                    let Veb := 0
                    if sgt(invInput, 0) { Veb := 500000000000000000 }
                    
                    let coeff := sdiv(mul(Rb, Is), vt)

                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        let x := sdiv(mul(Veb, scale), vt)
                        if sgt(x, 30000000000000000000) { x := 30000000000000000000 }
                        if slt(x, 0) { x := 0 }

                        let expTerm := fixedExp(x)
                        let Ib := sdiv(mul(Is, sub(expTerm, scale)), scale)
                        let F := sub(sub(invInput, Veb), sdiv(mul(Ib, Rb), scale))
                        let dF := sub(0, add(scale, sdiv(mul(coeff, expTerm), scale)))

                        Veb := sub(Veb, sdiv(mul(F, scale), dF))
                    }

                    let finalX := sdiv(mul(Veb, scale), vt)
                    if sgt(finalX, 30000000000000000000) { finalX := 30000000000000000000 }
                    if slt(finalX, 0) { finalX := 0 }

                    let finalIb := sdiv(mul(Is, sub(fixedExp(finalX), scale)), scale)
                    let Ic := mul(finalIb, beta)
                    outVal := add(sub(0, 9000000000000000000), sdiv(mul(Ic, Rc), scale))
                }
                case 2 {
                    // DUG: Diode Universal Germanium (Vf ~ 0.15V, Is = 1uA)
                    let Is := 1000000000000 // 1uA
                    let Rs := 10000000000000000000000 // 10k
                    
                    let Vd := 0
                    if sgt(input, 0) { Vd := 100000000000000000 } // 100mV start
                    
                    let coeff := sdiv(mul(Rs, Is), vt)

                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        let x := sdiv(mul(Vd, scale), vt)
                        if sgt(x, 30000000000000000000) { x := 30000000000000000000 }
                        if slt(x, 0) { x := 0 }

                        let expTerm := fixedExp(x)
                        let Id := sdiv(mul(Is, sub(expTerm, scale)), scale)
                        let F := sub(sub(input, Vd), sdiv(mul(Id, Rs), scale))
                        let dF := sub(0, add(scale, sdiv(mul(coeff, expTerm), scale)))

                        Vd := sub(Vd, sdiv(mul(F, scale), dF))
                    }
                    outVal := Vd
                }
                case 3 {
                    // DUS: Diode Universal Silicon (Vf ~ 0.6V, Is = 1nA)
                    let Is := 1000000000 // 1nA
                    let Rs := 10000000000000000000000 // 10k
                    
                    let Vd := 0
                    if sgt(input, 0) { Vd := 400000000000000000 } // 400mV start
                    
                    let coeff := sdiv(mul(Rs, Is), vt)

                    for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                        let x := sdiv(mul(Vd, scale), vt)
                        if sgt(x, 30000000000000000000) { x := 30000000000000000000 }
                        if slt(x, 0) { x := 0 }

                        let expTerm := fixedExp(x)
                        let Id := sdiv(mul(Is, sub(expTerm, scale)), scale)
                        let F := sub(sub(input, Vd), sdiv(mul(Id, Rs), scale))
                        let dF := sub(0, add(scale, sdiv(mul(coeff, expTerm), scale)))

                        Vd := sub(Vd, sdiv(mul(F, scale), dF))
                    }
                    outVal := Vd
                }

                mstore(0, outVal)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
