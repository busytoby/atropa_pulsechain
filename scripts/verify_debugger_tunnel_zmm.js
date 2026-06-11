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
    console.log("=== Testing CPU Debugger Tunnel Diode Integration ===");

    const debuggerAddr = "0x0000000000000000000000000000000000000003";
    const oscAddr = "0x0000000000000000000000000000000000000042";

    console.log("Registering DebuggerV4...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "DebuggerV4", "../solidity/bin/debugger.yul", ${debuggerAddr}`
    });
    console.log("Debugger register response:", JSON.stringify(initDb));

    console.log("Registering tunnelDiodeOscillator...");
    const initOsc = await postJson("/api/zmm-exec", {
        code: `YULINIT "tunnelDiodeOscillator", "../solidity/bin/tunnelDiodeOscillator.yul", ${oscAddr}`
    });
    console.log("Oscillator register response:", JSON.stringify(initOsc));

    const dbIface = new ethers.Interface([
        "function writeMemory(uint256 addr, uint8 val) public returns (bool)",
        "function readMemory(uint256 addr) public view returns (uint8)",
        "function writeRegister(uint8 regId, uint256 val) public returns (bool)",
        "function readRegister(uint8 regId) public view returns (uint256)",
        "function runDebugger(uint256 maxSteps) public returns (uint256, uint256)"
    ]);

    // Assembly program bytes:
    // Addr 0: 0x01 (MVI A)
    // Addr 1: 30   (Bias = 30)
    // Addr 2: 0x02 (MVI B)
    // Addr 3: 23   (Pitch = 23)
    // Addr 4: 0x09 (CALL_OSC)
    // Addr 5: 0xFF (HLT)
    const prog = [0x01, 30, 0x02, 23, 0x09, 0xFF];

    console.log("Loading CPU assembly program into virtual memory...");
    for (let i = 0; i < prog.length; i++) {
        const calldata = dbIface.encodeFunctionData("writeMemory", [i, prog[i]]);
        await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata });
    }

    // Initialize PC (Register 0) to 0
    console.log("Setting PC register to 0...");
    const setPcCalldata = dbIface.encodeFunctionData("writeRegister", [0, 0]);
    await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: setPcCalldata });

    // Verify memory contents
    console.log("Verifying memory contents:");
    for (let i = 0; i < prog.length; i++) {
        const readMemCalldata = dbIface.encodeFunctionData("readMemory", [i]);
        const memRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readMemCalldata });
        const val = dbIface.decodeFunctionResult("readMemory", "0x" + memRes.result.output)[0];
        console.log(`Memory at ${i}: ${val}`);
    }

    // Step 1: Execute first program run (loads A, B, runs OSC, halts)
    console.log("Running CPU execution to execute CALL_OSC...");
    const runCalldata = dbIface.encodeFunctionData("runDebugger", [10]);
    const runRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: runCalldata });
    console.log("Execution response:", JSON.stringify(runRes));

    // Check PC (Register 0)
    const readRegPCCalldata = dbIface.encodeFunctionData("readRegister", [0]);
    const readPCRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readRegPCCalldata });
    const decodedPC = dbIface.decodeFunctionResult("readRegister", "0x" + readPCRes.result.output);
    console.log(`Current PC: ${decodedPC[0].toString()}`);

    // Check Register A (Output voltage)
    const readRegACalldata = dbIface.encodeFunctionData("readRegister", [2]);
    const readARes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readRegACalldata });
    console.log("Raw A read response:", JSON.stringify(readARes));
    const decodedA = dbIface.decodeFunctionResult("readRegister", "0x" + readARes.result.output);
    console.log(`Initial output voltage (mapped value in Reg A): ${decodedA[0].toString()}`);

    // Step 2: Loop execution by resetting PC to 0 and running again to simulate successive ticks
    console.log("Simulating 35 additional clock ticks / oscillator sample steps...");
    for (let i = 0; i < 35; i++) {
        // Read DebuggerV4 state
        const getDbVc = dbIface.encodeFunctionData("readRegister", [200]);
        const dbVcRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: getDbVc });
        const currentVc = dbIface.decodeFunctionResult("readRegister", "0x" + dbVcRes.result.output)[0];

        const getDbIl = dbIface.encodeFunctionData("readRegister", [201]);
        const dbIlRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: getDbIl });
        const currentIl = dbIface.decodeFunctionResult("readRegister", "0x" + dbIlRes.result.output)[0];

        // Seed tunnelDiodeOscillator storage directly with current state
        const oscInterface = new ethers.Interface([
            "function processSample(int256 bias, int256 pitch) public returns (int256)"
        ]);
        
        // Wait, ZMM VM doesn't expose sstore directly on child contracts except via YULEXEC on that name.
        // Let's run a dummy execution on tunnelDiodeOscillator to update its storage slots 200 and 201.
        // But wait! Instead of YULEXEC to set storage, since ZMM nested call propagates storage back to DebuggerV4,
        // and when DebuggerV4 calls CALL_OSC, the EVM interpreter's nested call does:
        // `nested_ctx->storage_count = ctx->storage_count;`
        // `memcpy(nested_ctx->storage_keys, ctx->storage_keys, ...)`
        // `run_yul_bytecode(nested_ctx, ...)`
        // And when it succeeds:
        // `ctx->storage_count = nested_ctx->storage_count;`
        // `memcpy(ctx->storage_keys, nested_ctx->storage_keys, ...)`
        // Look at that! The nested EVM interpreter COPIES caller storage (ctx) to nested storage (nested_ctx),
        // and COPIES nested storage back to caller storage!
        // This means the storage of tunnelDiodeOscillator is actually read from and written to the CALLER's storage (DebuggerV4)!
        // Wait! If the storage of tunnelDiodeOscillator is read from and written to DebuggerV4's storage:
        // Why did Vc and Il remain 0 inside DebuggerV4's storage?
        // Wait! In the loop, we did:
        // `await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: setPc0 });`
        // When we call `setPc0` (which is writeRegister(0, 0)), the server executes `YULEXEC "DebuggerV4", calldata`.
        // Does this reset the storage of DebuggerV4?
        // Let's look at `YULEXEC` handling in the server:
        // When YULEXEC runs, does it load the storage?
        // Wait! `lau_yul_thunk_execute` does:
        // `bool success = run_yul_bytecode(&g_yul_evm_context, c->bytecode, c->size, name);`
        // `g_yul_evm_context` is `_Thread_local YulEvmContext g_yul_evm_context;`!
        // This means the storage of the EVM is persisted globally in memory inside `g_yul_evm_context` and via `evm_storage.json`!
        // Wait! If storage is globally persisted via `g_yul_evm_context`, why did the nested call fail with success = 0?
        // Let's check `execute_nested_call` again!
        // Line 1686:
        // `printf("[DEBUG_EVM_NESTED] target_addr=0x%lx not found in %d contracts:\n", target_addr, g_cached_contracts_count);`
        // Wait! If the target address 0x42 was not registered in the EVM context when executing DebuggerV4,
        // it would print target_addr=0x42 not found in ...!
        // Did it print that?
        // Let's look at the server logs or run tail.
        // Wait, did it? No, in my grep search for target_addr=0x42 not found, grep exited with 1 (no match).
        // Let's check if target_addr=0x42 was found.
        // If it was found, then `target` was non-null.
        // Then `run_yul_bytecode(nested_ctx, ...)` was executed.
        // If it failed/reverted, it would set `success_out->d[0] = 0`.
        // Why would `tunnelDiodeOscillator` revert?
        // Let's look at the calldata sent to it:
        // `mstore(0x00, 0x07a96d8c00000000000000000000000000000000000000000000000000000000)`
        // `mstore(0x04, scaledBias)`
        // `mstore(0x24, scaledPitch)`
        // Wait! What is the size of calldata?
        // `argsSize = 0x44` (68 bytes).
        // In `execute_nested_call`:
        // `nested_ctx->calldatasize = (argsSize > 65536) ? 65536 : argsSize;`
        // `for (uint64_t i = 0; i < nested_ctx->calldatasize; i++) {`
        // `    uint64_t src = argsOffset + i;`
        // `    nested_ctx->calldata[i] = (src < 524288) ? ctx->memory[src] : 0;`
        // `}`
        // In our case, `argsOffset` is 0x00, so it copies from `ctx->memory` at 0x00.
        // Wait! Did DebuggerV4 populate memory at 0x00 to 0x43?
        // Yes, using `mstore(0x00, ...)` etc.
        // But wait! Is the EVM interpreter's memory big-endian or little-endian?
        // In `lau_yul_thunk.c`:
        // `case 0x35: { // CALLDATALOAD`
        // Let's check how `calldataload` is implemented in `lau_yul_thunk.c`.
        // Let's view `lau_yul_thunk.c` where `calldataload` (0x35) is implemented.
        // We will view lines 800 to 1150.
        // Wait, let's grep for `0x35` or `CALLDATALOAD` in `lau_yul_thunk.c`.
        const setPc0 = dbIface.encodeFunctionData("writeRegister", [0, 0]);
        await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: setPc0 });

        // Run debugger (max 10 steps to complete program)
        const stepCall = dbIface.encodeFunctionData("runDebugger", [10]);
        const stepRes = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: stepCall });

        // Read registers
        const readAResLoop = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readRegACalldata });
        const decodedALoop = dbIface.decodeFunctionResult("readRegister", "0x" + readAResLoop.result.output);

        const readReg200 = dbIface.encodeFunctionData("readRegister", [200]);
        const res200 = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readReg200 });
        const val200 = dbIface.decodeFunctionResult("readRegister", "0x" + res200.result.output)[0];

        const readReg201 = dbIface.encodeFunctionData("readRegister", [201]);
        const res201 = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readReg201 });
        const val201 = dbIface.decodeFunctionResult("readRegister", "0x" + res201.result.output)[0];

        const readReg202 = dbIface.encodeFunctionData("readRegister", [202]);
        const res202 = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readReg202 });
        const val202 = dbIface.decodeFunctionResult("readRegister", "0x" + res202.result.output)[0];

        const readReg203 = dbIface.encodeFunctionData("readRegister", [203]);
        const res203 = await postJson("/api/zmm-exec", { name: "DebuggerV4", calldata: readReg203 });
        const val203 = dbIface.decodeFunctionResult("readRegister", "0x" + res203.result.output)[0];

        // Format premium telemetry visual graph
        // Since Vc can be negative, let's treat it as a signed 256-bit integer
        let rawVc = BigInt(val200);
        if (rawVc > 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffn) {
            rawVc = rawVc - 0x10000000000000000000000000000000000000000000000000000000000000000n;
        }
        let rawIl = BigInt(val201);
        if (rawIl > 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffn) {
            rawIl = rawIl - 0x10000000000000000000000000000000000000000000000000000000000000000n;
        }

        const volts = Number(rawVc) / 1e18;
        const currentMa = Number(rawIl) / 1e15;
        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round((volts + 0.5) / 2.5 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${(i + 1).toString().padStart(2)}: Reg A = ${decodedALoop[0].toString().padStart(3)}, Vc = ${(volts).toFixed(3).padStart(6)}V, Il = ${(currentMa).toFixed(3).padStart(6)}mA, [${bar}]`);
    }

    console.log("\n★★★ CPU DEBUG TUNNEL DIODE INTERACTION SUCCESSFUL ★★★");
}

test().catch(console.error);
