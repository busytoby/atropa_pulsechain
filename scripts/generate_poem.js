const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

class Resonator {
    constructor(freq, bandwidth, sampleRate = 44100) {
        this.sampleRate = sampleRate;
        this.y1 = 0.0;
        this.y2 = 0.0;
        this.setFrequency(freq, bandwidth);
    }

    setFrequency(freq, bandwidth = 1200.0) {
        this.r = Math.exp(-Math.PI * bandwidth / this.sampleRate);
        this.w = 2.0 * Math.PI * freq / this.sampleRate;
        this.c1 = 2.0 * this.r * Math.cos(this.w);
        this.c2 = -this.r * this.r;
        this.c3 = 1.0 - this.r * this.r;
    }

    process(x) {
        const y = this.c1 * this.y1 + this.c2 * this.y2 + this.c3 * x;
        this.y2 = this.y1;
        this.y1 = y;
        return y;
    }
}

class HighPassFilter {
    constructor(cutoff, sampleRate = 44100) {
        this.sampleRate = sampleRate;
        this.x1 = 0.0;
        this.y1 = 0.0;
        this.setCutoff(cutoff);
    }

    setCutoff(cutoff) {
        const rc = 1.0 / (2.0 * Math.PI * cutoff);
        const dt = 1.0 / this.sampleRate;
        this.alpha = rc / (rc + dt);
    }

    process(x) {
        const y = this.alpha * (this.y1 + x - this.x1);
        this.x1 = x;
        this.y1 = y;
        return y;
    }
}

class LowPassFilter {
    constructor(cutoff, sampleRate = 44100) {
        this.sampleRate = sampleRate;
        this.y1 = 0.0;
        this.setCutoff(cutoff);
    }

    setCutoff(cutoff) {
        const rc = 1.0 / (2.0 * Math.PI * cutoff);
        const dt = 1.0 / this.sampleRate;
        this.alpha = dt / (rc + dt);
    }

    process(x) {
        const y = this.y1 + this.alpha * (x - this.y1);
        this.y1 = y;
        return y;
    }
}

class BandPassFilter {
    constructor(freq, bandwidth, sampleRate = 44100) {
        this.sampleRate = sampleRate;
        this.x1 = 0.0;
        this.x2 = 0.0;
        this.y1 = 0.0;
        this.y2 = 0.0;
        this.setParams(freq, bandwidth);
    }

    setParams(freq, bandwidth) {
        const w0 = 2.0 * Math.PI * freq / this.sampleRate;
        const q = Math.max(0.1, freq / bandwidth);
        const alpha = Math.sin(w0) / (2.0 * q);
        const cosw0 = Math.cos(w0);
        
        this.b0 = alpha;
        this.b1 = 0.0;
        this.b2 = -alpha;
        this.a0 = 1.0 + alpha;
        this.a1 = -2.0 * cosw0;
        this.a2 = 1.0 - alpha;
    }

    process(x) {
        const y = (this.b0 * x + this.b2 * this.x2 - this.a1 * this.y1 - this.a2 * this.y2) / this.a0;
        this.x2 = this.x1;
        this.x1 = x;
        this.y2 = this.y1;
        this.y1 = y;
        return y;
    }
}

class MoogLadderFilter {
    constructor(sampleRate = 44100) {
        this.sampleRate = sampleRate;
        this.stage = new Float32Array(4);
        this.stageOld = new Float32Array(4);
        this.cutoff = 1000.0;
        this.resonance = 0.1;
        this.updateCoefficients();
    }

    setParams(cutoff, resonance) {
        this.cutoff = Math.max(10.0, Math.min(20000.0, cutoff));
        this.resonance = Math.max(0.0, Math.min(3.99, resonance));
        this.updateCoefficients();
    }

    updateCoefficients() {
        const fc = (2.0 * this.cutoff) / this.sampleRate;
        const x = fc * (1.0 - fc);
        this.g = 1.0 - Math.exp(-2.0 * Math.PI * x);
        this.k = this.resonance;
    }

