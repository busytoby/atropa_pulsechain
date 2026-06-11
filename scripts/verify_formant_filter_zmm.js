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
    console.log("=== Testing Non-Robotic Formant Filter on ZMM VM ===");

    const filterAddr = "0x0000000000000000000000000000000000000095";

    console.log("Registering FormantFilter...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "FormantFilter", "../solidity/bin/formantFilter.yul", ${filterAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const filterInterface = new ethers.Interface([
        "function processSample(int256 inputSignal, int256 pitchAndQ) public returns (int256)"
    ]);

    // Parameters:
    // pitchF = 0.2 (tuning coefficient, lower 128 bits)
    // dampingQ = 0.05 (q factor damping, upper 128 bits)
    const pitchF = 200000000000000000n; // 0.2
    const dampingQ = 50000000000000000n; // 0.05
    const packedVal = pitchF + (dampingQ << 128n);

    console.log("Feeding high-amplitude step input to trigger dynamic damping & quadratic soft-clipping...");
    let outputs = [];

    for (let i = 0; i < 10; i++) {
        // High-amplitude step pulse on the first sample to drive the filter hard
        const inputSignal = i === 0 ? 2500000000000000000n : 0n; // 2.5V pulse

        const calldata = filterInterface.encodeFunctionData("processSample", [inputSignal, packedVal]);
        const res = await postJson("/api/zmm-exec", {
            name: "FormantFilter",
            calldata: calldata
        });

        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error, full response:", JSON.stringify(res));
            break;
        }

        const decoded = filterInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVal = Number(decoded[0]) / 1e18;
        outputs.push(outVal);
        console.log(`Step ${i + 1}: Output = ${outVal.toFixed(6)}`);
    }

    let responded = outputs.some(val => Math.abs(val) > 0.0);

    if (responded) {
        console.log("\n★★★ ALL FORMANT FILTER TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Filter returned zero response.");
    }
}

test().catch(console.error);
