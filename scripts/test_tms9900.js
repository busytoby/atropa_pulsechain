const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing TMS9900 Workspace Pointer Thread Scheduler (tms9900.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/tms9900.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/tms9900.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const tmsABI = [
        "function createThread(uint256 startPC, uint256 priority) external returns (uint256)",
        "function readThreadRegister(uint256 threadId, uint256 regId) external view returns (uint256)",
        "function writeThreadRegister(uint256 threadId, uint256 regId, uint256 val) external returns (uint256)",
        "function getThreadState(uint256 threadId) external view returns (uint256 wp, uint256 pc, uint256 status, uint256 priority)",
        "function writeMemory(uint256 addr, uint8 val) external returns (uint256)",
        "function runScheduler(uint256 totalSteps) external returns (uint256[] memory path)"
    ];

    const factory = new ethers.ContractFactory(tmsABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`TMS9900 Scheduler contract deployed at: ${contractAddr}\n`);

    const gasLimit = 5000000;

    // Load Program for Thread 0 (Starts at 0)
    // 0: LI R1, 10 -> [0x01, 1, 10]
    // 3: LI R2, 20 -> [0x01, 2, 20]
    // 6: ADD R1, R2 -> [0x02, 1, 2]
    // 9: YIELD -> [0x03]
    // 10: LI R1, 5 -> [0x01, 1, 5]
    // 13: ADD R1, R2 -> [0x02, 1, 2]
    // 16: HLT -> [0xFF]
    const prog0 = [
        0x01, 1, 10,
        0x01, 2, 20,
        0x02, 1, 2,
        0x03,
        0x01, 1, 5,
        0x02, 1, 2,
        0xFF
    ];

    // Load Program for Thread 1 (Starts at 100)
    // 100: LI R1, 50 -> [0x01, 1, 50]
    // 103: YIELD -> [0x03]
    // 104: LI R2, 100 -> [0x01, 2, 100]
    // 107: ADD R1, R2 -> [0x02, 1, 2]
    // 110: HLT -> [0xFF]
    const prog1 = [
        0x01, 1, 50,
        0x03,
        0x01, 2, 100,
        0x02, 1, 2,
        0xFF
    ];

    console.log("Loading programs into scheduler memory...");
    for (let i = 0; i < prog0.length; i++) {
        await (await contract.writeMemory(i, prog0[i], { gasLimit })).wait();
    }
    for (let i = 0; i < prog1.length; i++) {
        await (await contract.writeMemory(100 + i, prog1[i], { gasLimit })).wait();
    }

    console.log("Creating Thread 0 (startPC = 0, priority = 1)...");
    await (await contract.createThread(0, 1, { gasLimit })).wait();
    
    console.log("Creating Thread 1 (startPC = 100, priority = 2)...");
    await (await contract.createThread(100, 2, { gasLimit })).wait();

    // Verify initial states
    let state0 = await contract.getThreadState(0);
    let state1 = await contract.getThreadState(1);
    console.log(`Thread 0 Init State -> WP: ${state0.wp}, PC: ${state0.pc}, status: ${state0.status}, priority: ${state0.priority}`);
    console.log(`Thread 1 Init State -> WP: ${state1.wp}, PC: ${state1.pc}, status: ${state1.status}, priority: ${state1.priority}`);

    if (Number(state0.status) !== 1 || Number(state1.status) !== 1) {
        throw new Error("Threads should be in status 1 (Ready)");
    }

    console.log("\nRunning Scheduler for 30 steps...");
    // Let's call runScheduler statically to inspect the returned path array
    let pathTrace = await contract.runScheduler.staticCall(30);
    console.log(`Thread scheduling execution path trace: [${pathTrace.map(t => t.toString()).join(", ")}]`);

    // Run scheduler for real to update states
    await (await contract.runScheduler(30, { gasLimit })).wait();

    // Verify ending register values
    let r2_t0 = await contract.readThreadRegister(0, 2);
    let r2_t1 = await contract.readThreadRegister(1, 2);
    console.log(`Thread 0 Register R2 value: ${r2_t0} (Expected: 35)`);
    console.log(`Thread 1 Register R2 value: ${r2_t1} (Expected: 150)`);

    state0 = await contract.getThreadState(0);
    state1 = await contract.getThreadState(1);
    console.log(`Thread 0 End State -> WP: ${state0.wp}, PC: ${state0.pc}, status: ${state0.status}`);
    console.log(`Thread 1 End State -> WP: ${state1.wp}, PC: ${state1.pc}, status: ${state1.status}`);

    if (Number(r2_t0) !== 35 || Number(r2_t1) !== 150) {
        throw new Error("Register arithmetic output mismatched!");
    }

    if (Number(state0.status) !== 2 || Number(state1.status) !== 2) {
        throw new Error("Threads did not exit/halt properly!");
    }

    console.log(">>> ALL TMS9900 SCHEDULER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
