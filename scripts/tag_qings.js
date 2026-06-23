const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

console.log("=============================================================");
console.log("Auncient Yul CPU Relational QING Tagging Utility");
console.log("=============================================================");

const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");
const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    // Read command line arguments: node scripts/tag_qings.js <qingA> <qingB>
    const args = process.argv.slice(2);
    if (args.length < 2) {
        console.log("Usage: node scripts/tag_qings.js <address_qingA> <address_qingB>");
        console.log("Example: node scripts/tag_qings.js 0x00000000000000000000000000000000000003e7 0x0000000000000000000000000000000000000001");
        process.exit(1);
    }

    const qingA = ethers.getAddress(args[0]);
    const qingB = ethers.getAddress(args[1]);

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Error: user_config.json not found");
        process.exit(1);
    }

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const arenaAddr = config.networks.localhost.arenaProcessorAddress;
    if (!arenaAddr) {
        console.error("Error: ArenaProcessor address not found in config");
        process.exit(1);
    }

    console.log(`Connecting to EVM provider at ${PROVIDER_URL}...`);
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const arenaContract = new ethers.Contract(arenaAddr, [
        "function tag(address qingA, address qingB) external",
        "function tags(address qing) external view returns (address[])"
    ], deployer);

    console.log(`Target Arena Processor: ${arenaAddr}`);
    console.log(`Executing TAG(${qingA}, ${qingB})...`);

    const tx = await arenaContract.tag(qingA, qingB);
    console.log(`Transaction sent. Hash: ${tx.hash}`);
    
    const receipt = await tx.wait();
    console.log(`✓ Transaction confirmed in block: ${receipt.blockNumber} (Status: ${receipt.status})`);

    console.log(`Querying tags for ${qingA} to verify relationship...`);
    const currentTags = await arenaContract.tags(qingA);
    console.log("On-chain linked tags (sorted by bar width):");
    console.log(Array.from(currentTags));
}

main().catch(err => {
    console.error("Tagging utility failed:", err.message);
    process.exit(1);
});
