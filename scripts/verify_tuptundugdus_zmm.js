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
    console.log("=== Testing Elektor TUP-TUN-DUG-DUS Primitives on ZMM VM ===");

    const address = "0x0000000000000000000000000000000000000091";

    console.log("Registering TupTunDugDus contract...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "TupTunDugDus", "../solidity/bin/tupTunDugDus.yul", ${address}`
    });
    console.log("Registration response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function process(uint8 componentType, int256 inputVal) public returns (int256)"
    ]);

    const tests = [
        { type: 0, name: "TUN (Silicon NPN)", inputs: [0.0, 0.4, 0.8, -0.4] },
        { type: 1, name: "TUP (Silicon PNP)", inputs: [0.0, -0.4, -0.8, 0.4] },
        { type: 2, name: "DUG (Germanium Diode)", inputs: [0.0, 0.2, 0.8, -0.8] },
        { type: 3, name: "DUS (Silicon Diode)", inputs: [0.0, 0.2, 0.8, -0.8] }
    ];

    for (const t of tests) {
        console.log(`\nTesting ${t.name}:`);
        console.log("-----------------------------------------");
        for (const input of t.inputs) {
            const inputScaled = BigInt(Math.round(input * 1e18));
            const calldata = iface.encodeFunctionData("process", [t.type, inputScaled]);
            const res = await postJson("/api/zmm-exec", { name: "TupTunDugDus", calldata });
            
            if (res.error) {
                console.error(`Error processing ${input}V:`, res.error);
                continue;
            }
            
            const decoded = iface.decodeFunctionResult("process", "0x" + res.result.output);
            const outputVolts = Number(decoded[0]) / 1e18;
            console.log(`  Input: ${input.toFixed(2).padStart(5)}V => Output: ${outputVolts.toFixed(3).padStart(6)}V`);
        }
    }

    console.log("\n★★★ TUP-TUN-DUG-DUS PRIMITIVES VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
