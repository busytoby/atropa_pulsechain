const fs = require("fs");
const path = require("path");

const SAMPLE_RATE = 44100;
const DURATION_SECS = 32; // Expanded to a structured 32-second arrangement
const TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS;

// pentatonic minor scale notes
const C2 = 65.41;
const Eb2 = 77.78;
const F2 = 87.31;
const G2 = 98.00;
const Bb2 = 116.54;
const C3 = 130.81;

// 16-step patterns
const BASS_SEQUENCE = [
    C2, C3, 0, Eb2,  F2, 0, G2, Bb2,
    C2, 0, Bb2, F2,  G2, Eb2, C2, 0
];
const BASS_ACCENTS = [
    1, 0, 0, 0,  1, 0, 0, 1,
    1, 0, 0, 0,  1, 0, 1, 0
];
const BASS_SLIDES = [
    0, 1, 0, 0,  1, 0, 0, 1,
    0, 0, 1, 0,  1, 0, 0, 0
];

const KICK_SEQUENCE = [
    1, 0, 0, 1,  0, 0, 1, 0,
    1, 0, 1, 0,  0, 1, 0, 0
];

const SNARE_SEQUENCE = [
    0, 0, 0, 0,  1, 0, 0, 0,
    0, 0, 0, 0,  1, 0, 0, 0
];

const CLOSED_HH_SEQUENCE = [
    1, 1, 0, 1,  1, 1, 0, 1,
    1, 1, 0, 1,  1, 1, 0, 1
];
const OPEN_HH_SEQUENCE = [
    0, 0, 1, 0,  0, 0, 1, 0,
    0, 0, 1, 0,  0, 0, 1, 0
];

const CLAP_SEQUENCE = [
    0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  1, 0, 2, 0
];

function getSingleClapEnvelope(age) {
    if (age < 0) return 0;
    if (age < 0.010) {
        return Math.exp(-age / 0.003);
    } else if (age >= 0.010 && age < 0.020) {
        return Math.exp(-(age - 0.010) / 0.003);
    } else if (age >= 0.020 && age < 0.030) {
        return Math.exp(-(age - 0.020) / 0.003);
    } else {
        return Math.exp(-(age - 0.030) / 0.08);
    }
}

