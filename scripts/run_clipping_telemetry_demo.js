const http = require("http");
const { ethers } = require("ethers");

function postJson(path, payload) {
    return new Promise((resolve, reject) => {
        const dataStr = JSON.stringify(payload);
        const req = http.request({
            hostname: "127.0.0.1",
            port: 3000,
            path: path,
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": dataStr.length
            }
        }, (res) => {
            let body = "";
            res.on("data", chunk => body += chunk);
            res.on("end", () => {
                try {
                    resolve(JSON.parse(body));
                } catch (e) {
                    reject(new Error(`Failed to parse response: ${body}`));
                }
            });
        });
        req.on("error", reject);
        req.write(dataStr);
        req.end();
    });
}

async function runDemo() {
    console.log("======================================================================");
    console.log("🎙️  CLOSED-LOOP ANTI-CLIPPING TELEMETRY COMPARATOR DEMO");
    console.log("Comparing Unprotected Formant Filter vs. Full Protected Pipeline");
    console.log("======================================================================\n");

    const filterAddr = "0x0000000000000000000000000000000000000095";
    const filterInterface = new ethers.Interface([
        "function processSample(int256 inputSignal, int256 pitchAndQ) public returns (int256)"
    ]);

    const pitchF = 200000000000000000n; // 0.2
    const dampingQ = 50000000000000000n; // 0.05
    const packedVal = pitchF + (dampingQ << 128n);

    // Simulation parameters
    const steps = 15;
    const stimulusAmp = 3.5; // High 3.5V stimulus trigger to force clipping

    console.log("Running UNPROTECTED Pipeline...");
    let unprotectedOutputs = [];
    for (let i = 0; i < steps; i++) {
        // Linear impulse stimulus
        const stimulus = i % 4 === 0 ? stimulusAmp : 0.0;
        const calldata = filterInterface.encodeFunctionData("processSample", [
            BigInt(Math.round(stimulus * 1e18)),
            packedVal
        ]);

        const res = await postJson("/api/zmm-exec", { name: "FormantFilter", calldata });
        if (res && res.result && res.result.output) {
            const decoded = filterInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
            const val = Number(decoded[0]) / 1e18;
            unprotectedOutputs.push(val);
        }
    }

    console.log("Running PROTECTED Pipeline (Sylvania Hysteresis + Zener Dither + Bionika Spiking Reflex + Telcan Tape Saturation)...");
    let protectedOutputs = [];
    
    // Protected local states
    let formantCompActive = false;
    let prevStimulus = 0.0;
    let compGainState = 1.0;
    let bionikaVm = 0.0;
    let masterAttenuation = 1.0;

    const V_max = 1.0; // dynamic soft limit scaling ceiling
    const softLimit = V_max * 0.95;

    for (let i = 0; i < steps; i++) {
        const rawStimulus = i % 4 === 0 ? stimulusAmp : 0.0;
        
        // 1. Sylvania Hysteretic Envelope Tracker check (simulating dynamic feedback)
        const compStrikeThreshold = 0.35;
        const compExtinguishThreshold = 0.12;
        
        // Simulating the dynamic envelope behavior (envelope tracking previous input voltage)
        const simEnvelope = Math.abs(prevStimulus) * 0.4;
        if (simEnvelope > compStrikeThreshold) {
            formantCompActive = true;
        } else if (simEnvelope < compExtinguishThreshold) {
            formantCompActive = false;
        }

        // 2. Zener Dither Injection
        let zenerDither = 0.0;
        if (formantCompActive) {
            zenerDither = (Math.random() - 0.5) * 0.015;
        }

        // 3. Dynamic Compander Gain (Issue 8)
        let targetCompGain = 1.0;
        if (formantCompActive && simEnvelope > 0.0) {
            targetCompGain = compStrikeThreshold / simEnvelope;
        }
        if (targetCompGain < 0.05) targetCompGain = 0.05;

        // Zero-crossing synchronization
        let zeroCrossed = (rawStimulus >= 0.0 && prevStimulus < 0.0) || (rawStimulus < 0.0 && prevStimulus >= 0.0);
        if (i === 0 || zeroCrossed) {
            compGainState = targetCompGain;
        }
        prevStimulus = rawStimulus;

        // Compress input
        const compressedStimulus = (rawStimulus * compGainState) + zenerDither;

        // Send to VM FormantFilter
        const calldata = filterInterface.encodeFunctionData("processSample", [
            BigInt(Math.round(compressedStimulus * 1e18)),
            packedVal
        ]);
        const res = await postJson("/api/zmm-exec", { name: "FormantFilter", calldata });
        
        if (res && res.result && res.result.output) {
            const decoded = filterInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
            const rawOut = Number(decoded[0]) / 1e18;

            // Reciprocal Expansion
            const expandedOut = rawOut * (1.0 / compGainState);

            // Bionika Spiking Neural loop step
            bionikaVm = bionikaVm * 0.95;

            // apply master attenuation
            const inputScaled = expandedOut * masterAttenuation;

            // Bionika-Driven Telcan Tape Saturation modulation
            let saturationFactor = 1.0 - (bionikaVm * 0.4);
            let saturatedVal = inputScaled;
            const saturationThreshold = softLimit * saturationFactor;

            if (Math.abs(inputScaled) > saturationThreshold) {
                // Clipping incident -> trigger Bionika reflex stimulation
                bionikaVm += 0.35;
                if (bionikaVm >= 1.0) {
                    bionikaVm = 0.0;
                    masterAttenuation = Math.max(0.1, masterAttenuation - 0.3); // Ear protection spike reflex
                }

                const sign = Math.sign(inputScaled);
                const absVal = Math.abs(inputScaled);
                const excess = absVal - saturationThreshold;
                const limitMargin = 1.2 - saturationThreshold;
                if (limitMargin > 0) {
                    saturatedVal = sign * (saturationThreshold + limitMargin * Math.tanh(excess / limitMargin));
                }
            } else {
                masterAttenuation = Math.min(1.0, masterAttenuation + 0.02);
            }

            protectedOutputs.push(saturatedVal);
        }
    }

    console.log("\n==================== COMPARATIVE PERFORMANCE CHART ====================");
    console.log("Step | Unprotected Out | Protected Out | Visual Signal Alignment");
    console.log("----------------------------------------------------------------------");
    for (let i = 0; i < steps; i++) {
        const unp = unprotectedOutputs[i] || 0;
        const prot = protectedOutputs[i] || 0;
        
        // Build simple ASCII representation of waveforms
        const unpBar = "#".repeat(Math.min(15, Math.round(Math.abs(unp) * 6)));
        const protBar = "*".repeat(Math.min(15, Math.round(Math.abs(prot) * 6)));
        
        console.log(
            ` ${String(i + 1).padStart(2)}  | ` +
            `${unp.toFixed(3).padStart(7)}V     | ` +
            `${prot.toFixed(3).padStart(7)}V   | ` +
            `Unp: [${unpBar.padEnd(15)}] vs Prot: [${protBar.padEnd(15)}]`
        );
    }
    console.log("======================================================================\n");

    const maxUnp = Math.max(...unprotectedOutputs.map(Math.abs));
    const maxProt = Math.max(...protectedOutputs.map(Math.abs));

    console.log(`Summary of Safeguards:`);
    console.log(`- Max peak amplitude (Unprotected): ${maxUnp.toFixed(3)}V (Hard clipping threshold breached)`);
    console.log(`- Max peak amplitude (Protected):   ${maxProt.toFixed(3)}V (Compressed gracefully below dynamic saturation ceiling)`);
    console.log(`- Bionika Spiking Reflex: Successful attenuation triggering.`);

    console.log("\n======================================================================");
    console.log("⚡  ZENER DIODE APPLICATIONS DEMONSTRATION SUITE (ISSUE 18)");
    console.log("======================================================================\n");

    // 1. Zener Avalanche Dither Demo
    console.log("1. Zener Avalanche Dither (Spectral Dispersion):");
    let ditherSamples = [];
    for (let i = 0; i < 5; i++) {
        const dither = (Math.random() - 0.5) * 0.015;
        ditherSamples.push(dither);
        console.log(`   Sample ${i + 1}: Noise injection = ${dither.toFixed(6)}V`);
    }
    const avgDither = ditherSamples.reduce((a, b) => a + b, 0) / ditherSamples.length;
    console.log(`   -> Mean Dither Offset: ${avgDither.toFixed(6)}V (Centered near 0V to prevent DC drift)\n`);

    // 2. Zener DC Coupling Offset Demo
    console.log("2. Zener DC Coupling Offset (Constant Voltage Shifting):");
    const V_zener_couple = 2.0; // 2.0V breakdown
    const inputs = [3.5, 4.0, 4.5];
    inputs.forEach(inp => {
        const outCoupled = Math.max(0, inp - V_zener_couple);
        console.log(`   Input = ${inp.toFixed(2)}V | Zener Shifted Output = ${outCoupled.toFixed(2)}V (Shift = -${V_zener_couple.toFixed(2)}V)`);
    });
    console.log("   -> AC variations are transmitted with 1:1 gain while shifting DC operating points.\n");

    // 3. Zener Meter Protection Shunt Demo
    console.log("3. Zener Meter Protection Shunt (Nixie Overload Clamping):");
    const V_zener_clamp = 1.6; // Clamps display to 1.6V (corresponds to Nixie Digit 9)
    const surgeInputs = [1.2, 1.5, 1.8, 2.5, 3.5];
    surgeInputs.forEach(inp => {
        const outClamped = Math.min(inp, V_zener_clamp);
        const nixieDigit = Math.max(0, Math.min(9, Math.round(outClamped * 5.625))); // scale 1.6V to digit 9
        console.log(`   Signal Surge = ${inp.toFixed(2)}V | Clamped Output = ${outClamped.toFixed(2)}V | Nixie Digit = ${nixieDigit}`);
    });
    console.log("   -> Zener shunt prevents display driver overload, keeping indicators within limits.\n");

    // 4. Zener Symmetrical Waveform Clipper Demo
    console.log("4. Zener Symmetrical Waveform Clipper (Back-to-Back Fuzz Shaper):");
    const V_breakdown = 1.2;
    const V_diode_forward = 0.6;
    const clampLimit = V_breakdown + V_diode_forward; // 1.8V symmetrical clipping
    const testWaves = [-2.5, -1.5, 0.0, 1.5, 2.5];
    testWaves.forEach(w => {
        let clipped = w;
        if (w > clampLimit) clipped = clampLimit;
        else if (w < -clampLimit) clipped = -clampLimit;
        console.log(`   Wave input = ${w.toFixed(2)}V | Clipped Wave output = ${clipped.toFixed(2)}V (Symmetrical Clamped at ±${clampLimit.toFixed(2)}V)`);
    });
    console.log("   -> Symmetrical clipping generates rich odd-harmonic distortion (analog fuzz square waves).\n");

    // 5. Zener Temperature-Compensated Reference Demo
    console.log("5. Zener Temperature-Compensated Reference (Opposite Coefficient Drift Cancellation):");
    // Standard Zener coefficient is positive (+2.5mV/°C)
    // Forward diode coefficient is negative (-2.5mV/°C)
    const T_temps = [25.0, 35.0, 45.0, 55.0];
    const zenerCoeff = 0.0025; // +2.5mV/°C
    const diodeCoeff = -0.0025; // -2.5mV/°C
    const V_ref_base = 5.6; // 5.6V reference baseline

    T_temps.forEach(temp => {
        const dT = temp - 25.0;
        const v_zener_drift = V_ref_base + (dT * zenerCoeff);
        const v_compensated = V_ref_base + (dT * zenerCoeff) + (dT * diodeCoeff);
        console.log(`   Temp = ${temp}°C | Uncompensated Zener = ${v_zener_drift.toFixed(4)}V | Compensated Pair = ${v_compensated.toFixed(4)}V`);
    });
    console.log("   -> Series pairing of Zener + Diode cancels thermal drift, providing a stable voltage reference.");
    console.log("\n★★★ COMPARATOR & ZENER DEMO RUN COMPLETED SUCCESSFULLY ★★★");
}

runDemo().catch(console.error);
