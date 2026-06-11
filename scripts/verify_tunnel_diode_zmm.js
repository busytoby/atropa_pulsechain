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
    console.log("=== Testing Tunnel Diode Oscillator on ZMM VM ===");

    const oscAddr = "0x0000000000000000000000000000000000000042";

    console.log("Registering tunnelDiodeOscillator...");
    // The relative path in YULINIT is from the server script's context.
    const initRes = await postJson("/api/zmm-exec", {
        code: `YULINIT "tunnelDiodeOscillatorV5", "../solidity/bin/tunnelDiodeOscillator.yul", ${oscAddr}`
    });
    console.log("Initialization response:", JSON.stringify(initRes));

    const oscInterface = new ethers.Interface([
        "function processSample(int256 bias, int256 pitch) public returns (int256)"
    ]);

    // Set a bias current in the Negative Differential Resistance region (e.g. 3mA)
    const biasCurrent = 3000000000000n; // 3uA/mA scale matching the physical parameters
    const pitch = 22675n; // matching dt around 22 microseconds (44.1kHz rate)

    console.log("Simulating 15 cycles/updates of the oscillator...");
    let lastOut = 0n;
    let oscillated = false;

    for (let i = 0; i < 15; i++) {
        const calldata = oscInterface.encodeFunctionData("processSample", [biasCurrent, pitch]);
        const res = await postJson("/api/zmm-exec", {
            name: "tunnelDiodeOscillatorV5",
            calldata: calldata
        });
        
        console.log("Raw ZMM response:", JSON.stringify(res));
        
        if (res.error || !res.result || !res.result.output) {
            console.error("Execution error:", res.error);
            break;
        }

        const decoded = oscInterface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVal = decoded[0];
        console.log(`Sample ${i + 1}: Output Voltage = ${outVal.toString()} (Raw)`);
        
        if (i > 0 && outVal !== lastOut) {
            oscillated = true;
        }
        lastOut = outVal;
    }

    if (oscillated) {
        console.log("\n★★★ ALL TUNNEL DIODE OSCILLATOR TESTS PASSED ★★★");
    } else {
        console.log("\n❌ Oscillator failed to update or produce changing states.");
    }
}

test().catch(console.error);
