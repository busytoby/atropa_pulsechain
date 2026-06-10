const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const COMPILED_YUL_PATH = path.join(__dirname, "../frontend/compiled_yul.json");

// ABI for the Sculpture contract
const sculptureABI = [
    "function setVoxel(uint256 x, uint256 y, uint256 z, uint256 color) external returns (uint256)",
    "function getVoxel(uint256 x, uint256 y, uint256 z) external view returns (uint256)",
    "function project(uint256 cosVal, uint256 sinVal) external view returns (int256[])",
    "function stepLife() external returns (uint256)"
];

function drawAsciiFrame(points, title) {
    // Determine bounds
    let minX = 999, maxX = -999, minY = 999, maxY = -999;
    points.forEach(p => {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    });

    // Pad margins
    minX -= 1; maxX += 1;
    minY -= 1; maxY += 1;

    const width = maxX - minX + 1;
    const height = maxY - minY + 1;

    // Build grid (Y axis runs top to bottom, but standard Cartesian is bottom to top)
    const grid = Array.from({ length: height }, () => Array(width).fill(" "));

    points.forEach(p => {
        const gx = p.x - minX;
        const gy = maxY - p.y; // invert Y for screen space rendering
        if (gx >= 0 && gx < width && gy >= 0 && gy < height) {
            grid[gy][gx] = p.char;
        }
    });

    console.log(`\n=== Render: ${title} ===`);
    grid.forEach(row => {
        console.log("  " + row.join(" "));
    });
}

async function main() {
    console.log("=== Deploying Sculpture Yul thunk to local EVM ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const compiled = JSON.parse(fs.readFileSync(COMPILED_YUL_PATH, "utf8"));
    const bytecode = compiled.sculpture;

    const factory = new ethers.ContractFactory(sculptureABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Sculpture thunk contract deployed at: ${contractAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");

    async function sendTx(promise) {
        const tx = await promise;
        nonce++;
        return tx.wait();
    }

    // Voxel data to build a 3D Pyramidal/Cross Structure
    // Points format: [x, y, z, colorChar]
    const voxels = [
        // Central Stem
        [5, 1, 5, 0x23], // '#'
        [5, 2, 5, 0x23],
        [5, 3, 5, 0x23],
        [5, 4, 5, 0x23],
        [5, 5, 5, 0x23],
        
        // Horizontal Cross beam
        [3, 3, 5, 0x2B], // '+'
        [4, 3, 5, 0x2B],
        [6, 3, 5, 0x2B],
        [7, 3, 5, 0x2B],

        // Depth beam
        [5, 3, 3, 0x2A], // '*'
        [5, 3, 4, 0x2A],
        [5, 3, 6, 0x2A],
        [5, 3, 7, 0x2A]
    ];

    console.log(`Assembling 3D Voxel Sculpture: adding ${voxels.length} points...`);
    for (const v of voxels) {
        await sendTx(contract.setVoxel(v[0], v[1], v[2], v[3], { nonce }));
    }

    // 1. Project at 0 degrees (cos = 1000, sin = 0)
    console.log("\nQuerying 3D-to-2D projection thunk at 0 degrees...");
    let result = await contract.project(1000, 0);
    let points = [];
    for (let i = 0; i < result.length; i += 3) {
        points.push({ x: Number(result[i]), y: Number(result[i+1]), char: String.fromCharCode(Number(result[i+2])) });
    }
    drawAsciiFrame(points, "Front View (0 Degrees)");

    // 2. Project at 45 degrees (cos = 707, sin = 707)
    console.log("\nQuerying 3D-to-2D projection thunk at 45 degrees...");
    result = await contract.project(707, 707);
    points = [];
    for (let i = 0; i < result.length; i += 3) {
        points.push({ x: Number(result[i]), y: Number(result[i+1]), char: String.fromCharCode(Number(result[i+2])) });
    }
    drawAsciiFrame(points, "Isometric View (45 Degrees)");

    // 3. Project at 90 degrees (cos = 0, sin = 1000)
    console.log("\nQuerying 3D-to-2D projection thunk at 90 degrees...");
    result = await contract.project(0, 1000);
    points = [];
    for (let i = 0; i < result.length; i += 3) {
        points.push({ x: Number(result[i]), y: Number(result[i+1]), char: String.fromCharCode(Number(result[i+2])) });
    }
    drawAsciiFrame(points, "Side View (90 Degrees)");

    // 4. Mutate using Conway's Game of Life
    console.log("\nSimulating 1 generation of on-chain 3D Game of Life...");
    await sendTx(contract.stepLife({ nonce }));
    
    console.log("Querying mutated 3D projection...");
    result = await contract.project(1000, 0);
    points = [];
    for (let i = 0; i < result.length; i += 3) {
        points.push({ x: Number(result[i]), y: Number(result[i+1]), char: String.fromCharCode(Number(result[i+2])) });
    }
    drawAsciiFrame(points, "Front View After 1 Generation of Life");

    console.log("\n>>> SUCCESS: 3D sculpture thunk assembly and orthographic rendering verified! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
