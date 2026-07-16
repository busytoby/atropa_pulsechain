object "ZenerDither" {
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

                    let seed := and(statePack, 0xffffffffffffffffffffffffffffffff)
                    let isClipping := shr(128, statePack)

                    let output := input
                    if isClipping {
                        // Generate pseudo-random Zener noise (dither) using Keccak256 of seed
                        mstore(0, seed)
                        let rand := keccak256(0, 32)
                        // Scale to micro-dither: range [-10mV, +10mV] -> [-10000000000000000, +10000000000000000]
                        let noiseMagnitude := mod(rand, 20000000000000000)
                        let noise := sub(noiseMagnitude, 10000000000000000)
                        output := add(input, noise)
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndFeedback) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)

                let seed := and(statePack, 0xffffffffffffffffffffffffffffffff)
                let isClipping := shr(128, statePack)

                let output := input
                if isClipping {
                    mstore(0, seed)
                    let rand := keccak256(0, 32)
                    let noiseMagnitude := mod(rand, 20000000000000000)
                    let noise := sub(noiseMagnitude, 10000000000000000)
                    output := add(input, noise)
                }

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
