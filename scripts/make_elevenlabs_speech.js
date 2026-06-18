const fs = require("fs");
const path = require("path");

class ZmmElevenLabsThunk {
    constructor() {
        this.voices = {
            "child": {
                name: "Child",
                gender: "female",
                basePitch: 320, // High child pitch
                latentCoefficients: [0.25, -0.15, 0.35, -0.05, 0.08, -0.06, 0.15, -0.04, 0.02]
            }
        };

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

        this.activeVoice = "child";
        this.currentPitch = 320;
        this.targetPitch = 320;
        this.currentEnergy = 0;
        this.targetEnergy = 0;
        this.reflectionCoefficients = new Float32Array(10);
        this.targetK = new Float32Array(10);
        this.lfsrState = 0xACE1;
        this.phase = 0;
    }

    nextNoiseSample() {
        let bit = this.lfsrState & 1;
        this.lfsrState >>= 1;
        if (bit === 1) {
            this.lfsrState ^= 0xB400;
        }
        return ((this.lfsrState & 0xFF) / 255.0) * 2.0 - 1.0;
    }

    projectLatentCoefficients(phonemeKey) {
        const voice = this.voices[this.activeVoice];
        const phone = this.phonemeMap[phonemeKey] || { type: "silent", energy: 0, formants: [0,0,0] };

        this.targetEnergy = phone.energy;
        this.targetPitch = voice.basePitch;

        if (phone.type === "nasal") {
            this.targetPitch = voice.basePitch * 0.95;
        } else if (phone.type === "voiced" && phonemeKey === "ee") {
            this.targetPitch = voice.basePitch * 1.05;
        }

        for (let i = 0; i < 10; i++) {
            if (i < 6) {
                let baseK = voice.latentCoefficients[i] || 0;
                let formantMod = 0.0;

                if (phone.type === "voiced" || phone.type === "nasal") {
                    if (i < 3) {
                        formantMod = phone.formants[i] * 0.3;
                    }
                } else if (phone.type === "unvoiced") {
                    if (i < 4) {
                        formantMod = -0.4;
                    }
                }

                let K = baseK + formantMod;
                if (K > 0.99) K = 0.99;
                if (K < -0.99) K = -0.99;

                this.targetK[i] = K;
            } else {
                this.targetK[i] = this.targetK[5] / Math.pow(2, i - 5);
            }
        }

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

    synthesizeLatticeSamples(numSamples, inputSignalType) {
        const K = this.reflectionCoefficients;
        const samples = new Float32Array(numSamples);
        const delayLine = new Float32Array(11); 

        for (let t = 0; t < numSamples; t++) {
            const alpha = 0.002; 
            this.currentPitch += alpha * (this.targetPitch - this.currentPitch);
            this.currentEnergy += alpha * (this.targetEnergy - this.currentEnergy);
            for (let i = 0; i < 10; i++) {
                K[i] += alpha * (this.targetK[i] - K[i]);
            }

            let excitation = 0.0;
            if (inputSignalType === "voiced") {
                this.phase += 1;
                const jitter = (Math.sin(this.phase * 0.05) + Math.cos(this.phase * 0.11)) * 0.01;
                const period = Math.round(16000 / (this.currentPitch * (1.0 + jitter)));
                const tMod = this.phase % period;
                
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
                
                pulse = pulse - 0.25;
                excitation = pulse * 0.92 + this.nextNoiseSample() * 0.08;
            } else if (inputSignalType === "unvoiced") {
                excitation = this.nextNoiseSample() * 0.55;
            }

            // Scale excitation by amplitude energy envelope
            const amp = (this.currentEnergy / 100.0) * 0.45;
            let forward = excitation * amp;

            // 9th-order Lattice Filter structure (reflections)
            for (let i = 8; i >= 0; i--) {
                forward = forward - K[i] * delayLine[i];
                delayLine[i + 1] = delayLine[i] + K[i] * forward;
            }
            delayLine[0] = forward;
            samples[t] = forward;
        }
        return samples;
    }
}

// Convert arbitrary text characters to voice phonemes
function textToPhonemes(text) {
    const clean = text.toLowerCase().replace(/[^a-z\s]/g, "");
    const phonemes = [];
    
    for (let char of clean) {
        if (char === " ") {
            phonemes.push("p"); // silent pause
        } else if (["a", "o"].includes(char)) {
            phonemes.push("aa");
        } else if (["e", "i", "y"].includes(char)) {
            phonemes.push("ee");
        } else if (["u", "w"].includes(char)) {
            phonemes.push("oo");
        } else if (char === "m") {
            phonemes.push("m");
        } else if (char === "n") {
            phonemes.push("n");
        } else if (["s", "c", "z"].includes(char)) {
            phonemes.push("s");
        } else if (char === "h") {
            phonemes.push("sh");
        } else if (["f", "v"].includes(char)) {
            phonemes.push("f");
        } else {
            phonemes.push("p");
        }
    }
    return phonemes;
}

function main() {
    const lorePath = path.join(__dirname, "../lore/associahedron_lore.md");
    console.log(`[ElevenLabs ZMM] Synthesizing speech directly from: ${lorePath}`);
    
    const loreContent = fs.readFileSync(lorePath, "utf8");
    const phonemes = textToPhonemes(loreContent);
    
    const engine = new ZmmElevenLabsThunk();
    const sampleRate = 16000;
    
    // Each phoneme lasts for 65ms
    const phonemeDurationSecs = 0.065;
    const samplesPerPhoneme = Math.round(sampleRate * phonemeDurationSecs);
    const totalSamples = samplesPerPhoneme * phonemes.length;
    
    console.log(`[ElevenLabs ZMM] Total Phonemes: ${phonemes.length}, Total Speech Duration: ${(totalSamples / sampleRate).toFixed(2)}s`);
    
    const outputBuffer = new Float32Array(totalSamples);
    
    for (let i = 0; i < phonemes.length; i++) {
        const ph = phonemes[i];
        const projection = engine.projectLatentCoefficients(ph);
        const pcm = engine.synthesizeLatticeSamples(samplesPerPhoneme, projection.type);
        outputBuffer.set(pcm, i * samplesPerPhoneme);
    }
    
    const outWav = path.join(__dirname, "../temp_narration_raw.wav");
    writeWavFile(outputBuffer, outWav, sampleRate);
    console.log(`[ElevenLabs ZMM] Vocal track saved successfully to: ${outWav}`);
}

function writeWavFile(audioBuffer, wavPath, sampleRate) {
    const numSamples = audioBuffer.length;
    const buffer = Buffer.alloc(44 + numSamples * 2);

    buffer.write("RIFF", 0, "ascii");
    buffer.writeUInt32LE(36 + numSamples * 2, 4);
    buffer.write("WAVE", 8, "ascii");
    
    buffer.write("fmt ", 12, "ascii");
    buffer.writeUInt32LE(16, 16);
    buffer.writeUInt16LE(1, 20);
    buffer.writeUInt16LE(1, 22);
    buffer.writeUInt32LE(sampleRate, 24);
    buffer.writeUInt32LE(sampleRate * 2, 28);
    buffer.writeUInt16LE(2, 32);
    buffer.writeUInt16LE(16, 34);
    
    buffer.write("data", 36, "ascii");
    buffer.writeUInt32LE(numSamples * 2, 40);

    let offset = 44;
    for (let i = 0; i < numSamples; i++) {
        let s = Math.max(-1, Math.min(1, audioBuffer[i]));
        let val = s < 0 ? s * 0x8000 : s * 0x7FFF;
        buffer.writeInt16LE(Math.floor(val), offset);
        offset += 2;
    }

    fs.writeFileSync(wavPath, buffer);
}

main();
