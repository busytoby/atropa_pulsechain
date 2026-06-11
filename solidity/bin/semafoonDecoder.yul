/*
    Wien-Bridge Selective Tone Decoder (Semafoon PTT Receiver).
    Implements a second-order IIR bandpass filter tuned to a selective calling frequency (e.g., 1000 Hz).
    Smoothes the rectified output using an RC integrator to determine if the tone is present.
*/
object "SemafoonDecoder" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            
            // IIR Coefficients (18-decimal fixed point) for bandpass response:
            // Tuned to narrow band around target frequency
            function B0() -> val { val := 50000000000000000 }      // 0.05
            function B1() -> val { val := 0 }
            function B2() -> val { val := sub(0, 50000000000000000) }     // -0.05
            function A1() -> val { val := sub(0, 1850000000000000000) }   // -1.85
            function A2() -> val { val := 900000000000000000 }      // 0.90
            
            // processSample(int256 inputSignal, int256 rcDecay) -> (int256 filteredOutput, int256 envelope)
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let decay := calldataload(36)
                let scale := SCALE()

                // Load historical states
                let x_1 := sload(100)
                let x_2 := sload(101)
                let y_1 := sload(102)
                let y_2 := sload(103)
                let envelope := sload(104)

                // Calculate second-order IIR difference equation:
                // y = b0*x + b1*x_1 + b2*x_2 - a1*y_1 - a2*y_2
                let term_b0 := sdiv(mul(B0(), input), scale)
                let term_b1 := sdiv(mul(B1(), x_1), scale)
                let term_b2 := sdiv(mul(B2(), x_2), scale)
                let term_a1 := sdiv(mul(A1(), y_1), scale)
                let term_a2 := sdiv(mul(A2(), y_2), scale)

                let y := sub(add(add(term_b0, term_b1), term_b2), add(term_a1, term_a2))

                // Shift input & output history
                sstore(100, input)
                sstore(101, x_1)
                sstore(102, y)
                sstore(103, y_1)

                // Rectified Envelope detection:
                // envelope = envelope * (1 - decay) + abs(y) * decay
                let abs_y := y
                if slt(y, 0) {
                    abs_y := sub(0, y)
                }

                let decay_term := sdiv(mul(sub(scale, decay), envelope), scale)
                let input_term := sdiv(mul(decay, abs_y), scale)
                envelope := add(decay_term, input_term)

                sstore(104, envelope)

                mstore(0, y)
                mstore(32, envelope)
                return(0, 64)
            }
            revert(0, 0)
        }
    }
}
