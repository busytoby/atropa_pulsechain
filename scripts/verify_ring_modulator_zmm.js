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
    console.log("=== Testing Ring Modulator on ZMM VM ===");

    const modAddr = "0x0000000000000000000000000000000000000051";

    console.log("Registering RingModulator...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "RingModulator", "../solidity/bin/ringModulator.yul", ${modAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const modInterface = new ethers.Interface([
        "function processSample(int256 inputSignal, int256 carrierSignal) public returns (int256)"
    ]);

    // Test values: 0.5 * -0.8 = -0.4
    const inputVal = 500000000000000000n; // 0.5
    const carrierVal = -800000000000000000n; // -0.8

    console.log("Executing multiplication step...");
    const calldata = modInterface.encodeFunctionData("processSample", [inputVal, carrierVal]);
    const res = await postJson("/api/zmm-exec", {
        name: "RingModulator",
        calldata: calldata
    });

    if (res.error || !res.result || !res.result.output) {
        console.error("Execution error, full response:", JSON.stringify(res));
        return;
    }

    const decoded = modInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
    const outVal = Number(decoded[0]) / 1e18;
    console.log(`Input: 0.5, Carrier: -0.8 => Output = ${outVal}`);

    if (Math.abs(outVal - (-0.4)) < 1e-9) {
        console.log("\n★★★ ALL RING MODULATOR TESTS PASSED ★★★");
    } else {
        console.log(`\n❌ Incorrect multiplication result: expected -0.4, got ${outVal}`);
    }
}

test().catch(console.error);
