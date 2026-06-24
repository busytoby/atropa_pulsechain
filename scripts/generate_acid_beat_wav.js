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

// TR-808 Kick Triggers: Four-on-the-floor beat
const KICK_SEQUENCE = [
    1, 0, 0, 0,  1, 0, 0, 0,
    1, 0, 0, 0,  1, 0, 0, 0
];

// TR-808 Snare Triggers: Classic backbeat on steps 4 and 12
const SNARE_SEQUENCE = [
    0, 0, 0, 0,  1, 0, 0, 0,
    0, 0, 0, 0,  1, 0, 0, 0
];

// TR-808 Closed & Open Hihat Sequences
const CLOSED_HH_SEQUENCE = [
    0, 1, 0, 1,  0, 1, 0, 1,
    0, 1, 0, 1,  0, 1, 0, 1
];
const OPEN_HH_SEQUENCE = [
    0, 0, 1, 0,  0, 0, 1, 0,
    0, 0, 1, 0,  0, 0, 1, 0
];

// TR-808 Cowbell Triggers: Syncopated accents
const COWBELL_SEQUENCE = [
    0, 0, 1, 0,  0, 0, 0, 1,
    0, 1, 0, 0,  0, 0, 1, 0
];

// TR-808 Clap Triggers: Syncopated handclaps (1 = single, 2 = double/flam)
const CLAP_SEQUENCE = [
    0, 0, 0, 0,  0, 0, 0, 0,
    1, 0, 0, 0,  0, 0, 2, 1
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
    console.log("GENERATING MODERNIZED Z-TRAVELLER STEREO ACID BEAT WAV");
    console.log("===============================================================");

    // Left and Right channels for Stereo mapping
    const leftBuffer = new Float32Array(TOTAL_SAMPLES);
    const rightBuffer = new Float32Array(TOTAL_SAMPLES);

    const tempoBPM = 120;
    const stepDurationSamples = Math.floor(SAMPLE_RATE * (60 / tempoBPM) / 4); // 16th note step length

    // Pre-calculate step start samples with 32% swing (heavy shuffle)
    const swingAmount = 0.32;
    const stepStartSamples = [];
    let accumSample = 0;
    const totalStepsInTrack = Math.ceil(TOTAL_SAMPLES / stepDurationSamples) + 2;
    for (let step = 0; step < totalStepsInTrack; step++) {
        stepStartSamples.push(accumSample);
        const isEven = (step % 2 === 0);
        const stepLen = isEven 
            ? Math.floor(stepDurationSamples * (1 + swingAmount))
            : Math.floor(stepDurationSamples * (1 - swingAmount));
        accumSample += stepLen;
    }

    // 303 State
    let currentFreq = BASS_SEQUENCE[0];
    let filterCutoff = 350; 
    let envDecay = 0.16; 
    let resonance = 0.88; 
    let envMod = 1200; 
    let phase303 = 0;
    let p0 = 0, p1 = 0, p2 = 0, p3 = 0; // Lowpass poles
    
    // Smooth envelope state for slide support
    let currentEnvVal = 0;
    let currentAmpVal = 0;
    let lastNoteTriggerSample = -999999;
    let lastGateDuration = 0;

    // 808 State (Track sample index of the last triggers)
    let lastKickTriggerSample = -999999; 
    let lastSnareTriggerSample = -999999;
    let lastClosedHhTriggerSample = -999999;
    let lastOpenHhTriggerSample = -999999;
    let lastCowbellTriggerSample = -999999;
    let lastClapTriggerSample = -999999;
    let lastClapType = 0; // 1 = single, 2 = double (flam)
    
    // Highpass Filter State for Hihat (centered at 7kHz)
    let hp_in_prev = 0;
    let hp_out_prev = 0;
    const hp_rc = 1.0 / (2.0 * Math.PI * 7000.0);
    const hp_dt = 1.0 / SAMPLE_RATE;
    const hp_alpha = hp_rc / (hp_rc + hp_dt);

    // Bandpass Filter State for Snare Noise (centered around 1.5kHz)
    let bp_p0 = 0, bp_p1 = 0;
    const bp_freq = 1500.0;
    const bp_q = 1.2;
    const bp_f = Math.tan(Math.PI * bp_freq / SAMPLE_RATE);
    const bp_r = 1.0 / bp_q;

    // Bandpass Filter State for Cowbell (centered around 800Hz)
    let cb_bp_p0 = 0, cb_bp_p1 = 0;
    const cb_bp_freq = 800.0;
    const cb_bp_q = 1.5;
    const cb_bp_f = Math.tan(Math.PI * cb_bp_freq / SAMPLE_RATE);
    const cb_bp_r = 1.0 / cb_bp_q;

    // Bandpass Filter State for Clap (centered around 1.0kHz)
    let cl_bp_p0 = 0, cl_bp_p1 = 0;
    const cl_bp_freq = 1000.0;
    const cl_bp_q = 1.0;
    const cl_bp_f = Math.tan(Math.PI * cl_bp_freq / SAMPLE_RATE);
    const cl_bp_r = 1.0 / cl_bp_q;

    // Cowbell Oscillators Phase
    let cb_phase1 = 0;
    let cb_phase2 = 0;

    // 808 Hihat Metallic Sound Source: Six detuned square oscillators
    const hhFreqs = [205.3, 369.6, 522.0, 565.1, 675.9, 716.9];
    let hhPhases = [0, 0, 0, 0, 0, 0];

    let currentStep = 0;

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const timeSecs = i / SAMPLE_RATE;

        // ----------------------------------------------------
        // Modernized Z-Traveller Coordinate Modulations
        // ----------------------------------------------------
        // Map trajectory: morph from In-Phase (0 to 4s) to Out-of-Phase (4 to 8s)
        const phaseOffset = timeSecs < 4.0 ? 0.0 : Math.PI;
        
        // Z-Traveller and X-Quaternion oscillations
        const playerX = 150.0 * Math.sin(2.0 * Math.PI * 0.25 * timeSecs);
        const playerZ = 150.0 * Math.sin(2.0 * Math.PI * 0.25 * timeSecs + phaseOffset);

        const qX = playerX / 300.0;
        const qZ = playerZ / 300.0;

        // Track the current step index based on boundaries
        while (currentStep < stepStartSamples.length - 1 && i >= stepStartSamples[currentStep + 1]) {
            currentStep++;
        }

        const stepIndex = currentStep % BASS_SEQUENCE.length;
        const sampleInStep = i - stepStartSamples[currentStep];

        // 3:5 Polyrhythmic index for cowbell (loops every 5 steps instead of 16)
        const cowbellStepIndex = currentStep % 5;

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
            // Trigger cowbell polyrhythmically (step 2 & 4 of the 5-step cycle)
            if (cowbellStepIndex === 2 || cowbellStepIndex === 4) {
                lastCowbellTriggerSample = i;
            }
            if (CLAP_SEQUENCE[stepIndex] > 0) {
                lastClapTriggerSample = i;
                lastClapType = CLAP_SEQUENCE[stepIndex];
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
            const sine1 = Math.sin(2 * Math.PI * 180 * snareAgeSecs);
            const sine2 = Math.sin(2 * Math.PI * 330 * snareAgeSecs);
            const shellEnv = Math.exp(-snareAgeSecs / 0.08);
            const shell = (sine1 * 0.6 + sine2 * 0.4) * shellEnv;

            const rattleEnv = Math.exp(-snareAgeSecs / 0.16);
            const rattle = snareNoise * rattleEnv * 1.1;

            snareSample = Math.tanh(shell + rattle);
        }

        // C. Synthesize Hihats (detuned metallic square waves source + HPF)
        let hhSample = 0;
        const closedHhAge = (i - lastClosedHhTriggerSample) / SAMPLE_RATE;
        const openHhAge = (i - lastOpenHhTriggerSample) / SAMPLE_RATE;

        // Sum detuned square wave oscillators
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

        if (closedHhAge >= 0 && closedHhAge < 0.1) {
            const env = Math.exp(-closedHhAge / 0.035); // 35ms decay
            hhSample += filteredHihat * env * 0.35;
        }
        if (openHhAge >= 0 && openHhAge < 0.5) {
            const env = Math.exp(-openHhAge / 0.22);  // 220ms decay
            hhSample += filteredHihat * env * 0.35;
        }

        // D. Synthesize 808 Cowbell (Dual detuned square wave oscillators + BPF)
        let cowbellSample = 0;
        const cowbellAgeSecs = (i - lastCowbellTriggerSample) / SAMPLE_RATE;

        // Two detuned square waves: 540Hz and 800Hz
        cb_phase1 += 540 / SAMPLE_RATE;
        cb_phase2 += 800 / SAMPLE_RATE;
        if (cb_phase1 >= 1.0) cb_phase1 -= 2.0;
        if (cb_phase2 >= 1.0) cb_phase2 -= 2.0;

        const sq1 = cb_phase1 >= 0.0 ? 1.0 : -1.0;
        const sq2 = cb_phase2 >= 0.0 ? 1.0 : -1.0;
        const cb_mix = (sq1 * 0.5) + (sq2 * 0.5);

        const cb_bp_high = cb_mix - cb_bp_p0 * cb_bp_r - cb_bp_p1;
        const cb_bp_band = cb_bp_p0 + cb_bp_f * cb_bp_high;
        cb_bp_p0 = cb_bp_band;
        cb_bp_p1 = cb_bp_p1 + cb_bp_f * cb_bp_band;
        const filteredCowbell = cb_bp_band;

        if (cowbellAgeSecs >= 0 && cowbellAgeSecs < 0.4) {
            const env = Math.exp(-cowbellAgeSecs / 0.08); // 80ms decay
            cowbellSample = Math.tanh(filteredCowbell * env * 1.5);
        }

        // E. Synthesize 808 Clap (Double-clap / Flam engine spaced by 70ms)
        let clapSample = 0;
        const clapAgeSecs = (i - lastClapTriggerSample) / SAMPLE_RATE;

        // Bandpass filter noise at 1.0kHz
        const cl_bp_high = noise - cl_bp_p0 * cl_bp_r - cl_bp_p1;
        const cl_bp_band = cl_bp_p0 + cl_bp_f * cl_bp_high;
        cl_bp_p0 = cl_bp_band;
        cl_bp_p1 = cl_bp_p1 + cl_bp_f * cl_bp_band;
        const clapNoise = cl_bp_band;

        if (clapAgeSecs >= 0 && clapAgeSecs < 0.5) {
            const env1 = getSingleClapEnvelope(clapAgeSecs);
            let env2 = 0;
            if (lastClapType === 2) {
                env2 = getSingleClapEnvelope(clapAgeSecs - 0.070);
            }
            const clapEnv = Math.max(env1, env2);
            clapSample = Math.tanh(clapNoise * clapEnv * 1.4);
        }

        // ----------------------------------------------------
        // 2. Synthesize 303 Bassline modulated by Z-Traveller
        // ----------------------------------------------------
        const targetFreq = BASS_SEQUENCE[stepIndex];
        const isAccent = BASS_ACCENTS[stepIndex];
        const isSlide = BASS_SLIDES[stepIndex];

        // Trigger note logic: if it's step start and not sliding, reset note envelope
        if (sampleInStep === 0) {
            if (!isSlide) {
                lastNoteTriggerSample = i;
                currentEnvVal = 1.0;
            }
            const actualStepDuration = stepStartSamples[currentStep + 1] - stepStartSamples[currentStep];
            lastGateDuration = isSlide ? actualStepDuration : Math.floor(actualStepDuration * 0.65);
        }

        // Modulation 1: Pitch modulated by X-Quaternion
        const pitchMod = 1.0 + (qX * 0.2);
        const modulatedTargetFreq = targetFreq * pitchMod;

        // Smooth pitch transitions (sliding frequency)
        const slideSpeed = isSlide ? 0.06 : 0.25; // Accelerate normal note attack
        currentFreq += (modulatedTargetFreq - currentFreq) * slideSpeed;

        // Sawtooth phase accumulation
        phase303 += currentFreq / SAMPLE_RATE;
        if (phase303 >= 1.0) phase303 -= 1.0;
        let oscSample = phase303 * 2.0 - 1.0; // Scaled properly

        // Decay note envelopes smoothly over steps
        const decayTime = isAccent ? envDecay * 1.5 : envDecay;
        const decaySamples = decayTime * SAMPLE_RATE;
        currentEnvVal *= Math.exp(-1.0 / decaySamples);

        // Modulation 2: Formant / Filter Cutoff modulated by Z-Traveller
        const formantMod = 1.0 + (qZ * 0.2);
        const dynamicCutoff = (filterCutoff + (isAccent ? envMod * 1.6 : envMod) * currentEnvVal) * formantMod;

        // 4-pole diode ladder simulation
        const cutoffCoeff = (2.0 * Math.PI * dynamicCutoff) / SAMPLE_RATE;
        const input = oscSample - resonance * p3;
        p0 += cutoffCoeff * (input - p0);
        p1 += cutoffCoeff * (p0 - p1);
        p2 += cutoffCoeff * (p1 - p2);
        p3 += cutoffCoeff * (p2 - p3);
        let filterOutput = p3;

        // Bass gain / distortion
        const bassGain = isAccent ? 3.6 : 2.2;
        let bassSample = Math.tanh(filterOutput * bassGain);

        // Smooth gate amplitude to prevent clicks on release
        const ageInStep = i - stepStartSamples[currentStep];
        let targetAmp = 0;
        if (ageInStep < lastGateDuration) {
            targetAmp = isAccent ? 0.35 : 0.22;
        } else {
            const releaseSamples = 0.015 * SAMPLE_RATE;
            targetAmp = (isAccent ? 0.35 : 0.22) * Math.exp(-(ageInStep - lastGateDuration) / releaseSamples);
        }
        currentAmpVal += (targetAmp - currentAmpVal) * 0.08; // Smooth jump protection

        // Blend raw distorted filter out with sub-bass fundamental wave
        const subOsc = Math.sin(2.0 * Math.PI * phase303);
        bassSample = (bassSample * 0.7 + subOsc * 0.3) * currentAmpVal;

        // ----------------------------------------------------
        // 3. Stereo Spatial Placement & Mixing
        // ----------------------------------------------------
        // Pan maps directly to spatial x-position of traveller
        const panVal = Math.max(-1.0, Math.min(1.0, qX));
        
        // Stereo gains
        const leftGain = (1.0 - panVal) / 2.0;
        const rightGain = (1.0 + panVal) / 2.0;

        // Drums mixed slightly centered/wide
        const monoDrums = (kickSample * 0.38) + (snareSample * 0.18) + (hhSample * 0.1) + (cowbellSample * 0.06) + (clapSample * 0.08);
        
        // Dynamic panning applied to the TB-303 Bassline
        const bassL = bassSample * leftGain;
        const bassR = bassSample * rightGain;

        leftBuffer[i] = Math.tanh(monoDrums * 0.45 + bassL * 0.55); 
        rightBuffer[i] = Math.tanh(monoDrums * 0.45 + bassR * 0.55); 
    }

    // Write combined Stereo WAV file
    const wavPath = path.join(__dirname, "../teddy303_808_acid_beat.wav");
    const buffer = writeStereoWavFile(leftBuffer, rightBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`Acid Beat WAV successfully generated at: ${wavPath}`);
}

