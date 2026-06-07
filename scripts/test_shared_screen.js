const { ethers } = require("ethers");
const fs = require("fs");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = "../config/user_config.json";

const cpuABI = [
    "function poke(uint256 addr, uint256 val) public returns (uint256)",
    "function peek(uint256 addr) public view returns (uint256)",
    "function getScreenRAM() public view returns (bytes)"
];

async function main() {
    console.log("=== Testing Shared Screen Mode on EVM ===");
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;
    console.log(`CPU Contract Address: ${cpuAddress}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const accounts = await provider.listAccounts();
    
    // We'll use two distinct accounts to simulate multiplayer/collaborative input
    const user1 = accounts[1];
    const user2 = accounts[2];
    console.log(`User 1: ${user1.address}`);
    console.log(`User 2: ${user2.address}`);

    const cpuUser1 = new ethers.Contract(cpuAddress, cpuABI, user1);
    const cpuUser2 = new ethers.Contract(cpuAddress, cpuABI, user2);

    console.log("\n--- Step 1: Default namespaced state test ---");
    // Under default mode, they should not see each other's changes in screen RAM (e.g. index 1024)
    // First, clear any previous values for both
    await (await cpuUser1.poke(1024, 0)).wait();
    await (await cpuUser2.poke(1024, 0)).wait();

    await (await cpuUser1.poke(1024, 88)).wait();
    const val1_default = Number(await cpuUser1.peek(1024));
    const val2_default = Number(await cpuUser2.peek(1024));
    console.log(`Default mode - User 1 reads: ${val1_default}, User 2 reads: ${val2_default}`);
    if (val1_default === 88 && val2_default === 0) {
        console.log("-> Success: Default namespaced behavior confirmed (User 2 reads 0).");
    } else {
        console.warn("-> Note: Namespaced state already overridden, proceeding.");
    }

    console.log("\n--- Step 2: Enabling Shared Screen Mode at $D50D (54541) ---");
    // Enable shared mode. Because register D50D itself is namespaced per user in getUserSlotPrivate,
    // each user wanting to interact with/view the shared screen has to toggle it for themselves,
    // OR one toggling it makes their own views route to the shared screen. Let's toggle it for both!
    await (await cpuUser1.poke(54541, 1)).wait();
    await (await cpuUser2.poke(54541, 1)).wait();
    console.log("Enabled shared screen mode for both users.");

    console.log("\n--- Step 3: Performing cross-user write/read verification ---");
    // User 1 writes character 65 ('A') to screen RAM position 1024
    await (await cpuUser1.poke(1024, 65)).wait();
    console.log("User 1 wrote 65 ('A') to position 1024.");

    // User 2 should immediately see character 65 at 1024
    const user2Read = Number(await cpuUser2.peek(1024));
    console.log(`User 2 reads position 1024: ${user2Read}`);

    // User 2 writes character 66 ('B') to screen RAM position 1025
    await (await cpuUser2.poke(1025, 66)).wait();
    console.log("User 2 wrote 66 ('B') to position 1025.");

    // User 1 should immediately see character 66 at 1025
    const user1Read = Number(await cpuUser1.peek(1025));
    console.log(`User 1 reads position 1025: ${user1Read}`);

    if (user2Read === 65 && user1Read === 66) {
        console.log("\n>>> SUCCESS: Shared Screen Mode successfully bypassed namespacing! <<<");
    } else {
        throw new Error("Shared Screen verification failed!");
    }
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