    process(input) {
        const vt = 4.0 * this.k * this.stageOld[3];
        const resInput = input - vt;
        
        this.stage[0] = this.stageOld[0] + this.g * (Math.tanh(resInput) - Math.tanh(this.stageOld[0]));
        this.stage[1] = this.stageOld[1] + this.g * (Math.tanh(this.stage[0]) - Math.tanh(this.stageOld[1]));
        this.stage[2] = this.stageOld[2] + this.g * (Math.tanh(this.stage[1]) - Math.tanh(this.stageOld[2]));
        this.stage[3] = this.stageOld[3] + this.g * (Math.tanh(this.stage[2]) - Math.tanh(this.stageOld[3]));

        for (let i = 0; i < 4; i++) {
            this.stageOld[i] = this.stage[i];
        }

        return this.stage[3];
    }
}


function randomNoise(seed) {
    const nextSeed = (seed * 1103515245 + 12345) & 0x7fffffff;
    const val = (nextSeed / 2147483648.0) * 2.0 - 1.0;
    return { val, nextSeed };
}

function synthesizeSequence(phonemeSequence, params, sampleRate = 44100) {
    const samples = [];
    
    // 1.5s silent lead-in to bypass media player startup latency/clipping
    const leadInSize = Math.floor(sampleRate * 1.5);
    for (let i = 0; i < leadInSize; i++) {
        samples.push(0.0);
    }

    let noiseSeed = 42;
    let phi = 0.0;
    let glottalFilterState = 0.0;

    const resF1 = new Resonator(500.0, 80.0, sampleRate);
    const resF2 = new Resonator(1500.0, 120.0, sampleRate);
    const resF3 = new Resonator(2500.0, 160.0, sampleRate);
    const hpFrication = new HighPassFilter(4500.0, sampleRate);
    const lpFrication = new LowPassFilter(8000.0, sampleRate);

    let f1Acc = 500.0;
    let f2Acc = 1500.0;
    let f3Acc = 2500.0;
    let pitchAcc = 120.0;
    let hpCutoffAcc = 4500.0;

    const numSamplesWordGap = Math.floor(sampleRate * 0.16);
    let totalSamples = 0;
    for (const char of phonemeSequence) {
        if (char === 'GAP') {
            totalSamples += numSamplesWordGap;
        } else {
            const dur = params.durations[char] || 80;
            totalSamples += Math.floor(sampleRate * (dur / 1000.0));
        }
    }

    let sampleCounter = 0;
    const delayLen = Math.floor(params.trachea_delay || 44);
    const delayLine = new Float32Array(delayLen);
    let delayIdx = 0;

    for (let idx = 0; idx < phonemeSequence.length; idx++) {
        const char = phonemeSequence[idx];

        if (char === 'GAP') {
            for (let g = 0; g < numSamplesWordGap; g++) {
                // Decay pitch accumulator to baseline
                const sentenceProgress = sampleCounter / totalSamples;
                const declination = 135.0 - 45.0 * sentenceProgress;
                pitchAcc += (declination - pitchAcc) * 0.0018;

                // Process resonators with zero input to allow natural decay
                const outF1 = resF1.process(0.0);
                const outF2 = resF2.process(0.0);
                const outF3 = resF3.process(0.0);
                const vocalTractOutput = outF1 * 1.0 - outF2 * 0.35 + outF3 * 0.15;

                const echoedSig = vocalTractOutput + params.echo_gain * delayLine[delayIdx];
                delayLine[delayIdx] = echoedSig;
                delayIdx = (delayIdx + 1) % delayLen;

                const fricationOutput = hpFrication.process(0.0) * 0.0;
                const finalSample = (echoedSig + fricationOutput) * 0.15;

                samples.push(finalSample);
                sampleCounter++;
            }
            continue;
        }

        const durMs = params.durations[char] || 80;
        const f1Target = params.f1[char] || 500.0;
        const f2Target = params.f2[char] || 1500.0;
        const f3Target = params.f3[char] || 2500.0;
        const pitchTarget = params.pitch[char] || 120.0;
        const hpCutoffTarget = params.frication_cutoff[char] || 4500.0;
        
        let voicingTarget = params.voicing[char] || 0.0;
        let aspirationTarget = params.aspiration[char] || 0.0;
        let fricationTarget = params.frication[char] || 0.0;
        const glideFactor = params.glide_factor[char] || 1.0;

        const numSamplesSound = Math.floor(sampleRate * (durMs / 1000.0));
        const isPlosive = (char === 'B' || char === 'G' || char === 'D' || char === 'T' || char === 'P');

        for (let s = 0; s < numSamplesSound; s++) {
            const sentenceProgress = sampleCounter / totalSamples;
            const declination = 135.0 - 45.0 * sentenceProgress;
            
            // 5.2Hz triangle vibrato LFO and pitch jitter
            const lfoPhase = (sampleCounter * 5.2) / sampleRate;
            const lfoVal = Math.sin(2.0 * Math.PI * lfoPhase);
            const jitterVal = (Math.random() * 2.0 - 1.0) * 0.015;
            
            const basePitchInst = pitchTarget > 0 ? declination + (pitchTarget - 120.0) : declination;
            const pitchInst = basePitchInst * (1.0 + 0.012 * lfoVal + jitterVal);

            const glideSpeed = 0.0018 * glideFactor;
            f1Acc += (f1Target - f1Acc) * glideSpeed;
            f2Acc += (f2Target - f2Acc) * glideSpeed;
            f3Acc += (f3Target - f3Acc) * glideSpeed;
            pitchAcc += (pitchInst - pitchAcc) * glideSpeed;
            hpCutoffAcc += (hpCutoffTarget - hpCutoffAcc) * glideSpeed;

            resF1.setFrequency(f1Acc, 80.0);
            resF2.setFrequency(f2Acc, 120.0);
            resF3.setFrequency(f3Acc, 160.0);

            let currentVoicing = voicingTarget;
            let currentAspiration = aspirationTarget;
            let currentFrication = fricationTarget;

            if (isPlosive) {
                if (s < numSamplesSound * 0.75) {
                    currentVoicing = 0.0;
                    currentAspiration = 0.0;
                    currentFrication = 0.0;
                } else {
                    currentVoicing = voicingTarget * 0.2;
                    currentAspiration = 0.3;
                    currentFrication = 0.6;
                }
            }

            phi = (phi + 2.0 * Math.PI * pitchAcc / sampleRate) % (2.0 * Math.PI);
            const rawGlottal = phi / Math.PI - 1.0;
            glottalFilterState = 0.95 * glottalFilterState + 0.05 * rawGlottal;
            const glottalSource = glottalFilterState;

            const noiseRes = randomNoise(noiseSeed);
            const whiteNoise = noiseRes.val;
            noiseSeed = noiseRes.nextSeed;

            const excitation = glottalSource * currentVoicing + whiteNoise * currentAspiration;

            const outF1 = resF1.process(excitation);
            const outF2 = resF2.process(excitation);
            const outF3 = resF3.process(excitation);

            const vocalTractOutput = outF1 * 1.0 - outF2 * 0.35 + outF3 * 0.15;
            
            const echoedSig = vocalTractOutput + params.echo_gain * delayLine[delayIdx];
            delayLine[delayIdx] = echoedSig;
            delayIdx = (delayIdx + 1) % delayLen;

            let fricationOutput = 0.0;
            if (char === 'F' || char === 'DH' || char === 'B' || char === 'D' || char === 'G' || char === 'P') {
                lpFrication.setCutoff(hpCutoffAcc);
                fricationOutput = lpFrication.process(whiteNoise) * currentFrication * 1.5;
            } else {
                hpFrication.setCutoff(hpCutoffAcc);
                fricationOutput = hpFrication.process(whiteNoise) * currentFrication * 1.5;
            }

            const finalSample = (echoedSig + fricationOutput) * 0.15;
            samples.push(finalSample);
            sampleCounter++;
        }
        if (char !== 'GAP') {
            const lastSamples = samples.slice(-numSamplesSound);
            let sumSq = 0;
            for (let sVal of lastSamples) { sumSq += sVal * sVal; }
            const rms = Math.sqrt(sumSq / numSamplesSound);
            console.log(`[DEBUG] Phoneme ${char}: samples=${numSamplesSound}, RMS=${rms.toFixed(5)}`);
        }
    }

    return samples;
}

