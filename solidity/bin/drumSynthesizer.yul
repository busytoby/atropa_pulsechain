object "DrumSynthesizer" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 trigger, int256 pitchAndDecay) -> int256 outputSignal
            // selector: 0x07a96d8c (standard 2-parameter processSample)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let trigger := calldataload(4)
                let packedVal := calldataload(36)
                
                // Unpack pitchK (lower 128 bits) and decayD (upper 128 bits)
                let pitchK := and(packedVal, 0xffffffffffffffffffffffffffffffff)
                let decayD := shr(128, packedVal)
                let scale := SCALE()

                // Load state (displacement = slot 100, velocity = slot 101)
                let y := sload(100)
                let v := sload(101)

                // If trigger is high, inject impulse energy into velocity
                if gt(trigger, 0) {
                    v := add(v, 1000000000000000000) // +1.0 impulse
                }

                // Update state:
                // y_new = y + v
                let y_new := add(y, v)

                // v_new = v - (pitchK * y_new)/scale - (decayD * v)/scale
                let springForce := sdiv(mul(pitchK, y_new), scale)
                let dampingForce := sdiv(mul(decayD, v), scale)
                let v_new := sub(sub(v, springForce), dampingForce)

                // Save states
                sstore(100, y_new)
                sstore(101, v_new)

                // Return displacement as the audio signal
                mstore(0, y_new)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
