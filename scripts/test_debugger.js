const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing On-Line Debugger & CPU Emulator (debugger.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/debugger.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/debugger.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const debuggerABI = [
        "function writeRegister(uint8 regId, uint256 val) external returns (uint256)",
        "function readRegister(uint8 regId) external view returns (uint256)",
        "function writeMemory(uint256 addr, uint8 val) external returns (uint256)",
        "function readMemory(uint256 addr) external view returns (uint8)",
        "function setBreakpoint(uint256 addr) external returns (uint256)",
        "function clearBreakpoint(uint256 addr) external returns (uint256)",
        "function getBreakpoints() external view returns (uint256[] memory)",
        "function runDebugger(uint256 maxSteps) external returns (uint256 exitCode, uint256 stepsExecuted)"
    ];

    const factory = new ethers.ContractFactory(debuggerABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Debugger contract deployed at: ${contractAddr}\n`);

    const gasLimit = 150000;

    // Load Program:
    // Address 0x0000: 0x01 (MVI A)
    // Address 0x0001: 0x05 (data)
    // Address 0x0002: 0x02 (MVI B)
    // Address 0x0003: 0x03 (data)
    // Address 0x0004: 0x03 (ADD B)
    // Address 0x0005: 0x08 (STA)
    // Address 0x0006: 0x00 (addr high)
    // Address 0x0007: 0x10 (addr low: 0x0010)
    // Address 0x0008: 0xFF (HLT)
    const program = [
        0x01, 0x05,
        0x02, 0x03,
        0x03,
        0x08, 0x00, 0x10,
        0xFF
    ];

    console.log("Loading program into virtual memory...");
    for (let i = 0; i < program.length; i++) {
        await (await contract.writeMemory(i, program[i], { gasLimit })).wait();
    }

    // Set breakpoint at 0x0004 (ADD B)
    console.log("Setting breakpoint at 0x0004...");
    await (await contract.setBreakpoint(4, { gasLimit })).wait();

    // Verify breakpoint was set
    let bps = await contract.getBreakpoints();
    console.log(`Current breakpoints: [${bps.map(b => b.toString()).join(", ")}]`);
    if (bps.length !== 1 || Number(bps[0]) !== 4) {
        throw new Error("Failed to set breakpoint correctly!");
    }

    // Initialize PC to 0
    await (await contract.writeRegister(0, 0, { gasLimit })).wait();

    // Run debugger
    console.log("Running debugger...");
    let tx = await contract.runDebugger.staticCall(10);
    console.log(`staticCall runDebugger result -> exitCode: ${tx.exitCode}, stepsExecuted: ${tx.stepsExecuted}`);
    
    // Execute tx for real
    await (await contract.runDebugger(10, { gasLimit })).wait();

    // Inspect state
    let pc = await contract.readRegister(0);
    let a = await contract.readRegister(2);
    let b = await contract.readRegister(3);
    console.log(`After breakpoint stop -> PC: ${pc}, A: ${a}, B: ${b}`);

    if (Number(pc) !== 4 || Number(a) !== 5 || Number(b) !== 3) {
        throw new Error("Debugger state mismatch after hitting breakpoint!");
    }

    // Clear breakpoint at 0x0004
    console.log("Clearing breakpoint at 0x0004...");
    await (await contract.clearBreakpoint(4, { gasLimit })).wait();
    bps = await contract.getBreakpoints();
    console.log(`Breakpoints after clear: [${bps.map(b => b.toString()).join(", ")}]`);
    if (bps.length !== 0) {
        throw new Error("Breakpoint not cleared!");
    }

    // Resume execution
    console.log("Resuming execution...");
    let resumeTx = await contract.runDebugger.staticCall(10);
    console.log(`staticCall resume result -> exitCode: ${resumeTx.exitCode}, stepsExecuted: ${resumeTx.stepsExecuted}`);

    await (await contract.runDebugger(10, { gasLimit })).wait();

    // Verify end state
    pc = await contract.readRegister(0);
    a = await contract.readRegister(2);
    let flags = await contract.readRegister(4);
    let mem10 = await contract.readMemory(0x10);
    console.log(`Final state -> PC: ${pc}, A: ${a}, Flags: ${flags}, Memory[0x0010]: ${mem10}`);

    if (Number(pc) !== 8 || Number(a) !== 8 || Number(mem10) !== 8) {
        throw new Error("Execution completion verification failed!");
    }

    console.log(">>> ALL ONLINE DEBUGGER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
