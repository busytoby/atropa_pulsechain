const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

// 6502 Opcodes
const OP = {
    LDA_IM: 0xA9, 
    STA_ZP: 0x85, 
    LDX_IM: 0xA2, 
    INX: 0xE8,    
    STA_ABS: 0x8D,
    JMP: 0x4C     
};

function assemble(codeLines) {
    const bytes = [];
    const labels = {};
    const placeholders = [];
    let origin = 0x200; 
    let currentPC = origin;

    for (let line of codeLines) {
        line = line.trim().split(";")[0].trim();
        if (!line) continue;

        if (line.endsWith(":")) {
            const labelName = line.slice(0, -1).trim();
            labels[labelName] = currentPC;
            continue;
        }

        const tokens = line.split(/\s+/);
        const mnemonic = tokens[0].toUpperCase();
        const arg = tokens[1] || "";

        if (mnemonic === "LDA") {
            if (arg.startsWith("#$")) {
                bytes.push(OP.LDA_IM, parseInt(arg.slice(2), 16));
                currentPC += 2;
            }
        } else if (mnemonic === "STA") {
            const val = parseInt(arg.replace("$", ""), 16);
            if (val <= 0xFF) {
                bytes.push(OP.STA_ZP, val);
                currentPC += 2;
            } else {
                bytes.push(OP.STA_ABS, val & 0xFF, (val >> 8) & 0xFF);
                currentPC += 3;
            }
        } else if (mnemonic === "LDX") {
            if (arg.startsWith("#$")) {
                bytes.push(OP.LDX_IM, parseInt(arg.slice(2), 16));
                currentPC += 2;
            }
        } else if (mnemonic === "INX") {
            bytes.push(OP.INX);
            currentPC += 1;
        } else if (mnemonic === "JMP") {
            const labelTarget = arg;
            placeholders.push({ pcOffset: bytes.length + 1, target: labelTarget });
            bytes.push(OP.JMP, 0x00, 0x00);
            currentPC += 3;
        }
    }

    for (const p of placeholders) {
        const addr = labels[p.target];
        if (addr === undefined) {
            throw new Error(`Undefined label: ${p.target}`);
        }
        bytes[p.pcOffset] = addr & 0xFF;
        bytes[p.pcOffset + 1] = (addr >> 8) & 0xFF;
    }

    return { bytes, origin };
}

