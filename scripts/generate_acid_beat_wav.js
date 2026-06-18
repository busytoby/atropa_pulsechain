const fs = require("fs");
const path = require("path");

const SAMPLE_RATE = 44100;
const DURATION_SECS = 8; // 2 full bars of the sequence
const TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS;

// TB-303 Notes
const C2 = 65.41;
const C3 = 130.81;
const Eb2 = 77.78;
const G2 = 98.00;
const Bb2 = 116.54;

const BASS_SEQUENCE = [
    C2, C2, C3, C2,  Eb2, G2, Bb2, C3,
    C2, C3, C2, Eb2, G2,  Bb2, C3, C2
];
const BASS_ACCENTS = [
    0, 0, 1, 0,  0, 0, 0, 1,
    0, 1, 0, 0,  0, 0, 1, 0
];
const BASS_SLIDES = [
    0, 1, 0, 0,  1, 0, 0, 0,
    0, 0, 1, 0,  1, 0, 0, 1
];

// TR-808 Kick Triggers: Four-on-the-floor beat (Steps 0, 4, 8, 12)
const KICK_SEQUENCE = [
    1, 0, 0, 0,  1, 0, 0, 0,
    1, 0, 0, 0,  1, 0, 0, 0
];

function main() {
    console.log("===============================================================");
    console.log("GENERATING COMBINED 303 ACID BASS & 808 BEAT WAV");
    console.log("===============================================================");

    const audioBuffer = new Float32Array(TOTAL_SAMPLES);
    const tempoBPM = 120;
    const stepDurationSamples = Math.floor(SAMPLE_RATE * (60 / tempoBPM) / 4); // 16th note step length

    // 303 State
    let currentFreq = BASS_SEQUENCE[0];
    let filterCutoff = 320; 
    let envDecay = 0.14; 
    let resonance = 0.85; 
    let envMod = 1300; 
    let phase303 = 0;
    let p0 = 0, p1 = 0, p2 = 0, p3 = 0; // Lowpass poles

    // 808 State (Track sample index of the last kick trigger)
    let lastKickTriggerSample = -999999; 

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const stepIndex = Math.floor(i / stepDurationSamples) % BASS_SEQUENCE.length;
        const sampleInStep = i % stepDurationSamples;

        // ----------------------------------------------------
        // 1. Trigger 808 Kick Drum
        // ----------------------------------------------------
        if (sampleInStep === 0 && KICK_SEQUENCE[stepIndex] === 1) {
            lastKickTriggerSample = i;
        }

        // Synthesize 808 Kick
        let kickSample = 0;
        const kickAgeSecs = (i - lastKickTriggerSample) / SAMPLE_RATE;
        if (kickAgeSecs >= 0 && kickAgeSecs < 1.0) {
            // Rapid pitch sweep (from 150Hz down to 48Hz) simulating Twin-T resonator behavior
            const pitchEnv = Math.exp(-kickAgeSecs / 0.038);
            const kickFreq = 48 + 102 * pitchEnv;
            
            // Calculate sine wave phase
            const kickPhase = 2 * Math.PI * kickFreq * kickAgeSecs;
            const sine = Math.sin(kickPhase);
            
            // Click transient (first 3.5ms noise click)
            let click = 0;
            if (kickAgeSecs < 0.0035) {
                click = (Math.random() * 2 - 1) * 0.16 * (1 - kickAgeSecs / 0.0035);
            }
            
            // Sub-bass amplitude decay envelope
            const ampEnv = Math.exp(-kickAgeSecs / 0.42);
            
            // Warm waveshaper saturation
            kickSample = Math.tanh((sine + click) * ampEnv * 1.6);
        }

        // ----------------------------------------------------
        // 2. Synthesize 303 Bassline
        // ----------------------------------------------------
        const targetFreq = BASS_SEQUENCE[stepIndex];
        const isAccent = BASS_ACCENTS[stepIndex];
        const isSlide = BASS_SLIDES[stepIndex];

        // Slide glide frequency interpolation
        const slideSpeed = isSlide ? 0.08 : 0.005;
        currentFreq += (targetFreq - currentFreq) * slideSpeed;

        // Sawtooth wave oscillator
        phase303 += currentFreq / SAMPLE_RATE;
        if (phase303 >= 1.0) phase303 -= 2.0;
        let oscSample = phase303;

        // Cutoff envelope decay sweep
        const decayTime = isAccent ? envDecay * 1.5 : envDecay;
        const decaySamples = decayTime * SAMPLE_RATE;
        const envVal = Math.exp(-sampleInStep / decaySamples);
        const dynamicCutoff = filterCutoff + (isAccent ? envMod * 1.6 : envMod) * envVal;

        // Diode-ladder 4-Pole Lowpass
        const cutoffCoeff = (2 * Math.PI * dynamicCutoff) / SAMPLE_RATE;
        const input = oscSample - resonance * p3;
        p0 += cutoffCoeff * (input - p0);
        p1 += cutoffCoeff * (p0 - p1);
        p2 += cutoffCoeff * (p1 - p2);
        p3 += cutoffCoeff * (p2 - p3);
        let filterOutput = p3;

        // Triode valve saturation waveshaper
        const bassGain = isAccent ? 4.2 : 2.4;
        let bassSample = Math.tanh(filterOutput * bassGain);

        // VCA Gate envelope
        const gateDuration = isSlide ? stepDurationSamples : Math.floor(stepDurationSamples * 0.7);
        let amp = 0;
        if (sampleInStep < gateDuration) {
            amp = isAccent ? 0.4 : 0.26;
        } else {
            const releaseSamples = 0.02 * SAMPLE_RATE;
            amp = (isAccent ? 0.4 : 0.26) * Math.exp(-(sampleInStep - gateDuration) / releaseSamples);
        }
        bassSample = bassSample * amp;

        // ----------------------------------------------------
        // 3. Mix & Clip
        // ----------------------------------------------------
        // Blend Kick and Bass (adjusting volume so they mix nicely without clipping)
        const mixed = (kickSample * 0.6) + (bassSample * 0.4);
        audioBuffer[i] = Math.tanh(mixed); // Final safety limiter saturation
    }

    // Write combined WAV file
    const wavPath = path.join(__dirname, "../teddy303_808_acid_beat.wav");
    const buffer = writeWavFile(audioBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`Acid Beat WAV successfully generated at: ${wavPath}`);
}

function writeWavFile(audioBuffer) {
    const numSamples = audioBuffer.length;
    const buffer = Buffer.alloc(44 + numSamples * 2);

    buffer.write("RIFF", 0, "ascii");
    buffer.writeUInt32LE(36 + numSamples * 2, 4);
    buffer.write("WAVE", 8, "ascii");
    
    buffer.write("fmt ", 12, "ascii");
    buffer.writeUInt32LE(16, 16);
    buffer.writeUInt16LE(1, 20);
    buffer.writeUInt16LE(1, 22);
    buffer.writeUInt32LE(SAMPLE_RATE, 24);
    buffer.writeUInt32LE(SAMPLE_RATE * 2, 28);
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

    return buffer;
}

main();
