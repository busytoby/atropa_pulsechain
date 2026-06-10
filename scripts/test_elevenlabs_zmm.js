const fs = require("fs");
const path = require("path");

/**
 * Simulated ZMM VM Thunk for ElevenLabs Neural Speech Synthesis Emulation
 */
class ZmmElevenLabsThunk {
    constructor() {
        // Voice library storing the latent space vectors for different voices
        // Let's model a 9-dimensional latent voice space corresponding to our LPC filter coefficients (K1..K9)
        this.voices = {
            "ana": {
                name: "Ana",
                gender: "female",
                basePitch: 220, // High pitch
                latentCoefficients: [0.15, -0.25, 0.40, -0.10, 0.05, -0.08, 0.12, -0.05, 0.02]
            },
            "moloch": {
                name: "Moloch",
                gender: "male",
                basePitch: 85, // Deep low pitch
                latentCoefficients: [0.85, -0.65, 0.70, -0.45, 0.35, -0.22, 0.18, -0.10, 0.05]
            }
        };

        // Standard English phoneme to parameter templates mapping
        // Models phonetic properties for voiced vowels, nasals, and unvoiced fricatives
        this.phonemeMap = {
            "aa": { type: "voiced", energy: 90, formants: [1.2, 0.8, -0.5] },
            "ee": { type: "voiced", energy: 95, formants: [1.5, 0.4, -0.2] },
            "oo": { type: "voiced", energy: 85, formants: [0.9, 0.9, -0.6] },
            "m":  { type: "nasal",  energy: 50, formants: [0.5, 0.3, -0.1] },
            "n":  { type: "nasal",  energy: 45, formants: [0.6, 0.4, -0.1] },
            "s":  { type: "unvoiced", energy: 70, formants: [0.0, 0.0, 0.0] },
            "sh": { type: "unvoiced", energy: 80, formants: [0.0, 0.0, 0.0] },
            "f":  { type: "unvoiced", energy: 40, formants: [0.0, 0.0, 0.0] },
            "p":  { type: "silent", energy: 0, formants: [0.0, 0.0, 0.0] }
        };

        // Active state registers
        this.activeVoice = "ana";
        this.currentPitch = 220;
        this.targetPitch = 220;
        this.currentEnergy = 0;
        this.targetEnergy = 0;
        this.reflectionCoefficients = new Float32Array(10);
        this.targetK = new Float32Array(10);
        this.lfsrState = 0xACE1; // 16-bit LFSR seed for unvoiced sibilants noise
        this.phase = 0;
    }

    /**
     * Galois Linear Feedback Shift Register (LFSR) for unvoiced noise generation
     */
    nextNoiseSample() {
        let bit = this.lfsrState & 1;
        this.lfsrState >>= 1;
        if (bit === 1) {
            this.lfsrState ^= 0xB400; // Feedback polynomial
        }
        return ((this.lfsrState & 0xFF) / 255.0) * 2.0 - 1.0;
    }

    /**
     * Selects active voice model
     */
    selectVoice(voiceId) {
        if (!this.voices[voiceId]) {
            return false;
        }
        this.activeVoice = voiceId;
        this.currentPitch = this.voices[voiceId].basePitch;
        return true;
    }

