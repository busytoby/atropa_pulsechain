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

            // Helper trigonometric functions
            function getSin(angle) -> s {
                angle := mod(angle, 16)
                switch angle
                case 0 { s := 0 }
                case 1 { s := 38 }
                case 2 { s := 70 }
                case 3 { s := 92 }
                case 4 { s := 100 }
                case 5 { s := 92 }
                case 6 { s := 70 }
                case 7 { s := 38 }
                case 8 { s := 0 }
                case 9 { s := sub(0, 38) }
                case 10 { s := sub(0, 70) }
                case 11 { s := sub(0, 92) }
                case 12 { s := sub(0, 100) }
                case 13 { s := sub(0, 92) }
                case 14 { s := sub(0, 70) }
                case 15 { s := sub(0, 38) }
            }

            function getCos(angle) -> c {
                angle := mod(angle, 16)
                switch angle
                case 0 { c := 100 }
                case 1 { c := 92 }
                case 2 { c := 70 }
                case 3 { c := 38 }
                case 4 { c := 0 }
                case 5 { c := sub(0, 38) }
                case 6 { c := sub(0, 70) }
                case 7 { c := sub(0, 92) }
                case 8 { c := sub(0, 100) }
                case 9 { c := sub(0, 92) }
                case 10 { c := sub(0, 70) }
                case 11 { c := sub(0, 38) }
                case 12 { c := 0 }
                case 13 { c := 38 }
                case 14 { c := 70 }
                case 15 { c := 92 }
            }

            // executeMath(uint256 op, uint256 a, uint256 b) -> (uint256 res, uint256 status)
            // Selector: 0x2f32067e
            if eq(selector, 0x2f32067e) {
                let op := calldataload(4)
                let opA := calldataload(36)
                if gt(opA, 32767) { opA := sub(opA, 65536) }
                let opB := calldataload(68)
                if gt(opB, 32767) { opB := sub(opB, 65536) }

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
                    res := mul(opA, opB)
                }
                case 4 { // Divide
                    if iszero(opB) {
                        status := 1
                    }
                    if opB {
                        res := div(opA, opB)
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
                    if lt(opB, 0) {
                        status := 1
                    }
                    if iszero(lt(opB, 0)) {
                        res := 1
                        for { let i := 0 } lt(i, opB) { i := add(i, 1) } {
                            res := mul(res, opA)
                        }
                    }
                }
                case 7 { // Square Root
                    if lt(opA, 0) {
                        status := 1
                    }
                    if iszero(lt(opA, 0)) {
                        let temp := opA
                        let x := temp
                        let y := 1
                        if gt(temp, 0) {
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                x := div(add(x, y), 2)
                                y := div(temp, x)
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

                mstore(0x00, res)
                mstore(0x20, status)
                return(0x00, 64)
            }

            revert(0, 0)
        }
    }
}
