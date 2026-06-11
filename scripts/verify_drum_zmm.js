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
    console.log("=== Testing Drum Synthesizer Resonator on ZMM VM ===");

    const drumAddr = "0x0000000000000000000000000000000000000046";

    console.log("Registering DrumSynthesizerV2...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "DrumSynthesizerV2", "../solidity/bin/drumSynthesizer.yul", ${drumAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const drumInterface = new ethers.Interface([
        "function processSample(int256 trigger, int256 pitchAndDecay) public returns (int256)"
    ]);

    // Parameters
    const pitchK = 50000000000000000n; // 0.05 spring constant
    const decayD = 20000000000000000n; // 0.02 damping coefficient
    const packedVal = pitchK + (decayD << 128n);

    console.log("Triggering drum hit and simulating 15 steps...");
    let outputs = [];

    for (let i = 0; i < 15; i++) {
        // Trigger is active on the first step only
        const triggerVal = i === 0 ? 1000000000000000000n : 0n;

        const calldata = drumInterface.encodeFunctionData("processSample", [triggerVal, packedVal]);
        console.log(`Step ${i + 1} Calldata: ${calldata}`);
        
        const res = await postJson("/api/zmm-exec", {
            name: "DrumSynthesizerV2",
            calldata: calldata
        });

        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error, full response:", JSON.stringify(res));
            break;
        }

        const decoded = drumInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVal = Number(decoded[0]) / 1e18;
        outputs.push(outVal);
        console.log(`Step ${i + 1}: Output = ${outVal.toFixed(6)}`);
    }

    // Verify oscillation occurred and amplitudes decay
    let oscillates = false;
    let crossZero = 0;
    for (let i = 1; i < outputs.length; i++) {
        if (outputs[i] !== outputs[i - 1]) oscillates = true;
        if ((outputs[i] > 0 && outputs[i - 1] < 0) || (outputs[i] < 0 && outputs[i - 1] > 0)) {
            crossZero++;
        }
    }

    if (oscillates && crossZero > 0) {
        console.log("\n★★★ ALL DRUM SYNTHESIZER TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Drum failed to oscillate or decay correctly.");
    }
}

test().catch(console.error);
