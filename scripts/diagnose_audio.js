const fs = require('fs');
const path = require('path');

function diagnoseWav(filename) {
    const filePath = path.isAbsolute(filename) ? filename : path.join(__dirname, '../', filename);
    if (!fs.existsSync(filePath)) {
        console.log(`[AUDIO_MONITOR] Error: File not found at ${filePath}`);
        return;
    }

    const buffer = fs.readFileSync(filePath);
    const size = buffer.length;

    // Parse WAV Header
    const chunkId = buffer.toString('ascii', 0, 4);
    const format = buffer.toString('ascii', 8, 12);
    const sampleRate = buffer.readUInt32LE(24);
    const bitsPerSample = buffer.readUInt16LE(34);
    const dataOffset = 44;

    console.log(`\n=== AUDIO DIAGNOSTIC REPORT: ${path.basename(filename)} ===`);
    console.log(`- Header Integrity: ${chunkId === 'RIFF' && format === 'WAVE' ? '✓ VALID' : '✗ INVALID'}`);
    console.log(`- File Size: ${size} bytes`);
    console.log(`- Sample Rate: ${sampleRate} Hz`);
    console.log(`- Bit Depth: ${bitsPerSample}-bit`);

    if (size <= dataOffset) {
        console.log(`- Status: ✗ NO DATA SAMPLES PRESENT`);
        return;
    }

    let min = 0;
    let max = 0;
    let sum = 0;
    let nonZeroCount = 0;
    let clipCount = 0;
    const samples = [];

    for (let i = dataOffset; i < size; i += 2) {
        if (i + 2 > size) break;
        const val = buffer.readInt16LE(i);
        samples.push(val);
        sum += val;
        if (val !== 0) nonZeroCount++;
        if (val > max) max = val;
        if (val < min) min = val;
        if (val >= 32760 || val <= -32760) clipCount++;
    }

    const totalSamples = samples.length;
    const mean = sum / totalSamples;
    const clipRate = (clipCount / totalSamples) * 100;
    const activeRate = (nonZeroCount / totalSamples) * 100;

    // Calculate RMS (Root Mean Square) for volume
    let squareSum = 0;
    for (let s of samples) {
        squareSum += s * s;
    }
    const rms = Math.sqrt(squareSum / totalSamples);

    console.log(`- Total Audio Samples: ${totalSamples}`);
    console.log(`- Active Signal Rate: ${activeRate.toFixed(2)}% (${nonZeroCount} non-zero samples)`);
    console.log(`- Amplitude Limits: Min = ${min} | Max = ${max}`);
    console.log(`- DC Offset (Mean): ${mean.toFixed(2)} (Center should be ~0)`);
    console.log(`- Volume (RMS): ${rms.toFixed(2)} (Healthy range: 1000 - 15000)`);
    console.log(`- Clipping Rate: ${clipRate.toFixed(2)}% (${clipCount} railed samples)`);
    
    // Evaluation
    let status = "✓ HEALTHY (Stable waveform, no clipping, healthy volume)";
    if (activeRate === 0) {
        status = "✗ SILENT (No audio activity detected)";
    } else if (clipRate > 5.0) {
        status = "✗ CLIPPING (Output is railed; will sound like a loud machine/squeal)";
    } else if (rms < 500) {
        status = "⚠ WHISPER (Audio is stable but extremely quiet)";
    } else if (Math.abs(mean) > 1000) {
        status = "⚠ DC OFFSET BIAS (Waveform is shifted; may cause speaker pops)";
    }
    console.log(`- Overall Status: ${status}\n================================================\n`);
}

// Support command line args
const targetFile = process.argv[2] || 'server_test_c_synth.wav';
diagnoseWav(targetFile);