function writeStereoWavFile(leftBuffer, rightBuffer) {
    const numSamples = leftBuffer.length;
    const numChannels = 2;
    const bytesPerSample = 2; // 16-bit
    const blockAlign = numChannels * bytesPerSample;
    const byteRate = SAMPLE_RATE * blockAlign;
    const subChunk2Size = numSamples * blockAlign;

    const buffer = Buffer.alloc(44 + subChunk2Size);

    buffer.write("RIFF", 0, "ascii");
    buffer.writeUInt32LE(36 + subChunk2Size, 4);
    buffer.write("WAVE", 8, "ascii");
    
    buffer.write("fmt ", 12, "ascii");
    buffer.writeUInt32LE(16, 16);
    buffer.writeUInt16LE(1, 20); // AudioFormat: PCM
    buffer.writeUInt16LE(numChannels, 22);
    buffer.writeUInt32LE(SAMPLE_RATE, 24);
    buffer.writeUInt32LE(byteRate, 28);
    buffer.writeUInt16LE(blockAlign, 32);
    buffer.writeUInt16LE(16, 34); // BitsPerSample: 16
    
    buffer.write("data", 36, "ascii");
    buffer.writeUInt32LE(subChunk2Size, 40);

    let offset = 44;
    for (let i = 0; i < numSamples; i++) {
        // Left Channel
        let sL = Math.max(-1, Math.min(1, leftBuffer[i]));
        let valL = sL < 0 ? sL * 0x8000 : sL * 0x7FFF;
        buffer.writeInt16LE(Math.floor(valL), offset);
        offset += 2;

        // Right Channel
        let sR = Math.max(-1, Math.min(1, rightBuffer[i]));
        let valR = sR < 0 ? sR * 0x8000 : sR * 0x7FFF;
        buffer.writeInt16LE(Math.floor(valR), offset);
        offset += 2;
    }

    return buffer;
}

main();
