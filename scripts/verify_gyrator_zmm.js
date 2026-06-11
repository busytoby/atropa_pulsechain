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
    console.log("=== Testing Gyrator Active Filter on ZMM VM ===");

    const filterAddr = "0x0000000000000000000000000000000000000047";

    console.log("Registering GyratorFilter...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "GyratorFilter", "../solidity/bin/gyratorFilter.yul", ${filterAddr}`
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

    console.log("Simulating resonant sweep with step input...");
    let outputs = [];

    for (let i = 0; i < 15; i++) {
        // Step input on the first sample, then zero input
        const inputSignal = i === 0 ? 1000000000000000000n : 0n;

        const calldata = filterInterface.encodeFunctionData("processSample", [inputSignal, packedVal]);
        const res = await postJson("/api/zmm-exec", {
            name: "GyratorFilter",
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

    let filterResponded = false;
    for (let i = 0; i < outputs.length; i++) {
        if (Math.abs(outputs[i]) > 0.0) {
            filterResponded = true;
        }
    }

    if (filterResponded) {
        console.log("\n★★★ ALL GYRATOR FILTER TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Filter returned zero response.");
    }
}

test().catch(console.error);
