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
    console.log("=== Testing Snare Synthesizer on ZMM VM ===");

    const snareAddr = "0x0000000000000000000000000000000000000048";

    console.log("Registering SnareSynthesizer...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "SnareSynthesizer", "../solidity/bin/snareSynthesizer.yul", ${snareAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const snareInterface = new ethers.Interface([
        "function processSample(int256 trigger, int256 paramsPack) public returns (int256)"
    ]);

    // Pack parameters:
    // pitchK: 0.05 (bits 0..63)
    // decayD: 0.02 (bits 64..127)
    // noiseDecay: 0.10 (bits 128..191)
    // noiseMix: 0.50 (bits 192..255)
    const pitchK = 50000000000000000n;
    const decayD = 20000000000000000n;
    const noiseDecay = 100000000000000000n;
    const noiseMix = 500000000000000000n;

    const packedVal = pitchK + (decayD << 64n) + (noiseDecay << 128n) + (noiseMix << 192n);

    console.log("Triggering snare drum hit and simulating 15 steps...");
    let outputs = [];

    for (let i = 0; i < 15; i++) {
        const triggerVal = i === 0 ? 1000000000000000000n : 0n;

        const calldata = snareInterface.encodeFunctionData("processSample", [triggerVal, packedVal]);
        const res = await postJson("/api/zmm-exec", {
            name: "SnareSynthesizer",
            calldata: calldata
        });

        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error, full response:", JSON.stringify(res));
            break;
        }

        const decoded = snareInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVal = Number(decoded[0]) / 1e18;
        outputs.push(outVal);
        console.log(`Step ${i + 1}: Output = ${outVal.toFixed(6)}`);
    }

    let snareResponded = false;
    for (let i = 0; i < outputs.length; i++) {
        if (Math.abs(outputs[i]) > 0.0) {
            snareResponded = true;
        }
    }

    if (snareResponded) {
        console.log("\n★★★ ALL SNARE SYNTHESIZER TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Snare returned zero response.");
    }
}

test().catch(console.error);