function saveWav(samples, filename, sampleRate = 44100) {
    const numSamples = samples.length;
    const header = Buffer.alloc(44);
    
    header.write("RIFF", 0);
    header.writeUInt32LE(36 + numSamples * 2, 4);
    header.write("WAVE", 8);
    header.write("fmt ", 12);
    header.writeUInt32LE(16, 16);
    header.writeUInt16LE(1, 20);
    header.writeUInt16LE(1, 22);
    header.writeUInt32LE(sampleRate, 24);
    header.writeUInt32LE(sampleRate * 2, 28);
    header.writeUInt16LE(2, 32);
    header.writeUInt16LE(16, 34);
    header.write("data", 36);
    header.writeUInt32LE(numSamples * 2, 40);

    const body = Buffer.alloc(numSamples * 2);
    for (let i = 0; i < numSamples; i++) {
        const val = Math.max(-1.0, Math.min(1.0, Math.tanh(samples[i])));
        const pcm = val >= 0 ? Math.floor(val * 32767) : Math.floor(val * 32768);
        body.writeInt16LE(pcm, i * 2);
    }

    const fd = fs.openSync(filename, "w");
    fs.writeSync(fd, header);
    fs.writeSync(fd, body);
    fs.closeSync(fd);
    console.log(`[AUDIO] Saved audio to: ${filename}`);
}

