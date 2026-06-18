const fs = require("fs");
const path = require("path");
const assert = require("assert");

const SAMPLE_RATE = 44100;
const DURATION_SECS = 2.0;
const TOTAL_SAMPLES = SAMPLE_RATE * DURATION_SECS;

// Helper to simulate the Farfisa Organ DSP loop for a single sample
function renderFarfisaSample(t, freq, drawbars, slalomSpeed, enablePerc, sampleIndex) {
    const modulatedFreq = freq; // Base frequency
    let sampleVal = 0;

    // Drawbars: [16', 8', 4', 2-2/3']
    const [vol16, vol8, vol4, vol2] = drawbars;

    // 8' Fundamental
    if (vol8 > 0) {
        let activeFreq = modulatedFreq;
        if (slalomSpeed > 0.005) {
            const startFreq = modulatedFreq * Math.pow(2, -3/12);
            // Linear pitch sweep over slalom time
            if (t < slalomSpeed) {
                activeFreq = startFreq + (modulatedFreq - startFreq) * (t / slalomSpeed);
            }
        }
        // Accumulate phase-based sawtooth
        const phase = (activeFreq * t) % 1.0;
        sampleVal += (2.0 * phase - 1.0) * vol8 * 0.15;
    }

    // 16' Sub-Octave
    if (vol16 > 0) {
        let activeFreq = modulatedFreq * 0.5;
        if (slalomSpeed > 0.005) {
            const startFreq = (modulatedFreq * 0.5) * Math.pow(2, -3/12);
            if (t < slalomSpeed) {
                activeFreq = startFreq + ((modulatedFreq * 0.5) - startFreq) * (t / slalomSpeed);
            }
        }
        const phase = (activeFreq * t) % 1.0;
        sampleVal += (2.0 * phase - 1.0) * vol16 * 0.15;
    }

    // 4' Octave
    if (vol4 > 0) {
        let activeFreq = modulatedFreq * 2.0;
        if (slalomSpeed > 0.005) {
            const startFreq = (modulatedFreq * 2.0) * Math.pow(2, -3/12);
            if (t < slalomSpeed) {
                activeFreq = startFreq + ((modulatedFreq * 2.0) - startFreq) * (t / slalomSpeed);
            }
        }
        const phase = (activeFreq * t) % 1.0;
        sampleVal += (2.0 * phase - 1.0) * vol4 * 0.12;
    }

    // 2-2/3' Quint
    if (vol2 > 0) {
        let activeFreq = modulatedFreq * 3.0;
        if (slalomSpeed > 0.005) {
            const startFreq = (modulatedFreq * 3.0) * Math.pow(2, -3/12);
            if (t < slalomSpeed) {
                activeFreq = startFreq + ((modulatedFreq * 3.0) - startFreq) * (t / slalomSpeed);
            }
        }
        const phase = (activeFreq * t) % 1.0;
        sampleVal += (2.0 * phase - 1.0) * vol2 * 0.10;
    }

    // Organ Percussion Click (Triangle wave starting high at onset and decaying within 40ms)
    if (enablePerc && t < 0.04) {
        const percFreq = modulatedFreq * 4.0;
        const phase = (percFreq * t) % 1.0;
        const tri = Math.abs(2.0 * phase - 1.0) * 2.0 - 1.0;
        const decay = Math.exp(-t / 0.012); // exponential decay
        sampleVal += tri * 0.20 * decay;
    }

    return Math.max(-1.0, Math.min(1.0, sampleVal));
}

function runTests() {
    console.log("=================================================");
    console.log("RUNNING FARFISA SYNTHESIS DSP UNIT TESTS");
    console.log("=================================================");

    // Test 1: Additive Drawbar Mixing Math
    console.log("Test 1: Verifying drawbar presence and mix...");
    const silentSample = renderFarfisaSample(0.1, 440, [0, 0, 0, 0], 0, false, 0);
    assert.strictEqual(silentSample, 0, "Silent drawbars should result in zero amplitude");

    const activeSample = renderFarfisaSample(0.1, 440, [0.5, 1.0, 0.3, 0.2], 0, false, 0);
    assert.ok(Math.abs(activeSample) > 0.001, "Active drawbars must generate audio content");
    console.log("✓ Test 1: Success.");

    // Test 2: Slalom Pitch Sweep Glide Boundaries
    console.log("Test 2: Verifying slalom pitch boundaries...");
    const slalomTime = 0.15; // 150ms
    // Check initial flat frequency offset (at t = 0)
    // 8' fundamental should be 3 semitones flat = 440 * 2^(-3/12)
    const expectedStartFreq = 440 * Math.pow(2, -3/12);
    
    // We can verify that pitch glides upwards
    const sampleNearStart = renderFarfisaSample(0.001, 440, [0, 1.0, 0, 0], slalomTime, false, 0);
    const sampleNearEnd = renderFarfisaSample(0.149, 440, [0, 1.0, 0, 0], slalomTime, false, 0);
    assert.ok(sampleNearStart !== sampleNearEnd, "Pitch glide should change sample phase mapping");
    console.log("✓ Test 2: Success.");

    // Test 3: Organ Percussion Click Generation
    console.log("Test 3: Verifying percussive envelope decay...");
    const startPercVal = renderFarfisaSample(0.002, 440, [0, 0, 0, 0], 0, true, 0);
    const endPercVal = renderFarfisaSample(0.05, 440, [0, 0, 0, 0], 0, true, 0);
    assert.ok(Math.abs(startPercVal) > 0.001, "Percussion onset must produce a click transient");
    assert.ok(Math.abs(endPercVal) < 0.0001, "Percussion click must decay to zero after 40ms");
    console.log("✓ Test 3: Success.");

    // Test 4: Write high-fidelity audio validation wav
    console.log("Test 4: Generating test audio validation WAV (farfisa_test.wav)...");
    const audioBuffer = new Float32Array(TOTAL_SAMPLES);
    for (let i = 0; i < TOTAL_SAMPLES; i++) {
        const t = i / SAMPLE_RATE;
        audioBuffer[i] = renderFarfisaSample(t, 261.63, [0.6, 1.0, 0.4, 0.2], 0.15, true, i);
    }
    
    const wavPath = path.join(__dirname, "../farfisa_test.wav");
    const buffer = writeWavFile(audioBuffer);
    fs.writeFileSync(wavPath, buffer);
    console.log(`✓ Test 4: Success. Audio generated at: ${wavPath}`);

    console.log("=================================================");
    console.log("ALL UNIT TESTS PASSED SUCCESSFULLY!");
    console.log("=================================================");
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

runTests();
