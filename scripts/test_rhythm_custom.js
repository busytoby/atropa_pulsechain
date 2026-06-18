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

async function test() {
    console.log("===============================================================");
    console.log("STARTING ADVANCED RHYTHM GENERATOR COMPLEXITY TEST");
    console.log("===============================================================");

    // Generate unique name and address to ensure clean compilation and clean storage
    const uniqueId = Math.floor(Math.random() * 800000) + 100000;
    const contractName = `RhythmGenerator_${uniqueId}`;
    const rhythmAddr = uniqueId;
    console.log(`Using contract name: ${contractName} at address: ${rhythmAddr}`);

    console.log("Initializing RhythmGenerator on ZMM VM...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "${contractName}", "../solidity/bin/rhythmGenerator.yul", ${rhythmAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const rhythmInterface = new ethers.Interface([
        "function processSample(int256 triggerClock, int256 patternSelect) public returns (int256)",
        "function setCustomPatternStep(uint256 patternId, uint256 stepIdx, uint256 triggers) public",
        "function setPatternLength(uint256 patternId, uint256 length) public",
        "function getPatternStepTriggers(uint256 patternId, uint256 stepIdx) public view returns (uint256)",
        "function getCurrentStep() public view returns (uint256)"
    ]);

    // Query stepIdx before any clock ticks
    let initialStepCalldata = rhythmInterface.encodeFunctionData("getCurrentStep", []);
    let initialStepRes = await postJson("/api/zmm-exec", {
        name: contractName,
        calldata: initialStepCalldata
    });
    let initialStepDecoded = rhythmInterface.decodeFunctionResult("getCurrentStep", "0x" + initialStepRes.result.output);
    console.log(`[DEBUG] Initial step index before ticks: ${initialStepDecoded[0]}`);

    // Helper to trigger one clock step
    const stepClock = async (patternId) => {
        // Step clock high (rising edge)
        let calldata = rhythmInterface.encodeFunctionData("processSample", [1000000000000000000n, patternId]);
        let res = await postJson("/api/zmm-exec", {
            name: contractName,
            calldata: calldata
        });
        let decoded = rhythmInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        let triggerOut = Number(decoded[0]);

        // Get current step
        let stepCalldata = rhythmInterface.encodeFunctionData("getCurrentStep", []);
        let stepRes = await postJson("/api/zmm-exec", {
            name: contractName,
            calldata: stepCalldata
        });
        let stepDecoded = rhythmInterface.decodeFunctionResult("getCurrentStep", "0x" + stepRes.result.output);
        let currentStepIdx = Number(stepDecoded[0]);

        // Step clock low
        calldata = rhythmInterface.encodeFunctionData("processSample", [0n, patternId]);
        await postJson("/api/zmm-exec", {
            name: contractName,
            calldata: calldata
        });

        return { triggerOut, currentStepIdx };
    };

    // Helper to send general transaction to RhythmGenerator
    const sendTx = async (funcName, args) => {
        let calldata = rhythmInterface.encodeFunctionData(funcName, args);
        let res = await postJson("/api/zmm-exec", {
            name: contractName,
            calldata: calldata
        });
        return res;
    };

    // --- Part 1: Verify Pre-programmed Auncient Clave Polyrhythm (Pattern 2) ---
    console.log("\n--- Part 1: Verifying Pre-programmed Auncient Clave Polyrhythm (Pattern 2) ---");
    const expectedClave = [0, 4, 9, 6, 0, 5];
    for (let step = 0; step < 6; step++) {
        const { triggerOut, currentStepIdx } = await stepClock(2n);
        console.log(`Clave Tick ${step + 1} (stepIdx ${currentStepIdx}) => Triggers: ${triggerOut} (Expected: ${expectedClave[step]})`);
        if (triggerOut !== expectedClave[step]) {
            throw new Error(`Trigger mismatch at tick ${step + 1}: expected ${expectedClave[step]}, got ${triggerOut}`);
        }
    }

    // --- Part 2: Verify Pre-programmed Auncient Hip-Hop Boom Bap Beat (Pattern 4) ---
    console.log("\n--- Part 2: Verifying Pre-programmed Auncient Hip-Hop Boom Bap Beat (Pattern 4) ---");
    // Starting step is 6. Ticks 7-12 should correspond to steps 7-12.
    // In Pattern 4:
    // Step 7: 4
    // Step 8: 133
    // Step 9: 4
    // Step 10: 4
    // Step 11: 5
    // Step 12: 134
    const expectedBoomBap = [4, 133, 4, 4, 5, 134];
    for (let step = 0; step < 6; step++) {
        const { triggerOut, currentStepIdx } = await stepClock(4n);
        console.log(`Boom Bap Tick ${step + 1} (stepIdx ${currentStepIdx}) => Triggers: ${triggerOut} (Expected: ${expectedBoomBap[step]})`);
        if (triggerOut !== expectedBoomBap[step]) {
            throw new Error(`Trigger mismatch at Boom Bap tick ${step + 1}: expected ${expectedBoomBap[step]}, got ${triggerOut}`);
        }
    }

    // --- Part 3: Verify User-Programmable Custom Patterns ---
    console.log("\n--- Part 3: Verifying User-Programmable Custom Patterns (Pattern 5) ---");
    // Define a custom pattern 5:
    // Step 0: Kick (1)
    // Step 1: Snare (2)
    // Step 2: Open Hihat (8)
    console.log("Programming custom pattern 5 steps...");
    await sendTx("setCustomPatternStep", [5n, 0n, 1n]);
    await sendTx("setCustomPatternStep", [5n, 1n, 2n]);
    await sendTx("setCustomPatternStep", [5n, 2n, 8n]);

    // Let's read them back using getPatternStepTriggers
    for (let idx = 0; idx < 3; idx++) {
        let readRes = await sendTx("getPatternStepTriggers", [5n, BigInt(idx)]);
        let decoded = rhythmInterface.decodeFunctionResult("getPatternStepTriggers", "0x" + readRes.result.output);
        console.log(`Custom Pattern 5 Step ${idx} trigger read-back: ${decoded[0]}`);
        if (Number(decoded[0]) !== (idx === 0 ? 1 : idx === 1 ? 2 : 8)) {
            throw new Error(`Trigger read-back mismatch at index ${idx}`);
        }
    }

    // Step clock to verify step index sequence and triggers:
    // Current stepIdx is 12. Next tick will increment to 13, 14, 15, then wrap to 0, 1, 2.
    console.log("Stepping clock to wrap around and trigger custom steps...");
    const expectedCustom = [1, 2, 8];
    for (let tick = 0; tick < 6; tick++) {
        const { triggerOut, currentStepIdx } = await stepClock(5n);
        console.log(`Tick ${tick + 1} (stepIdx ${currentStepIdx}) => Triggers: ${triggerOut}`);
        if (currentStepIdx < 3) {
            let expectedVal = expectedCustom[currentStepIdx];
            if (triggerOut !== expectedVal) {
                throw new Error(`Custom trigger mismatch at step ${currentStepIdx}: expected ${expectedVal}, got ${triggerOut}`);
            }
        }
    }

    // --- Part 4: Verify Custom Pattern Length / Polyrhythms ---
    console.log("\n--- Part 4: Verifying Configurable Pattern Length (Pattern 5 -> Length 3) ---");
    // Current stepIdx is 2. Let's set the pattern length of pattern 5 to 3 steps.
    await sendTx("setPatternLength", [5n, 3n]);

    // Now step the clock. It should wrap from step 2 -> 0 -> 1 -> 2 -> 0...
    // Let's verify 4 ticks.
    const expectedLengthWrap = [0, 1, 2, 0];
    for (let tick = 0; tick < 4; tick++) {
        const { triggerOut, currentStepIdx } = await stepClock(5n);
        console.log(`Tick ${tick + 1} (stepIdx ${currentStepIdx}) => Triggers: ${triggerOut} (Expected stepIdx: ${expectedLengthWrap[tick]})`);
        if (currentStepIdx !== expectedLengthWrap[tick]) {
            throw new Error(`Length wrap mismatch: expected stepIdx ${expectedLengthWrap[tick]}, got ${currentStepIdx}`);
        }
    }

    console.log("\n★★★ ALL ADVANCED RHYTHM SEQUENCER TESTS PASSED ★★★");
}

test().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
