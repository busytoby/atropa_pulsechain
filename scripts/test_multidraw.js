const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const cpuABI = [
    "function poke(uint256 addr, uint256 val) public returns (uint256)",
    "function peek(uint256 addr) public view returns (uint256)",
    "function pokeBytes(uint256 startAddr, bytes calldata data) public returns (uint256)",
    "function initializeGame(uint256 pcVal, bytes calldata program, uint256[] calldata addrs, uint256[] calldata vals) public returns (uint256)",
    "function runSteps(uint256 maxSteps) public returns (uint256)",
    "function getCPUState() public view returns (uint256, uint256, uint256, uint256, uint256, uint256)"
];

async function main() {
    console.log("=== Running Ahoy! Multi-Draw 64 Emulation Test ===");
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;
    console.log(`CPU Contract Address: ${cpuAddress}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    const cpu = new ethers.Contract(cpuAddress, cpuABI, deployer);

    // Multi-Draw 64 Core Assembly Loop
    // LDA #160 -> STA $02 (X_LO)
    // LDA #0   -> STA $03 (X_HI)
    // LDA #100 -> STA $04 (Y)
    // LDA $DC00 -> AND #0x08 -> BNE NO_RIGHT -> INC $02
    // LDA $DC00 -> AND #0x10 -> BNE NO_FIRE
    // LDA #1 -> STA $D570 (Draw Pixel Command)
    // LDA $02 -> STA $D571 (X0_LO)
    // LDA $03 -> STA $D572 (X0_HI)
    // LDA $04 -> STA $D573 (Y0)
    // LDA #1 -> STA $D577 (Color) -> STA $D578 (Trigger)
    // BRK
    const program = [
        0xA9, 0xA0,       // LDA #160
        0x85, 0x02,       // STA $02
        0xA9, 0x00,       // LDA #0
        0x85, 0x03,       // STA $03
        0xA9, 0x64,       // LDA #100
        0x85, 0x04,       // STA $04
        0xAD, 0x00, 0xDC, // LDA $DC00
        0x29, 0x08,       // AND #$08
        0xD0, 0x02,       // BNE NO_RIGHT
        0xE6, 0x02,       // INC $02
        // NO_RIGHT:
        0xAD, 0x00, 0xDC, // LDA $DC00
        0x29, 0x10,       // AND #$10
        0xD0, 0x1C,       // BNE NO_FIRE (skip 28 bytes)
        0xA9, 0x01,       // LDA #1
        0x8D, 0x70, 0xD5, // STA $D570
        0xA5, 0x02,       // LDA $02
        0x8D, 0x71, 0xD5, // STA $D571
        0xA5, 0x03,       // LDA $03
        0x8D, 0x72, 0xD5, // STA $D572
        0xA5, 0x04,       // LDA $04
        0x8D, 0x73, 0xD5, // STA $D573
        0xA9, 0x01,       // LDA #1
        0x8D, 0x77, 0xD5, // STA $D577
        0x8D, 0x78, 0xD5, // STA $D578
        // NO_FIRE:
        0x00              // BRK
    ];

    // Simulate Joystick Port 2 input ($DC00 = 56320)
    // We press RIGHT and FIRE: value is 0xFF ^ 8 (Right) ^ 16 (Fire) = 231 (0xE7)
    const joystickVal = 231;

    console.log("Flashing Multi-Draw program and setting joystick active-low state...");
    await (await cpu.initializeGame(8192, ethers.getBytes(new Uint8Array(program)), [56320], [joystickVal])).wait();

    console.log("Executing simulation runSteps(100)...");
    await (await cpu.runSteps(150)).wait();

    const state = await cpu.getCPUState();
    console.log(`Execution ended. PC reached: ${state[5]}`);

    // Verify coordinates updated in zero-page
    const finalX = Number(await cpu.peek(2));
    const finalY = Number(await cpu.peek(4));
    console.log(`Zero Page Coordinates: X=${finalX} (Expected: 161), Y=${finalY} (Expected: 100)`);

    // Verify pixel plotted in Doodle Graphics canvas framebuffer
    // Pixel (161, 100) corresponds to cell:
    // cellX = 161 / 8 = 20
    // cellY = 100 / 8 = 12
    // lineY = 100 % 8 = 4
    // bitIdx = 7 - (161 % 8) = 6
    // Address: 8192 + (cellY * 320) + (cellX * 8) + lineY = 8192 + 3840 + 160 + 4 = 12196
    const canvasByte = Number(await cpu.peek(12196));
    console.log(`Canvas byte at 12196: ${canvasByte} (Expected bit 6 to be set, i.e. (canvasByte & 64) !== 0)`);

    if (finalX === 161 && (canvasByte & 64) !== 0) {
        console.log("\n>>> SUCCESS: Multi-Draw 64 emulation verified successfully! <<<");
    } else {
        throw new Error("Multi-Draw 64 verification failed!");
    }
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
