object "SubharmonicDivider" {
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

                    let clock := and(statePack, 0xffffffffffffffffffffffffffffffff)
                    let isClipping := shr(128, statePack)

                    // Divide frequency: divide clock index by 2
                    let subharmonic := scale // +1V
                    if and(div(clock, 2), 1) {
                        subharmonic := sub(0, scale) // -1V
                    }

                    let output := input
                    if isClipping {
                        // Mix 70% input signal and 30% sub-harmonic tone
                        let inputTerm := sdiv(mul(input, 700000000000000000), scale)
                        let subTerm := sdiv(mul(subharmonic, 300000000000000000), scale)
                        output := add(inputTerm, subTerm)
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

                let clock := and(statePack, 0xffffffffffffffffffffffffffffffff)
                let isClipping := shr(128, statePack)

                let subharmonic := scale
                if and(div(clock, 2), 1) {
                    subharmonic := sub(0, scale)
                }

                let output := input
                if isClipping {
                    let inputTerm := sdiv(mul(input, 700000000000000000), scale)
                    let subTerm := sdiv(mul(subharmonic, 300000000000000000), scale)
                    output := add(inputTerm, subTerm)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
