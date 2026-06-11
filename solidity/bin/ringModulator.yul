object "RingModulator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 inputSignal, int256 carrierSignal) -> int256 outputSignal
            // selector: 0x07a96d8c (standard processSample signature)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let carrier := calldataload(36)
                let scale := SCALE()

                // Signed multiplication and scaling: y[n] = (input * carrier) / scale
                let output := sdiv(mul(input, carrier), scale)

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
