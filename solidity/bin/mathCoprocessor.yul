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

            // High-precision interpolated Sine function (scaled 1e18)
            function getSin(theta) -> s {
                let pi := 3141592653589793238
                let twoPi := 6283185307179586476
                let halfPi := 1570796326794896619
                
                // Modulo twoPi
                theta := mod(theta, twoPi)
                if slt(theta, 0) { theta := add(theta, twoPi) }
                
                // Quadrant symmetry mapping
                let phi := theta
                let sign := 1
                
                if sgt(theta, pi) {
                    phi := sub(theta, pi)
                    sign := sub(0, 1)
                }
                
                if sgt(phi, halfPi) {
                    phi := sub(pi, phi)
                }
                
                // Interpolation step: delta = halfPi / 16
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
                    if slt(opA, 0) {
                        status := 1
                    }
                    if iszero(slt(opA, 0)) {
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

                mstore(0x00, res)
                mstore(0x20, status)
                return(0x00, 64)
            }

            revert(0, 0)
        }
    }
}