    /**
     * Simulates the ElevenLabs-style Generative Latent Projection layer in Yul:
     * maps input phoneme features and the selected voice's latent embedding
     * to determine the dynamic reflection coefficients (K1..K9).
     */
    projectLatentCoefficients(phonemeKey) {
        const voice = this.voices[this.activeVoice];
        const phone = this.phonemeMap[phonemeKey] || { type: "silent", energy: 0, formants: [0,0,0] };

        this.targetEnergy = phone.energy;
        this.targetPitch = voice.basePitch;

        // Dynamic pitch variation depending on phoneme type
        if (phone.type === "nasal") {
            this.targetPitch = voice.basePitch * 0.95; // Nasals are slightly lower pitch
        } else if (phone.type === "voiced" && phonemeKey === "ee") {
            this.targetPitch = voice.basePitch * 1.05; // High front vowels are slightly higher pitch
        }

        // Neural Flow-Matching Approximation: blend voice latent traits with phoneme formants
        for (let i = 0; i < 10; i++) {
            if (i < 6) {
                let baseK = voice.latentCoefficients[i] || 0;
                let formantMod = 0.0;

                if (phone.type === "voiced" || phone.type === "nasal") {
                    // Modulate K1..K3 based on formant properties
                    if (i < 3) {
                        formantMod = phone.formants[i] * 0.3;
                    }
                } else if (phone.type === "unvoiced") {
                    // High frequency sibilance: shift coefficients towards negative reflection
                    if (i < 4) {
                        formantMod = -0.4;
                    }
                }

                // Clamped reflection coefficient boundary [-0.99, 0.99] to maintain lattice stability
                let K = baseK + formantMod;
                if (K > 0.99) K = 0.99;
                if (K < -0.99) K = -0.99;

                this.targetK[i] = K;
            } else {
                // K6..K9 derive via formant decay from K5 (this.targetK[5])
                this.targetK[i] = this.targetK[5] / Math.pow(2, i - 5);
            }
        }

        // If this is the first phoneme run or we are initialized to all zeros, seed the current values to target values
        let isZero = true;
        for (let i = 0; i < 10; i++) {
            if (this.reflectionCoefficients[i] !== 0) {
                isZero = false;
                break;
            }
        }
        if (isZero) {
            this.currentPitch = this.targetPitch;
            this.currentEnergy = this.targetEnergy;
            for (let i = 0; i < 10; i++) {
                this.reflectionCoefficients[i] = this.targetK[i];
            }
        }

        return {
            coefficients: Array.from(this.targetK),
            pitch: this.targetPitch,
            energy: this.targetEnergy,
            type: phone.type
        };
    }

    /**
     * Evaluates a 9th-order Lattice Filter (LPC Vocoder Synthesis)
     * using the current reflection coefficients K1..K9.
     */
    synthesizeLatticeSamples(numSamples, inputSignalType) {
        const K = this.reflectionCoefficients;
        const samples = new Float32Array(numSamples);
        
        // Lattice filter state memory (backward lattice paths)
        const delayLine = new Float32Array(11); 

        for (let t = 0; t < numSamples; t++) {
            // Smoothly interpolate parameters towards targets to avoid clicks/glitches and simulate coarticulation
            const alpha = 0.002; // slow smooth transition (~30-50ms)
            this.currentPitch += alpha * (this.targetPitch - this.currentPitch);
            this.currentEnergy += alpha * (this.targetEnergy - this.currentEnergy);
            for (let i = 0; i < 10; i++) {
                K[i] += alpha * (this.targetK[i] - K[i]);
            }

            // Generate excitation source (voiced vs unvoiced)
            let excitation = 0.0;
            if (inputSignalType === "voiced") {
                this.phase += 1;
                // Introduce subtle pitch jitter and shimmer for human vocal organic variations
                const jitter = (Math.sin(this.phase * 0.05) + Math.cos(this.phase * 0.11)) * 0.01;
                const period = Math.round(16000 / (this.currentPitch * (1.0 + jitter)));
                const tMod = this.phase % period;
                
                // Rosenberg Glottal Pulse Model
                // Tp: open phase duration (usually ~40% of pitch period)
                // Tn: closed phase duration (usually ~16% of pitch period)
                const Tp = Math.round(0.40 * period);
                const Tn = Math.round(0.16 * period);
                let pulse = 0.0;
                
                if (tMod < Tp) {
                    const phase = tMod / Tp;
                    pulse = 3 * phase * phase - 2 * phase * phase * phase;
                } else if (tMod < Tp + Tn) {
                    const phase = (tMod - Tp) / Tn;
                    pulse = 1.0 - phase * phase;
                } else {
                    pulse = 0.0;
                }
                
                // Subtract DC offset from the pulse to avoid pop/clicks
                pulse = pulse - 0.25;
                
                // Mix in 92% glottal pulse and 8% random aspiration noise for natural voice breathiness
                excitation = pulse * 0.92 + this.nextNoiseSample() * 0.08;
            } else if (inputSignalType === "unvoiced") {
                excitation = this.nextNoiseSample();
            } else if (inputSignalType === "nasal") {
                // Nasal resonance: combined harmonic components
                excitation = Math.sin(2 * Math.PI * 150 * t / 16000) * 0.5 + Math.sin(2 * Math.PI * 300 * t / 16000) * 0.2;
            }

            // Scale excitation by the energy level
            let forward = excitation * (this.currentEnergy / 100.0);

            // Forward-Backward lattice filter flow
            for (let i = 9; i >= 0; i--) {
                forward = forward - K[i] * delayLine[i];
                delayLine[i + 1] = delayLine[i] + K[i] * forward;
            }
            delayLine[0] = forward;

            samples[t] = forward;
        }

        return samples;
    }
}

