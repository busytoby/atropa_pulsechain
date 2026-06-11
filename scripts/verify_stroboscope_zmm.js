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
    console.log("=== Testing StroboscopeOscillator on ZMM VM ===");

    const oscAddr = "0x0000000000000000000000000000000000000088";

    console.log("Registering StroboscopeOscillator...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "StroboscopeOscillator", "../solidity/bin/stroboscopeOscillator.yul", ${oscAddr}`
    });
    console.log("StroboscopeOscillator register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 bias, int256 pitch) public returns (int256)"
    ]);

    // Feed a constant Vcc of 10V (10e18) to trigger relaxation oscillation
    const vccInput = 10000000000000000000n; // 10V

    console.log("\nSimulating 25 steps of stroboscopic gas-discharge relaxation oscillation:");
    for (let i = 0; i < 25; i++) {
        const calldata = iface.encodeFunctionData("processSample", [vccInput, 0n]);
        const res = await postJson("/api/zmm-exec", { name: "StroboscopeOscillator", calldata });
        
        if (res.error) {
            console.error(`Step ${i + 1} Error:`, res.error);
            continue;
        }

        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        const volts = Number(outVal) / 1e18;

        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round(volts / 10.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${ (i + 1).toString().padStart(2) }: Output = ${volts.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    console.log("\n★★★ STROBOSCOPE OSCILLATOR VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
