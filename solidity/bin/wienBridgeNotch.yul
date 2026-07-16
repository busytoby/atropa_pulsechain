object "WienBridgeNotch" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                // Load filter state history (slots 100-103)
                let x1 := sload(100)
                let x2 := sload(101)
                let y1 := sload(102)
                let y2 := sload(103)

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let cos_theta := and(statePack, 0xffffffffffffffffffffffffffffffff)
                    let isClipping := shr(128, statePack)

                    let output := input
                    if isClipping {
                        // Apply Wien-Bridge notch:
                        // y[n] = x[n] - 2*cos_theta*x[n-1] + x[n-2] - a1*y[n-1] - a2*y[n-2]
                        // where a1 = -1.9 * cos_theta, a2 = 0.9025
                        let b1 := sdiv(mul(sub(0, 2000000000000000000), cos_theta), scale)
                        let a1 := sdiv(mul(sub(0, 1900000000000000000), cos_theta), scale)
                        let a2 := 902500000000000000 // r^2 = 0.9025

                        let term1 := sdiv(mul(b1, x1), scale)
                        let term2 := sdiv(mul(a1, y1), scale)
                        let term3 := sdiv(mul(a2, y2), scale)

                        output := sub(add(add(input, term1), x2), add(term2, term3))
                    }

                    // Shift history
                    x2 := x1
                    x1 := input
                    y2 := y1
                    y1 := output

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                // Save filter history
                sstore(100, x1)
                sstore(101, x2)
                sstore(102, y1)
                sstore(103, y2)

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFreq) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let cos_theta := and(statePack, 0xffffffffffffffffffffffffffffffff)
                let isClipping := shr(128, statePack)

                let x1 := sload(100)
                let x2 := sload(101)
                let y1 := sload(102)
                let y2 := sload(103)

                let output := input
                if isClipping {
                    let b1 := sdiv(mul(sub(0, 2000000000000000000), cos_theta), scale)
                    let a1 := sdiv(mul(sub(0, 1900000000000000000), cos_theta), scale)
                    let a2 := 902500000000000000

                    let term1 := sdiv(mul(b1, x1), scale)
                    let term2 := sdiv(mul(a1, y1), scale)
                    let term3 := sdiv(mul(a2, y2), scale)

                    output := sub(add(add(input, term1), x2), add(term2, term3))
                }

                sstore(100, input)
                sstore(101, x1)
                sstore(102, output)
                sstore(103, y1)

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
