object "SnareSynthesizer" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSample(int256 trigger, int256 paramsPack) -> int256 outputSignal
            // selector: 0x07a96d8c (standard 2-parameter processSample)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let trigger := calldataload(4)
                let packedVal := calldataload(36)
                
                // Unpack parameters (64 bits each):
                // pitchK: bits 0..63
                // decayD: bits 64..127
                // noiseDecay: bits 128..191
                // noiseMix: bits 192..255
                let pitchK := and(packedVal, 0xffffffffffffffff)
                let decayD := and(shr(64, packedVal), 0xffffffffffffffff)
                let noiseDecay := and(shr(128, packedVal), 0xffffffffffffffff)
                let noiseMix := and(shr(192, packedVal), 0xffffffffffffffff)
                let scale := SCALE()

                // Load state:
                // y = slot 100 (resonator displacement)
                // v = slot 101 (resonator velocity)
                // seed = slot 102 (LCG noise seed)
                // env_noise = slot 103 (noise envelope)
                let y := sload(100)
                let v := sload(101)
                let seed := sload(102)
                let env_noise := sload(103)

                // Initialize seed if uninitialized
                if eq(seed, 0) {
                    seed := 123456789
                }

                // If trigger is high, inject velocity and reset noise envelope
                if gt(trigger, 0) {
                    v := add(v, 1000000000000000000) // +1.0 tonal impulse
                    env_noise := 1000000000000000000 // 1.0 noise envelope peak
                }

                // 1. Update LCG Noise Generator: seed = (seed * 1103515245 + 12345) % 2^31
                seed := and(add(mul(seed, 1103515245), 12345), 0x7fffffff)
                sstore(102, seed)

                // Map seed to noise between -0.5V and +0.5V (scaled to 1e18)
                // noise = (seed % 1000000) * 1000000000000 - 500000000000000000
                let noiseVal := sub(mul(mod(seed, 1000000), 1000000000000), 500000000000000000)

                // 2. Update Noise Envelope: env_noise = env_noise - (env_noise * noiseDecay)/scale
                let envDecay := sdiv(mul(env_noise, noiseDecay), scale)
                if sgt(envDecay, env_noise) {
                    envDecay := env_noise
                }
                env_noise := sub(env_noise, envDecay)
                sstore(103, env_noise)

                // 3. Update Resonator State:
                let y_new := add(y, v)
                let springForce := sdiv(mul(pitchK, y_new), scale)
                let dampingForce := sdiv(mul(decayD, v), scale)
                let v_new := sub(sub(v, springForce), dampingForce)
                sstore(100, y_new)
                sstore(101, v_new)

                // 4. Mix outputs: out = ((scale - noiseMix) * y_new + noiseMix * (noiseVal * env_noise / scale)) / scale
                let tonePart := mul(sub(scale, noiseMix), y_new)
                let shapedNoise := sdiv(mul(noiseVal, env_noise), scale)
                let noisePart := mul(noiseMix, shapedNoise)
                let outVal := sdiv(add(tonePart, noisePart), scale)

                // Return final mixed signal
                mstore(0, outVal)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
