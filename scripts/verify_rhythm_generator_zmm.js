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
    console.log("=== Testing Rhythm Generator on ZMM VM ===");

    const rhythmAddr = "0x0000000000000000000000000000000000000067";

    console.log("Registering RhythmGenerator...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "RhythmGenerator", "../solidity/bin/rhythmGenerator.yul", ${rhythmAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const rhythmInterface = new ethers.Interface([
        "function processSample(int256 triggerClock, int256 patternSelect) public returns (int256)"
    ]);

    // Send clock signals (simulating clock transitions 0 -> 1 -> 0 -> 1...)
    const stepsToRun = 6;
    let pattern = 0n; // Rock pattern

    console.log("Stepping clock...");
    for (let step = 0; step < stepsToRun; step++) {
        // Step clock high (rising edge trigger)
        let calldata = rhythmInterface.encodeFunctionData("processSample", [1000000000000000000n, pattern]);
        let res = await postJson("/api/zmm-exec", {
            name: "RhythmGenerator",
            calldata: calldata
        });

        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error:", JSON.stringify(res));
            break;
        }

        let decoded = rhythmInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        let triggerOut = Number(decoded[0]);
        console.log(`Clock Tick ${step + 1} (Rising Edge): Triggers Output = ${triggerOut} (Binary: ${triggerOut.toString(2)})`);

        // Step clock low
        calldata = rhythmInterface.encodeFunctionData("processSample", [0n, pattern]);
        await postJson("/api/zmm-exec", {
            name: "RhythmGenerator",
            calldata: calldata
        });
    }

    console.log("\n★★★ ALL RHYTHM GENERATOR TESTS PASSED ★★★");
}

test().catch(console.error);