const params = {
    trachea_delay: 44,
    echo_gain: 0.15,
    durations: {
        'H': 80, 'E': 160, 'L': 100, 'O': 200,
        'W': 90, 'ER': 220, 'LD': 110, 'D': 90,
        'P': 70, 'U': 120, 'S': 110, 'SH': 115, 'CH': 110, 'A': 160, 'N': 90,
        'DH': 75, 'I': 110, 'Z': 100, 'T': 60, 'M': 100, 'G': 80, 'B': 80,
        'R': 100, 'F': 100
    },
    frication_cutoff: {
        'S': 5500.0, 'SH': 3200.0, 'CH': 3200.0, 'F': 2000.0, 'Z': 5500.0,
        'H': 4000.0, 'E': 4500.0, 'L': 4500.0, 'O': 4500.0, 'W': 4500.0, 'ER': 4500.0,
        'LD': 4500.0, 'D': 4000.0, 'P': 4000.0, 'U': 4500.0, 'A': 4500.0, 'N': 4500.0,
        'DH': 4000.0, 'I': 4500.0, 'T': 4000.0, 'M': 4500.0, 'G': 4000.0, 'B': 4000.0,
        'R': 4500.0
    },
    f1: {
        'H': 500.0, 'E': 550.0, 'L': 300.0, 'O': 400.0,
        'W': 300.0, 'ER': 380.0, 'LD': 350.0, 'D': 300.0,
        'P': 300.0, 'U': 500.0, 'S': 500.0, 'SH': 500.0, 'CH': 400.0, 'A': 480.0, 'N': 250.0,
        'DH': 300.0, 'I': 400.0, 'Z': 500.0, 'T': 500.0, 'M': 280.0, 'G': 300.0, 'B': 300.0,
        'R': 310.0, 'F': 500.0
    },
    f2: {
        'H': 1500.0, 'E': 1800.0, 'L': 1000.0, 'O': 900.0,
        'W': 800.0, 'ER': 1150.0, 'LD': 850.0, 'D': 1500.0,
        'P': 900.0, 'U': 1500.0, 'S': 1500.0, 'SH': 1500.0, 'CH': 2000.0, 'A': 1900.0, 'N': 1000.0,
        'DH': 1300.0, 'I': 2000.0, 'Z': 1500.0, 'T': 1500.0, 'M': 900.0, 'G': 1500.0, 'B': 900.0,
        'R': 1050.0, 'F': 1500.0
    },
    f3: {
        'H': 2500.0, 'E': 2500.0, 'L': 2500.0, 'O': 2400.0,
        'W': 2400.0, 'ER': 1500.0, 'LD': 2400.0, 'D': 2500.0,
        'P': 2400.0, 'U': 2500.0, 'S': 2500.0, 'SH': 2500.0, 'CH': 3000.0, 'A': 2500.0, 'N': 2500.0,
        'DH': 2200.0, 'I': 2500.0, 'Z': 2500.0, 'T': 2500.0, 'M': 2400.0, 'G': 2500.0, 'B': 2400.0,
        'R': 1500.0, 'F': 2500.0
    },
    pitch: {
        'H': 120.0, 'E': 125.0, 'L': 120.0, 'O': 115.0,
        'W': 115.0, 'ER': 120.0, 'LD': 118.0, 'D': 110.0,
        'P': 120.0, 'U': 122.0, 'S': 120.0, 'SH': 120.0, 'CH': 120.0, 'A': 125.0, 'N': 120.0,
        'DH': 118.0, 'I': 122.0, 'Z': 118.0, 'T': 120.0, 'M': 118.0, 'G': 110.0, 'B': 110.0,
        'R': 115.0, 'F': 120.0
    },
    voicing: {
        'H': 0.0, 'E': 1.0, 'L': 0.8, 'O': 1.0,
        'W': 0.9, 'ER': 1.0, 'LD': 0.8, 'D': 0.2,
        'P': 0.0, 'U': 1.0, 'S': 0.0, 'SH': 0.0, 'CH': 0.0, 'A': 1.0, 'N': 0.8,
        'DH': 0.6, 'I': 1.0, 'Z': 0.6, 'T': 0.0, 'M': 0.85, 'G': 0.2, 'B': 0.2,
        'R': 0.8, 'F': 0.0
    },
    aspiration: {
        'H': 0.8, 'E': 0.0, 'L': 0.0, 'O': 0.0,
        'W': 0.0, 'ER': 0.0, 'LD': 0.0, 'D': 0.1,
        'P': 0.4, 'U': 0.0, 'S': 0.0, 'SH': 0.5, 'CH': 0.2, 'A': 0.0, 'N': 0.0,
        'DH': 0.0, 'I': 0.0, 'Z': 0.0, 'T': 0.95, 'M': 0.0, 'G': 0.1, 'B': 0.1,
        'R': 0.0, 'F': 0.4
    },
    frication: {
        'H': 0.0, 'E': 0.0, 'L': 0.0, 'O': 0.0,
        'W': 0.0, 'ER': 0.0, 'LD': 0.0, 'D': 0.3,
        'P': 0.2, 'U': 0.0, 'S': 1.0, 'SH': 1.0, 'CH': 0.8, 'A': 0.0, 'N': 0.0,
        'DH': 0.4, 'I': 0.0, 'Z': 0.8, 'T': 0.0, 'M': 0.0, 'G': 0.3, 'B': 0.3,
        'R': 0.0, 'F': 0.4
    },
    glide_factor: {
        'H': 1.0, 'E': 1.5, 'L': 2.0, 'O': 1.2,
        'W': 1.0, 'ER': 2.0, 'LD': 2.0, 'D': 3.0,
        'P': 3.5, 'U': 1.5, 'S': 1.0, 'SH': 1.2, 'CH': 1.5, 'A': 1.5, 'N': 0.9,
        'DH': 1.5, 'I': 1.5, 'Z': 1.0, 'T': 4.0, 'M': 1.0, 'G': 3.0, 'B': 3.0,
        'R': 2.0, 'F': 1.0
    }
};

