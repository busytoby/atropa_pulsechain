object "Mostronome" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            function sinApprox(x) -> val {
                let pi2 := 6283185307179586476 // 2 * pi * 1e18
                x := mod(x, pi2)
                let sign := 1
                let pi := 3141592653589793238 // pi * 1e18
                if gt(x, pi) {
                    x := sub(x, pi)
                    sign := sub(0, 1)
                }
                let scale := SCALE()
                let x2 := sdiv(mul(x, x), scale)
                let x3 := sdiv(mul(x2, x), scale)
                let x5 := sdiv(mul(x3, x2), scale)
                x5 := sdiv(x5, 120)
                let x7 := sdiv(mul(x5, x2), scale)
                x7 := sdiv(x7, 42) // 5040 / 120 = 42
                let res := add(sub(x, sdiv(x3, 6)), x5)
                res := sub(res, x7)
                val := mul(res, sign)
            }

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let amplitude := sload(100)
                let phase := sload(101)

                if iszero(amplitude) {
                    amplitude := scale
                    phase := 0
                }

                let memOffset := 0x80
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let sinVal := sinApprox(phase)
                    let output := sdiv(mul(amplitude, sinVal), scale)
                    mstore(add(memOffset, mul(i, 32)), output)

                    amplitude := sdiv(mul(amplitude, 995000000000000000), scale)
                    phase := add(phase, 142475736859300057)
                }

                sstore(100, amplitude)
                sstore(101, phase)

                return(memOffset, mul(count, 32))
            }

            // processSample(uint256 trigger) -> int256 sampleValue
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let trigger := calldataload(4)
                let scale := SCALE()

                let amplitude := sload(100)
                let phase := sload(101)

                if trigger {
                    amplitude := scale
                    phase := 0
                }

                let sinVal := sinApprox(phase)
                let output := sdiv(mul(amplitude, sinVal), scale)

                amplitude := sdiv(mul(amplitude, 995000000000000000), scale)
                phase := add(phase, 142475736859300057)

                sstore(100, amplitude)
                sstore(101, phase)

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
