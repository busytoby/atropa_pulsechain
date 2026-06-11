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
    console.log("=== Testing Bionika Spiking Threshold Neuron on ZMM VM ===");

    const bioAddress = "0x0000000000000000000000000000000000000088";

    console.log("Registering BionikaNeuron...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "BionikaNeuron", "../solidity/bin/bionikaNeuron.yul", ${bioAddress}`
    });
    console.log("Bionika registration response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 inputSynapse, int256 leakRate) public returns (int256, int256)"
    ]);

    const steps = 15;
    const leak = 100000000000000000n; // 0.1 leak rate (10% decay per tick)
    const stimulus = 400000000000000000n; // 0.4V input pulse per step

    console.log("\nSimulating 15 clock ticks of leaky integration and spike generation:");
    console.log("----------------------------------------------------------------------");

    for (let i = 0; i < steps; i++) {
        const calldata = iface.encodeFunctionData("processSample", [stimulus, leak]);
        const res = await postJson("/api/zmm-exec", { name: "BionikaNeuron", calldata });
        
        const decoded = iface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const spike = Number(decoded[0]) / 1e18;
        const potential = Number(decoded[1]) / 1e18;

        const width = 20;
        const norm = Math.max(0, Math.min(width - 1, Math.round(potential * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx <= norm ? "█" : "-").join("");

        const spikeIndicator = spike > 0 ? "⚡ SPIKE!" : "        ";
        console.log(`Step ${(i + 1).toString().padStart(2)}: Stimulus = 0.4V | Potential = ${potential.toFixed(3).padStart(6)}V | [${bar}] | ${spikeIndicator}`);
    }

    console.log("\n★★★ BIONIKA ARTIFICIAL NEURON VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