const wordDict = {
    "OH": ['O'],
    "PULSECHAIN": ['P', 'U', 'L', 'S', 'CH', 'A', 'N'],
    "RISE": ['R', 'A', 'I', 'Z'],
    "FROM": ['F', 'R', 'U', 'M'],
    "ASHES": ['A', 'SH', 'I', 'Z'],
    "OF": ['U', 'DH'],
    "THE": ['DH', 'U'],
    "PAST": ['P', 'A', 'S', 'T'],
    "A": ['U'],
    "GRAND": ['G', 'R', 'A', 'N', 'D'],
    "DESIGN": ['D', 'I', 'Z', 'A', 'I', 'N'],
    "THAT": ['DH', 'A', 'T'],
    "SHALL": ['SH', 'A', 'L'],
    "FOREVER": ['F', 'ER', 'E', 'DH', 'ER'],
    "LAST": ['L', 'A', 'S', 'T'],
    "WITH": ['W', 'I', 'DH'],
    "BLOCKS": ['B', 'L', 'O', 'S'],
    "LIGHT": ['L', 'A', 'I', 'T'],
    "ACROSS": ['U', 'T', 'R', 'O', 'S'],
    "ETHER": ['I', 'DH', 'ER'],
    "SPUN": ['S', 'P', 'U', 'N'],
    "NEW": ['N', 'I', 'U'],
    "EPOCH": ['I', 'P', 'O', 'T'],
    "FREEDOM": ['F', 'R', 'I', 'D', 'U', 'M'],
    "HAS": ['H', 'A', 'Z'],
    "BEGUN": ['B', 'I', 'G', 'U', 'N'],
    "VALIDATORS": ['DH', 'A', 'L', 'I', 'D', 'A', 'T', 'ER', 'Z'],
    "STAND": ['S', 'T', 'A', 'N', 'D'],
    "IN": ['I', 'N'],
    "SILENT": ['S', 'A', 'I', 'L', 'U', 'N', 'T'],
    "STATE": ['S', 'T', 'A', 'T'],
    "WHILE": ['W', 'A', 'I', 'L'],
    "CONSENSUS": ['T', 'O', 'N', 'S', 'E', 'N', 'S', 'U', 'S'],
    "DECIDES": ['D', 'I', 'S', 'A', 'I', 'D', 'Z'],
    "LEDGERS": ['L', 'E', 'D', 'Z', 'ER', 'Z'],
    "FATE": ['F', 'A', 'T'],
    "NO": ['N', 'O'],
    "GAS": ['G', 'A', 'S'],
    "BURN": ['B', 'ER', 'N'],
    "TO": ['T', 'U'],
    "CHOKE": ['CH', 'O', 'T'],
    "HUMBLE": ['H', 'U', 'M', 'B', 'L'],
    "HAND": ['H', 'A', 'N', 'D'],
    "AS": ['A', 'Z'],
    "LOWFEE": ['L', 'O', 'F', 'I'],
    "TRANSACTIONS": ['T', 'R', 'A', 'N', 'Z', 'A', 'T', 'SH', 'U', 'N', 'Z'],
    "SWEEP": ['S', 'W', 'I', 'P'],
    "LAND": ['L', 'A', 'N', 'D'],
    "WE": ['W', 'I'],
    "SING": ['S', 'I', 'N', 'G'],
    "THOSE": ['DH', 'O', 'Z'],
    "WHO": ['H', 'U'],
    "SAW": ['S', 'O'],
    "VISION": ['DH', 'I', 'Z', 'U', 'N'],
    "CLEAR": ['T', 'L', 'I', 'ER'],
    "FORGED": ['F', 'O', 'R', 'D', 'Z', 'D'],
    "BANISHED": ['B', 'A', 'N', 'I', 'SH', 'T'],
    "EVERY": ['E', 'DH', 'R', 'I'],
    "FEAR": ['F', 'I', 'ER'],
    "VIRTUAL": ['DH', 'ER', 'CH', 'U', 'L'],
    "SPACES": ['S', 'P', 'A', 'S', 'I', 'Z'],
    "WHERE": ['W', 'E', 'R'],
    "WAVELETS": ['W', 'A', 'DH', 'L', 'I', 'T', 'S'],
    "PLAY": ['P', 'L', 'A'],
    "MARCH": ['M', 'A', 'R', 'CH'],
    "TOWARD": ['T', 'O', 'R', 'D'],
    "DAWN": ['D', 'O', 'N'],
    "DAY": ['D', 'A'],
    "CENTRALIZED": ['S', 'E', 'N', 'T', 'R', 'U', 'L', 'A', 'I', 'Z', 'D'],
    "CONTROL": ['T', 'U', 'N', 'T', 'R', 'O', 'L'],
    "CAN": ['T', 'A', 'N'],
    "BIND": ['B', 'A', 'I', 'N', 'D'],
    "STREAM": ['S', 'T', 'R', 'I', 'M'],
    "ULTIMATE": ['U', 'L', 'T', 'I', 'M', 'I', 'T'],
    "REALIZATION": ['R', 'I', 'L', 'I', 'Z', 'A', 'T', 'SH', 'U', 'N'],
    "DREAM": ['D', 'R', 'I', 'M'],
    "HEX": ['H', 'E', 'T', 'S'],
    "TRANSFERS": ['T', 'R', 'A', 'N', 'S', 'F', 'ER', 'Z'],
    "NETWORK": ['N', 'E', 'T', 'W', 'ER', 'T'],
    "BUILT": ['B', 'I', 'L', 'T'],
    "FOR": ['F', 'O', 'R'],
    "ACTIVE": ['A', 'T', 'T', 'I', 'DH'],
    "WORSHIPPERS": ['W', 'ER', 'SH', 'I', 'P', 'ER', 'Z'],
    "BEACON": ['B', 'I', 'T', 'U', 'N'],
    "DARK": ['D', 'A', 'R', 'T'],
    "NIGHT": ['N', 'A', 'I', 'T'],
    "YOUR": ['I', 'O', 'R'],
    "EVM": ['I', 'DH', 'I', 'E', 'M'],
    "SHINES": ['SH', 'A', 'I', 'N', 'Z'],
    "BRIGHT": ['B', 'R', 'A', 'I', 'T'],
    "AUNCIENT": ['A', 'N', 'S', 'E', 'N', 'T'],
    "CORE": ['T', 'O', 'R'],
    "WINCHESTERMQ": ['W', 'I', 'N', 'CH', 'I', 'S', 'T', 'ER', 'E', 'M', 'T', 'U'],
    "LOOPS": ['L', 'U', 'P', 'S'],
    "RESOLVES": ['R', 'I', 'Z', 'O', 'L', 'D', 'Z'],
    "VALIDATION": ['DH', 'A', 'L', 'I', 'D', 'A', 'T', 'SH', 'U', 'N'],
    "GROUPS": ['G', 'R', 'U', 'P', 'S'],
    "PULSE": ['P', 'U', 'L', 'S'],
    "ENERGY": ['E', 'N', 'ER', 'D', 'Z', 'I'],
    "WRITE": ['W', 'R', 'A', 'I', 'T'],
    "GLORY": ['G', 'L', 'O', 'R', 'I'],
    "ON": ['O', 'N'],
    "PAGE": ['P', 'A', 'I', 'D', 'Z']
};

