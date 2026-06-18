const fs = require("fs");
const path = require("path");

const SAMPLE_RATE = 44100;
const DURATION_SECS = 4;
const TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS;

// TR-808 Kick Triggers (Steps 0, 4, 8, 12) on a 16-step grid
const KICK_SEQUENCE = [
    1, 0, 0, 0,  1, 0, 0, 0,
    1, 0, 0, 0,  1, 0, 0, 0
];

function main() {
    console.log("===============================================================");
    console.log("GENERATING STANDALONE TR-808 KICK DRUM SYNTHESIS WAV");
    console.log("===============================================================");

    const audioBuffer = new Float32Array(TOTAL_SAMPLES);
    const tempoBPM = 120;
    const stepDurationSamples = Math.floor(SAMPLE_RATE * (60 / tempoBPM) / 4); // 16th note step length

    let lastKickTriggerSample = -999999; 

    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const stepIndex = Math.floor(i / stepDurationSamples) % KICK_SEQUENCE.length;
        const sampleInStep = i % stepDurationSamples;

        // Trigger Kick on steps 0, 4, 8, 12
        if (sampleInStep === 0 && KICK_SEQUENCE[stepIndex] === 1) {
            lastKickTriggerSample = i;
        }

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
            
            // Sub-bass amplitude decay envelope (420ms decay)
            const ampEnv = Math.exp(-kickAgeSecs / 0.42);
            
            // Warm waveshaper saturation
            kickSample = Math.tanh((sine + click) * ampEnv * 1.6);
        }

        audioBuffer[i] = kickSample * 0.5; // Scale volume to prevent clipping
    }

    // Write combined WAV file
    const wavPath = path.join(__dirname, "../teddy808_kick_synthesis.wav");
    const buffer = writeWavFile(audioBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`Standalone 808 WAV successfully generated at: ${wavPath}`);
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
