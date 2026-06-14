# Dr. Dobb's Journal Issue #4 (April 1976) & TSFi2 Vocal Formant Synthesis

This document reviews **Dr. Dobb's Journal** (Vol. 1, No. 4, April 1976), focusing on Lloyd Rice's landmark article **"Hardware & Software for Speech Synthesis"**, and details how the underlying formant synthesis algorithms are modeled inside the **TSFi2 speech synthesizer**.

---

## 1. Key Articles & Architectural Alignment

| Dr. Dobb's Issue #4 Concept | Original 1976 Technology | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Speech Synthesis** (Lloyd Rice) | Modeling the human vocal tract using analog/digital formant resonators ($F_1, F_2, F_3$) representing vocal tract resonances. | **Vocal & Speech Synthesizer** | Multi-channel formant filters (`formantFilter.yul`) mapping excitation pulses through resonant bandpass chains. |
| **MINOL Interpreter** (Erik Mueller) | A highly compacted Tiny BASIC variant including string manipulation occupying only 1.75K bytes. | **Gas-Optimized VM Interpreters** | Micro-bytecode interpreter contracts designed to execute within strict gas limits. |
| **8080 System Monitor** (Charlie Pack) | ROM-based console debugger allowing memory dumps, instruction steps, and binary patch entry. | **ZMM Emulator Interface** | RPC-based state viewer showing current memory registers and stack frames. |

---

## 2. Lloyd Rice's Formant Synthesis Model

Lloyd Rice's speech synthesis approach (which led to the Computalker CT-1 hardware) modeled the vocal tract as a series of cascade or parallel bandpass filters. The input stimulus is either:
1. **Glottal Pulses**: Periodic impulse waves simulating vocal fold vibration (voiced sounds like vowels).
2. **White Noise**: Random noise simulating turbulent air release (voiceless sounds like "sh" or "f").

The signal is filtered by three main formants:
*   **Formant 1 ($F_1$)**: Resonates between $250 \text{ Hz}$ and $1000 \text{ Hz}$, tracking mouth opening.
*   **Formant 2 ($F_2$)**: Resonates between $800 \text{ Hz}$ and $2500 \text{ Hz}$, tracking tongue position.
*   **Formant 3 ($F_3$)**: Resonates between $1500 \text{ Hz}$ and $3000 \text{ Hz}$, tracking tongue tip configuration.

For example, to synthesize the vowel **/ɑ/** (as in "father"), the synthesizer sets the filter parameters to:
*   $F_1 = 730 \text{ Hz}$
*   $F_2 = 1090 \text{ Hz}$
*   $F_3 = 2440 \text{ Hz}$

### Yul Formant Excitation Mapping

In our formant filter contracts, we route the excitation source (pulse/noise) through three parallel resonant bandpass filters using coefficients derived from the target formant frequencies:

```yul
// Yul formant vocal synthesis chain
function synthesizeVowel(excitationType, pitchFreq, baseAddr) {
    let sampleRate := 44100
    let bufferLength := 256
    
    // 1. Generate excitation source
    for { let i := 0 } lt(i, bufferLength) { i := add(i, 1) } {
        let stimulus := 0
        switch excitationType
        case 0 { // Voiced: glottal pulse train
            stimulus := glottalPulse(i, pitchFreq, sampleRate)
        }
        case 1 { // Voiceless: white noise
            stimulus := pseudoRandomNoise(i)
        }
        
        // 2. Filter through formant resonators (F1, F2, F3)
        // ah vowel formant center frequencies
        let f1Out := applyResonator(stimulus, 730, 50)  // F1: 730 Hz, BW: 50 Hz
        let f2Out := applyResonator(stimulus, 1090, 70) // F2: 1090 Hz, BW: 70 Hz
        let f3Out := applyResonator(stimulus, 2440, 110)// F3: 2440 Hz, BW: 110 Hz
        
        // Combine outputs to form synthesized vocal output
        let vocalSample := add(add(f1Out, f2Out), f3Out)
        mstore(add(baseAddr, mul(i, 32)), vocalSample)
    }
}

function glottalPulse(index, pitch, rate) -> pulse {
    // Generate simple periodic impulse wave
    let period := div(rate, pitch)
    pulse := 0
    if eq(mod(index, period), 0) {
        pulse := 1000000 // Fixed-point amplitude scale
    }
}
```

---

## 3. String Packing in Memory-Constrained Environments (MINOL)

Erik Mueller's MINOL interpreter achieved string manipulation under 1.75K bytes by storing string literals inline within the instruction stream and utilizing dynamic pointer offsets instead of allocating individual heap blocks. We apply this optimization in the **TSFi2 ZMM VM** when packing speech phoneme tokens, reading strings directly from the calldata stream without intermediate memory allocations.

---

## 4. Conclusion

*Dr. Dobb's Journal* Issue #4 established early paradigms for software-directed speech synthesis and memory-efficient interpreter design. By translating Lloyd Rice's formant filter models and MINOL's zero-allocation string storage rules into our Yul-based audio/speech framework, we maintain high-performance synthesis within tight execution boundaries.
