const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const cpuABI = [
    "function getCPUState() public view returns (uint256, uint256, uint256, uint256, uint256, uint256)",
    "function runSteps(uint256 maxSteps) public returns (uint256)",
    "function poke(uint256 addr, uint256 val) public returns (uint256)",
    "function peek(uint256 addr) public view returns (uint256)",
    "function pokeBytes(uint256 startAddr, bytes calldata data) public returns (uint256)",
    "function batchPoke(uint256[] calldata addrs, uint256[] calldata vals) public returns (uint256)"
];

async function runSimulation(index, useBug) {
    console.log(`\n--- Running Simulation with Index ${index} (using ${useBug ? "Fylstra's ASL bug" : "Gates' CLC + ADC correction"}) ---`);
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const cpu = new ethers.Contract(cpuAddress, cpuABI, signer);

    let nonce = await provider.getTransactionCount(signer.address, "pending");

    async function sendTx(promise) {
        const tx = await promise;
        nonce++;
        return tx.wait();
    }

    // Reset CPU Registers (PC = 0x2000, SP = 0xFF, SR = 0)
    await sendTx(cpu.batchPoke([0x80, 0x81, 0x82, 0x83, 0x84, 0x85], [0, 0, 0, 0xFF, 0, 0x2000], { nonce }));

    // Write Branch routines to memory
    // Routine 0 ($3100): LDA #$A0; STA $50; RTS
    await sendTx(cpu.pokeBytes(0x3100, ethers.getBytes("0xA9A0855060"), { nonce }));
    // Routine 1 ($3200): LDA #$B0; STA $50; RTS
    await sendTx(cpu.pokeBytes(0x3200, ethers.getBytes("0xA9B0855060"), { nonce }));
    // Routine 2 ($3300): LDA #$C0; STA $50; RTS
    await sendTx(cpu.pokeBytes(0x3300, ethers.getBytes("0xA9C0855060"), { nonce }));

    // Write Jump Table at $3000:
    // Offset 0: JMP $3100 (4C 00 31)
    // Offset 3: JMP $3200 (4C 00 32)
    // Offset 6: JMP $3300 (4C 00 33)
    await sendTx(cpu.pokeBytes(0x3000, ethers.getBytes("0x4C00314C00324C0033"), { nonce }));

    // Main routine bytecode
    const mainProg = [];
    
    // 1. Initialize Index in Acc and ZP $10
    mainProg.push(0xA9, index); // LDA #index
    mainProg.push(0x85, 0x10);  // STA $10
    
    if (useBug) {
        // Original bug: shift left (ASL) only multiplies by 2
        mainProg.push(0x0A);    // ASL A (offset = index * 2)
    } else {
        // Gates' fix: offset = (index * 2) + index
        mainProg.push(0x0A);    // ASL A (index * 2)
        mainProg.push(0x18);    // CLC
        mainProg.push(0x65, 0x10); // ADC $10 (add index)
    }
    
    mainProg.push(0xAA);        // TAX
    
    // Copy 3-byte JMP instruction from $3000,X into executable scratch RAM at $40
    mainProg.push(0xBD, 0x00, 0x30); // LDA $3000,X
    mainProg.push(0x85, 0x40);       // STA $40
    mainProg.push(0xBD, 0x01, 0x30); // LDA $3001,X
    mainProg.push(0x85, 0x41);       // STA $41
    mainProg.push(0xBD, 0x02, 0x30); // LDA $3002,X
    mainProg.push(0x85, 0x42);       // STA $42
    
    // Execute subroutine via jump scratch space
    mainProg.push(0x20, 0x40, 0x00); // JSR $0040
    
    mainProg.push(0x00);             // BRK

    const mainProgHex = ethers.hexlify(new Uint8Array(mainProg));
    await sendTx(cpu.pokeBytes(0x2000, mainProgHex, { nonce }));

    console.log("Running simulation steps...");
    await sendTx(cpu.runSteps(150, { nonce }));

    const result = Number(await cpu.peek(0x50));
    console.log(`Subroutine output value ($50): 0x${result.toString(16).toUpperCase()}`);
    return result;
}

async function main() {
    // 1. Run correct lookup (Gates' fix) for index 1
    const resCorrect = await runSimulation(1, false);
    if (resCorrect !== 0xB0) {
        console.error("Gates correction simulation failed! Expected 0xB0.");
        process.exit(1);
    }
    console.log("-> Success: Gates correction correctly routed to Routine 1 (0xB0)!");

    // 2. Run bugged lookup (Fylstra's ASL bug) for index 1
    try {
        const resBug = await runSimulation(1, true);
        console.log(`-> Observation: Bugged lookup routed to offset 2, resulting in value 0x${resBug.toString(16).toUpperCase()}`);
    } catch (err) {
        console.log("-> Success: Fylstra'\''s bugged run crashed/reverted as expected! Invalid opcode or address routing caused a CPU exception.");
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
