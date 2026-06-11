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
    console.log("=== Testing Steam Whistle & Avalanche Noise on ZMM VM ===");

    const whistleAddr = "0x0000000000000000000000000000000000000049";

    console.log("Registering SteamWhistle...");
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "SteamWhistle", "../solidity/bin/steamWhistle.yul", ${whistleAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const whistleInterface = new ethers.Interface([
        "function processSample(int256 trigger, int256 pitchAndNoiseMix) public returns (int256)"
    ]);

    // Parameters
    const pitch = 1500000000000000000n; // 1.5 pitch factor
    const noiseMix = 300000000000000000n; // 0.3 noise mix
    const packedVal = pitch + (noiseMix << 128n);

    console.log("Triggering whistle and simulating 15 steps...");
    let outputs = [];

    for (let i = 0; i < 15; i++) {
        const triggerVal = i === 0 ? 1000000000000000000n : 0n;

        const calldata = whistleInterface.encodeFunctionData("processSample", [triggerVal, packedVal]);
        const res = await postJson("/api/zmm-exec", {
            name: "SteamWhistle",
            calldata: calldata
        });

        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error, full response:", JSON.stringify(res));
            break;
        }

        const decoded = whistleInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVal = Number(decoded[0]) / 1e18;
        outputs.push(outVal);
        console.log(`Step ${i + 1}: Output = ${outVal.toFixed(6)}`);
    }

    let responded = false;
    for (let i = 0; i < outputs.length; i++) {
        if (Math.abs(outputs[i]) > 0.0) {
            responded = true;
        }
    }

    if (responded) {
        console.log("\n★★★ ALL STEAM WHISTLE TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Whistle returned zero response.");
    }
}

test().catch(console.error);
