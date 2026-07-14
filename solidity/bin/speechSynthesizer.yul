object "SpeechSynthesizer" {
    code {
        // Initcode: Copy runtime code to memory and return it for deployment
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            // Revert on call values
            if callvalue() { revert(0, 0) }

            // Ensure calldatasize is at least 4 bytes
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // Helper to get current CPU address
            function getCpuAddress() -> cpuAddr {
                cpuAddr := sload(0)
                if iszero(cpuAddr) {
                    cpuAddr := 0x270049e5af030c305Fe92C3af096A5E3445B7fb7 // Default hardcoded localhost CPU
                }
            }

            // Helper to call peekUser(address user, uint256 addr) -> uint256
            function peekUser(cpu, user, addr) -> val {
                mstore(0x1000, shl(224, 0x0ccd522c)) // selector for peekUser(address,uint256)
                mstore(0x1004, user)
                mstore(0x1024, addr)
                let ok := staticcall(gas(), cpu, 0x1000, 68, 0x2000, 32)
                if iszero(ok) { revert(0, 0) }
                val := mload(0x2000)
            }

            // Helper to call pokeUser(address user, uint256 addr, uint256 val) -> uint256
            function pokeUser(cpu, user, addr, val) {
                mstore(0x1000, shl(224, 0xa7260f02)) // selector for pokeUser(address,uint256,uint256)
                mstore(0x1004, user)
                mstore(0x1024, addr)
                mstore(0x1044, val)
                let ok := call(gas(), cpu, 0, 0x1000, 100, 0x2000, 32)
                if iszero(ok) { revert(0, 0) }
            }

            // Fixed-point Taylor series approximation of J0(x) scaled by 1000
            function besselJ0Scaled(x) -> val {
                let x2 := div(mul(x, x), 1000)
                let term1 := div(mul(x2, 250), 1000)
                let x4 := div(mul(x2, x2), 1000)
                let term2 := div(mul(x4, 15625), 1000000) // 1/64 = 0.015625
                let x6 := div(mul(x4, x2), 1000)
                let term3 := div(x6, 2304)
                
                let pos := add(1000, term2)
                let neg := add(term1, term3)
                if gt(pos, neg) {
                    val := sub(pos, neg)
                }
                if iszero(gt(pos, neg)) {
                    val := 0
                }
            }

            // Simple exponential decay approximation for K0(x) scaled by 1000
            function besselK0Scaled(x) -> val {
                let x2 := div(mul(x, x), 1000)
                let denom := add(add(1000, x), div(x2, 2))
                val := div(1000000, denom)
            }

            // ----------------------------------------------------------------
            // Method: bindCpuAddress(address cpu) -> bool
            // Selector: 0x678575ea
            // ----------------------------------------------------------------
            if eq(selector, 0x678575ea) {
                let cpu := and(calldataload(4), 0xffffffffffffffffffffffffffffffffffffffff)
                sstore(0, cpu)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: getSpeechState() -> (uint256, uint256, uint256, uint256)
            // Selector: 0x8040caa2
            // ----------------------------------------------------------------
            if eq(selector, 0x8040caa2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let p := peekUser(cpu, callerAddr, 54784)
                let inf := peekUser(cpu, callerAddr, 54785)
                let busy := peekUser(cpu, callerAddr, 54786)
                let counter := peekUser(cpu, callerAddr, 54787)
                
                mstore(0x00, p)
                mstore(0x20, inf)
                mstore(0x40, busy)
                mstore(0x60, counter)
                return(0x00, 128)
            }

            // ----------------------------------------------------------------
            // Method: getSpeechStateExtended() -> (uint256, uint256, uint256, uint256, uint256, uint256, uint256)
            // Selector: 0xd0e83b8b
            // ----------------------------------------------------------------
            if eq(selector, 0xd0e83b8b) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let p := peekUser(cpu, callerAddr, 54784)
                let inf := peekUser(cpu, callerAddr, 54785)
                let busy := peekUser(cpu, callerAddr, 54786)
                let counter := peekUser(cpu, callerAddr, 54787)
                let glide := peekUser(cpu, callerAddr, 54788)
                let vibrato := peekUser(cpu, callerAddr, 54789)
                let acousticHash := peekUser(cpu, callerAddr, 54796)
                
                mstore(0x00, p)
                mstore(0x20, inf)
                mstore(0x40, busy)
                mstore(0x60, counter)
                mstore(0x80, glide)
                mstore(0xa0, vibrato)
                mstore(0xc0, acousticHash)
                return(0x00, 224)
            }

            // ----------------------------------------------------------------
            // Method: writeSpeechControlsExtended(uint256 glide, uint256 vibrato) -> uint256
            // Selector: 0x789b21f0
            // ----------------------------------------------------------------
            if eq(selector, 0x789b21f0) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let glide := calldataload(4)
                let vibrato := calldataload(36)
                
                pokeUser(cpu, callerAddr, 54788, glide)
                pokeUser(cpu, callerAddr, 54789, vibrato)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writePhoneme(uint256 phoneme, uint256 inflection) -> uint256
            // Selector: 0xe435320e
            // ----------------------------------------------------------------
            if eq(selector, 0xe435320e) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let phoneme := calldataload(4)
                let inflection := calldataload(36)
                
                pokeUser(cpu, callerAddr, 54785, inflection)
                pokeUser(cpu, callerAddr, 54784, phoneme)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: clearBusy() -> uint256
            // Selector: 0x761abbb0
            // ----------------------------------------------------------------
            if eq(selector, 0x761abbb0) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                pokeUser(cpu, callerAddr, 54786, 0)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: getTMS5220State() -> (uint256, uint256, uint256)
            // Selector: 0xada6ddc2
            // ----------------------------------------------------------------
            if eq(selector, 0xada6ddc2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let cmd := peekUser(cpu, callerAddr, 54792)
                let status := peekUser(cpu, callerAddr, 54794)
                let fifoLen := peekUser(cpu, callerAddr, 54795)
                
                mstore(0x00, cmd)
                mstore(0x20, status)
                mstore(0x40, fifoLen)
                return(0x00, 96)
            }

            // ----------------------------------------------------------------
            // Method: writeTMS5220Command(uint256 command) -> uint256
            // Selector: 0x285a5991
            // ----------------------------------------------------------------
            if eq(selector, 0x285a5991) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let command := calldataload(4)
                pokeUser(cpu, callerAddr, 54792, command)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writeTMS5220Data(uint256 data) -> uint256
            // Selector: 0x5dcd4af2
            // ----------------------------------------------------------------
            if eq(selector, 0x5dcd4af2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let data := calldataload(4)
                pokeUser(cpu, callerAddr, 54793, data)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: bindMusicMaker(address musicMaker) -> bool
            // Selector: 0x665d2526
            // ----------------------------------------------------------------
            if eq(selector, 0x665d2526) {
                let musicMaker := and(calldataload(4), 0xffffffffffffffffffffffffffffffffffffffff)
                sstore(1, musicMaker)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writeFormants(uint256 f1, uint256 f2, uint256 f3) -> bool
            // Selector: 0x07c01cb1
            // ----------------------------------------------------------------
            if eq(selector, 0x07c01cb1) {
                let f1 := calldataload(4)
                let f2 := calldataload(36)
                let f3 := calldataload(68)

                let musicMaker := sload(1)
                if iszero(iszero(musicMaker)) {
                    // Voice 1 Freq Low/High/Control
                    let freq1 := mul(f1, 10)
                    let lo1 := and(freq1, 0xff)
                    let hi1 := and(shr(8, freq1), 0xff)
                    
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54272)
                    mstore(0x1124, lo1)
                    let dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54273)
                    mstore(0x1124, hi1)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54276)
                    mstore(0x1124, 17) // Sawtooth
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }

                    // Voice 2 Freq Low/High/Control
                    let freq2 := mul(f2, 10)
                    let lo2 := and(freq2, 0xff)
                    let hi2 := and(shr(8, freq2), 0xff)
                    
                    mstore(0x1104, 54279)
                    mstore(0x1124, lo2)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54280)
                    mstore(0x1124, hi2)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54283)
                    mstore(0x1124, 17) // Sawtooth
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }

                    // Voice 3 Freq Low/High/Control
                    let freq3 := mul(f3, 10)
                    let lo3 := and(freq3, 0xff)
                    let hi3 := and(shr(8, freq3), 0xff)
                    
                    mstore(0x1104, 54286)
                    mstore(0x1124, lo3)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54287)
                    mstore(0x1124, hi3)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    mstore(0x1104, 54290)
                    mstore(0x1124, 17) // Sawtooth
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writeReflectionCoefficients(uint256[9] coefficients, uint256 pitch, uint256 energy) -> bool
            // Selector: 0xbd7856a3
            // ----------------------------------------------------------------
            if eq(selector, 0xbd7856a3) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                
                // Poke 9 reflection coefficients to addresses 54800 to 54808
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    let coef := calldataload(add(4, mul(i, 32)))
                    pokeUser(cpu, callerAddr, add(54800, i), coef)
                }

                let pitch := calldataload(292)
                let energy := calldataload(324)

                // Retrieve Vaesen Trauma parameters from host memory
                let physTrauma := peekUser(cpu, callerAddr, 55043)
                let mentTrauma := peekUser(cpu, callerAddr, 55044)

                // Adjust pitch and energy based on Vaesen Trauma states
                let currentPitch := pitch
                let currentEnergy := energy

                // 1. Physical Trauma -> Volume / Energy suppression
                if eq(physTrauma, 1) { // Exhausted
                    currentEnergy := div(mul(currentEnergy, 80), 100)
                }
                if eq(physTrauma, 2) { // Battered
                    currentEnergy := div(mul(currentEnergy, 60), 100)
                }
                if eq(physTrauma, 3) { // Broken
                    currentEnergy := div(mul(currentEnergy, 30), 100)
                }

                // 2. Mental Trauma -> Pitch & Jitter modulation
                let jitter := 0
                if eq(mentTrauma, 1) { // Shaken
                    jitter := 4
                }
                if eq(mentTrauma, 2) { // Terrified
                    currentPitch := add(currentPitch, 20)
                    jitter := 12
                }
                if eq(mentTrauma, 3) { // Panicked
                    currentPitch := add(currentPitch, 40)
                    jitter := 25
                    // Periodic stutter: mute voice based on slot/counter
                    let counter := sload(102)
                    sstore(102, add(counter, 1))
                    if iszero(and(counter, 0x02)) {
                        currentEnergy := 0
                    }
                }
                if eq(mentTrauma, 4) { // Melancholic
                    if gt(currentPitch, 30) {
                        currentPitch := sub(currentPitch, 25)
                    }
                    currentEnergy := div(mul(currentEnergy, 50), 100)
                }

                // Calculate random jitter using a Linear Congruential Generator (LCG)
                if gt(jitter, 0) {
                    let seed := sload(101)
                    if iszero(seed) {
                        seed := 12345
                    }
                    seed := add(mul(seed, 1664525), 1013904223)
                    sstore(101, seed)
                    
                    let randVal := mod(seed, mul(jitter, 2))
                    if lt(randVal, jitter) {
                        if gt(currentPitch, randVal) {
                            currentPitch := sub(currentPitch, randVal)
                        }
                    }
                    if iszero(lt(randVal, jitter)) {
                        currentPitch := add(currentPitch, sub(randVal, jitter))
                    }
                }

                pokeUser(cpu, callerAddr, 54809, currentPitch)
                pokeUser(cpu, callerAddr, 54810, currentEnergy)

                // Retrieve the previously written pitch to perform smoothing/interpolation
                let prevPitch := sload(100)
                if iszero(prevPitch) {
                    prevPitch := currentPitch
                }
                // Calculate smoothed pitch (average of previous and current) to eliminate robotic buzz/steppiness
                let smoothedPitch := div(add(prevPitch, currentPitch), 2)
                sstore(100, currentPitch)

                // If musicMaker address is set, poke frequency registers to voice 1
                let musicMaker := sload(1)
                if iszero(iszero(musicMaker)) {
                    // Qwen-level Speech Synthesis: Voiced vs. Unvoiced Waveform Routing
                    // If pitch is 0, route to C64 SID Noise generator (waveform control 129) for unvoiced sibilants (s, sh, f).
                    // If pitch > 0, route to C64 SID Sawtooth generator (waveform control 17) for voiced vowels.
                    let controlVal := 17
                    let targetPitch := smoothedPitch
                    if iszero(currentPitch) {
                        controlVal := 129
                        targetPitch := 80 // Default base noise frequency
                    }

                    let freq := mul(targetPitch, 10)
                    let lo := and(freq, 0xff)
                    let hi := and(shr(8, freq), 0xff)
                    
                    // Poke Voice 1 Freq Low
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54272)
                    mstore(0x1124, lo)
                    let dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    // Poke Voice 1 Freq High
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54273)
                    mstore(0x1124, hi)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    // Poke Voice 1 Control (17 for Sawtooth, 129 for Noise)
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54276)
                    mstore(0x1124, controlVal)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: selectVoice(uint256) -> bool
            // Selector: 0x3899c77b
            // ----------------------------------------------------------------
            if eq(selector, 0x3899c77b) {
                let voiceId := calldataload(4)
                sstore(2, voiceId)
                sstore(200, 0) // Reset coefficient smoothing state
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: generateNeuralSpeech(bytes32) -> bool
            // Selector: 0xc69b606a
            // ----------------------------------------------------------------
            if eq(selector, 0xc69b606a) {
                let key := calldataload(4)
                let voiceId := sload(2) // 0 = Ana, 1 = Moloch
                let basePitch := 420
                if eq(voiceId, 1) {
                    basePitch := 150
                }
                
                let energy := 0
                let soundType := 0 // 0 = silent, 1 = voiced, 2 = unvoiced, 3 = nasal

                // Check phoneme keys by matching prefixes
                let twoChars := and(key, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let oneChar := and(key, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                // aa (0x6161)
                if eq(twoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    energy := 90
                    soundType := 1
                }
                // ee (0x6565)
                if eq(twoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    energy := 95
                    soundType := 1
                }
                // oo (0x6f6f)
                if eq(twoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    energy := 85
                    soundType := 1
                }
                // sh (0x7368)
                if eq(twoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                    energy := 80
                    soundType := 2
                }
                // s (0x73)
                if eq(oneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                    if iszero(eq(twoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                        energy := 70
                        soundType := 2
                    }
                }
                // f (0x66)
                if eq(oneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                    energy := 40
                    soundType := 2
                }
                // m (0x6d)
                if eq(oneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                    energy := 50
                    soundType := 3
                }
                // n (0x6e)
                if eq(oneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                    energy := 45
                    soundType := 3
                }
                // ih (0x6968)
                if eq(twoChars, 0x6968000000000000000000000000000000000000000000000000000000000000) {
                    energy := 90
                    soundType := 1
                }
                // eh (0x6568)
                if eq(twoChars, 0x6568000000000000000000000000000000000000000000000000000000000000) {
                    energy := 88
                    soundType := 1
                }
                // ah (0x6168)
                if eq(twoChars, 0x6168000000000000000000000000000000000000000000000000000000000000) {
                    energy := 92
                    soundType := 1
                }
                // th (0x7468)
                if eq(twoChars, 0x7468000000000000000000000000000000000000000000000000000000000000) {
                    energy := 30
                    soundType := 2
                }
                // z (0x7a)
                if eq(oneChar, 0x7a00000000000000000000000000000000000000000000000000000000000000) {
                    energy := 60
                    soundType := 1
                }
                // p (0x70)
                if eq(oneChar, 0x7000000000000000000000000000000000000000000000000000000000000000) {
                    energy := 50
                    soundType := 2
                }
                // t (0x74)
                if eq(oneChar, 0x7400000000000000000000000000000000000000000000000000000000000000) {
                    if iszero(eq(twoChars, 0x7468000000000000000000000000000000000000000000000000000000000000)) {
                        energy := 52
                        soundType := 2
                    }
                }
                // k (0x6b)
                if eq(oneChar, 0x6b00000000000000000000000000000000000000000000000000000000000000) {
                    energy := 48
                    soundType := 2
                }
                // ng (0x6e67)
                if eq(twoChars, 0x6e67000000000000000000000000000000000000000000000000000000000000) {
                    energy := 40
                    soundType := 3
                }

                let cpu := getCpuAddress()
                let callerAddr := caller()

                // Default coefficients for Ana (0) or Moloch (1)
                let isInit := sload(200)
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    let K := 10
                    
                    if iszero(voiceId) {
                        if eq(i, 0) { K := 15 }
                        if eq(i, 1) { K := sub(0, 25) }
                        if eq(i, 2) { K := 40 }
                        if eq(i, 3) { K := sub(0, 10) }
                        if eq(i, 4) { K := 5 }
                        if eq(i, 5) { K := sub(0, 8) }
                        if eq(i, 6) { K := 12 }
                        if eq(i, 7) { K := sub(0, 5) }
                        if eq(i, 8) { K := 2 }
                    }
                    if eq(voiceId, 1) {
                        if eq(i, 0) { K := 85 }
                        if eq(i, 1) { K := sub(0, 65) }
                        if eq(i, 2) { K := 70 }
                        if eq(i, 3) { K := sub(0, 45) }
                        if eq(i, 4) { K := 35 }
                        if eq(i, 5) { K := sub(0, 22) }
                        if eq(i, 6) { K := 18 }
                        if eq(i, 7) { K := sub(0, 10) }
                        if eq(i, 8) { K := 5 }
                    }

                    if eq(soundType, 1) {
                        if lt(i, 3) {
                            K := add(K, 15)
                        }
                    }
                    if eq(soundType, 2) {
                        if lt(i, 4) {
                            K := sub(K, 40)
                        }
                    }

                    if sgt(K, 99) { K := 99 }
                    if slt(K, sub(0, 99)) { K := sub(0, 99) }

                    let smoothedK := K
                    if isInit {
                        let prevK := sload(add(101, i))
                        smoothedK := div(add(mul(K, 3), prevK), 4)
                    }
                    sstore(add(101, i), smoothedK)

                    pokeUser(cpu, callerAddr, add(54800, i), smoothedK)
                }

                if iszero(energy) {
                    sstore(200, 0)
                }
                if energy {
                    sstore(200, 1)
                }

                pokeUser(cpu, callerAddr, 54809, basePitch)
                pokeUser(cpu, callerAddr, 54810, energy)

                let prevPitch := sload(100)
                if iszero(prevPitch) {
                    prevPitch := basePitch
                }
                let smoothedPitch := div(add(prevPitch, basePitch), 2)
                sstore(100, basePitch)

                let musicMaker := sload(1)
                if iszero(iszero(musicMaker)) {
                    let controlVal := 17
                    let targetPitch := smoothedPitch

                    if eq(soundType, 2) {
                        controlVal := 129
                        targetPitch := 80
                    }
                    if iszero(soundType) {
                        controlVal := 0
                        targetPitch := 0
                    }

                    let freq := mul(targetPitch, 10)
                    let lo := and(freq, 0xff)
                    let hi := and(shr(8, freq), 0xff)

                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54272)
                    mstore(0x1124, lo)
                    let dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54273)
                    mstore(0x1124, hi)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)

                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54276)
                    mstore(0x1124, controlVal)
                }
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: synthesizeAudio(bytes32 phonemeKey, uint256 numSamples) -> bytes
            // Selector: 0xfae0b699
            // ----------------------------------------------------------------
            if eq(selector, 0xfae0b699) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let synthKey := calldataload(4)
                let numSamples := calldataload(36)
                if gt(numSamples, 1000) { numSamples := 1000 }

                let synthVoiceId := sload(2) // 0 = Ana, 1 = Moloch
                let synthBasePitch := 420
                if eq(synthVoiceId, 1) {
                    synthBasePitch := 150
                }
                
                let synthEnergy := 0
                let synthSoundType := 0 // 0 = silent, 1 = voiced, 2 = unvoiced, 3 = nasal
                let voicingStrength := 0 // 0 = unvoiced, 100 = fully voiced

                // Check phoneme keys by matching prefixes
                let synthTwoChars := and(synthKey, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let synthOneChar := and(synthKey, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 90
                    synthSoundType := 1
                    voicingStrength := 85
                }
                if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 95
                    synthSoundType := 1
                    voicingStrength := 90
                }
                if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 85
                    synthSoundType := 1
                    voicingStrength := 80
                }
                if eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 80
                    synthSoundType := 2
                }
                if eq(synthOneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                    if iszero(eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                        synthEnergy := 70
                        synthSoundType := 2
                    }
                }
                if eq(synthOneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 40
                    synthSoundType := 2
                }
                if eq(synthOneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 50
                    synthSoundType := 3
                    voicingStrength := 65
                }
                if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 45
                    synthSoundType := 3
                    voicingStrength := 60
                }

                // Project K1..K9 coefficients into memory slots 0x4000 to 0x4008
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    let K := 10
                    if iszero(synthVoiceId) {
                        if eq(i, 0) { K := 15 }
                        if eq(i, 1) { K := sub(0, 25) }
                        if eq(i, 2) { K := 40 }
                        if eq(i, 3) { K := sub(0, 10) }
                        if eq(i, 4) { K := 5 }
                        if eq(i, 5) { K := sub(0, 8) }
                        if eq(i, 6) { K := 12 }
                        if eq(i, 7) { K := sub(0, 5) }
                        if eq(i, 8) { K := 2 }
                    }
                    if eq(synthVoiceId, 1) {
                        if eq(i, 0) { K := 85 }
                        if eq(i, 1) { K := sub(0, 65) }
                        if eq(i, 2) { K := 70 }
                        if eq(i, 3) { K := sub(0, 45) }
                        if eq(i, 4) { K := 35 }
                        if eq(i, 5) { K := sub(0, 22) }
                        if eq(i, 6) { K := 18 }
                        if eq(i, 7) { K := sub(0, 10) }
                        if eq(i, 8) { K := 5 }
                    }

                    if eq(synthSoundType, 1) {
                        if lt(i, 3) { K := add(K, 15) }
                    }
                    if eq(synthSoundType, 2) {
                        if lt(i, 4) { K := sub(K, 40) }
                    }

                    if sgt(K, 99) { K := 99 }
                    if slt(K, sub(0, 99)) { K := sub(0, 99) }

                    mstore(add(0x4000, mul(i, 32)), K)
                }

                // Restore Delay Line from CPU storage registers (54800 - 54809) for SYNC persistence
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    let savedVal := peekUser(cpu, callerAddr, add(54800, i))
                    mstore(add(0x2000, mul(i, 32)), savedVal)
                }

                // Initialize smoothing buffer at 0x4100 with target coefficients
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    mstore(add(0x4100, mul(i, 32)), mload(add(0x4000, mul(i, 32))))
                }

                // Setup ABI bytes headers at 0x5000
                mstore(0x5000, 0x20)
                mstore(0x5020, mul(numSamples, 2))

                let seed := sload(101)
                if iszero(seed) { seed := 12345 }

                // Synthesis Loop
                for { let t := 0 } lt(t, numSamples) { t := add(t, 1) } {
                    // Smoothly interpolate coefficients
                    for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                        let cur := mload(add(0x4100, mul(i, 32)))
                        let target := mload(add(0x4000, mul(i, 32)))
                        let diff := sub(target, cur)
                        cur := add(cur, sdiv(diff, 256))
                        mstore(add(0x4100, mul(i, 32)), cur)
                    }

                    let excitation := 0

                    if eq(synthSoundType, 1) {
                        // LFSR step for continuous breathy noise
                        let bit := and(seed, 1)
                        seed := shr(1, seed)
                        if bit {
                            seed := xor(seed, 0xB400)
                        }
                        let noise := sub(mod(seed, 200), 100) // [-100, 100]

                        // Extended Pitch contour tracking with Glide
                        let baseContourPitch := synthBasePitch
                        let targetPitch := peekUser(cpu, callerAddr, 54785)
                        if iszero(targetPitch) { targetPitch := synthBasePitch }
                        
                        let glideRate := peekUser(cpu, callerAddr, 54788)
                        let currentPitch := baseContourPitch
                        
                        // Linear glide interpolation step per sample time 't'
                        if and(glideRate, gt(t, 0)) {
                            let steps := mul(t, glideRate)
                            if lt(steps, 1000) {
                                let diff := sub(targetPitch, baseContourPitch)
                                currentPitch := add(baseContourPitch, sdiv(mul(diff, steps), 1000))
                            }
                            if iszero(lt(steps, 1000)) {
                                currentPitch := targetPitch
                            }
                        }
                        if iszero(glideRate) {
                            currentPitch := targetPitch
                        }

                        let jitterPercent := sub(mod(seed, 17), 8) // [-8, 8] per-mil
                        let jitteredPitch := add(currentPitch, sdiv(mul(currentPitch, jitterPercent), 1000))
                        
                        // 5.2Hz triangle vibrato LFO with dynamic Vibrato Depth ($D605)
                        let vibDepth := peekUser(cpu, callerAddr, 54789)
                        if iszero(vibDepth) { vibDepth := 6 } // default depth
                        
                        let vibratoPhase := mod(div(t, 128), 24)
                        let vibratoOffset := 0
                        if lt(vibratoPhase, 12) {
                            vibratoOffset := sub(vibratoPhase, 6)
                        }
                        if iszero(lt(vibratoPhase, 12)) {
                            vibratoOffset := sub(18, vibratoPhase)
                        }
                        vibratoOffset := sdiv(mul(vibratoOffset, vibDepth), 6)
                        let finalPitch := add(jitteredPitch, vibratoOffset)

                        let period := div(16000, finalPitch)
                        if iszero(period) { period := 72 }
                        let tMod := mod(t, period)
                        
                        let Tp := div(mul(period, 40), 100)
                        let Tn := div(mul(period, 16), 100)
                        let pulse := 0
                        
                        if lt(tMod, Tp) {
                            let ph := div(mul(tMod, 100), Tp)
                            let ph2 := div(mul(ph, ph), 100)
                            let ph3 := div(mul(ph2, ph), 100)
                            pulse := sub(mul(3, ph2), mul(2, ph3))
                        }
                        if and(iszero(lt(tMod, Tp)), lt(tMod, add(Tp, Tn))) {
                            let ph := div(mul(sub(tMod, Tp), 100), Tn)
                            let ph2 := div(mul(ph, ph), 100)
                            pulse := sub(100, ph2)
                        }
                        
                        // Amplitude Shimmer (2% cycles)
                        let shimmerPercent := sub(mod(seed, 5), 2) // [-2, 2]%
                        let shimmerFactor := add(100, shimmerPercent)

                        // Dynamic Mixed Excitation (MELP-style)
                        let pulseScaled := mul(sub(pulse, 29), 10)
                        let noiseScaled := mul(noise, 10)
                        let mixedExcitation := div(add(mul(pulseScaled, voicingStrength), mul(noiseScaled, sub(100, voicingStrength))), 100)
                        excitation := sdiv(mul(mixedExcitation, shimmerFactor), 100)
                    }
                    if eq(synthSoundType, 2) {
                        let bit := and(seed, 1)
                        seed := shr(1, seed)
                        if bit {
                            seed := xor(seed, 0xB400)
                        }
                        excitation := sub(mod(seed, 2000), 1000)
                    }
                    if eq(synthSoundType, 3) {
                        let period := 106
                        let val := mod(t, period)
                        excitation := sub(mul(val, 20), 1000)
                    }

                    let forward := sdiv(mul(excitation, synthEnergy), 100)

                    for { let i := 9 } gt(i, 0) { } {
                        i := sub(i, 1)
                        let K := mload(add(0x4100, mul(i, 32)))
                        let delayVal := mload(add(0x2000, mul(i, 32)))
                        let nextForward := sub(forward, sdiv(mul(K, delayVal), 100))
                        let nextDelay := add(delayVal, sdiv(mul(K, nextForward), 100))
                        mstore(add(0x2000, mul(add(i, 1), 32)), nextDelay)
                        forward := nextForward
                    }
                    mstore(0x2000, forward)

                    // Convert to 16-bit PCM (signed)
                    let pcmVal := mul(forward, 8)
                    if sgt(pcmVal, 32767) { pcmVal := 32767 }
                    if slt(pcmVal, sub(0, 32768)) { pcmVal := sub(0, 32768) }

                    // Accumulate deterministic Acoustic Hash ($D60C / 54796)
                    let currentHash := peekUser(cpu, callerAddr, 54796)
                    currentHash := and(add(mul(currentHash, 33), and(pcmVal, 0xFFFF)), 0xFFFFFFFF)
                    pokeUser(cpu, callerAddr, 54796, currentHash)

                    // Pack 16-bit values sequentially starting at offset 0x5040
                    let byteOffset := mul(t, 2)
                    let wordOffset := div(byteOffset, 32)
                    let bitShift := sub(240, mul(mod(byteOffset, 32), 8))
                    
                    let targetAddress := add(0x5040, mul(wordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask := not(shl(bitShift, 0xFFFF))
                    currentWord := and(currentWord, mask)
                    let cleanPcm := and(pcmVal, 0xFFFF)
                    mstore(targetAddress, currentWord)
                }
                // Save Delay Line back to CPU storage registers for SYNC persistence
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    let finalVal := mload(add(0x2000, mul(i, 32)))
                    pokeUser(cpu, callerAddr, add(54800, i), finalVal)
                }
                let totalBytes := mul(numSamples, 2)
                let paddedBytesLen := mul(div(add(totalBytes, 31), 32), 32)
                return(0x5000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: parseTextToPhonemes(string memory text) -> bytes32[]
            // Selector: 0x9a3c6cfe
            // ----------------------------------------------------------------
            if eq(selector, 0x9a3c6cfe) {
                let offset := calldataload(4)
                let len := calldataload(add(4, offset))
                
                let arrayLength := 0
                let memStart := 0x6040

                let textStart := add(36, offset)
                let i := 0
                for { } lt(i, len) { } {
                    let char1 := byte(0, calldataload(add(textStart, i)))
                    
                    let char2 := 0
                    if lt(add(i, 1), len) {
                        char2 := byte(0, calldataload(add(textStart, add(i, 1))))
                    }

                    let phoneme := 0
                    let advance := 1

                    if and(eq(char1, 115), eq(char2, 104)) {
                        phoneme := 0x7368000000000000000000000000000000000000000000000000000000000000
                        advance := 2
                    }
                    if and(eq(char1, 101), eq(char2, 101)) {
                        phoneme := 0x6565000000000000000000000000000000000000000000000000000000000000
                        advance := 2
                    }
                    if and(eq(char1, 111), eq(char2, 111)) {
                        phoneme := 0x6f6f000000000000000000000000000000000000000000000000000000000000
                        advance := 2
                    }
                    if and(eq(char1, 97), eq(char2, 97)) {
                        phoneme := 0x6161000000000000000000000000000000000000000000000000000000000000
                        advance := 2
                    }

                    if iszero(phoneme) {
                        if eq(char1, 97) {
                            phoneme := 0x6161000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 101) {
                            phoneme := 0x6565000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 111) {
                            phoneme := 0x6f6f000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 115) {
                            phoneme := 0x7300000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 102) {
                            phoneme := 0x6600000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 109) {
                            phoneme := 0x6d00000000000000000000000000000000000000000000000000000000000000
                        }
                        if eq(char1, 110) {
                            phoneme := 0x6e00000000000000000000000000000000000000000000000000000000000000
                        }
                    }

                    if iszero(iszero(phoneme)) {
                        mstore(add(memStart, mul(arrayLength, 32)), phoneme)
                        arrayLength := add(arrayLength, 1)
                    }

                    i := add(i, advance)
                }

                mstore(0x6000, 0x20)
                mstore(0x6020, arrayLength)
                
                return(0x6000, add(64, mul(arrayLength, 32)))
            }

            // ----------------------------------------------------------------
            // Method: synthesizeWav(bytes32 phonemeKey, uint256 numSamples) -> bytes
            // Selector: 0x02ceea6d
            // ----------------------------------------------------------------
            if eq(selector, 0x02ceea6d) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let synthKey := calldataload(4)
                let numSamples := calldataload(36)
                if gt(numSamples, 1000) { numSamples := 1000 }

                let synthVoiceId := sload(2) // 0 = Ana, 1 = Moloch
                let synthBasePitch := 420
                if eq(synthVoiceId, 1) {
                    synthBasePitch := 150
                }
                
                let synthEnergy := 0
                let synthSoundType := 0 // 0 = silent, 1 = voiced, 2 = unvoiced, 3 = nasal
                let voicingStrength := 0 // 0 = unvoiced, 100 = fully voiced

                // Check phoneme keys by matching prefixes
                let synthTwoChars := and(synthKey, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let synthOneChar := and(synthKey, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 90
                    synthSoundType := 1
                    voicingStrength := 85
                }
                if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 95
                    synthSoundType := 1
                    voicingStrength := 90
                }
                if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 85
                    synthSoundType := 1
                    voicingStrength := 80
                }
                if eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 80
                    synthSoundType := 2
                }
                if eq(synthOneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                    if iszero(eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                        synthEnergy := 70
                        synthSoundType := 2
                    }
                }
                if eq(synthOneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 40
                    synthSoundType := 2
                }
                if eq(synthOneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 50
                    synthSoundType := 3
                    voicingStrength := 65
                }
                if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 45
                    synthSoundType := 3
                    voicingStrength := 60
                }

                // Project K1..K9 coefficients into memory slots 0x4000 to 0x4008
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    let K := 10
                    if iszero(synthVoiceId) {
                        if eq(i, 0) { K := 15 }
                        if eq(i, 1) { K := sub(0, 25) }
                        if eq(i, 2) { K := 40 }
                        if eq(i, 3) { K := sub(0, 10) }
                        if eq(i, 4) { K := 5 }
                        if eq(i, 5) { K := sub(0, 8) }
                        if eq(i, 6) { K := 12 }
                        if eq(i, 7) { K := sub(0, 5) }
                        if eq(i, 8) { K := 2 }
                    }
                    if eq(synthVoiceId, 1) {
                        if eq(i, 0) { K := 85 }
                        if eq(i, 1) { K := sub(0, 65) }
                        if eq(i, 2) { K := 70 }
                        if eq(i, 3) { K := sub(0, 45) }
                        if eq(i, 4) { K := 35 }
                        if eq(i, 5) { K := sub(0, 22) }
                        if eq(i, 6) { K := 18 }
                        if eq(i, 7) { K := sub(0, 10) }
                        if eq(i, 8) { K := 5 }
                    }

                    if eq(synthSoundType, 1) {
                        if lt(i, 3) { K := add(K, 15) }
                    }
                    if eq(synthSoundType, 2) {
                        if lt(i, 4) { K := sub(K, 40) }
                    }

                    if sgt(K, 99) { K := 99 }
                    if slt(K, sub(0, 99)) { K := sub(0, 99) }

                    mstore(add(0x4000, mul(i, 32)), K)
                }

                // Restore Delay Line from CPU storage registers (54800 - 54809) for SYNC persistence
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    let savedVal := peekUser(cpu, callerAddr, add(54800, i))
                    mstore(add(0x2000, mul(i, 32)), savedVal)
                }

                // Initialize smoothing buffer at 0x4100 with target coefficients
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    mstore(add(0x4100, mul(i, 32)), mload(add(0x4000, mul(i, 32))))
                }

                // Setup ABI bytes headers at 0x5000
                mstore(0x5000, 0x20)
                let totalBytes := add(44, mul(numSamples, 2))
                mstore(0x5020, totalBytes)

                // Assemble WAV Header Word 0 (Bytes 0-31)
                let chunkSizeVal := add(36, mul(numSamples, 2))
                let chunkSizeLE := or(shl(24, and(chunkSizeVal, 0xFF)), or(shl(16, and(shr(8, chunkSizeVal), 0xFF)), or(shl(8, and(shr(16, chunkSizeVal), 0xFF)), and(shr(24, chunkSizeVal), 0xFF))))

                let word0 := 0x524946460000000057415645666d74201000000001000100803e0000007d0000
                word0 := or(and(word0, not(shl(192, 0xFFFFFFFF))), shl(192, chunkSizeLE))
                mstore(0x5040, word0)

                // Assemble WAV Header Word 1 (Bytes 32-43)
                let subchunk2Val := mul(numSamples, 2)
                let subchunk2LE := or(shl(24, and(subchunk2Val, 0xFF)), or(shl(16, and(shr(8, subchunk2Val), 0xFF)), or(shl(8, and(shr(16, subchunk2Val), 0xFF)), and(shr(24, subchunk2Val), 0xFF))))

                let word1 := 0x0200100064617461000000000000000000000000000000000000000000000000
                word1 := or(and(word1, not(shl(160, 0xFFFFFFFF))), shl(160, subchunk2LE))
                mstore(0x5060, word1)

                let seed := sload(101)
                if iszero(seed) { seed := 12345 }

                // Synthesis Loop
                for { let t := 0 } lt(t, numSamples) { t := add(t, 1) } {
                    // Smoothly interpolate coefficients
                    for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                        let cur := mload(add(0x4100, mul(i, 32)))
                        let target := mload(add(0x4000, mul(i, 32)))
                        let diff := sub(target, cur)
                        cur := add(cur, sdiv(diff, 256))
                        mstore(add(0x4100, mul(i, 32)), cur)
                    }

                    let excitation := 0

                    if eq(synthSoundType, 1) {
                        // LFSR step for continuous breathy noise
                        let bit := and(seed, 1)
                        seed := shr(1, seed)
                        if bit {
                            seed := xor(seed, 0xB400)
                        }
                        let noise := sub(mod(seed, 200), 100) // [-100, 100]

                        // Pitch contour + Jitter (using seed)
                        let baseContourPitch := synthBasePitch
                        let jitterPercent := sub(mod(seed, 17), 8) // [-8, 8] per-mil
                        let jitteredPitch := add(baseContourPitch, sdiv(mul(baseContourPitch, jitterPercent), 1000))
                        
                        // 5.2Hz triangle vibrato LFO (phase step = 128 samples)
                        let vibratoPhase := mod(div(t, 128), 24)
                        let vibratoOffset := 0
                        if lt(vibratoPhase, 12) {
                            vibratoOffset := sub(vibratoPhase, 6)
                        }
                        if iszero(lt(vibratoPhase, 12)) {
                            vibratoOffset := sub(18, vibratoPhase)
                        }
                        let finalPitch := add(jitteredPitch, vibratoOffset)

                        let period := div(16000, finalPitch)
                        if iszero(period) { period := 72 }
                        let tMod := mod(t, period)
                        
                        let Tp := div(mul(period, 40), 100)
                        let Tn := div(mul(period, 16), 100)
                        let pulse := 0
                        
                        if lt(tMod, Tp) {
                            let ph := div(mul(tMod, 100), Tp)
                            let ph2 := div(mul(ph, ph), 100)
                            let ph3 := div(mul(ph2, ph), 100)
                            pulse := sub(mul(3, ph2), mul(2, ph3))
                        }
                        if and(iszero(lt(tMod, Tp)), lt(tMod, add(Tp, Tn))) {
                            let ph := div(mul(sub(tMod, Tp), 100), Tn)
                            let ph2 := div(mul(ph, ph), 100)
                            pulse := sub(100, ph2)
                        }
                        
                        // Amplitude Shimmer (2% cycles)
                        let shimmerPercent := sub(mod(seed, 5), 2) // [-2, 2]%
                        let shimmerFactor := add(100, shimmerPercent)

                        // Dynamic Mixed Excitation (MELP-style)
                        let pulseScaled := mul(sub(pulse, 29), 10)
                        let noiseScaled := mul(noise, 10)
                        let mixedExcitation := div(add(mul(pulseScaled, voicingStrength), mul(noiseScaled, sub(100, voicingStrength))), 100)
                        excitation := sdiv(mul(mixedExcitation, shimmerFactor), 100)
                    }
                    if eq(synthSoundType, 2) {
                        let bit := and(seed, 1)
                        seed := shr(1, seed)
                        if bit {
                            seed := xor(seed, 0xB400)
                        }
                        excitation := sub(mod(seed, 2000), 1000)
                    }
                    if eq(synthSoundType, 3) {
                        let period := 106
                        let val := mod(t, period)
                        excitation := sub(mul(val, 20), 1000)
                    }

                    let forward := sdiv(mul(excitation, synthEnergy), 100)

                    for { let i := 9 } gt(i, 0) { } {
                        i := sub(i, 1)
                        let K := mload(add(0x4100, mul(i, 32)))
                        let delayVal := mload(add(0x2000, mul(i, 32)))
                        let nextForward := sub(forward, sdiv(mul(K, delayVal), 100))
                        let nextDelay := add(delayVal, sdiv(mul(K, nextForward), 100))
                        mstore(add(0x2000, mul(add(i, 1), 32)), nextDelay)
                        forward := nextForward
                    }
                    mstore(0x2000, forward)

                    // Convert to 16-bit PCM (signed)
                    let pcmVal := mul(forward, 8)
                    if sgt(pcmVal, 32767) { pcmVal := 32767 }
                    if slt(pcmVal, sub(0, 32768)) { pcmVal := sub(0, 32768) }

                    // Pack 16-bit values sequentially starting at offset 0x5040 (offset 44 bytes in payload)
                    let pcmValLE := or(shl(8, and(pcmVal, 0xFF)), and(shr(8, pcmVal), 0xFF))

                    let byteOffset := add(44, mul(t, 2))
                    let wordOffset := div(byteOffset, 32)
                    let bitShift := sub(240, mul(mod(byteOffset, 32), 8))
                    
                    let targetAddress := add(0x5040, mul(wordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask := not(shl(bitShift, 0xFFFF))
                    currentWord := and(currentWord, mask)
                    mstore(targetAddress, currentWord)
                }
                // Save Delay Line back to CPU storage registers for SYNC persistence
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    let finalVal := mload(add(0x2000, mul(i, 32)))
                    pokeUser(cpu, callerAddr, add(54800, i), finalVal)
                }

                let paddedBytesLen := mul(div(add(totalBytes, 31), 32), 32)
                return(0x5000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: predictMel(bytes32[] phonemes) -> bytes
            // Selector: 0x1153f2d3
            // ----------------------------------------------------------------
            if eq(selector, 0x1153f2d3) {
                let arrayOffset := calldataload(4)
                let numPhonemes := calldataload(add(4, arrayOffset))
                
                mstore(0x6000, 0x20)
                let totalOutputBytes := mul(numPhonemes, 8)
                mstore(0x6020, totalOutputBytes)

                for { let pIdx := 0 } lt(pIdx, numPhonemes) { pIdx := add(pIdx, 1) } {
                    let key := calldataload(add(add(36, arrayOffset), mul(pIdx, 32)))
                    
                    for { let j := 0 } lt(j, 8) { j := add(j, 1) } { mstore(add(0x3000, mul(j, 32)), 0) }

                    let synthTwoChars := and(key, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                    let synthOneChar := and(key, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                    if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 90) mstore(0x3020, 10) mstore(0x3040, sub(0, 50)) mstore(0x3060, 80)
                        mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }
                    if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 95) mstore(0x3020, 20) mstore(0x3040, sub(0, 30)) mstore(0x3060, 90)
                        mstore(0x3080, sub(0, 10)) mstore(0x30a0, 15) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 85) mstore(0x3020, 5) mstore(0x3040, sub(0, 60)) mstore(0x3060, 70)
                        mstore(0x3080, sub(0, 30)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 15)) mstore(0x30e0, 0)
                    }
                    if eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 10) mstore(0x3020, sub(0, 80)) mstore(0x3040, 80) mstore(0x3060, sub(0, 20))
                        mstore(0x3080, 90) mstore(0x30a0, sub(0, 90)) mstore(0x30c0, 70) mstore(0x30e0, sub(0, 50))
                    }
                    if eq(synthOneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                        if iszero(eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                            mstore(0x3000, 5) mstore(0x3020, sub(0, 90)) mstore(0x3040, 90) mstore(0x3060, sub(0, 30))
                            mstore(0x3080, 95) mstore(0x30a0, sub(0, 95)) mstore(0x30c0, 80) mstore(0x30e0, sub(0, 60))
                        }
                    }
                    if eq(synthOneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 5) mstore(0x3020, sub(0, 50)) mstore(0x3040, 40) mstore(0x3060, sub(0, 10))
                        mstore(0x3080, 50) mstore(0x30a0, sub(0, 40)) mstore(0x30c0, 30) mstore(0x30e0, sub(0, 20))
                    }
                    if eq(synthOneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 40) mstore(0x3020, 30) mstore(0x3040, sub(0, 10)) mstore(0x3060, 20)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 35) mstore(0x3020, 25) mstore(0x3040, sub(0, 15)) mstore(0x3060, 15)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }

                    for { let r := 0 } lt(r, 8) { r := add(r, 1) } {
                        let dot := 0
                        let bias := 0
                        if eq(r, 0) { bias := 10 }
                        if eq(r, 1) { bias := sub(0, 5) }
                        if eq(r, 2) { bias := 15 }
                        if eq(r, 3) { bias := 0 }
                        if eq(r, 4) { bias := 5 }
                        if eq(r, 5) { bias := sub(0, 10) }
                        if eq(r, 6) { bias := 20 }
                        if eq(r, 7) { bias := sub(0, 15) }
                        dot := mul(bias, 100)

                        for { let c := 0 } lt(c, 8) { c := add(c, 1) } {
                            let weight := 0
                            if eq(r, 0) {
                                if eq(c, 0) { weight := 50 }
                                if eq(c, 1) { weight := sub(0, 20) }
                                if eq(c, 2) { weight := 10 }
                                if eq(c, 3) { weight := 0 }
                                if eq(c, 4) { weight := 5 }
                                if eq(c, 5) { weight := sub(0, 5) }
                                if eq(c, 6) { weight := 10 }
                                if eq(c, 7) { weight := sub(0, 10) }
                            }
                            if eq(r, 1) {
                                if eq(c, 0) { weight := sub(0, 10) }
                                if eq(c, 1) { weight := 60 }
                                if eq(c, 2) { weight := sub(0, 20) }
                                if eq(c, 3) { weight := 15 }
                                if eq(c, 4) { weight := 0 }
                                if eq(c, 5) { weight := 10 }
                                if eq(c, 6) { weight := sub(0, 5) }
                                if eq(c, 7) { weight := 5 }
                            }
                            if eq(r, 2) {
                                if eq(c, 0) { weight := 20 }
                                if eq(c, 1) { weight := sub(0, 10) }
                                if eq(c, 2) { weight := 70 }
                                if eq(c, 3) { weight := sub(0, 30) }
                                if eq(c, 4) { weight := 10 }
                                if eq(c, 5) { weight := 5 }
                                if eq(c, 6) { weight := sub(0, 10) }
                                if eq(c, 7) { weight := 15 }
                            }
                            if eq(r, 3) {
                                if eq(c, 0) { weight := sub(0, 5) }
                                if eq(c, 1) { weight := 15 }
                                if eq(c, 2) { weight := sub(0, 15) }
                                if eq(c, 3) { weight := 80 }
                                if eq(c, 4) { weight := sub(0, 20) }
                                if eq(c, 5) { weight := 10 }
                                if eq(c, 6) { weight := 0 }
                                if eq(c, 7) { weight := 5 }
                            }
                            if eq(r, 4) {
                                if eq(c, 0) { weight := 10 }
                                if eq(c, 1) { weight := 0 }
                                if eq(c, 2) { weight := 15 }
                                if eq(c, 3) { weight := sub(0, 10) }
                                if eq(c, 4) { weight := 65 }
                                if eq(c, 5) { weight := sub(0, 15) }
                                if eq(c, 6) { weight := 20 }
                                if eq(c, 7) { weight := sub(0, 20) }
                            }
                            if eq(r, 5) {
                                if eq(c, 0) { weight := sub(0, 15) }
                                if eq(c, 1) { weight := 10 }
                                if eq(c, 2) { weight := 5 }
                                if eq(c, 3) { weight := 10 }
                                if eq(c, 4) { weight := sub(0, 10) }
                                if eq(c, 5) { weight := 75 }
                                if eq(c, 6) { weight := sub(0, 5) }
                                if eq(c, 7) { weight := 10 }
                            }
                            if eq(r, 6) {
                                if eq(c, 0) { weight := 5 }
                                if eq(c, 1) { weight := sub(0, 5) }
                                if eq(c, 2) { weight := sub(0, 10) }
                                if eq(c, 3) { weight := 5 }
                                if eq(c, 4) { weight := 15 }
                                if eq(c, 5) { weight := sub(0, 10) }
                                if eq(c, 6) { weight := 85 }
                                if eq(c, 7) { weight := sub(0, 25) }
                            }
                            if eq(r, 7) {
                                if eq(c, 0) { weight := sub(0, 10) }
                                if eq(c, 1) { weight := 10 }
                                if eq(c, 2) { weight := 20 }
                                if eq(c, 3) { weight := sub(0, 15) }
                                if eq(c, 4) { weight := sub(0, 20) }
                                if eq(c, 5) { weight := 15 }
                                if eq(c, 6) { weight := sub(0, 15) }
                                if eq(c, 7) { weight := 90 }
                            }

                            let inputVal := mload(add(0x3000, mul(c, 32)))
                            dot := add(dot, mul(weight, inputVal))
                        }

                        let scaledVal := sdiv(dot, 100)
                        if slt(scaledVal, 0) { scaledVal := 0 }
                        if sgt(scaledVal, 255) { scaledVal := 255 }

                        mstore8(add(0x3100, r), scaledVal)
                    }

                    let pcmWord := mload(0x3100)
                    let targetByteOffset := mul(pIdx, 8)
                    let targetWordOffset := div(targetByteOffset, 32)
                    let targetShift := sub(248, mul(mod(targetByteOffset, 32), 8))

                    let targetAddress := add(0x6040, mul(targetWordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask64 := not(shl(targetShift, 0xFFFFFFFFFFFFFFFF))
                    currentWord := and(currentWord, mask64)
                    let cleanBytes := and(shr(224, pcmWord), 0xFFFFFFFFFFFFFFFF)
                    currentWord := or(currentWord, shl(targetShift, cleanBytes))
                    mstore(targetAddress, currentWord)
                }

                let paddedBytesLen := mul(div(add(totalOutputBytes, 31), 32), 32)
                return(0x6000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: synthesizeNeuralWav(bytes melFrames, uint256 upsampleFactor) -> bytes
            // Selector: 0x20c4433b
            // ----------------------------------------------------------------
            if eq(selector, 0x20c4433b) {
                let bytesOffset := calldataload(4)
                let upsampleFactor := calldataload(36)
                if gt(upsampleFactor, 3000) { upsampleFactor := 3000 }

                let name := calldataload(68)
                let baseSlot := 0
                if name {
                    mstore(0x00, name)
                    mstore(0x20, 0x9999)
                    baseSlot := keccak256(0x00, 0x40)
                }

                let melLen := calldataload(add(4, bytesOffset))
                let numFrames := div(melLen, 8)
                if iszero(numFrames) { numFrames := 1 }

                let totalSamples := mul(numFrames, upsampleFactor)

                // Setup ABI bytes headers at 0x5000
                mstore(0x5000, 0x20)
                let totalBytes := add(44, mul(totalSamples, 2))
                mstore(0x5020, totalBytes)

                // Assemble WAV Header Word 0 (Bytes 0-31)
                let chunkSizeVal := add(36, mul(totalSamples, 2))
                let chunkSizeLE := or(shl(24, and(chunkSizeVal, 0xFF)), or(shl(16, and(shr(8, chunkSizeVal), 0xFF)), or(shl(8, and(shr(16, chunkSizeVal), 0xFF)), and(shr(24, chunkSizeVal), 0xFF))))

                let word0 := 0x524946460000000057415645666d74201000000001000100803e0000007d0000
                word0 := or(and(word0, not(shl(192, 0xFFFFFFFF))), shl(192, chunkSizeLE))
                mstore(0x5040, word0)

                // Assemble WAV Header Word 1 (Bytes 32-43)
                let subchunk2Val := mul(totalSamples, 2)
                let subchunk2LE := or(shl(24, and(subchunk2Val, 0xFF)), or(shl(16, and(shr(8, subchunk2Val), 0xFF)), or(shl(8, and(shr(16, subchunk2Val), 0xFF)), and(shr(24, subchunk2Val), 0xFF))))

                let word1 := 0x0200100064617461000000000000000000000000000000000000000000000000
                word1 := or(and(word1, not(shl(160, 0xFFFFFFFF))), shl(160, subchunk2LE))
                mstore(0x5060, word1)

                // Seed for LFSR noise
                let seed := 0xACE1

                // Clear delay line memory (13 slots: 0 to 12)
                for { let i := 0 } lt(i, 13) { i := add(i, 1) } {
                    mstore(add(0x2000, mul(i, 32)), 0)
                }

                // Phase accumulator for glottal pulse
                let phase := 0

                // Keep track of current parameters for smoothing
                let currentPitch := 420
                let currentEnergy := 0
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    mstore(add(0x4300, mul(i, 32)), 0)
                }
                mstore(0x4200, 0)

                for { let f := 0 } lt(f, numFrames) { f := add(f, 1) } {
                    mstore(0x4500, currentPitch)
                    mstore(0x4520, currentEnergy)
                    for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                        mstore(add(0x4400, mul(i, 32)), mload(add(0x4300, mul(i, 32))))
                    }
                    let wordIdx := div(mul(f, 8), 32)
                    let byteOffsetInWord := mod(mul(f, 8), 32)
                    let calldataWord := calldataload(add(add(36, bytesOffset), mul(wordIdx, 32)))
                    let frameBytes := shr(sub(192, mul(byteOffsetInWord, 8)), calldataWord)

                    // Extract frame energy and reflection coefficients
                    let m0 := and(shr(56, frameBytes), 0xFF)
                    let m1 := and(shr(48, frameBytes), 0xFF)
                    let m2 := and(shr(40, frameBytes), 0xFF)
                    let m3 := and(shr(32, frameBytes), 0xFF)
                    let m4 := and(shr(24, frameBytes), 0xFF)
                    let m5 := and(shr(16, frameBytes), 0xFF)
                    let m6 := and(shr(8, frameBytes), 0xFF)
                    let m7 := and(frameBytes, 0xFF)

                    // Set target energy
                    let targetEnergy := m0

                    // Target reflection coefficients mapped from [0, 255] to [-95, 95]
                    let k0 := sdiv(mul(sub(m1, 128), 95), 128)
                    let k1 := sdiv(mul(sub(m2, 128), 95), 128)
                    let k2 := sdiv(mul(sub(m3, 128), 95), 128)
                    let k3 := sdiv(mul(sub(m4, 128), 95), 128)
                    let k4 := sdiv(mul(sub(m5, 128), 95), 128)
                    let k5 := sdiv(mul(sub(m6, 128), 95), 128)

                    mstore(0x4000, k0)
                    mstore(0x4020, k1)
                    mstore(0x4040, k2)
                    mstore(0x4060, k3)
                    mstore(0x4080, k4)
                    mstore(0x40a0, k5)
                    mstore(0x40c0, sdiv(k5, 2))       // K6 (formant decay)
                    mstore(0x40e0, sdiv(k5, 4))       // K7
                    mstore(0x4100, sdiv(k5, 8))       // K8
                    mstore(0x4120, sdiv(k5, 16))      // K9
 
                    // Determine target pitch: extract from m7 (8th byte of Mel frame)
                    let targetPitch := m7
                    if iszero(targetPitch) { targetPitch := 420 }

                    // Apply Autotune Pitch Correction if on-chain pitch contour exists
                    if baseSlot {
                        let autotunedPitch := sload(add(baseSlot, add(8, mod(f, 24))))
                        if autotunedPitch {
                            targetPitch := autotunedPitch
                        }
                    }

                    for { let u := 0 } lt(u, upsampleFactor) { u := add(u, 1) } {
                        // Calculate sample index for Bessel timeline modulation
                        let sampleIdx := add(mul(f, upsampleFactor), u)
                        
                        // Modulate target pitch with Bessel J0 (efferent topicality cadence)
                        // x parameter maps from 0 to 750 (representing 0.0 to 0.75 rad) over synthesis duration
                        let x_param := div(mul(sampleIdx, 750), totalSamples)
                        let j0 := besselJ0Scaled(x_param)
                        // Pitch factor = (850 + J0 * 15 / 10) / 1000 [maps 85% to 115%]
                        let pitchFactor := add(850, div(mul(j0, 15), 10))
                        let activeTargetPitch := div(mul(targetPitch, pitchFactor), 1000)

                        // Step 1: Parameter transition interpolation
                        mstore(0x4540, div(mul(u, 128), upsampleFactor)) // frac
                        
                        currentPitch := div(add(mul(mload(0x4500), sub(128, mload(0x4540))), mul(activeTargetPitch, mload(0x4540))), 128)
                        currentEnergy := div(add(mul(mload(0x4520), sub(128, mload(0x4540))), mul(m0, mload(0x4540))), 128)
                        for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                            let prevK := mload(add(0x4400, mul(i, 32)))
                            let targetK := mload(add(0x4000, mul(i, 32)))
                            let curK := div(add(mul(prevK, sub(128, mload(0x4540))), mul(targetK, mload(0x4540))), 128)
                            mstore(add(0x4300, mul(i, 32)), curK)
                        }

                        let excitation := 0
                        // Voiced vs Unvoiced based on target energy (m0) threshold
                        if gt(m0, 30) {
                            phase := add(phase, 1)

                            // Step 2: LFSR Noise step
                            let bit := and(seed, 1)
                            seed := shr(1, seed)
                            if bit { seed := xor(seed, 0xB400) }
                            let noise := sub(mod(seed, 200), 100) // [-100, 100]

                            // Step 3: Pitch Jitter (FM) using seed state
                            // Modulate currentPitch by [-1%, +1%]
                            let jitterPercent := sub(mod(seed, 21), 10) // [-10, 10]
                            let jitteredPitch := add(currentPitch, sdiv(mul(currentPitch, jitterPercent), 1000))
                            let period := sdiv(16000, jitteredPitch)
                            if iszero(period) { period := 72 }

                            // Step 4: Amplitude Shimmer (AM)
                            // Modulate amplitude by [-8%, +8%]
                            let shimmerPercent := sub(mod(seed, 17), 8) // [-8, 8]
                            let shimmerFactor := add(100, shimmerPercent) // [92, 108]%

                            let tMod := mod(phase, period)
                            mstore(0x45a0, div(mul(period, 40), 100)) // Tp
                            mstore(0x45c0, div(mul(period, 16), 100)) // Tn
                            mstore(0x45e0, 0) // pulse
                            
                            // Liljencrants-Fant (LF) glottal pulse rise & decay approximation
                            if lt(tMod, mload(0x45a0)) {
                                let ph := div(mul(tMod, 128), mload(0x45a0))
                                let rise := add(ph, div(mul(ph, ph), 256))
                                let ph_sq := div(mul(ph, ph), 128)
                                let ph_cube := div(mul(ph_sq, ph), 128)
                                let sin_approx := sub(mul(3, ph_sq), mul(2, ph_cube))
                                mstore(0x45e0, div(mul(rise, sin_approx), 128))
                            }
                            if and(iszero(lt(tMod, mload(0x45a0))), lt(tMod, add(mload(0x45a0), mload(0x45c0)))) {
                                let ph := div(mul(sub(tMod, mload(0x45a0)), 128), mload(0x45c0))
                                let decay := sub(128, ph)
                                if slt(decay, 0) { decay := 0 }
                                mstore(0x45e0, div(mul(100, decay), 128))
                            }
                            
                            // Apply shimmer factor to pulse
                            mstore(0x45e0, div(mul(mload(0x45e0), shimmerFactor), 100))

                            // Step 5: Multi-Band Mixed Excitation (MBE) Filter
                            mstore(0x4600, 50) // voicingStrength
                            if gt(m0, 35) {
                                mstore(0x4600, add(50, div(mul(sub(m0, 35), 40), 45)))
                            }
                            if gt(mload(0x4600), 90) { mstore(0x4600, 90) }

                            let lastPulse := mload(0x4220)
                            let pulseLP := sdiv(add(mload(0x45e0), lastPulse), 2)
                            mstore(0x4220, mload(0x45e0))

                            let lastNoise := mload(0x4240)
                            let noiseHP := sdiv(sub(noise, lastNoise), 2)
                            mstore(0x4240, noise)

                            let pulseScaled := sdiv(mul(pulseLP, mload(0x4600)), 10)
                            let noiseScaled := sdiv(mul(noiseHP, sub(100, mload(0x4600))), 10)
                            excitation := add(pulseScaled, noiseScaled)
                        }
                        if iszero(gt(m0, 30)) {
                            let bit := and(seed, 1)
                            seed := shr(1, seed)
                            if bit { seed := xor(seed, 0xB400) }
                            excitation := sub(mod(seed, 2000), 1000)
                        }

                        // Apply Bessel K0 amplitude decay at the release phase
                        let ampScale := 1000
                        let decayThreshold := div(mul(totalSamples, 85), 100)
                        if gt(sampleIdx, decayThreshold) {
                            // Map remaining 15% of samples to 0..1500 for K0 decay
                            let decayIdx := div(mul(sub(sampleIdx, decayThreshold), 1500), sub(totalSamples, decayThreshold))
                            ampScale := besselK0Scaled(decayIdx)
                        }
                        excitation := div(mul(excitation, ampScale), 1000)

                        // Filter forward path
                        let forward := sdiv(mul(excitation, currentEnergy), 100)

                        for { let i := 10 } gt(i, 0) { } {
                            i := sub(i, 1)
                            let K := mload(add(0x4300, mul(i, 32)))
                            
                            // On-chain Post-filter Spectral Sharpening: Scale K by gamma^i (gamma = 0.88)
                            let gamma := 128
                            if eq(i, 1) { gamma := 112 }
                            if eq(i, 2) { gamma := 98 }
                            if eq(i, 3) { gamma := 86 }
                            if eq(i, 4) { gamma := 75 }
                            if eq(i, 5) { gamma := 66 }
                            if eq(i, 6) { gamma := 58 }
                            if eq(i, 7) { gamma := 51 }
                            if eq(i, 8) { gamma := 44 }
                            if eq(i, 9) { gamma := 39 }
                            let K_sharp := sdiv(mul(K, gamma), 128)
                            if sgt(K_sharp, 120) { K_sharp := 120 }
                            if slt(K_sharp, sub(0, 120)) { K_sharp := sub(0, 120) }
                            
                            let delayVal := mload(add(0x2000, mul(i, 32)))
                            let nextForward := sub(forward, sdiv(mul(K_sharp, delayVal), 128))
                            let nextDelay := add(delayVal, sdiv(mul(K_sharp, nextForward), 128))
                            mstore(add(0x2000, mul(add(i, 1), 32)), nextDelay)
                            forward := nextForward
                        }
                        mstore(0x2000, forward)

                        // Scale to 16-bit PCM range
                        let sampleVal := mul(forward, 3)

                        // Step 5: Adaptive Spectral Tilt Filter (1st-order Low Pass)
                        let lastOut := mload(0x4200)
                        let coeff1 := 70
                        let coeff2 := 30
                        if iszero(gt(targetEnergy, 30)) {
                            coeff1 := 95
                            coeff2 := 5
                        }
                        let smoothed := add(sdiv(mul(sampleVal, coeff1), 100), sdiv(mul(lastOut, coeff2), 100))
                        mstore(0x4200, smoothed)
                        sampleVal := smoothed

                        if sgt(sampleVal, 32767) { sampleVal := 32767 }
                        if slt(sampleVal, sub(0, 32768)) { sampleVal := sub(0, 32768) }

                        sampleIdx := add(mul(f, upsampleFactor), u)
                        let pcmValLE := or(shl(8, and(sampleVal, 0xFF)), and(shr(8, sampleVal), 0xFF))

                        let byteOffset := add(44, mul(sampleIdx, 2))
                        let wordOffset := div(byteOffset, 32)
                        let bitShift := sub(240, mul(mod(byteOffset, 32), 8))
                        
                        let targetAddress := add(0x5040, mul(wordOffset, 32))
                        let currentWord := mload(targetAddress)
                        let mask := not(shl(bitShift, 0xFFFF))
                        currentWord := and(currentWord, mask)
                        currentWord := or(currentWord, shl(bitShift, pcmValLE))
                        mstore(targetAddress, currentWord)
                    }
                }

                let paddedBytesLen := mul(div(add(totalBytes, 31), 32), 32)
                return(0x5000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: registerSpeaker(bytes32 name, bytes32[8] embedding)
            // Selector: 0x5f60cdf4
            // ----------------------------------------------------------------
            if eq(selector, 0x5f60cdf4) {
                let name := calldataload(4)
                
                mstore(0x00, name)
                mstore(0x20, 0x9999)
                let baseSlot := keccak256(0x00, 0x40)

                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    let val := calldataload(add(36, mul(i, 32)))
                    sstore(add(baseSlot, i), val)
                }
                
                // Load optional dynamic pitch contour for autotuning if passed
                if gt(calldatasize(), 292) {
                    let pitchCount := div(sub(calldatasize(), 292), 32)
                    if gt(pitchCount, 24) { pitchCount := 24 }
                    for { let i := 0 } lt(i, pitchCount) { i := add(i, 1) } {
                        let pitchVal := calldataload(add(292, mul(i, 32)))
                        sstore(add(baseSlot, add(8, i)), pitchVal)
                    }
                }
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // Method: predictMelSpeaker(bytes32[] phonemes, bytes32 name) -> bytes
            // Selector: 0xe0b7f154
            // ----------------------------------------------------------------
            if eq(selector, 0xe0b7f154) {
                let arrayOffset := calldataload(4)
                let name := calldataload(36)
                let numPhonemes := calldataload(add(4, arrayOffset))

                mstore(0x6000, 0x20)
                let totalOutputBytes := mul(numPhonemes, 8)
                mstore(0x6020, totalOutputBytes)

                mstore(0x00, name)
                mstore(0x20, 0x9999)
                let baseSlot := keccak256(0x00, 0x40)

                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    mstore(add(0x4000, mul(i, 32)), sload(add(baseSlot, i)))
                }

                for { let pIdx := 0 } lt(pIdx, numPhonemes) { pIdx := add(pIdx, 1) } {
                    let key := calldataload(add(add(36, arrayOffset), mul(pIdx, 32)))
                    
                    for { let j := 0 } lt(j, 8) { j := add(j, 1) } { mstore(add(0x3000, mul(j, 32)), 0) }

                    let synthTwoChars := and(key, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                    let synthOneChar := and(key, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                    if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 90) mstore(0x3020, 10) mstore(0x3040, sub(0, 50)) mstore(0x3060, 80)
                        mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }
                    if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 95) mstore(0x3020, 20) mstore(0x3040, sub(0, 30)) mstore(0x3060, 90)
                        mstore(0x3080, sub(0, 10)) mstore(0x30a0, 15) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 85) mstore(0x3020, 5) mstore(0x3040, sub(0, 60)) mstore(0x3060, 70)
                        mstore(0x3080, sub(0, 30)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 15)) mstore(0x30e0, 0)
                    }
                    if eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 10) mstore(0x3020, sub(0, 80)) mstore(0x3040, 80) mstore(0x3060, sub(0, 20))
                        mstore(0x3080, 90) mstore(0x30a0, sub(0, 90)) mstore(0x30c0, 70) mstore(0x30e0, sub(0, 50))
                    }
                    if eq(synthOneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                        if iszero(eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                            mstore(0x3000, 5) mstore(0x3020, sub(0, 90)) mstore(0x3040, 90) mstore(0x3060, sub(0, 30))
                            mstore(0x3080, 95) mstore(0x30a0, sub(0, 95)) mstore(0x30c0, 80) mstore(0x30e0, sub(0, 60))
                        }
                    }
                    if eq(synthOneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 5) mstore(0x3020, sub(0, 50)) mstore(0x3040, 40) mstore(0x3060, sub(0, 10))
                        mstore(0x3080, 50) mstore(0x30a0, sub(0, 40)) mstore(0x30c0, 30) mstore(0x30e0, sub(0, 20))
                    }
                    if eq(synthOneChar, 0x6800000000000000000000000000000000000000000000000000000000000000) { // "h"
                        mstore(0x3000, 15) mstore(0x3020, sub(0, 40)) mstore(0x3040, 30) mstore(0x3060, sub(0, 5))
                        mstore(0x3080, 40) mstore(0x30a0, sub(0, 30)) mstore(0x30c0, 20) mstore(0x30e0, sub(0, 10))
                    }
                    if eq(synthOneChar, 0x6c00000000000000000000000000000000000000000000000000000000000000) { // "l"
                        mstore(0x3000, 70) mstore(0x3020, 15) mstore(0x3040, sub(0, 40)) mstore(0x3060, 60)
                        mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 5)
                    }
                    if eq(synthOneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 40) mstore(0x3020, 30) mstore(0x3040, sub(0, 10)) mstore(0x3060, 20)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 35) mstore(0x3020, 25) mstore(0x3040, sub(0, 15)) mstore(0x3060, 15)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }

                    for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                        let phVal := mload(add(0x3000, mul(j, 32)))
                        let spkVal := mload(add(0x4000, mul(j, 32)))
                        mstore(add(0x3000, mul(j, 32)), add(phVal, spkVal))
                    }

                    for { let r := 0 } lt(r, 8) { r := add(r, 1) } {
                        let dot := 0
                        let bias := 0
                        if eq(r, 0) { bias := 10 }
                        if eq(r, 1) { bias := sub(0, 5) }
                        if eq(r, 2) { bias := 15 }
                        if eq(r, 3) { bias := 0 }
                        if eq(r, 4) { bias := 5 }
                        if eq(r, 5) { bias := sub(0, 10) }
                        if eq(r, 6) { bias := 20 }
                        if eq(r, 7) { bias := sub(0, 15) }
                        dot := mul(bias, 100)

                        for { let c := 0 } lt(c, 8) { c := add(c, 1) } {
                            let weight := 0
                            if eq(r, 0) {
                                if eq(c, 0) { weight := 50 }
                                if eq(c, 1) { weight := sub(0, 20) }
                                if eq(c, 2) { weight := 10 }
                                if eq(c, 3) { weight := 0 }
                                if eq(c, 4) { weight := 5 }
                                if eq(c, 5) { weight := sub(0, 5) }
                                if eq(c, 6) { weight := 10 }
                                if eq(c, 7) { weight := sub(0, 10) }
                            }
                            if eq(r, 1) {
                                if eq(c, 0) { weight := sub(0, 10) }
                                if eq(c, 1) { weight := 60 }
                                if eq(c, 2) { weight := sub(0, 20) }
                                if eq(c, 3) { weight := 15 }
                                if eq(c, 4) { weight := 0 }
                                if eq(c, 5) { weight := 10 }
                                if eq(c, 6) { weight := sub(0, 5) }
                                if eq(c, 7) { weight := 5 }
                            }
                            if eq(r, 2) {
                                if eq(c, 0) { weight := 20 }
                                if eq(c, 1) { weight := sub(0, 10) }
                                if eq(c, 2) { weight := 70 }
                                if eq(c, 3) { weight := sub(0, 30) }
                                if eq(c, 4) { weight := 10 }
                                if eq(c, 5) { weight := 5 }
                                if eq(c, 6) { weight := sub(0, 10) }
                                if eq(c, 7) { weight := 15 }
                            }
                            if eq(r, 3) {
                                if eq(c, 0) { weight := sub(0, 5) }
                                if eq(c, 1) { weight := 15 }
                                if eq(c, 2) { weight := sub(0, 15) }
                                if eq(c, 3) { weight := 80 }
                                if eq(c, 4) { weight := sub(0, 20) }
                                if eq(c, 5) { weight := 10 }
                                if eq(c, 6) { weight := 0 }
                                if eq(c, 7) { weight := 5 }
                            }
                            if eq(r, 4) {
                                if eq(c, 0) { weight := 10 }
                                if eq(c, 1) { weight := 0 }
                                if eq(c, 2) { weight := 15 }
                                if eq(c, 3) { weight := sub(0, 10) }
                                if eq(c, 4) { weight := 65 }
                                if eq(c, 5) { weight := sub(0, 15) }
                                if eq(c, 6) { weight := 20 }
                                if eq(c, 7) { weight := sub(0, 20) }
                            }
                            if eq(r, 5) {
                                if eq(c, 0) { weight := sub(0, 15) }
                                if eq(c, 1) { weight := 10 }
                                if eq(c, 2) { weight := 5 }
                                if eq(c, 3) { weight := 10 }
                                if eq(c, 4) { weight := sub(0, 10) }
                                if eq(c, 5) { weight := 75 }
                                if eq(c, 6) { weight := sub(0, 5) }
                                if eq(c, 7) { weight := 10 }
                            }
                            if eq(r, 6) {
                                if eq(c, 0) { weight := 5 }
                                if eq(c, 1) { weight := sub(0, 5) }
                                if eq(c, 2) { weight := sub(0, 10) }
                                if eq(c, 3) { weight := 5 }
                                if eq(c, 4) { weight := 15 }
                                if eq(c, 5) { weight := sub(0, 10) }
                                if eq(c, 6) { weight := 85 }
                                if eq(c, 7) { weight := sub(0, 25) }
                            }
                            if eq(r, 7) {
                                if eq(c, 0) { weight := sub(0, 10) }
                                if eq(c, 1) { weight := 10 }
                                if eq(c, 2) { weight := 20 }
                                if eq(c, 3) { weight := sub(0, 15) }
                                if eq(c, 4) { weight := sub(0, 20) }
                                if eq(c, 5) { weight := 15 }
                                if eq(c, 6) { weight := sub(0, 15) }
                                if eq(c, 7) { weight := 90 }
                            }

                            let inputVal := mload(add(0x3000, mul(c, 32)))
                            dot := add(dot, mul(weight, inputVal))
                        }

                        let scaledVal := add(sdiv(dot, 100), 128)
                        if slt(scaledVal, 0) { scaledVal := 0 }
                        if sgt(scaledVal, 255) { scaledVal := 255 }

                        mstore8(add(0x3100, r), scaledVal)
                    }

                    let pcmWord := mload(0x3100)
                    let targetByteOffset := mul(pIdx, 8)
                    let targetWordOffset := div(targetByteOffset, 32)
                    let targetShift := sub(248, mul(mod(targetByteOffset, 32), 8))

                    let targetAddress := add(0x6040, mul(targetWordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask64 := not(shl(targetShift, 0xFFFFFFFFFFFFFFFF))
                    currentWord := and(currentWord, mask64)
                    let cleanBytes := and(shr(224, pcmWord), 0xFFFFFFFFFFFFFFFF)
                    currentWord := or(currentWord, shl(targetShift, cleanBytes))
                    mstore(targetAddress, currentWord)
                }

                let paddedBytesLen := mul(div(add(totalOutputBytes, 31), 32), 32)
                return(0x6000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: setQuantizedWeights(uint256 slotOffset, uint256 packedWeights)
            // Selector: 0x200c2ae0
            // ----------------------------------------------------------------
            if eq(selector, 0x200c2ae0) {
                let slotOffset := calldataload(4)
                let packedWeights := calldataload(36)
                
                sstore(add(0x7777, slotOffset), packedWeights)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // Method: predictMelQuantized(bytes32[] phonemes, bytes32 name) -> bytes
            // Selector: 0x18c1ab9a
            // ----------------------------------------------------------------
            if eq(selector, 0x18c1ab9a) {
                let arrayOffset := calldataload(4)
                let name := calldataload(36)
                let numPhonemes := calldataload(add(4, arrayOffset))

                mstore(0x6000, 0x20)
                let totalOutputBytes := mul(numPhonemes, 8)
                mstore(0x6020, totalOutputBytes)

                mstore(0x00, name)
                mstore(0x20, 0x9999)
                let baseSlot := keccak256(0x00, 0x40)

                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    mstore(add(0x4000, mul(i, 32)), sload(add(baseSlot, i)))
                }

                let weights0 := sload(0x7777)
                let weights1 := sload(0x7778)
                mstore(0x4800, weights0)
                mstore(0x4820, weights1)

                // --- PITCH CONTOUR GENERATION ENGINE ---
                let startIndex := 0
                for { let i := 0 } lt(i, add(numPhonemes, 1)) { i := add(i, 1) } {
                    let isEnd := eq(i, numPhonemes)
                    let isBoundary := isEnd
                    if iszero(isEnd) {
                        let curKey := calldataload(add(add(36, arrayOffset), mul(i, 32)))
                        let curPrefix := and(curKey, 0xFFFFFF0000000000000000000000000000000000000000000000000000000000)
                        if eq(curPrefix, 0x70615f0000000000000000000000000000000000000000000000000000000000) {
                            isBoundary := 1
                        }
                    }
                    if isBoundary {
                        let blockLength := sub(i, startIndex)
                        if gt(blockLength, 0) {
                            let boundaryKey := 0
                            if lt(i, numPhonemes) {
                                boundaryKey := calldataload(add(add(36, arrayOffset), mul(i, 32)))
                            }
                            
                            let basePitch := 220
                            if eq(name, 0x6d6f6c6f63680000000000000000000000000000000000000000000000000000) {
                                basePitch := 85
                            }
                            
                            for { let j := 0 } lt(j, blockLength) { j := add(j, 1) } {
                                let progressPercent := div(mul(j, 100), blockLength)
                                let pitch := div(mul(basePitch, sub(105, div(mul(progressPercent, 13), 100))), 100)
                                mstore(add(0x8000, mul(add(startIndex, j), 32)), pitch)
                            }
                            
                            // Specific punctuation checks
                            // boundaryType is PA_QUEST
                            if eq(and(boundaryKey, 0xFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000000000), 0x70615f7175657374000000000000000000000000000000000000000000000000) {
                                let raisedCount := 0
                                for { let k := sub(i, 1) } and(iszero(lt(k, startIndex)), lt(raisedCount, 2)) { k := sub(k, 1) } {
                                    let curKey := calldataload(add(add(36, arrayOffset), mul(k, 32)))
                                    let curPrefix := and(curKey, 0xFFFFFF0000000000000000000000000000000000000000000000000000000000)
                                    if iszero(eq(curPrefix, 0x70615f0000000000000000000000000000000000000000000000000000000000)) {
                                        let pVal := div(mul(basePitch, add(115, mul(raisedCount, 20))), 100)
                                        mstore(add(0x8000, mul(k, 32)), pVal)
                                        raisedCount := add(raisedCount, 1)
                                    }
                                }
                            }
                            
                            // boundaryType is PA_EXCL
                            if eq(and(boundaryKey, 0xFFFFFFFFFFFFFF000000000000000000000000000000000000000000000000), 0x70615f6578636c00000000000000000000000000000000000000000000000000) {
                                for { let k := startIndex } lt(k, i) { k := add(k, 1) } {
                                    let oldP := mload(add(0x8000, mul(k, 32)))
                                    mstore(add(0x8000, mul(k, 32)), div(mul(oldP, 115), 100))
                                }
                                for { let k := sub(i, 1) } iszero(lt(k, startIndex)) { k := sub(k, 1) } {
                                    let curKey := calldataload(add(add(36, arrayOffset), mul(k, 32)))
                                    let curPrefix := and(curKey, 0xFFFFFF0000000000000000000000000000000000000000000000000000000000)
                                    if iszero(eq(curPrefix, 0x70615f0000000000000000000000000000000000000000000000000000000000)) {
                                        mstore(add(0x8000, mul(k, 32)), div(mul(basePitch, 85), 100))
                                        break
                                    }
                                }
                            }
                            
                            // boundaryType is PA_COMMA
                            if eq(and(boundaryKey, 0xFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000000000), 0x70615f636f6d6d61000000000000000000000000000000000000000000000000) {
                                for { let k := sub(i, 1) } iszero(lt(k, startIndex)) { k := sub(k, 1) } {
                                    let curKey := calldataload(add(add(36, arrayOffset), mul(k, 32)))
                                    let curPrefix := and(curKey, 0xFFFFFF0000000000000000000000000000000000000000000000000000000000)
                                    if iszero(eq(curPrefix, 0x70615f0000000000000000000000000000000000000000000000000000000000)) {
                                        mstore(add(0x8000, mul(k, 32)), div(mul(basePitch, 108), 100))
                                        break
                                    }
                                }
                            }
                        }
                        
                        if lt(i, numPhonemes) {
                            let basePitch := 220
                            if eq(name, 0x6d6f6c6f63680000000000000000000000000000000000000000000000000000) {
                                basePitch := 85
                            }
                            mstore(add(0x8000, mul(i, 32)), basePitch)
                        }
                        startIndex := add(i, 1)
                    }
                }

                for { let pIdx := 0 } lt(pIdx, numPhonemes) { pIdx := add(pIdx, 1) } {
                    let key := calldataload(add(add(36, arrayOffset), mul(pIdx, 32)))
                    
                    for { let j := 0 } lt(j, 8) { j := add(j, 1) } { mstore(add(0x3000, mul(j, 32)), 0) }

                    let synthTwoChars := and(key, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                    let synthOneChar := and(key, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                    if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 90) mstore(0x3020, 10) mstore(0x3040, sub(0, 50)) mstore(0x3060, 80)
                        mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }
                    if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 95) mstore(0x3020, 20) mstore(0x3040, sub(0, 30)) mstore(0x3060, 90)
                        mstore(0x3080, sub(0, 10)) mstore(0x30a0, 15) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 85) mstore(0x3020, 5) mstore(0x3040, sub(0, 60)) mstore(0x3060, 70)
                        mstore(0x3080, sub(0, 30)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 15)) mstore(0x30e0, 0)
                    }
                    if eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 10) mstore(0x3020, sub(0, 80)) mstore(0x3040, 80) mstore(0x3060, sub(0, 20))
                        mstore(0x3080, 90) mstore(0x30a0, sub(0, 90)) mstore(0x30c0, 70) mstore(0x30e0, sub(0, 50))
                    }
                    if eq(synthOneChar, 0x7300000000000000000000000000000000000000000000000000000000000000) {
                        if iszero(eq(synthTwoChars, 0x7368000000000000000000000000000000000000000000000000000000000000)) {
                            mstore(0x3000, 5) mstore(0x3020, sub(0, 90)) mstore(0x3040, 90) mstore(0x3060, sub(0, 30))
                            mstore(0x3080, 95) mstore(0x30a0, sub(0, 95)) mstore(0x30c0, 80) mstore(0x30e0, sub(0, 60))
                        }
                    }
                    if eq(synthOneChar, 0x6600000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 5) mstore(0x3020, sub(0, 50)) mstore(0x3040, 40) mstore(0x3060, sub(0, 10))
                        mstore(0x3080, 50) mstore(0x30a0, sub(0, 40)) mstore(0x30c0, 30) mstore(0x30e0, sub(0, 20))
                    }
                    if eq(synthOneChar, 0x6800000000000000000000000000000000000000000000000000000000000000) { // "h"
                        mstore(0x3000, 15) mstore(0x3020, sub(0, 40)) mstore(0x3040, 30) mstore(0x3060, sub(0, 5))
                        mstore(0x3080, 40) mstore(0x30a0, sub(0, 30)) mstore(0x30c0, 20) mstore(0x30e0, sub(0, 10))
                    }
                    if eq(synthOneChar, 0x6c00000000000000000000000000000000000000000000000000000000000000) { // "l"
                        mstore(0x3000, 70) mstore(0x3020, 15) mstore(0x3040, sub(0, 40)) mstore(0x3060, 60)
                        mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 5)
                    }
                    if eq(synthOneChar, 0x6d00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 40) mstore(0x3020, 30) mstore(0x3040, sub(0, 10)) mstore(0x3060, 20)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                    }
                    if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                        mstore(0x3000, 35) mstore(0x3020, 25) mstore(0x3040, sub(0, 15)) mstore(0x3060, 15)
                        mstore(0x3080, sub(0, 5)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                    }

                    for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                        let phVal := mload(add(0x3000, mul(j, 32)))
                        let spkVal := mload(add(0x4000, mul(j, 32)))
                        mstore(add(0x3000, mul(j, 32)), add(phVal, spkVal))
                    }

                    for { let r := 0 } lt(r, 8) { r := add(r, 1) } {
                        let dot := 0
                        let bias := 0
                        if eq(r, 0) { bias := 10 }
                        if eq(r, 1) { bias := sub(0, 5) }
                        if eq(r, 2) { bias := 15 }
                        if eq(r, 3) { bias := 0 }
                        if eq(r, 4) { bias := 5 }
                        if eq(r, 5) { bias := sub(0, 10) }
                        if eq(r, 6) { bias := 20 }
                        if eq(r, 7) { bias := sub(0, 15) }
                        dot := mul(bias, 100)

                        for { let c := 0 } lt(c, 8) { c := add(c, 1) } {
                            let wIdx := add(mul(r, 8), c)
                            let rawWeight := 0
                            if lt(wIdx, 32) {
                                let shift := mul(wIdx, 8)
                                rawWeight := and(shr(shift, weights0), 0xFF)
                            }
                            if iszero(lt(wIdx, 32)) {
                                let shift := mul(sub(wIdx, 32), 8)
                                rawWeight := and(shr(shift, weights1), 0xFF)
                            }

                            let weight := 0
                            if and(rawWeight, 0x80) {
                                weight := or(rawWeight, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00)
                            }
                            if iszero(and(rawWeight, 0x80)) {
                                weight := rawWeight
                            }

                            let inputVal := mload(add(0x3000, mul(c, 32)))
                            dot := add(dot, mul(weight, inputVal))
                        }

                        let scaledVal := add(sdiv(dot, 100), 128)
                        if slt(scaledVal, 0) { scaledVal := 0 }
                        if sgt(scaledVal, 255) { scaledVal := 255 }

                        mstore8(add(0x3100, r), scaledVal)
                    }

                    let computedPitch := mload(add(0x8000, mul(pIdx, 32)))
                    mstore8(0x3107, computedPitch)

                    let pcmWord := mload(0x3100)
                    let targetByteOffset := mul(pIdx, 8)
                    let targetWordOffset := div(targetByteOffset, 32)
                    let targetShift := sub(248, mul(mod(targetByteOffset, 32), 8))

                    let targetAddress := add(0x6040, mul(targetWordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask64 := not(shl(targetShift, 0xFFFFFFFFFFFFFFFF))
                    currentWord := and(currentWord, mask64)
                    let cleanBytes := and(shr(192, pcmWord), 0xFFFFFFFFFFFFFFFF)
                    currentWord := or(currentWord, shl(targetShift, cleanBytes))
                    mstore(targetAddress, currentWord)
                }

                let paddedBytesLen := mul(div(add(totalOutputBytes, 31), 32), 32)
                return(0x6000, add(64, paddedBytesLen))
            }

            // ----------------------------------------------------------------
            // Method: trainOnPhoneme(bytes32 name, bytes32 phoneme) -> (uint256, uint256)
            // Selector: 0x7802b32f
            // ----------------------------------------------------------------
            if eq(selector, 0x7802b32f) {
                let name := calldataload(4)
                let key := calldataload(36)

                // Load speaker embedding
                mstore(0x00, name)
                mstore(0x20, 0x9999)
                let baseSlot := keccak256(0x00, 0x40)
                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    mstore(add(0x4000, mul(i, 32)), sload(add(baseSlot, i)))
                }

                // Load phoneme targets
                for { let j := 0 } lt(j, 8) { j := add(j, 1) } { mstore(add(0x3000, mul(j, 32)), 0) }
                let synthTwoChars := and(key, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let synthOneChar := and(key, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    mstore(0x3000, 90) mstore(0x3020, 10) mstore(0x3040, sub(0, 50)) mstore(0x3060, 80)
                    mstore(0x3080, sub(0, 20)) mstore(0x30a0, 10) mstore(0x30c0, sub(0, 10)) mstore(0x30e0, 5)
                }
                if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    mstore(0x3000, 95) mstore(0x3020, 20) mstore(0x3040, sub(0, 30)) mstore(0x3060, 90)
                    mstore(0x3080, sub(0, 10)) mstore(0x30a0, 15) mstore(0x30c0, sub(0, 5)) mstore(0x30e0, 10)
                }
                if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    mstore(0x3000, 85) mstore(0x3020, 5) mstore(0x3040, sub(0, 60)) mstore(0x3060, 70)
                    mstore(0x3080, sub(0, 30)) mstore(0x30a0, 5) mstore(0x30c0, sub(0, 15)) mstore(0x30e0, 0)
                }

                // Input vector x_c is phoneme + speaker embedding
                for { let j := 0 } lt(j, 8) { j := add(j, 1) } {
                    let phVal := mload(add(0x3000, mul(j, 32)))
                    let spkVal := mload(add(0x4000, mul(j, 32)))
                    mstore(add(0x3000, mul(j, 32)), add(phVal, spkVal))
                }

                // Load current weights
                let weights0 := sload(0x7777)
                let weights1 := sload(0x7778)

                // Run forward pass to get current outputs y_r
                for { let r := 0 } lt(r, 8) { r := add(r, 1) } {
                    let dot := 0
                    let bias := 0
                    if eq(r, 0) { bias := 10 }
                    if eq(r, 1) { bias := sub(0, 5) }
                    if eq(r, 2) { bias := 15 }
                    if eq(r, 4) { bias := 5 }
                    if eq(r, 5) { bias := sub(0, 10) }
                    if eq(r, 6) { bias := 20 }
                    if eq(r, 7) { bias := sub(0, 15) }
                    dot := mul(bias, 100)

                    for { let c := 0 } lt(c, 8) { c := add(c, 1) } {
                        let wIdx := add(mul(r, 8), c)
                        let rawWeight := 0
                        if lt(wIdx, 32) {
                            rawWeight := and(shr(mul(wIdx, 8), weights0), 0xFF)
                        }
                        if iszero(lt(wIdx, 32)) {
                            rawWeight := and(shr(mul(sub(wIdx, 32), 8), weights1), 0xFF)
                        }
                        let weight := 0
                        if and(rawWeight, 0x80) {
                            weight := or(rawWeight, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00)
                        }
                        if iszero(and(rawWeight, 0x80)) {
                            weight := rawWeight
                        }
                        let inputVal := mload(add(0x3000, mul(c, 32)))
                        dot := add(dot, mul(weight, inputVal))
                    }
                    let scaledVal := add(sdiv(dot, 100), 128)
                    if slt(scaledVal, 0) { scaledVal := 0 }
                    if sgt(scaledVal, 255) { scaledVal := 255 }
                    mstore8(add(0x3100, r), scaledVal)
                }

                // Delta Rule Weight Update
                let newWeights0 := 0
                let newWeights1 := 0

                for { let r := 0 } lt(r, 8) { r := add(r, 1) } {
                    // targetVal is the target features (original 0x3000 values offset by 128)
                    let targetVal := 128
                    if eq(r, 0) { targetVal := 218 } // 90 + 128
                    if eq(r, 1) { targetVal := 138 } // 10 + 128
                    if eq(r, 2) { targetVal := 78 }  // -50 + 128
                    
                    let yVal := and(mload(add(0x3100, r)), 0xFF)
                    let error := sub(targetVal, yVal) // e_r = target - output

                    for { let c := 0 } lt(c, 8) { c := add(c, 1) } {
                        let wIdx := add(mul(r, 8), c)
                        let rawWeight := 0
                        if lt(wIdx, 32) {
                            rawWeight := and(shr(mul(wIdx, 8), weights0), 0xFF)
                        }
                        if iszero(lt(wIdx, 32)) {
                            rawWeight := and(shr(mul(sub(wIdx, 32), 8), weights1), 0xFF)
                        }
                        let weight := 0
                        if and(rawWeight, 0x80) {
                            weight := or(rawWeight, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00)
                        }
                        if iszero(and(rawWeight, 0x80)) {
                            weight := rawWeight
                        }

                        // Delta step: w_new = w + error * input / 256
                        let inputVal := mload(add(0x3000, mul(c, 32)))
                        let delta := sdiv(mul(error, inputVal), 256)
                        let newW := add(weight, delta)
                        if slt(newW, sub(0, 128)) { newW := sub(0, 128) }
                        if sgt(newW, 127) { newW := 127 }

                        let cleanNewW := and(newW, 0xFF)
                        if lt(wIdx, 32) {
                            newWeights0 := or(newWeights0, shl(mul(wIdx, 8), cleanNewW))
                        }
                        if iszero(lt(wIdx, 32)) {
                            newWeights1 := or(newWeights1, shl(mul(sub(wIdx, 32), 8), cleanNewW))
                        }
                    }
                }

                sstore(0x7777, newWeights0)
                sstore(0x7778, newWeights1)

                mstore(0x5000, newWeights0)
                mstore(0x5020, newWeights1)
                return(0x5000, 64)
            }

            revert(0, 0)
        }
    }
}