function compileYul(filename) {
    const yulPath = path.join(__dirname, "../solidity/bin", filename);
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("=================================================");
    console.log("DEMO 1: Compiling & Running on CPU6502Emulator");
    console.log("=================================================");

    const srcCpu = [
        "START:",
        "LDA #$42",
        "STA $1000",
        "LDX #$0F",
        "INX",
        "JMP START"
    ];

    const { bytes: cpuBytes, origin: cpuOrigin } = assemble(srcCpu);
    console.log(`Assembled program bytes: [${cpuBytes.map(b => "0x" + b.toString(16).padStart(2, "0")).join(", ")}]`);

    // Write Demo 1 compiled bytes to demo6502_1.bin
    const cpuBinPath = path.join(__dirname, "../solidity/bin/demo6502_1.bin");
    fs.writeFileSync(cpuBinPath, Buffer.from(cpuBytes));
    console.log(`Saved compiled 6502 binary to: ${cpuBinPath}`);

    console.log("Compiling and deploying cpu6502.yul...");
    const cpuBytecode = compileYul("cpu6502.yul");
    const cpuABI = [
        "function poke(uint256 addr, uint256 val) external",
        "function peek(uint256 addr) external view returns (uint256)",
        "function runSteps(uint256 maxSteps) external returns (uint256)",
        "function getCPUState() external view returns (uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 SP, uint256 PC)"
    ];

    const cpuFactory = new ethers.ContractFactory(cpuABI, cpuBytecode, signer);
    const cpuContract = await cpuFactory.deploy();
    await cpuContract.waitForDeployment();
    const cpuAddr = await cpuContract.getAddress();
    console.log(`CPU6502Emulator deployed at: ${cpuAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");
    async function sendTx(contract, method, ...args) {
        const tx = await contract[method](...args, { nonce });
        nonce++;
        return tx.wait();
    }

    // Load code into CPU memory
    for (let i = 0; i < cpuBytes.length; i++) {
        await sendTx(cpuContract, "poke", cpuOrigin + i, cpuBytes[i]);
    }

    // Set PC register (0x85)
    await sendTx(cpuContract, "poke", 0x85, cpuOrigin);

    console.log("Running 10 steps of the CPU emulator...");
    await sendTx(cpuContract, "runSteps", 10);

    const state = await cpuContract.getCPUState();
    const memVal = await cpuContract.peek(0x1000);
    console.log(`\nState after execution:`);
    console.log(`Accumulator (A): 0x${state.A.toString(16).padStart(2, "0")}`);
    console.log(`X Register:      0x${state.X.toString(16).padStart(2, "0")}`);
    console.log(`PC Register:     0x${state.PC.toString(16).padStart(4, "0")}`);
    console.log(`Value at $1000:  0x${memVal.toString(16).padStart(2, "0")}`);

    console.log("\n=================================================");
    console.log("DEMO 2: Playing/Interacting on Folklore Yul CPU");
    console.log("=================================================");

    // For Folklore, let's assemble a simple game movement sequence:
    // Moves right twice, then moves left once.
    // Inputs: moves stored at 55025 (Right = 2, Left = 1)
    const srcFolk = [
        "LDA #$02",
        "STA $D6F1", // Slot 55025: move direction
        "LDA #$01",
        "STA $D6F0", // Slot 55024: trigger tick
        "LDA #$02",
        "STA $D6F1",
        "LDA #$01",
        "STA $D6F0",
        "LDA #$01",
        "STA $D6F1", // Left
        "LDA #$01",
        "STA $D6F0"
    ];
    const { bytes: folkBytes } = assemble(srcFolk);
    const folkBinPath = path.join(__dirname, "../solidity/bin/demofolklore_1.bin");
    fs.writeFileSync(folkBinPath, Buffer.from(folkBytes));
    console.log(`Saved compiled folklore binary to: ${folkBinPath}`);

    console.log("Compiling and deploying folklore.yul...");
    const folkBytecode = compileYul("folklore.yul");
    const folkABI = [
        "function poke(uint256 addr, uint256 val) external returns (uint256)",
        "function peek(uint256 addr) external view returns (uint256)"
    ];

    const folkFactory = new ethers.ContractFactory(folkABI, folkBytecode, signer);
    const folkContract = await folkFactory.deploy();
    await folkContract.waitForDeployment();
    const folkAddr = await folkContract.getAddress();
    console.log(`Folklore CPU deployed at: ${folkAddr}`);

    nonce = await provider.getTransactionCount(signer.address, "pending");

    // Let's initialize standard game settings in Folklore memory slots:
    // Slot 55027: Player X coordinate (starts at 100)
    // Slot 55028: Player Y coordinate (starts at 500)
    // Slot 55031: Player Energy (starts at 100)
    console.log("Initializing player coordinates and stats...");
    await sendTx(folkContract, "poke", 55027, 100);
    await sendTx(folkContract, "poke", 55028, 500);
    await sendTx(folkContract, "poke", 55031, 100); // 100 energy

    // Instruct moving right: Slot 55025 := 2 (Right movement)
    console.log("Commanding Player to move Right...");
    await sendTx(folkContract, "poke", 55025, 2);

    // Trigger physical environment tick: writing 1 to Slot 55024 executes physics processing on-chain
    console.log("Triggering Folklore physics cycle update...");
    await sendTx(folkContract, "poke", 55024, 1);

    // Read updated coordinates
    const px = await folkContract.peek(55027);
    const py = await folkContract.peek(55028);
    const energy = await folkContract.peek(55031);

    console.log(`\nNew coordinate: (${px}, ${py})`);
    console.log(`Player Energy:  ${energy}`);

    console.log("\n>>> SUCCESS: Both CPU and Folklore demos compiled & executed successfully! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});