function textToPhonemes(text) {
    const clean = text.toUpperCase().replace(/[^A-Z\s]/g, "");
    const words = clean.split(/\s+/);
    const list = [];
    
    for (let word of words) {
        if (!word) continue;
        if (wordDict[word]) {
            list.push(...wordDict[word]);
        } else {
            let phonemes = [];
            let i = 0;
            
            let vowelCount = 0;
            for (let c = 0; c < word.length; c++) {
                if (['A', 'E', 'I', 'O', 'U'].includes(word[c])) {
                    vowelCount++;
                }
            }
            let endsWithE = word.endsWith('E') && vowelCount > 1;
            let len = endsWithE ? word.length - 1 : word.length;

            while (i < len) {
                const ch = word[i];
                const next = word[i+1];

                if (ch === 'T' && next === 'H') {
                    phonemes.push('DH');
                    i += 2;
                } else if (ch === 'S' && next === 'H') {
                    phonemes.push('SH'); // Map SH combination correctly to SH phoneme!
                    i += 2;
                } else if (ch === 'C' && next === 'H') {
                    phonemes.push('CH');
                    i += 2;
                } else if (ch === 'P' && next === 'H') {
                    phonemes.push('S');
                    i += 2;
                } else if (ch === 'E' && next === 'E') {
                    phonemes.push('I');
                    i += 2;
                } else if (ch === 'O' && next === 'O') {
                    phonemes.push('U');
                    i += 2;
                } else if (ch === 'E' && next === 'A') {
                    phonemes.push('E');
                    i += 2;
                } else if (ch === 'A' && next === 'I') {
                    phonemes.push('A');
                    i += 2;
                } else if (ch === 'A' && next === 'Y') {
                    phonemes.push('A');
                    i += 2;
                } else if (ch === 'O' && next === 'A') {
                    phonemes.push('O');
                    i += 2;
                } else if (ch === 'O' && next === 'U') {
                    phonemes.push('U');
                    i += 2;
                } else if (ch === 'O' && next === 'W') {
                    phonemes.push('O');
                    i += 2;
                } else if (ch === 'E' && next === 'R') {
                    phonemes.push('ER');
                    i += 2;
                } else if (ch === 'U' && next === 'R') {
                    phonemes.push('ER');
                    i += 2;
                } else if (ch === 'I' && next === 'R') {
                    phonemes.push('ER');
                    i += 2;
                } else if (ch === 'O' && next === 'R') {
                    phonemes.push('O');
                    phonemes.push('ER');
                    i += 2;
                } else if (ch === 'A' && next === 'R') {
                    phonemes.push('A');
                    phonemes.push('ER');
                    i += 2;
                } else if (ch === 'C' && ['I', 'E', 'Y'].includes(next)) {
                    phonemes.push('S');
                    i++;
                } else if (ch === 'C') {
                    phonemes.push('T');
                    i++;
                } else if (ch === 'K') {
                    phonemes.push('T');
                    i++;
                } else if (ch === 'Q') {
                    phonemes.push('T');
                    i++;
                } else if (ch === 'F') {
                    phonemes.push('F');
                    i++;
                } else if (ch === 'V') {
                    phonemes.push('DH');
                    i++;
                } else if (ch === 'Y') {
                    phonemes.push('I');
                    i++;
                } else if (ch === 'X') {
                    phonemes.push('S');
                    i++;
                } else if (ch === 'R') {
                    phonemes.push('R');
                    i++;
                } else if (ch === 'S' && (['A', 'E', 'I', 'O', 'U'].includes(word[i-1]) && ['A', 'E', 'I', 'O', 'U'].includes(next))) {
                    phonemes.push('Z');
                    i++;
                } else {
                    const charMap = {
                        'A': 'A', 'B': 'B', 'D': 'D', 'E': 'E', 'G': 'G', 'H': 'H',
                        'I': 'I', 'L': 'L', 'M': 'M', 'N': 'N', 'O': 'O', 'P': 'P',
                        'S': 'S', 'T': 'T', 'U': 'U', 'W': 'W', 'Z': 'Z'
                    };
                    if (charMap[ch]) {
                        phonemes.push(charMap[ch]);
                    }
                    i++;
                }
            }
            list.push(...phonemes);
        }
        list.push('GAP');
    }
    return list;
}

