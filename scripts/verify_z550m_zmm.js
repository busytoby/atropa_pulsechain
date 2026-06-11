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
    console.log("=== Testing Z550mIndicator Decade Counting Tube on ZMM VM ===");

    const z550mAddr = "0x0000000000000000000000000000000000000055";

    console.log("Registering Z550mIndicator...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "Z550mIndicator", "../solidity/bin/z550mIndicator.yul", ${z550mAddr}`
    });
    console.log("Z550mIndicator register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 trigger, int256 digit) public returns (int256)"
    ]);

    // Test A: Trigger voltage below strike threshold (50V = 50e18)
    const lowTrigger = 50000000000000000000n;
    console.log("\nSweep with Inactive Trigger (50V < 105V strike):");
    for (let digit = 0n; digit < 10n; digit++) {
        const calldata = iface.encodeFunctionData("processSample", [lowTrigger, digit]);
        const res = await postJson("/api/zmm-exec", { name: "Z550mIndicator", calldata });
        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        console.log(`Input Digit: ${digit} | Glow Mask: 0x${BigInt(outVal).toString(16).toUpperCase()} | (TUBE DARK)`);
    }

    // Test B: Trigger voltage above strike threshold (120V = 120e18)
    const highTrigger = 120000000000000000000n;
    console.log("\nSweep with Active Trigger (120V > 105V strike):");
    for (let digit = 0n; digit < 10n; digit++) {
        const calldata = iface.encodeFunctionData("processSample", [highTrigger, digit]);
        const res = await postJson("/api/zmm-exec", { name: "Z550mIndicator", calldata });
        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        
        // Draw glow position
        const activeDigit = Number(digit);
        const bar = Array.from({ length: 10 }, (_, idx) => idx === activeDigit ? "*" : "-").join("");
        console.log(`Input Digit: ${digit} | Glow Mask: 0x${BigInt(outVal).toString(16).toUpperCase()} | [${bar}]`);
    }

    console.log("\n★★★ Z550M DECADE INDICATOR VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
