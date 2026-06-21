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

// TR-808 Snare Triggers: Classic backbeat on steps 4 and 12
const SNARE_SEQUENCE = [
    0, 0, 0, 0,  1, 0, 0, 0,
    0, 0, 0, 0,  1, 0, 0, 0
];

// TR-808 Closed & Open Hihat Sequences (featuring off-beat Open Hihats)
const CLOSED_HH_SEQUENCE = [
    0, 1, 0, 1,  0, 1, 0, 1,
    0, 1, 0, 1,  0, 1, 0, 1
];
const OPEN_HH_SEQUENCE = [
    0, 0, 1, 0,  0, 0, 1, 0,
    0, 0, 1, 0,  0, 0, 1, 0
];

function main() {
    console.log("===============================================================");
    console.log("GENERATING COMBINED 303 ACID BASS & 808 BEAT WAV WITH SNARE & HH");
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

    // 808 State (Track sample index of the last triggers)
    let lastKickTriggerSample = -999999; 
    let lastSnareTriggerSample = -999999;
    let lastClosedHhTriggerSample = -999999;
    let lastOpenHhTriggerSample = -999999;

    // Highpass Filter State for Hihat Noise
    let hp_in_prev = 0;
    let hp_out_prev = 0;
    const hp_rc = 1.0 / (2.0 * Math.PI * 7000.0); // 7kHz cutoff
    const hp_dt = 1.0 / SAMPLE_RATE;
    const hp_alpha = hp_rc / (hp_rc + hp_dt);

    // Bandpass Filter State for Snare Noise (centered around 1.5kHz)
    let bp_p0 = 0, bp_p1 = 0;
    const bp_freq = 1500.0;
    const bp_q = 1.2;
    const bp_f = Math.tan(Math.PI * bp_freq / SAMPLE_RATE);
    const bp_r = 1.0 / bp_q;
    const bp_a1 = 1.0 / (1.0 + bp_f * bp_r + bp_f * bp_f);
    const bp_a2 = bp_f * bp_r * bp_a1;
    const bp_a3 = bp_f * bp_f * bp_a1;

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const stepIndex = Math.floor(i / stepDurationSamples) % BASS_SEQUENCE.length;
        const sampleInStep = i % stepDurationSamples;

        // ----------------------------------------------------
        // 1. Trigger 808 Drums
        // ----------------------------------------------------
        if (sampleInStep === 0) {
            if (KICK_SEQUENCE[stepIndex] === 1) {
                lastKickTriggerSample = i;
            }
            if (SNARE_SEQUENCE[stepIndex] === 1) {
                lastSnareTriggerSample = i;
            }
            if (CLOSED_HH_SEQUENCE[stepIndex] === 1) {
                lastClosedHhTriggerSample = i;
            }
            if (OPEN_HH_SEQUENCE[stepIndex] === 1) {
                lastOpenHhTriggerSample = i;
            }
        }

        // A. Synthesize 808 Kick
        let kickSample = 0;
        const kickAgeSecs = (i - lastKickTriggerSample) / SAMPLE_RATE;
        if (kickAgeSecs >= 0 && kickAgeSecs < 1.0) {
            const pitchEnv = Math.exp(-kickAgeSecs / 0.038);
            const kickFreq = 48 + 102 * pitchEnv;
            const kickPhase = 2 * Math.PI * kickFreq * kickAgeSecs;
            const sine = Math.sin(kickPhase);
            
            let click = 0;
            if (kickAgeSecs < 0.0035) {
                click = (Math.random() * 2 - 1) * 0.16 * (1 - kickAgeSecs / 0.0035);
            }
            
            const ampEnv = Math.exp(-kickAgeSecs / 0.42);
            kickSample = Math.tanh((sine + click) * ampEnv * 1.6);
        }

        // B. Synthesize 808 Snare (Sine Resonators + Bandpass Noise)
        let snareSample = 0;
        const snareAgeSecs = (i - lastSnareTriggerSample) / SAMPLE_RATE;
        const noise = Math.random() * 2 - 1;

        // 2-pole resonant bandpass filter on noise for snare rattle
        const bp_high = noise - bp_p0 * bp_r - bp_p1;
        const bp_band = bp_p0 + bp_f * bp_high;
        bp_p0 = bp_band;
        bp_p1 = bp_p1 + bp_f * bp_band;
        const snareNoise = bp_band;

        if (snareAgeSecs >= 0 && snareAgeSecs < 0.5) {
            // Snare Shell Tones (180Hz & 330Hz sine waves)
            const sine1 = Math.sin(2 * Math.PI * 180 * snareAgeSecs);
            const sine2 = Math.sin(2 * Math.PI * 330 * snareAgeSecs);
            const shellEnv = Math.exp(-snareAgeSecs / 0.08);
            const shell = (sine1 * 0.6 + sine2 * 0.4) * shellEnv;

            // Rattle noise envelope (longer decay)
            const rattleEnv = Math.exp(-snareAgeSecs / 0.16);
            const rattle = snareNoise * rattleEnv * 1.1;

            snareSample = Math.tanh(shell + rattle);
        }

        // C. Synthesize Hihats (Noise Source + HPF Filter + Decay Envelope)
        let hhSample = 0;
        const closedHhAge = (i - lastClosedHhTriggerSample) / SAMPLE_RATE;
        const openHhAge = (i - lastOpenHhTriggerSample) / SAMPLE_RATE;

        const filteredNoise = hp_alpha * (hp_out_prev + noise - hp_in_prev);
        hp_in_prev = noise;
        hp_out_prev = filteredNoise;

        if (closedHhAge >= 0 && closedHhAge < 0.1) {
            const env = Math.exp(-closedHhAge / 0.035); // 35ms decay
            hhSample += filteredNoise * env * 0.18;
        }
        if (openHhAge >= 0 && openHhAge < 0.5) {
            const env = Math.exp(-openHhAge / 0.22);  // 220ms decay
            hhSample += filteredNoise * env * 0.18;
        }

        // ----------------------------------------------------
        // 2. Synthesize 303 Bassline
        // ----------------------------------------------------
        const targetFreq = BASS_SEQUENCE[stepIndex];
        const isAccent = BASS_ACCENTS[stepIndex];
        const isSlide = BASS_SLIDES[stepIndex];

        const slideSpeed = isSlide ? 0.08 : 0.005;
        currentFreq += (targetFreq - currentFreq) * slideSpeed;

        phase303 += currentFreq / SAMPLE_RATE;
        if (phase303 >= 1.0) phase303 -= 2.0;
        let oscSample = phase303;

        const decayTime = isAccent ? envDecay * 1.5 : envDecay;
        const decaySamples = decayTime * SAMPLE_RATE;
        const envVal = Math.exp(-sampleInStep / decaySamples);
        const dynamicCutoff = filterCutoff + (isAccent ? envMod * 1.6 : envMod) * envVal;

        const cutoffCoeff = (2 * Math.PI * dynamicCutoff) / SAMPLE_RATE;
        const input = oscSample - resonance * p3;
        p0 += cutoffCoeff * (input - p0);
        p1 += cutoffCoeff * (p0 - p1);
        p2 += cutoffCoeff * (p1 - p2);
        p3 += cutoffCoeff * (p2 - p3);
        let filterOutput = p3;

        const bassGain = isAccent ? 4.2 : 2.4;
        let bassSample = Math.tanh(filterOutput * bassGain);

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
        // Blending Kick, Snare, Hihat, and 303 Bassline
        const mixed = (kickSample * 0.4) + (snareSample * 0.25) + (bassSample * 0.25) + (hhSample * 0.1);
        audioBuffer[i] = Math.tanh(mixed); 
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
