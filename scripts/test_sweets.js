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

async function main() {
    console.log("=== Testing Dan Fylstra's SWEETS Text Editor Logic on 6502 CPU ===");
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;
    console.log("CPU 6502 Address:", cpuAddress);

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

    // Reset Registers (PC = 0x2000 / 8192, SP = 0xFF, SR = 0)
    console.log("Resetting CPU registers...");
    await sendTx(cpu.batchPoke([0x80, 0x81, 0x82, 0x83, 0x84, 0x85], [0, 0, 0, 0xFF, 0, 0x2000], { nonce }));

    // SWEETS 6502 program bytecode
    // It initializes buffer pointers at $1E-$1F and $20-$21 to $2100.
    // Then inserts 'A' (0x41), 'B' (0x42), and 'C' (0x43) using indirect indexed address ($20),Y.
    const prog = [
        0xA9, 0x00,       // LDA #$00
        0x85, 0x1E,       // STA $1E
        0x85, 0x20,       // STA $20
        0xA9, 0x21,       // LDA #$21
        0x85, 0x1F,       // STA $1F
        0x85, 0x21,       // STA $21
        0xA0, 0x00,       // LDY #$00
        
        // Insert 'A'
        0xA9, 0x41,       // LDA #$41
        0x91, 0x20,       // STA ($20),Y
        0xE6, 0x20,       // INC $20
        0xD0, 0x02,       // BNE NEXT1
        0xE6, 0x21,       // INC $21
        // NEXT1:
        
        // Insert 'B'
        0xA9, 0x42,       // LDA #$42
        0x91, 0x20,       // STA ($20),Y
        0xE6, 0x20,       // INC $20
        0xD0, 0x02,       // BNE NEXT2
        0xE6, 0x21,       // INC $21
        // NEXT2:
        
        // Insert 'C'
        0xA9, 0x43,       // LDA #$43
        0x91, 0x20,       // STA ($20),Y
        0xE6, 0x20,       // INC $20
        0xD0, 0x02,       // BNE NEXT3
        0xE6, 0x21,       // INC $21
        // NEXT3:
        
        0x00              // BRK
    ];

    console.log("Loading SWEETS program into RAM at $2000...");
    const bytesHex = ethers.hexlify(new Uint8Array(prog));
    await sendTx(cpu.pokeBytes(0x2000, bytesHex, { nonce }));

    console.log("Running SWEETS simulation steps...");
    await sendTx(cpu.runSteps(150, { nonce }));

    // Read screen/buffer contents at $2100
    const char1 = Number(await cpu.peek(0x2100));
    const char2 = Number(await cpu.peek(0x2101));
    const char3 = Number(await cpu.peek(0x2102));

    const cursorLow = Number(await cpu.peek(0x20));
    const cursorHigh = Number(await cpu.peek(0x21));

    console.log("\n=== Results ===");
    console.log(`Buffer [$2100]: ${String.fromCharCode(char1)} (0x${char1.toString(16)})`);
    console.log(`Buffer [$2101]: ${String.fromCharCode(char2)} (0x${char2.toString(16)})`);
    console.log(`Buffer [$2102]: ${String.fromCharCode(char3)} (0x${char3.toString(16)})`);
    console.log(`Cursor location pointers ($20-$21): $${cursorHigh.toString(16).padStart(2, "0")}${cursorLow.toString(16).padStart(2, "0")}`);

    if (char1 === 0x41 && char2 === 0x42 && char3 === 0x43 && cursorLow === 0x03 && cursorHigh === 0x21) {
        console.log("\n>>> SUCCESS: SWEETS text editor buffer entry emulation verified perfectly! <<<");
    } else {
        console.error("\n>>> FAILURE: SWEETS editor buffer values do not match expected result! <<<");
        process.exit(1);
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
