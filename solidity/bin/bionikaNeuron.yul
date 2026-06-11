/*
    Bionika Threshold Sensory Neuron.
    Simulates a bionic spiking neuron cell with leaky integration, threshold firing,
    and a refractory period.
*/
object "BionikaNeuron" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function THRESHOLD() -> val { val := mul(1, SCALE()) } // 1.0V threshold

            // processSample(int256 inputSynapse, int256 leakRate) -> (int256 outputSpike, int256 membranePotential)
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let leak := calldataload(36)
                let scale := SCALE()

                // Load internal states
                let potential := sload(100)
                let refractory := sload(101)

                let spike := 0

                // If in refractory period, decrement timer and output 0
                if sgt(refractory, 0) {
                    refractory := sub(refractory, 1)
                    potential := 0
                } {
                    // Accumulate input stimulus
                    potential := add(potential, input)

                    // Apply leaky exponential decay: potential = potential * (1 - leak)
                    let decay_factor := sub(scale, leak)
                    potential := sdiv(mul(potential, decay_factor), scale)

                    // Check threshold firing
                    if eq(sgt(potential, THRESHOLD()), 1) {
                        spike := scale // Fire a 1.0V spike
                        potential := 0 // Reset membrane potential
                        refractory := 3 // Set refractory period of 3 ticks
                    }
                }

                // Save states
                sstore(100, potential)
                sstore(101, refractory)

                mstore(0, spike)
                mstore(32, potential)
                return(0, 64)
            }
            revert(0, 0)
        }
    }
}
