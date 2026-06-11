/*
    Wien-Bridge Selective Tone Decoder (Semafoon PTT Receiver).
    Implements two parallel second-order IIR bandpass filters (FDM Multiplexed)
    tuned to Carrier A (1000 Hz) and Carrier B (2000 Hz) respectively.
*/
object "SemafoonDecoder" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            
            // IIR Coefficients for Bandpass A (1000 Hz):
            function B0() -> val { val := 50000000000000000 }      // 0.05
            function B2() -> val { val := sub(0, 50000000000000000) }     // -0.05
            function AA1() -> val { val := sub(0, 1850000000000000000) }   // -1.85
            function AA2() -> val { val := 900000000000000000 }      // 0.90

            // IIR Coefficients for Bandpass B (2000 Hz):
            function AB1() -> val { val := sub(0, 1450000000000000000) }   // -1.45
            function AB2() -> val { val := 900000000000000000 }      // 0.90
            
            // processSample(int256 inputSignal, int256 rcDecay) -> (int256 filteredOutput, int256 envelopeA, int256 envelopeB)
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let decay := calldataload(36)
                let scale := SCALE()

                // Load historical states for Filter A
                let x_1 := sload(100)
                let x_2 := sload(101)
                let yA_1 := sload(102)
                let yA_2 := sload(103)
                let envA := sload(104)

                // Load historical states for Filter B
                let yB_1 := sload(105)
                let yB_2 := sload(106)
                let envB := sload(107)

                // Shared input terms
                let term_b0 := sdiv(mul(B0(), input), scale)
                let term_b2 := sdiv(mul(B2(), x_2), scale)

                // 1. Process Wien-Bridge A (1000 Hz)
                let term_aA1 := sdiv(mul(AA1(), yA_1), scale)
                let term_aA2 := sdiv(mul(AA2(), yA_2), scale)
                let yA := sub(add(term_b0, term_b2), add(term_aA1, term_aA2))

                // 2. Process Wien-Bridge B (2000 Hz)
                let term_aB1 := sdiv(mul(AB1(), yB_1), scale)
                let term_aB2 := sdiv(mul(AB2(), yB_2), scale)
                let yB := sub(add(term_b0, term_b2), add(term_aB1, term_aB2))

                // Shift histories
                sstore(100, input)
                sstore(101, x_1)
                
                sstore(102, yA)
                sstore(103, yA_1)
                
                sstore(105, yB)
                sstore(106, yB_1)

                // Rectified Envelope detection A
                let abs_yA := yA
                if slt(yA, 0) { abs_yA := sub(0, yA) }
                let decay_termA := sdiv(mul(sub(scale, decay), envA), scale)
                let input_termA := sdiv(mul(decay, abs_yA), scale)
                envA := add(decay_termA, input_termA)
                sstore(104, envA)

                // Rectified Envelope detection B
                let abs_yB := yB
                if slt(yB, 0) { abs_yB := sub(0, yB) }
                let decay_termB := sdiv(mul(sub(scale, decay), envB), scale)
                let input_termB := sdiv(mul(decay, abs_yB), scale)
                envB := add(decay_termB, input_termB)
                sstore(107, envB)

                mstore(0, yA)
                mstore(32, envA)
                mstore(64, envB)
                return(0, 96)
            }
            revert(0, 0)
        }
    }
}
