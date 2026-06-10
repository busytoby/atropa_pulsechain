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
                let basePitch := 220
                if eq(voiceId, 1) {
                    basePitch := 85
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

                let cpu := getCpuAddress()
                let callerAddr := caller()

                // Default coefficients for Ana (0) or Moloch (1)
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

                    pokeUser(cpu, callerAddr, add(54800, i), K)
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
                let synthKey := calldataload(4)
                let numSamples := calldataload(36)
                if gt(numSamples, 1000) { numSamples := 1000 }

                let synthVoiceId := sload(2) // 0 = Ana, 1 = Moloch
                let synthBasePitch := 220
                if eq(synthVoiceId, 1) {
                    synthBasePitch := 85
                }
                
                let synthEnergy := 0
                let synthSoundType := 0 // 0 = silent, 1 = voiced, 2 = unvoiced, 3 = nasal

                // Check phoneme keys by matching prefixes
                let synthTwoChars := and(synthKey, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let synthOneChar := and(synthKey, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 90
                    synthSoundType := 1
                }
                if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 95
                    synthSoundType := 1
                }
                if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 85
                    synthSoundType := 1
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
                }
                if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 45
                    synthSoundType := 3
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

                // Clear Delay Line starting at 0x2000 (10 words)
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    mstore(add(0x2000, mul(i, 32)), 0)
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
                        cur := add(cur, sdiv(diff, 16))
                        mstore(add(0x4100, mul(i, 32)), cur)
                    }

                    let excitation := 0

                    if eq(synthSoundType, 1) {
                        // Pitch intonation contour (slight upward slide)
                        let pitch := add(synthBasePitch, div(t, 25))
                        let period := div(16000, pitch)
                        excitation := sub(0, 50)
                        if iszero(mod(t, period)) {
                            excitation := 1000
                        }
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

                    let forward := div(mul(excitation, synthEnergy), 100)

                    for { let i := 9 } gt(i, 0) { } {
                        i := sub(i, 1)
                        let K := mload(add(0x4100, mul(i, 32)))
                        let delayVal := mload(add(0x2000, mul(i, 32)))
                        let nextForward := sub(forward, div(mul(K, delayVal), 100))
                        let nextDelay := add(delayVal, div(mul(K, nextForward), 100))
                        mstore(add(0x2000, mul(add(i, 1), 32)), nextDelay)
                        forward := nextForward
                    }
                    mstore(0x2000, forward)

                    // Convert to 16-bit PCM (signed)
                    let pcmVal := mul(forward, 15)
                    if sgt(pcmVal, 32767) { pcmVal := 32767 }
                    if slt(pcmVal, sub(0, 32768)) { pcmVal := sub(0, 32768) }

                    // Pack 16-bit values sequentially starting at offset 0x5040
                    let byteOffset := mul(t, 2)
                    let wordOffset := div(byteOffset, 32)
                    let bitShift := sub(240, mul(mod(byteOffset, 32), 8))
                    
                    let targetAddress := add(0x5040, mul(wordOffset, 32))
                    let currentWord := mload(targetAddress)
                    let mask := not(shl(bitShift, 0xFFFF))
                    currentWord := and(currentWord, mask)
                    let cleanPcm := and(pcmVal, 0xFFFF)
                    currentWord := or(currentWord, shl(bitShift, cleanPcm))
                    mstore(targetAddress, currentWord)
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
                let synthKey := calldataload(4)
                let numSamples := calldataload(36)
                if gt(numSamples, 1000) { numSamples := 1000 }

                let synthVoiceId := sload(2) // 0 = Ana, 1 = Moloch
                let synthBasePitch := 220
                if eq(synthVoiceId, 1) {
                    synthBasePitch := 85
                }
                
                let synthEnergy := 0
                let synthSoundType := 0 // 0 = silent, 1 = voiced, 2 = unvoiced, 3 = nasal

                // Check phoneme keys by matching prefixes
                let synthTwoChars := and(synthKey, 0xFFFF000000000000000000000000000000000000000000000000000000000000)
                let synthOneChar := and(synthKey, 0xFF00000000000000000000000000000000000000000000000000000000000000)

                if eq(synthTwoChars, 0x6161000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 90
                    synthSoundType := 1
                }
                if eq(synthTwoChars, 0x6565000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 95
                    synthSoundType := 1
                }
                if eq(synthTwoChars, 0x6f6f000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 85
                    synthSoundType := 1
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
                }
                if eq(synthOneChar, 0x6e00000000000000000000000000000000000000000000000000000000000000) {
                    synthEnergy := 45
                    synthSoundType := 3
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

                // Clear Delay Line starting at 0x2000 (10 words)
                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                    mstore(add(0x2000, mul(i, 32)), 0)
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
                        cur := add(cur, sdiv(diff, 16))
                        mstore(add(0x4100, mul(i, 32)), cur)
                    }

                    let excitation := 0

                    if eq(synthSoundType, 1) {
                        // Pitch intonation contour (slight upward slide)
                        let pitch := add(synthBasePitch, div(t, 25))
                        let period := div(16000, pitch)
                        excitation := sub(0, 50)
                        if iszero(mod(t, period)) {
                            excitation := 1000
                        }
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

                    let forward := div(mul(excitation, synthEnergy), 100)

                    for { let i := 9 } gt(i, 0) { } {
                        i := sub(i, 1)
                        let K := mload(add(0x4100, mul(i, 32)))
                        let delayVal := mload(add(0x2000, mul(i, 32)))
                        let nextForward := sub(forward, div(mul(K, delayVal), 100))
                        let nextDelay := add(delayVal, div(mul(K, nextForward), 100))
                        mstore(add(0x2000, mul(add(i, 1), 32)), nextDelay)
                        forward := nextForward
                    }
                    mstore(0x2000, forward)

                    // Convert to 16-bit PCM (signed)
                    let pcmVal := mul(forward, 15)
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
                    currentWord := or(currentWord, shl(bitShift, pcmValLE))
                    mstore(targetAddress, currentWord)
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

                        let scaledVal := div(dot, 100)
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

            revert(0, 0)
        }
    }
}