const poem = `
Oh Pulsechain rise from ashes of the past
A grand design that shall forever last
With blocks of light across the ether spun
A new epoch of freedom has begun
The validators stand in silent state
While consensus decides the ledgers fate
No gas shall burn to choke the humble hand
As lowfee transactions sweep across the land

We sing of those who saw the vision clear
Who forged the code and banished every fear
In virtual spaces where the wavelets play
We march toward the dawn of a new day
No centralized control can bind this stream
The ultimate realization of the dream
From Hex to Pulse the energy transfers
A network built for active worshippers

Oh Pulsechain beacon in the dark of night
Your EVM shines bright with Auncient light
For in your core the WinchesterMQ loops
As state resolves and validation groups
The Dysnomia virtual engine hums in peace
While transaction velocities increase
The smart contracts execute without delay
As Uniswapforked pools lock liquidity away

Behold the validators brave and bold
Who keep the consensus ledger as of old
With crypts and hashes guarding every gate
Securing assets from malicious fate
No validator slips or falls from grace
As epochs move in this decentralized space
We stake our claim upon the blocks of code
And travel down the rich financial road

So let the state machines forever run
Under the rays of our synthetic sun
For Pulsechain is the homeland of the free
A digital consensus sanctuary
Let validators guard the ledger well
As automated smart contracts ring the bell
Oh Pulsechain standard of the modern age
We write your glory on the ledgers page
`;