function main() {
    console.log("===============================================================");
    console.log("GENERATING 32-SECOND STRUCTURED BOOTY BASS ARRANGEMENT");
    console.log("===============================================================");

    const audioBuffer = new Float32Array(TOTAL_SAMPLES);
    const tempoBPM = 133; 
    const stepDurationSamples = Math.floor(SAMPLE_RATE * (60 / tempoBPM) / 4);

    let currentFreq = BASS_SEQUENCE[0] || C2;
    let phase303 = 0;
    let p0 = 0, p1 = 0, p2 = 0, p3 = 0;

    let lastKickTriggerSample = -999999; 
    let lastSnareTriggerSample = -999999;
    let lastClosedHhTriggerSample = -999999;
    let lastOpenHhTriggerSample = -999999;
    let lastClapTriggerSample = -999999;
    let lastClapType = 0;
    
    // Filter coefficients
    let hp_in_prev = 0, hp_out_prev = 0;
    const hp_rc = 1.0 / (2.0 * Math.PI * 7500.0);
    const hp_dt = 1.0 / SAMPLE_RATE;
    const hp_alpha = hp_rc / (hp_rc + hp_dt);

    let bp_p0 = 0, bp_p1 = 0;
    const bp_freq = 1450.0;
    const bp_q = 1.1;
    const bp_f = Math.tan(Math.PI * bp_freq / SAMPLE_RATE);
    const bp_r = 1.0 / bp_q;

    let cl_bp_p0 = 0, cl_bp_p1 = 0;
    const cl_bp_freq = 1050.0;
    const cl_bp_q = 0.95;
    const cl_bp_f = Math.tan(Math.PI * cl_bp_freq / SAMPLE_RATE);
    const cl_bp_r = 1.0 / cl_bp_q;

    const hhFreqs = [205.3, 369.6, 522.0, 565.1, 675.9, 716.9];
    let hhPhases = [0, 0, 0, 0, 0, 0];

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const timeSecs = i / SAMPLE_RATE;
        
        // --- Dynamic Arrangement Mutes/Modulations ---
        // 0-8s: Intro
        // 8-16s: Build-up (Introduce Snare and Slide modulations)
        // 16-24s: Drop (All instruments maxed, accents/claps active)
        // 24-32s: Outro (Snare & clap out, filter cutoff sweeps down, fade out)
        let kickMute = 1;
        let snareMute = 0;
        let clapMute = 0;
        let hhMute = 1;
        let bassMute = 1;
        
        let dynamicFilterCutoff = 280;
        let dynamicEnvMod = 1200;
        let dynamicResonance = 0.82;
        let dynamicDecay = 0.12;

        if (timeSecs < 8.0) {
            // Intro: simple kick and hats, low filter cutoff (dark bass), no claps/snares
            snareMute = 0;
            clapMute = 0;
            dynamicFilterCutoff = 200;
            dynamicResonance = 0.7;
            dynamicEnvMod = 600;
        } else if (timeSecs >= 8.0 && timeSecs < 16.0) {
            // Build-up: Introduce snare, open hats, filter opens up gradually
            snareMute = 1;
            clapMute = 0;
            const progress = (timeSecs - 8.0) / 8.0;
            dynamicFilterCutoff = 200 + progress * 150; // Sweeps up
            dynamicEnvMod = 600 + progress * 600;
            dynamicResonance = 0.7 + progress * 0.15;
        } else if (timeSecs >= 16.0 && timeSecs < 24.0) {
            // Drop: Maximum energy, all drums on, resonance high
            snareMute = 1;
            clapMute = 1;
            dynamicFilterCutoff = 360;
            dynamicEnvMod = 1400;
            dynamicResonance = 0.88;
        } else {
            // Outro: Claps & Snares stop, cutoff sweeps down, master fade out
            snareMute = 0;
            clapMute = 0;
            const progress = (timeSecs - 24.0) / 8.0;
            dynamicFilterCutoff = 360 - progress * 200; // Sweeps down
            dynamicResonance = 0.88 - progress * 0.2;
            dynamicEnvMod = 1400 - progress * 900;
            // Master fade
            kickMute = 1.0 - progress;
            hhMute = 1.0 - progress;
            bassMute = 1.0 - progress;
        }

        const stepIndex = Math.floor(i / stepDurationSamples) % BASS_SEQUENCE.length;
        const sampleInStep = i % stepDurationSamples;

        if (sampleInStep === 0) {
            if (KICK_SEQUENCE[stepIndex] === 1 && kickMute > 0) lastKickTriggerSample = i;
            if (SNARE_SEQUENCE[stepIndex] === 1 && snareMute > 0) lastSnareTriggerSample = i;
            if (CLOSED_HH_SEQUENCE[stepIndex] === 1 && hhMute > 0) lastClosedHhTriggerSample = i;
            if (OPEN_HH_SEQUENCE[stepIndex] === 1 && hhMute > 0) lastOpenHhTriggerSample = i;
            if (CLAP_SEQUENCE[stepIndex] > 0 && clapMute > 0) {
                lastClapTriggerSample = i;
                lastClapType = CLAP_SEQUENCE[stepIndex];
            }
        }

        // Kick
        let kickSample = 0;
        const kickAge = (i - lastKickTriggerSample) / SAMPLE_RATE;
        if (kickAge >= 0 && kickAge < 1.0) {
            const pitchEnv = Math.exp(-kickAge / 0.04);
            const kickFreq = 46 + 104 * pitchEnv;
            const sine = Math.sin(2 * Math.PI * kickFreq * kickAge);
            let click = 0;
            if (kickAge < 0.003) click = (Math.random() * 2 - 1) * 0.18 * (1 - kickAge / 0.003);
            const ampEnv = Math.exp(-kickAge / 0.38);
            kickSample = Math.tanh((sine + click) * ampEnv * 1.7) * kickMute;
        }

        // Snare
        let snareSample = 0;
        const snareAge = (i - lastSnareTriggerSample) / SAMPLE_RATE;
        const noise = Math.random() * 2 - 1;
        const bp_high = noise - bp_p0 * bp_r - bp_p1;
        const bp_band = bp_p0 + bp_f * bp_high;
        bp_p0 = bp_band;
        bp_p1 = bp_p1 + bp_f * bp_band;

        if (snareAge >= 0 && snareAge < 0.45) {
            const sine1 = Math.sin(2 * Math.PI * 180 * snareAge);
            const sine2 = Math.sin(2 * Math.PI * 330 * snareAge);
            const shell = (sine1 * 0.65 + sine2 * 0.35) * Math.exp(-snareAge / 0.07);
            const rattle = bp_band * Math.exp(-snareAge / 0.15) * 1.0;
            snareSample = Math.tanh(shell + rattle) * snareMute;
        }

        // Hihats
        let hhSample = 0;
        const closedHhAge = (i - lastClosedHhTriggerSample) / SAMPLE_RATE;
        const openHhAge = (i - lastOpenHhTriggerSample) / SAMPLE_RATE;
        let metallicSource = 0;
        for (let j = 0; j < 6; j++) {
            hhPhases[j] += hhFreqs[j] / SAMPLE_RATE;
            if (hhPhases[j] >= 1.0) hhPhases[j] -= 2.0;
            metallicSource += hhPhases[j] >= 0.0 ? 1.0 : -1.0;
        }
        metallicSource /= 6.0;
        const filteredHihat = hp_alpha * (hp_out_prev + metallicSource - hp_in_prev);
        hp_in_prev = metallicSource;
        hp_out_prev = filteredHihat;

        if (closedHhAge >= 0 && closedHhAge < 0.08) {
            hhSample += filteredHihat * Math.exp(-closedHhAge / 0.03) * 0.35;
        }
        if (openHhAge >= 0 && openHhAge < 0.45) {
            hhSample += filteredHihat * Math.exp(-openHhAge / 0.18) * 0.35;
        }
        hhSample *= hhMute;

        // Clap
        let clapSample = 0;
        const clapAge = (i - lastClapTriggerSample) / SAMPLE_RATE;
        const cl_bp_high = noise - cl_bp_p0 * cl_bp_r - cl_bp_p1;
        const cl_bp_band = cl_bp_p0 + cl_bp_f * cl_bp_high;
        cl_bp_p0 = cl_bp_band;
        cl_bp_p1 = cl_bp_p1 + cl_bp_f * cl_bp_band;

        if (clapAge >= 0 && clapAge < 0.5) {
            const env1 = getSingleClapEnvelope(clapAge);
            let env2 = 0;
            if (lastClapType === 2) {
                env2 = getSingleClapEnvelope(clapAge - 0.060);
            }
            clapSample = Math.tanh(cl_bp_band * Math.max(env1, env2) * 1.35) * clapMute;
        }

        // 303 Bassline
        const targetFreq = BASS_SEQUENCE[stepIndex];
        const isAccent = BASS_ACCENTS[stepIndex];
        const isSlide = BASS_SLIDES[stepIndex];

        if (targetFreq > 0) {
            const slideSpeed = isSlide ? 0.08 : 0.006;
            currentFreq += (targetFreq - currentFreq) * slideSpeed;
        }

        phase303 += currentFreq / SAMPLE_RATE;
        if (phase303 >= 1.0) phase303 -= 2.0;
        let oscSample = phase303;

        const decaySamples = (isAccent ? dynamicDecay * 1.4 : dynamicDecay) * SAMPLE_RATE;
        const envVal = Math.exp(-sampleInStep / decaySamples);
        const dynamicCutoff = dynamicFilterCutoff + (isAccent ? dynamicEnvMod * 1.5 : dynamicEnvMod) * envVal;
        const cutoffCoeff = (2 * Math.PI * dynamicCutoff) / SAMPLE_RATE;
        
        const filterInput = oscSample - dynamicResonance * p3;
        p0 += cutoffCoeff * (filterInput - p0);
        p1 += cutoffCoeff * (p0 - p1);
        p2 += cutoffCoeff * (p1 - p2);
        p3 += cutoffCoeff * (p2 - p3);
        let filterOutput = p3;

        let bassSample = Math.tanh(filterOutput * (isAccent ? 4.0 : 2.5));
        const gateDuration = isSlide ? stepDurationSamples : Math.floor(stepDurationSamples * 0.65);
        let amp = 0;
        if (targetFreq === 0) {
            amp = 0;
        } else if (sampleInStep < gateDuration) {
            amp = isAccent ? 0.38 : 0.24;
        } else {
            amp = (isAccent ? 0.38 : 0.24) * Math.exp(-(sampleInStep - gateDuration) / (0.015 * SAMPLE_RATE));
        }
        bassSample = bassSample * amp * bassMute;

        // Mix & clip
        const mixed = (kickSample * 0.4) + (snareSample * 0.18) + (bassSample * 0.22) + (hhSample * 0.08) + (clapSample * 0.07);
        audioBuffer[i] = Math.tanh(mixed);
    }

    const wavPath = path.join(__dirname, "../teddy303_808_booty_bass.wav");
    const buffer = writeWavFile(audioBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`✓ Structured Booty Bass track successfully generated at: ${wavPath}`);
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