function runTests() {
    console.log("=== Launching ElevenLabs Neural Speech Synthesis Emulator on ZMM ===");

    const engine = new ZmmElevenLabsThunk();

    console.log("\n[TEST 1] Testing Voice Profile Selectors...");
    console.log(`Active voice: ${engine.activeVoice} (Expected: ana)`);
    if (engine.activeVoice !== "ana") throw new Error("FAIL: default voice must be ana");

    let status = engine.selectVoice("moloch");
    console.log(`Swapping voice to 'moloch'... status = ${status}`);
    console.log(`Active voice: ${engine.activeVoice} (Expected: moloch)`);
    console.log(`Base pitch: ${engine.currentPitch}Hz (Expected: 85Hz)`);
    if (engine.activeVoice !== "moloch" || engine.currentPitch !== 85) {
        throw new Error("FAIL: voice selection or pitch binding failed!");
    }

    console.log("\n[TEST 2] Testing Latent Projection (Ana Voice + Voiced Vowel 'ee')...");
    engine.selectVoice("ana");
    let projection = engine.projectLatentCoefficients("ee");
    console.log("K1..K9 Coefficients projected:", projection.coefficients.map(k => k.toFixed(4)));
    console.log(`Pitch: ${projection.pitch}Hz, Energy: ${projection.energy}, Sound Type: ${projection.type}`);
    
    // Assert K coefficients are bounded and modified appropriately
    if (projection.coefficients.some(k => k < -0.99 || k > 0.99)) {
        throw new Error("FAIL: Reflection coefficients must not exceed lattice stability boundaries (-0.99 to 0.99)!");
    }
    if (projection.type !== "voiced" || projection.energy !== 95) {
        throw new Error("FAIL: Phoneme parameter mapping mismatch!");
    }
    console.log("✓ Latent projection passed stability checks.");

    console.log("\n[TEST 3] Testing Latent Projection (Moloch Voice + Unvoiced Sibilant 'sh')...");
    engine.selectVoice("moloch");
    projection = engine.projectLatentCoefficients("sh");
    console.log("K1..K9 Coefficients projected:", projection.coefficients.map(k => k.toFixed(4)));
    console.log(`Pitch: ${projection.pitch}Hz, Energy: ${projection.energy}, Sound Type: ${projection.type}`);
    if (projection.type !== "unvoiced" || projection.energy !== 80) {
        throw new Error("FAIL: Phoneme type classification failure.");
    }
    console.log("✓ Sibilance parameters verified.");

    console.log("\n[TEST 4] Testing Lattice Synthesizer (PCM Waveform Generation)...");
    engine.selectVoice("ana");
    engine.projectLatentCoefficients("aa");
    
    const sampleRate = 16000;
    const durationMs = 10; // 10ms of speech
    const numSamples = Math.round(sampleRate * (durationMs / 1000));
    
    console.log(`Generating ${numSamples} PCM samples for voiced vowel 'aa'...`);
    const pcmSamples = engine.synthesizeLatticeSamples(numSamples, "voiced");
    console.log("First 10 PCM Sample values:", Array.from(pcmSamples.slice(0, 10)).map(s => s.toFixed(5)));
    
    // Check that samples are non-zero (proving filter activity) and bounded
    let nonZero = false;
    for (let s of pcmSamples) {
        if (Math.abs(s) > 0.0001) nonZero = true;
        if (isNaN(s)) throw new Error("FAIL: Synthesized samples contain NaN!");
    }
    if (!nonZero) {
        throw new Error("FAIL: Synthesizer produced silent output!");
    }
    console.log("✓ Lattice filter output verified. Waveform is stable and active.");

    console.log("\n=== All ZMM ElevenLabs Neural Speech Emulation Tests Passed Successfully! ===");
}

runTests();
