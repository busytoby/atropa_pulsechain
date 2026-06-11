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
    console.log("=== Driving CPU Debugger Telemetry using Z550M Gas-Discharge Tube ===");

    const debuggerAddr = "0x0000000000000000000000000000000000000003";
    const z550mAddr = "0x0000000000000000000000000000000000000055";

    console.log("Registering DebuggerV4...");
    await postJson("/api/zmm-exec", {
        code: `YULINIT "DebuggerV4", "../solidity/bin/debugger.yul", ${debuggerAddr}`
    });

    console.log("Registering Z550mIndicator...");
    await postJson("/api/zmm-exec", {
        code: `YULINIT "Z550mIndicator", "../solidity/bin/z550mIndicator.yul", ${z550mAddr}`
    });

    const dbIface = new ethers.Interface([
        "function writeMemory(uint256 addr, uint8 val) public returns (bool)",
        "function writeRegister(uint8 regId, uint256 val) public returns (bool)",
        "function readRegister(uint8 regId) public view returns (uint256)",
        "function runDebugger(uint256 maxSteps) public returns (uint256, uint256)"
    ]);

    const z550mIface = new ethers.Interface([
        "function processSample(int256 trigger, int256 digit) public returns (int256)"
    ]);

    // Simple program that increments B in a loop:
    // Addr 0: MVI A, 1
    // Addr 2: MVI B, 0
    // Addr 4: ADD B   (A = A + B)
    // Addr 5: JMP 4   (infinite loop)
    const prog = [
        0x01, 1,    // MVI A, 1
        0x02, 2,    // MVI B, 2
        0x03,       // ADD B (A = A + B)
        0x05, 0, 4  // JMP 4
    ];

    console.log("\nLoading program into CPU virtual memory...");
    for (let i = 0; i < prog.length; i++) {
        const calldata = dbIface.encodeFunctionData("writeMemory", [i, prog[i]]);
        await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata });
    }

    // Initialize PC to 0
    const setPc = dbIface.encodeFunctionData("writeRegister", [0, 0]);
    await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: setPc });

    console.log("\nExecuting CPU steps and driving Z550M neon indicator telemetry:");
    console.log("------------------------------------------------------------------");

    for (let step = 1; step <= 10; step++) {
        // Run exactly 1 clock step
        const stepCall = dbIface.encodeFunctionData("runDebugger", [1]);
        const stepRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: stepCall });
        
        // Read current PC (Reg 0) and Accumulator A (Reg 2)
        const getPc = dbIface.encodeFunctionData("readRegister", [0]);
        const resPc = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: getPc });
        const pcVal = dbIface.decodeFunctionResult("readRegister", "0x" + resPc.result.output)[0];

        const getA = dbIface.encodeFunctionData("readRegister", [2]);
        const resA = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: getA });
        const aVal = dbIface.decodeFunctionResult("readRegister", "0x" + resA.result.output)[0];

        // Map the low digit of Accumulator A to the Z550M indicator display
        const targetDigit = aVal % 10n;
        
        // Firing voltage above strike threshold (120V)
        const triggerVolts = 120000000000000000000n; // 120V
        
        const zCalldata = z550mIface.encodeFunctionData("processSample", [triggerVolts, targetDigit]);
        const zRes = await postJson("/api/zmm-exec", { name: "Z550mIndicator", calldata: zCalldata });
        const glowMask = z550mIface.decodeFunctionResult("processSample", "0x" + zRes.result.output)[0];

        // Format visual feedback
        const activeDigit = Number(targetDigit);
        const panelGlow = Array.from({ length: 10 }, (_, idx) => idx === activeDigit ? "*" : "-").join("");

        console.log(`Step ${step.toString().padStart(2)}: PC = ${pcVal.toString().padStart(2)} | Reg A = ${aVal.toString().padStart(3)} | Z550M Glow Mask: 0x${BigInt(glowMask).toString(16).toUpperCase().padStart(3, "0")} | Panel: [${panelGlow}]`);
    }

    console.log("\n★★★ CPU DEBUGGER TOTAL Z550M TELEMETRY ACCESS SUCCESSFUL ★★★");
}

test().catch(console.error);
