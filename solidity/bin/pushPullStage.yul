/*
    Complementary PNP/NPN Transistor Push-Pull Output Stage.
    Simulates complementary symmetry amplification (e.g. AC127/AC128 pairs).
    Models crossover distortion dead-zones when bias voltage is low,
    generating rich odd-order harmonics for audio synthesis.
*/
object "PushPullStage" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function DIODE_OFFSET() -> val { val := 200000000000000000 } // 200mV conduction threshold

            // processSample(int256 inputSignal, int256 biasOffset) -> int256 outputSignal
            // selector: 0x07a96d8c (standard processSample signature)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let bias := calldataload(36)
                let scale := SCALE()

                // Conduction offset reduced by bias voltage:
                let effective_threshold := 0
                if sgt(DIODE_OFFSET(), bias) {
                    effective_threshold := sub(DIODE_OFFSET(), bias)
                }

                let output := 0

                // Positive cycle: NPN transistor conducts (push)
                if sgt(input, effective_threshold) {
                    output := sub(input, effective_threshold)
                }
                // Negative cycle: PNP transistor conducts (pull)
                if slt(input, sub(0, effective_threshold)) {
                    output := add(input, effective_threshold)
                }

                // If input falls inside the dead-zone [-threshold, threshold], output remains 0 (crossover dead-zone)

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
