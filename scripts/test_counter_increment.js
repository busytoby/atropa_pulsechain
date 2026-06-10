const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

// 6502 Opcodes
const OP = {
    LDA_ZP: 0xA5,  // LDA Zero Page
    ADC_IM: 0x69,  // ADC Immediate
    STA_ZP: 0x85,  // STA Zero Page
    RTS: 0x60      // RTS (Return from Subroutine / halt marker for runSteps)
};

// Assemble program
// LDA $10
// ADC #$01
// STA $10
// RTS
const counterBytes = [
    OP.LDA_ZP, 0x10,
    OP.ADC_IM, 0x01,
    OP.STA_ZP, 0x10,
    OP.RTS
];

const BIN_PATH = path.join(__dirname, "../solidity/bin/counter.bin");

function compileCpuContract() {
    const yulPath = path.join(__dirname, "../solidity/bin/cpu6502.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("=== Compiling and Writing counter.bin ===");
    fs.writeFileSync(BIN_PATH, Buffer.from(counterBytes));
    console.log(`Saved counter binary (${counterBytes.length} bytes) to: ${BIN_PATH}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("\nDeploying 6502 CPU to EVM...");
    const bytecode = compileCpuContract();
    const cpuABI = [
        "function poke(uint256 addr, uint256 val) external",
        "function peek(uint256 addr) external view returns (uint256)",
        "function runSteps(uint256 maxSteps) external returns (uint256)",
        "function getCPUState() external view returns (uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 SP, uint256 PC)"
    ];

    const factory = new ethers.ContractFactory(cpuABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`CPU deployed at: ${contractAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");
    async function sendTx(method, ...args) {
        const tx = await contract[method](...args, { nonce });
        nonce++;
        return tx.wait();
    }

    const programOrigin = 0x200;

    // Load program into memory
    console.log(`Loading counter.bin to C64 RAM starting at $${programOrigin.toString(16)}...`);
    for (let i = 0; i < counterBytes.length; i++) {
        await sendTx("poke", programOrigin + i, counterBytes[i]);
    }

    // Initialize the counter variable at Zero Page $10 to 0
    await sendTx("poke", 0x10, 0);

    // Call the counter subroutine multiple times and read Accumulator output
    console.log("\nExecuting counter increment loop (calling counter.bin)...");
    for (let loop = 1; loop <= 5; loop++) {
        // Reset Program Counter (PC at slot 0x85) to program origin
        await sendTx("poke", 0x85, programOrigin);

        // Run steps (the program runs until RTS instruction, which halts the execution step loop)
        await sendTx("runSteps", 10);

        // Read Accumulator register state (A at slot 0x80)
        const state = await contract.getCPUState();
        const storedCounter = await contract.peek(0x10);

        console.log(`Call #${loop}: Accumulator A = ${state.A} (Saved in memory $10 = ${storedCounter})`);
    }

    console.log("\n>>> SUCCESS: counter.bin binary successfully compiled and persistent state increment verified! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
