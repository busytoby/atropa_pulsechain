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
    console.log("=== Testing Upgraded 6502 Emulator Features ===");
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;
    console.log("CPU Address:", cpuAddress);

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

    // Reset Registers
    await sendTx(cpu.batchPoke([0x80, 0x81, 0x82, 0x83, 0x84, 0x85], [0, 0, 0, 0xFF, 0, 8192], { nonce }));

    // 1. Test Indexed Absolute: LDA abs,X & LDA abs,Y
    console.log("\n--- Part 1: Testing Indexed Absolute (LDA abs,X / LDA abs,Y) ---");
    // Write test data array at $2200: [0x40, 0x50, 0x60]
    await sendTx(cpu.pokeBytes(0x2200, ethers.getBytes("0x405060"), { nonce }));

    // Program:
    // LDX #$01
    // LDA $2200,X  ; loads 0x50
    // STA $10      ; store in zp $10
    // LDY #$02
    // LDA $2200,Y  ; loads 0x60
    // STA $11      ; store in zp $11
    // BRK          ; opcode 0
    const prog1 = [
        0xA2, 0x01,       // LDX #$01
        0xBD, 0x00, 0x22, // LDA $2200,X
        0x85, 0x10,       // STA $10
        0xA0, 0x02,       // LDY #$02
        0xB9, 0x00, 0x22, // LDA $2200,Y
        0x85, 0x11,       // STA $11
        0x00              // BRK
    ];

    await sendTx(cpu.pokeBytes(8192, ethers.getBytes(new Uint8Array(prog1)), { nonce }));
    await sendTx(cpu.poke(0x85, 8192, { nonce })); // PC = 8192
    await sendTx(cpu.runSteps(15, { nonce }));

    let zp10 = Number(await cpu.peek(0x10));
    let zp11 = Number(await cpu.peek(0x11));
    console.log(`zp $10 (Expected: 80 / 0x50): ${zp10} (0x${zp10.toString(16)})`);
    console.log(`zp $11 (Expected: 96 / 0x60): ${zp11} (0x${zp11.toString(16)})`);

    if (zp10 === 0x50 && zp11 === 0x60) {
        console.log("SUCCESS: Indexed Absolute modes verified!");
    } else {
        throw new Error("FAIL: Indexed Absolute modes failed validation!");
    }

    // 2. Test Indirect Indexed Y: LDA (zp),Y
    console.log("\n--- Part 2: Testing Indirect Indexed Y (LDA (zp),Y) ---");
    // Pointer at zp $20: low=$00, high=$22 -> points to $2200
    await sendTx(cpu.batchPoke([0x20, 0x21], [0x00, 0x22], { nonce }));

    // Program:
    // LDY #$01
    // LDA ($20),Y  ; loads $2201 -> 0x50
    // STA $12      ; store in zp $12
    // BRK
    const prog2 = [
        0xA0, 0x01,       // LDY #$01
        0xB1, 0x20,       // LDA ($20),Y
        0x85, 0x12,       // STA $12
        0x00              // BRK
    ];
    await sendTx(cpu.pokeBytes(8192, ethers.getBytes(new Uint8Array(prog2)), { nonce }));
    await sendTx(cpu.poke(0x85, 8192, { nonce })); // PC = 8192
    await sendTx(cpu.runSteps(15, { nonce }));

    let zp12 = Number(await cpu.peek(0x12));
    console.log(`zp $12 (Expected: 80 / 0x50): ${zp12} (0x${zp12.toString(16)})`);

    if (zp12 === 0x50) {
        console.log("SUCCESS: Indirect Indexed Y mode verified!");
    } else {
        throw new Error("FAIL: Indirect Indexed Y mode failed validation!");
    }

    // 3. Test VIC-II Raster Interrupt Integration
    console.log("\n--- Part 3: Testing Hardware IRQ / Raster Interrupts ---");
    // Clear status flags and register IRQ vector
    // Set IRQ vector at $FFFE/$FFFF (65534/65535) to point to $2050 (low: $50, high: $20)
    await sendTx(cpu.batchPoke([65534, 65535], [0x50, 0x20], { nonce }));

    // Clear interrupt flag registers and set compare line target to 50
    await sendTx(cpu.poke(53266, 50, { nonce })); // target raster line = 50
    await sendTx(cpu.poke(53274, 1, { nonce }));  // enable raster interrupts
    await sendTx(cpu.poke(53273, 0xFF, { nonce })); // clear pending flags
    await sendTx(cpu.poke(0x13, 0, { nonce }));   // clear result tracker zp $13

    // Main Loop:
    // CLI          ; enable interrupts (opcode 0x58)
    // JMP $2001    ; loop forever
    const mainLoop = [
        0x58,             // CLI
        0x4C, 0x01, 0x20  // JMP $2001
    ];
    await sendTx(cpu.pokeBytes(8192, ethers.getBytes(new Uint8Array(mainLoop)), { nonce }));

    // IRQ Handler at $2050 (8272):
    // LDA #$AF     ; arbitrary check val
    // STA $13
    // LDA #$01
    // STA $D019    ; write-1-to-clear raster interrupt flag
    // RTI          ; return from interrupt (opcode 0x40)
    const irqHandler = [
        0xA9, 0xAF,       // LDA #$AF
        0x85, 0x13,       // STA $13
        0xA9, 0x01,       // LDA #$01
        0x8D, 0x19, 0xD0, // STA $D019
        0x40              // RTI
    ];
    await sendTx(cpu.pokeBytes(8272, ethers.getBytes(new Uint8Array(irqHandler)), { nonce }));

    await sendTx(cpu.poke(0x85, 8192, { nonce })); // PC = 8192
    await sendTx(cpu.poke(0x84, 0, { nonce }));    // clear SR (insures I flag is clear)

    console.log("Simulating 350 cycles...");
    await sendTx(cpu.runSteps(350, { nonce }));

    let zp13 = Number(await cpu.peek(0x13));
    console.log(`zp $13 (Expected: 175 / 0xAF if interrupt fired): ${zp13}`);

    if (zp13 === 0xAF) {
        console.log("SUCCESS: Hardware IRQ Interrupt validation PASSED!");
    } else {
        throw new Error("FAIL: Interrupt did not fire or complete successfully!");
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