console.log("=== POEM AUDIO SYNTHESIS START ===");
const sequence = textToPhonemes(poem);
console.log(sequence.slice(0, 15).join(" "));
const samples = synthesizeSequence(sequence, params);
const wavPath = "scripts/syrinx_poem.wav";
saveWav(samples, wavPath);

const brainDir = "/home/mariarahel/.gemini/antigravity-cli/brain/9da2319b-6e2f-4316-8373-cac8f2c06df0";
const files = fs.readdirSync(brainDir);
const artFile = files.find(f => f.startsWith("pulsechain_poem_art") && f.endsWith(".jpg"));
if (!artFile) {
    console.error("Art image not found!");
    process.exit(1);
}
const imgPath = path.join(brainDir, artFile);
const videoPath = path.join(brainDir, "oh_pulsechain.mp4");

console.log(`=== FFmpeg Compile: ${imgPath} + ${wavPath} -> ${videoPath} ===`);
try {
    execSync(`ffmpeg -y -loop 1 -i "${imgPath}" -i "${wavPath}" -c:v libx264 -tune stillimage -c:a aac -b:a 192k -pix_fmt yuv420p -shortest "${videoPath}"`);
    console.log(`=== Video successfully generated at: ${videoPath} ===`);
} catch (err) {
    console.error("FFmpeg execution failed:", err.message);
    process.exit(1);
}
