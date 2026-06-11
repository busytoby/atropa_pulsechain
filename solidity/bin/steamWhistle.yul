object "SteamWhistle" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18
            function V_STRIKE() -> val { val := 5000000000000000000 } // 5V strike potential
            function V_EXTINGUISH() -> val { val := 1000000000000000000 } // 1V extinguish potential
            function V_CC() -> val { val := 9000000000000000000 } // 9V supply rail

            // processSample(int256 trigger, int256 pitchAndNoiseMix) -> int256 outputSignal
            // selector: 0x07a96d8c (standard 2-parameter processSample)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let trigger := calldataload(4)
                let packedVal := calldataload(36)
                
                // Unpack:
                // pitch: bits 0..127
                // noiseMix: bits 128..191
                // mode: bits 192..255 (0 = Whistle, 1 = Flickering Flame)
                let pitch := and(packedVal, 0xffffffffffffffffffffffffffffffff)
                let noiseMix := and(shr(128, packedVal), 0xffffffffffffffff)
                let mode := and(shr(192, packedVal), 0xffffffffffffffff)
                let scale := SCALE()

                // Load state:
                // Vc = slot 100 (relaxation capacitor voltage)
                // is_discharging = slot 101 (thyristor discharge latch)
                // seed = slot 102 (noise generator LCG seed)
                // noise_lp = slot 103 (low-pass filtered noise state)
                let Vc := sload(100)
                let is_discharging := sload(101)
                let seed := sload(102)
                let noise_lp := sload(103)

                // Initialize states if empty
                if eq(seed, 0) {
                    seed := 987654321
                }
                if eq(Vc, 0) {
                    Vc := V_EXTINGUISH()
                }

                // 1. DUG Germanium Avalanche Noise Generator (LCG + Low-Pass Filter)
                seed := and(add(mul(seed, 1664525), 1013904223), 0xffffffff)
                sstore(102, seed)

                // Raw noise between -1V and +1V (scaled by 1e18)
                let rawNoise := sub(mul(mod(seed, 2000000), 1000000000000), 1000000000000000000)

                // Filter noise (alpha = 0.15) for acoustic dampening
                let noise_diff := sub(rawNoise, noise_lp)
                noise_lp := add(noise_lp, sdiv(mul(noise_diff, 150000000000000000), scale))
                sstore(103, noise_lp)

                // 2. TUP-TUN Relaxation Oscillator
                let whistleVal := 0
                if eq(is_discharging, 0) {
                    // Charging phase: Vc rises towards V_CC
                    let rate := 30000000000000000 // 0.03 default charging increment

                    // Flickering Flame Mode alters the charge rate & introduces random modulation
                    if eq(mode, 1) {
                        // Much slower rate for low-frequency flicker (1/100 speed)
                        rate := 300000000000000
                        // Modulate charging rate with noise to simulate flicker instability
                        let modVal := add(scale, sdiv(mul(rawNoise, 500000000000000000), scale)) // 0.5 to 1.5 multiplier
                        rate := sdiv(mul(rate, modVal), scale)
                    } {
                        // Whistle mode: pitch triggers increase charge rate
                        if gt(trigger, 0) {
                            rate := mul(rate, 2)
                        }
                    }

                    let diff := sub(V_CC(), Vc)
                    let dv := sdiv(mul(mul(diff, rate), pitch), mul(scale, scale))
                    Vc := add(Vc, dv)

                    if sgt(Vc, V_STRIKE()) {
                        is_discharging := 1
                    }
                } {
                    // Rapid discharge phase: Vc falls to V_EXTINGUISH
                    let discharge_rate := 300000000000000000 // 0.3
                    if eq(mode, 1) {
                        // Slower discharge for flame crackles
                        discharge_rate := 50000000000000000 // 0.05
                    }
                    let dv := sdiv(mul(sub(Vc, V_EXTINGUISH()), discharge_rate), scale)
                    Vc := sub(Vc, dv)

                    if slt(Vc, add(V_EXTINGUISH(), 100000000000000000)) {
                        Vc := V_EXTINGUISH()
                        is_discharging := 0
                    }
                }
                sstore(100, Vc)
                sstore(101, is_discharging)

                // Center the oscillator output
                whistleVal := sub(Vc, 3000000000000000000)

                // 3. Mix Whistle/Flame Tone and Low-Pass Noise
                let tonePart := mul(sub(scale, noiseMix), whistleVal)
                let noisePart := mul(noiseMix, noise_lp)
                let outVal := sdiv(add(tonePart, noisePart), scale)

                mstore(0, outVal)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
