object "TapeSaturation" {
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

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let output := input
                    let isClipping := shr(128, statePack)

                    if isClipping {
                        // Apply Telcan Tape Saturation (polynomial approximation of tanh)
                        let limit := 1200000000000000000 // 1.2V
                        let neg_limit := sub(0, limit)

                        if sgt(input, limit) {
                            output := 800000000000000000 // 0.8V clamp
                        }
                        if slt(input, neg_limit) {
                            output := sub(0, 800000000000000000) // -0.8V clamp
                        }
                        // Use iszero of sgt/slt because Yul has no sle/sge
                        if and(iszero(sgt(input, limit)), iszero(slt(input, neg_limit))) {
                            // y = x * (1 - 0.25 * x^2)
                            let x2 := sdiv(mul(input, input), scale)
                            let factor := sub(scale, sdiv(x2, 4))
                            output := sdiv(mul(input, factor), scale)
                        }
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFlags) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let output := input
                let isClipping := shr(128, statePack)

                if isClipping {
                    let limit := 1200000000000000000
                    let neg_limit := sub(0, limit)

                    if sgt(input, limit) {
                        output := 800000000000000000
                    }
                    if slt(input, neg_limit) {
                        output := sub(0, 800000000000000000)
                    }
                    if and(iszero(sgt(input, limit)), iszero(slt(input, neg_limit))) {
                        let x2 := sdiv(mul(input, input), scale)
                        let factor := sub(scale, sdiv(x2, 4))
                        output := sdiv(mul(input, factor), scale)
                    }
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
