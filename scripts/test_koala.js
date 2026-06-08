const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

async function main() {
    console.log("=== STEP 1: Loading Deployed GraphicsSystem ===");
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const graphicsSystemAddress = config.networks.localhost.graphicsSystemAddress;
    console.log(`Graphics System Address: ${graphicsSystemAddress}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const gInterface = new ethers.Interface([
        "function updateKoalaPad(uint8 x, uint8 y, uint8 buttonState) external returns (uint256)",
        "function getKoalaPadState() external view returns (uint8 x, uint8 y, uint8 buttonState)",
        "function decodeKoalaPixel(bytes calldata fileData, uint16 x, uint16 y) external view returns (uint8 color)"
    ]);

    const gContract = new ethers.Contract(graphicsSystemAddress, gInterface, deployer);

    console.log("\n=== STEP 2: Testing KoalaPad Analog Touch Input ===");
    // Simulate touch at X=150, Y=90, button pressed (1)
    const tx = await gContract.updateKoalaPad(150, 90, 1);
    await tx.wait();
    console.log("Touch event sent: X=150, Y=90, Button=1");

    const state = await gContract.getKoalaPadState();
    console.log(`Touch event retrieved: X=${state.x}, Y=${state.y}, Button=${state.buttonState}`);

    if (Number(state.x) !== 150 || Number(state.y) !== 90 || Number(state.buttonState) !== 1) {
        console.error("FAIL: KoalaPad state mismatch!");
        process.exit(1);
    }
    console.log("SUCCESS: KoalaPad state successfully set and retrieved!");

    console.log("\n=== STEP 3: Testing KoalaPainter (.koa) Format Decoder ===");
    // Koala format files are 10003 bytes
    const mockKoa = new Uint8Array(10003);

    // Header 2 bytes (load address, ignored)
    mockKoa[0] = 0x00;
    mockKoa[1] = 0x60;

    // Background color (last byte in file) = 6 (Blue)
    mockKoa[10002] = 6;

    // Pixel 1: (15, 17) -> CX=1, CY=2, PY=1, PX=7.
    // CellIndex = 2 * 40 + 1 = 81.
    // Bitmap byte index = 2 + (81 * 8) + 1 = 651.
    // Bits 1-0 of byte 651 set to 3 (binary 11).
    mockKoa[651] = 0x03; // set lower two bits to 11
    // Color RAM byte index = 2 + 8000 + 1000 + cellIndex = 9083.
    mockKoa[9083] = 14; // Light Blue

    // Pixel 2: (20, 42) -> CX=2, CY=5, PY=2, PX=4.
    // CellIndex = 5 * 40 + 2 = 202.
    // Bitmap byte index = 2 + (202 * 8) + 2 = 1620.
    // Bits 3-2 of byte 1620 set to 2 (binary 10).
    mockKoa[1620] = 0x08; // set bits 3-2 to 10 (which is 0x08)
    // Screen RAM byte index = 2 + 8000 + cellIndex = 8204.
    mockKoa[8204] = 0x25; // Red (2) in upper nibble, Green (5) in lower nibble.

    console.log("Decoding test pixel (20, 42) expecting Screen RAM color source 2 -> Green (5)...");
    const colorPixel2 = await gContract.decodeKoalaPixel(mockKoa, 20, 42);
    console.log(`Pixel (20, 42) Color: ${colorPixel2} (Expected: 5)`);
    if (Number(colorPixel2) !== 5) {
        console.error(`FAIL: Expected color 5, got ${colorPixel2}`);
        process.exit(1);
    }

    console.log("Decoding test pixel (15, 17) expecting Color RAM color source 3 -> Light Blue (14)...");
    const colorPixel1 = await gContract.decodeKoalaPixel(mockKoa, 15, 17);
    console.log(`Pixel (15, 17) Color: ${colorPixel1} (Expected: 14)`);
    if (Number(colorPixel1) !== 14) {
        console.error(`FAIL: Expected color 14, got ${colorPixel1}`);
        process.exit(1);
    }

    console.log("Decoding test pixel (100, 100) expecting default background color -> Blue (6)...");
    const colorBg = await gContract.decodeKoalaPixel(mockKoa, 100, 100);
    console.log(`Pixel (100, 100) Color: ${colorBg} (Expected: 6)`);
    if (Number(colorBg) !== 6) {
        console.error(`FAIL: Expected color 6, got ${colorBg}`);
        process.exit(1);
    }

    console.log("SUCCESS: KoalaPainter (.koa) decoder successfully validated!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
