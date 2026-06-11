object "MathCoprocessor" {
    code {
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // 17-Point Quarter-Sine Lookup Table (scaled 1e18)
            function getSineLUT(idx) -> s {
                switch idx
                case 0 { s := 0 }
                case 1 { s := 98017140329560608 }
                case 2 { s := 195090322016128256 }
                case 3 { s := 290284677254462368 }
                case 4 { s := 382683432365089792 }
                case 5 { s := 471396736825997696 }
                case 6 { s := 555570233019602240 }
                case 7 { s := 634393284163645440 }
                case 8 { s := 707106781186547584 }
                case 9 { s := 773010453362736896 }
                case 10 { s := 831469612302545280 }
                case 11 { s := 881921263618349056 }
                case 12 { s := 923879532511286784 }
                case 13 { s := 956940335732208896 }
                case 14 { s := 980785280403230464 }
                case 15 { s := 995184726672197760 }
                case 16 { s := 1000000000000000000 }
                default { s := 1000000000000000000 }
            }

            // Radian-based fixed-point Sine (scaled 1e18)
            function getSin(theta) -> s {
                let pi := 3141592653589793238
                let twoPi := 6283185307179586476
                let halfPi := 1570796326794896619
                
                theta := mod(theta, twoPi)
                if slt(theta, 0) { theta := add(theta, twoPi) }
                
                let phi := theta
                let sign := 1
                
                if sgt(theta, pi) {
                    phi := sub(theta, pi)
                    sign := sub(0, 1)
                }
                
                if sgt(phi, halfPi) {
                    phi := sub(pi, phi)
                }
                
                let delta := 98174770424681038
                let idx := div(phi, delta)
                let frac := mod(phi, delta)
                
                let y0 := getSineLUT(idx)
                let y1 := getSineLUT(add(idx, 1))
                
                let interp := add(y0, sdiv(mul(sub(y1, y0), frac), delta))
                s := mul(interp, sign)
            }

            function getCos(theta) -> c {
                let halfPi := 1570796326794896619
                c := getSin(add(theta, halfPi))
            }

            // Normalized Phase (0 to 1e18) fixed-point Sine (scaled 1e18)
            function getSinNormalized(phi) -> s {
                let one := 1000000000000000000
                phi := mod(phi, one)
                if slt(phi, 0) { phi := add(phi, one) }

                let q1 := 250000000000000000
                let q2 := 500000000000000000
                let q3 := 750000000000000000

                let idx_input := phi
                let sign := 1

                if and(iszero(slt(phi, q1)), slt(phi, q2)) {
                    idx_input := sub(q2, phi)
                }
                if and(iszero(slt(phi, q2)), slt(phi, q3)) {
                    idx_input := sub(phi, q2)
                    sign := sub(0, 1)
                }
                if iszero(slt(phi, q3)) {
                    idx_input := sub(one, phi)
                    sign := sub(0, 1)
                }

                // delta = q1 / 16 = 15625000000000000
                let delta := 15625000000000000
                let idx := div(idx_input, delta)
                let frac := mod(idx_input, delta)

                let y0 := getSineLUT(idx)
                let y1 := getSineLUT(add(idx, 1))

                let interp := add(y0, sdiv(mul(sub(y1, y0), frac), delta))
                s := mul(interp, sign)
            }

            function getCosNormalized(phi) -> c {
                c := getSinNormalized(add(phi, 250000000000000000))
            }

            // Fixed-point Square Root helper (scaled 1e18)
            function getSqrt(opA) -> res {
                if slt(opA, 0) {
                    res := 0
                    leave
                }
                let temp := opA
                let x := temp
                let y := 1000000000000000000
                if gt(temp, 0) {
                    for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                        x := sdiv(add(x, y), 2)
                        y := sdiv(mul(temp, 1000000000000000000), x)
                    }
                }
                res := x
            }

            // Bessel Function J0(x) polynomial / cosine approximation (scaled 1e18)
            function getJ0(x) -> j {
                let scale := 1000000000000000000
                if slt(x, 0) { x := sub(0, x) }
                
                // Polynomial approximation for x <= 3.0
                if iszero(gt(x, 3000000000000000000)) {
                    let x3 := sdiv(x, 3)
                    let y := sdiv(mul(x3, x3), scale)
                    
                    let c1 := 2249999700000000000
                    let c2 := 1265620800000000000
                    let c3 := 316386600000000000
                    let c4 := 44447900000000000
                    let c5 := 3944400000000000
                    let c6 := 210000000000000
                    
                    let term := sdiv(mul(c6, y), scale)
                    term := sub(term, c5)
                    term := sdiv(mul(term, y), scale)
                    term := add(term, c4)
                    term := sdiv(mul(term, y), scale)
                    term := sub(term, c3)
                    term := sdiv(mul(term, y), scale)
                    term := add(term, c2)
                    term := sdiv(mul(term, y), scale)
                    term := sub(term, c1)
                    term := sdiv(mul(term, y), scale)
                    j := add(scale, term)
                    leave
                }
                
                // Asymptotic cosine approximation for x > 3.0
                let pi := 3141592653589793238
                let quarterPi := 785398163397448309
                let den := sdiv(mul(pi, x), scale)
                let amp := 0
                if den {
                    let inner := sdiv(mul(2000000000000000000, scale), den)
                    amp := getSqrt(inner)
                }
                let angle := sub(x, quarterPi)
                let cosTerm := getCos(angle)
                j := sdiv(mul(amp, cosTerm), scale)
            }

            // Bessel Function J1(x) polynomial / cosine approximation (scaled 1e18)
            function getJ1(x) -> j {
                let scale := 1000000000000000000
                let sign := 1
                if slt(x, 0) {
                    x := sub(0, x)
                    sign := sub(0, 1)
                }
                
                // Polynomial approximation for x <= 3.0
                if iszero(gt(x, 3000000000000000000)) {
                    let x3 := sdiv(x, 3)
                    let y := sdiv(mul(x3, x3), scale)
                    
                    let c0 := 500000000000000000
                    let c1 := 562499850000000000
                    let c2 := 210935730000000000
                    let c3 := 39542890000000000
                    let c4 := 4433190000000000
                    let c5 := 317610000000000
                    let c6 := 11090000000000
                    
                    let term := sdiv(mul(c6, y), scale)
                    term := sub(term, c5)
                    term := sdiv(mul(term, y), scale)
                    term := add(term, c4)
                    term := sdiv(mul(term, y), scale)
                    term := sub(term, c3)
                    term := sdiv(mul(term, y), scale)
                    term := add(term, c2)
                    term := sdiv(mul(term, y), scale)
                    term := sub(term, c1)
                    term := sdiv(mul(term, y), scale)
                    let factor := add(c0, term)
                    j := mul(sdiv(mul(x, factor), scale), sign)
                    leave
                }
                
                // Asymptotic cosine approximation for x > 3.0
                let pi := 3141592653589793238
                let threeQuarterPi := 2356194490192344928
                let den := sdiv(mul(pi, x), scale)
                let amp := 0
                if den {
                    let inner := sdiv(mul(2000000000000000000, scale), den)
                    amp := getSqrt(inner)
                }
                let angle := sub(x, threeQuarterPi)
                let cosTerm := getCos(angle)
                j := mul(sdiv(mul(amp, cosTerm), scale), sign)
            }

            // executeMath(uint256 op, uint256 a, uint256 b) -> (uint256 res, uint256 status)
            // Selector: 0x2f32067e
            if eq(selector, 0x2f32067e) {
                let op := calldataload(4)
                let opA := calldataload(36)
                let opB := calldataload(68)

                let res := 0
                let status := 0

                switch op
                case 1 { // Add
                    res := add(opA, opB)
                }
                case 2 { // Subtract
                    res := sub(opA, opB)
                }
                case 3 { // Multiply
                    res := sdiv(mul(opA, opB), 1000000000000000000)
                }
                case 4 { // Divide
                    if iszero(opB) {
                        status := 1
                    }
                    if opB {
                        res := sdiv(mul(opA, 1000000000000000000), opB)
                    }
                }
                case 5 { // Modulo
                    if iszero(opB) {
                        status := 1
                    }
                    if opB {
                        res := mod(opA, opB)
                    }
                }
                case 6 { // Power
                    res := 1000000000000000000
                    for { let i := 0 } lt(i, opB) { i := add(i, 1) } {
                        res := sdiv(mul(res, opA), 1000000000000000000)
                    }
                }
                case 7 { // Square Root
                    res := getSqrt(opA)
                    if slt(opA, 0) { status := 1 }
                }
                case 8 { // Sine
                    res := getSin(opA)
                }
                case 9 { // Cosine
                    res := getCos(opA)
                }
                case 10 { // Get Dysnomia Primes
                    switch opA
                    case 1 { res := 953467954114363 }    // Motzkin's 36th Prime
                    case 2 { res := 953473 }             // Apogee Prime
                    case 3 { res := 954114361 }          // Apex Prime
                    case 4 { res := 953473954114361 }    // VERTEX of DYSNOMIA
                    default { status := 1 }
                }
                case 11 { // Normalized Sine
                    res := getSinNormalized(opA)
                }
                case 12 { // Normalized Cosine
                    res := getCosNormalized(opA)
                }
                case 13 { // Bessel J0(x)
                    res := getJ0(opA)
                }
                case 14 { // Bessel J1(x)
                    res := getJ1(opA)
                }

                mstore(0x00, res)
                mstore(0x20, status)
                return(0x00, 64)
            }

            revert(0, 0)
        }
    }
}
