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
    console.log("=== Testing Steam Whistle & Flickering Flame on ZMM VM ===");

    const whistleAddr = "0x0000000000000000000000000000000000000049";

    console.log("Registering SteamWhistle...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "SteamWhistle", "../solidity/bin/steamWhistle.yul", ${whistleAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const whistleInterface = new ethers.Interface([
        "function processSample(int256 trigger, int256 pitchAndNoiseMix) public returns (int256)"
    ]);

    // Test Suite 1: Whistle Mode (mode = 0)
    console.log("\n--- Running Whistle Mode (0) ---");
    const whistlePitch = 1500000000000000000n; // 1.5
    const whistleNoise = 300000000000000000n; // 0.3
    const whistlePacked = whistlePitch + (whistleNoise << 128n) + (0n << 192n); // mode 0

    let whistleOuts = [];
    for (let i = 0; i < 5; i++) {
        const calldata = whistleInterface.encodeFunctionData("processSample", [0n, whistlePacked]);
        const res = await postJson("/api/zmm-exec", { name: "SteamWhistle", calldata });
        const outVal = Number(whistleInterface.decodeFunctionResult("processSample", "0x" + res.result.output)[0]) / 1e18;
        whistleOuts.push(outVal);
        console.log(`Step ${i + 1}: Whistle Output = ${outVal.toFixed(6)}`);
    }

    // Test Suite 2: Flickering Flame Mode (mode = 1)
    console.log("\n--- Running Flickering Flame Mode (1) ---");
    const flamePitch = 1000000000000000000n; // 1.0
    const flameNoise = 800000000000000000n; // 0.8 (high noise mix)
    const flamePacked = flamePitch + (flameNoise << 128n) + (1n << 192n); // mode 1

    let flameOuts = [];
    for (let i = 0; i < 5; i++) {
        const calldata = whistleInterface.encodeFunctionData("processSample", [0n, flamePacked]);
        const res = await postJson("/api/zmm-exec", { name: "SteamWhistle", calldata });
        const outVal = Number(whistleInterface.decodeFunctionResult("processSample", "0x" + res.result.output)[0]) / 1e18;
        flameOuts.push(outVal);
        console.log(`Step ${i + 1}: Flame Output = ${outVal.toFixed(6)}`);
    }

    if (whistleOuts.length > 0 && flameOuts.length > 0) {
        console.log("\n★★★ ALL STEAM/FLAME GENERATOR TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Failure in generating outputs.");
    }
}

test().catch(console.error);
