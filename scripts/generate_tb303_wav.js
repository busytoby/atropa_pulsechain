const fs = require("fs");
const path = require("path");

const SAMPLE_RATE = 44100;
const DURATION_SECS = 4;
const TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS;

// 16-step acid bassline note frequencies
const C2 = 65.41;
const C3 = 130.81;
const Eb2 = 77.78;
const G2 = 98.00;
const Bb2 = 116.54;

const SEQUENCE = [
    C2, C2, C3, C2,  Eb2, G2, Bb2, C3,
    C2, C3, C2, Eb2, G2,  Bb2, C3, C2
];
const ACCENTS = [
    0, 0, 1, 0,  0, 0, 0, 1,
    0, 1, 0, 0,  0, 0, 1, 0
];
const SLIDES = [
    0, 1, 0, 0,  1, 0, 0, 0,
    0, 0, 1, 0,  1, 0, 0, 1
];

function main() {
    console.log("===============================================================");
    console.log("GENERATING TEDDY BEAR 303 ACID BASS SYNTHESIS DEMO WAV");
    console.log("===============================================================");

    const audioBuffer = new Float32Array(TOTAL_SAMPLES);

    // TB-303 Synthesizer state
    let currentFreq = SEQUENCE[0];
    let filterCutoff = 350; // Base cutoff in Hz
    let envDecay = 0.15; // Envelope decay in seconds
    let resonance = 0.82; // 0.0 to 0.99 (Q factor equivalent)
    let envMod = 1200; // Frequency sweep depth in Hz
    
    // Filter poles
    let p0 = 0, p1 = 0, p2 = 0, p3 = 0;
    
    // Oscillator phase
    let phase = 0;

    const tempoBPM = 120;
    const stepDurationSamples = Math.floor(SAMPLE_RATE * (60 / tempoBPM) / 4); // 16th note length

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const stepIndex = Math.floor(i / stepDurationSamples) % SEQUENCE.length;
        const sampleInStep = i % stepDurationSamples;
        const targetFreq = SEQUENCE[stepIndex];
        const isAccent = ACCENTS[stepIndex];
        const isSlide = SLIDES[stepIndex];

        // 1. Portamento / Slide glide
        const slideSpeed = isSlide ? 0.08 : 0.005; // lower is slower, higher is faster
        currentFreq += (targetFreq - currentFreq) * slideSpeed;

        // 2. Oscillator phase update (Sawtooth wave)
        phase += currentFreq / SAMPLE_RATE;
        if (phase >= 1.0) phase -= 2.0; // wrap around -1.0 to 1.0
        let oscSample = phase; // raw sawtooth

        // 3. Cutoff decay envelope sweep with slow LFO modulation over time
        const lfo = 0.5 + 0.5 * Math.sin(2 * Math.PI * 0.25 * (i / SAMPLE_RATE));
        const modulatedCutoff = filterCutoff + 250 * lfo;
        const decayTime = isAccent ? envDecay * 1.5 : envDecay;
        const decaySamples = decayTime * SAMPLE_RATE;
        const envVal = Math.exp(-sampleInStep / decaySamples);
        const dynamicCutoff = modulatedCutoff + (isAccent ? envMod * 1.6 : envMod) * envVal;

        // 4. DSP 4-Pole Lowpass Diode-Ladder simulation
        const cutoffCoeff = (2 * Math.PI * dynamicCutoff) / SAMPLE_RATE;
        
        // Feed forward with feedback resonance (accented steps get extra squelch)
        const dynamicResonance = isAccent ? 0.92 : resonance;
        const input = oscSample - dynamicResonance * p3;
        p0 += cutoffCoeff * (input - p0);
        p1 += cutoffCoeff * (p0 - p1);
        p2 += cutoffCoeff * (p1 - p2);
        p3 += cutoffCoeff * (p2 - p3);
        let filterOutput = p3;

        // 5. Triode Valve Overdrive / Non-linear tanhf Saturation
        const gain = isAccent ? 4.5 : 2.5;
        let saturated = filterOutput * gain;
        let outputSample = Math.tanh(saturated);

        // 6. Amplification envelope (VCA Gate)
        const gateDuration = isSlide ? stepDurationSamples : Math.floor(stepDurationSamples * 0.7);
        let amp = 0;
        if (sampleInStep < gateDuration) {
            amp = isAccent ? 0.45 : 0.3;
        } else {
            const releaseSamples = 0.02 * SAMPLE_RATE;
            amp = (isAccent ? 0.45 : 0.3) * Math.exp(-(sampleInStep - gateDuration) / releaseSamples);
        }
        audioBuffer[i] = outputSample * amp;
    }

    // Write WAV file
    const wavPath = path.join(__dirname, "../teddy303_acid_synthesis.wav");
    const buffer = writeWavFile(audioBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`WAV file successfully generated at: ${wavPath}`);
}

function writeWavFile(audioBuffer) {
    const numSamples = audioBuffer.length;
    const buffer = Buffer.alloc(44 + numSamples * 2);

    // RIFF Header
    buffer.write("RIFF", 0, "ascii");
    buffer.writeUInt32LE(36 + numSamples * 2, 4); // Chunk size
    buffer.write("WAVE", 8, "ascii");
    
    // fmt subchunk
    buffer.write("fmt ", 12, "ascii");
    buffer.writeUInt32LE(16, 16); // Subchunk size (16 for PCM)
    buffer.writeUInt16LE(1, 20);  // Audio format (1 = PCM)
    buffer.writeUInt16LE(1, 22);  // Channels (1 = Mono)
    buffer.writeUInt32LE(SAMPLE_RATE, 24); // Sample rate
    buffer.writeUInt32LE(SAMPLE_RATE * 2, 28); // Byte rate (SampleRate * Channels * BitsPerSample/8)
    buffer.writeUInt16LE(2, 32);  // Block align (Channels * BitsPerSample/8)
    buffer.writeUInt16LE(16, 34); // Bits per sample (16-bit)
    
    // data subchunk
    buffer.write("data", 36, "ascii");
    buffer.writeUInt32LE(numSamples * 2, 40); // Data subchunk size

    // Write PCM data
    let offset = 44;
    for (let i = 0; i < numSamples; i++) {
        // Clamp and scale float to 16-bit signed integer range
        let s = Math.max(-1, Math.min(1, audioBuffer[i]));
        let val = s < 0 ? s * 0x8000 : s * 0x7FFF;
        buffer.writeInt16LE(Math.floor(val), offset);
        offset += 2;
    }

    return buffer;
}

main();
