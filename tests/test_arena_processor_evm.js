const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const assert = require("assert");

const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");
const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== RUNNING EVM ARENA INTEGRATION TESTS ===");
    
    // 1. Connect to EVM
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    const attacker = signers[2];
    const accessor = signers[3];
    
    console.log(`Deployer: ${deployer.address}`);
    console.log(`Attacker: ${attacker.address}`);
    console.log(`Accessor: ${accessor.address}`);
    
    // Load config addresses
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const arenaAddr = config.networks.localhost.arenaProcessorAddress;
    const pkiAddr = config.networks.localhost.consensusPkiAddress;
    
    if (!arenaAddr || !pkiAddr) {
        throw new Error("ArenaProcessor or ConsensusPKI not deployed. Run deploy_yul.js first.");
    }
    
    // 2. Initialize contracts
    const arenaContract = new ethers.Contract(arenaAddr, [
        "function registerPlayerYue(uint256 yueCardId) external",
        "function systemEquipQing(uint256 cardId, uint256 pageIdx, uint256 u1, uint256 u2) external",
        "function approveAccessorForCard(uint256 cardId, address accessor, uint256 approved) external",
        "function processBatch(uint256 batchSize) external",
        "function tag(address qingA, address qingB) external",
        "function tags(address qing) external view returns (address[])"
    ], deployer);

    // 3. Clear owner slots, player count & round variables using anvil cheatcodes to ensure fresh test state
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x200), 32), ethers.zeroPadValue("0x00", 32)]);
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x100), 32), ethers.zeroPadValue("0x00", 32)]);
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x101), 32), ethers.zeroPadValue("0x00", 32)]);
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x102), 32), ethers.zeroPadValue(ethers.toBeHex(0xFFFFFFFF), 32)]);
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x103), 32), ethers.zeroPadValue("0x00", 32)]);
    await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x104), 32), ethers.zeroPadValue("0x00", 32)]);
    
    for (let i = 0; i < 5; i++) {
        const ownerSlot = ethers.zeroPadValue(ethers.toBeHex(0x7000 + i), 32);
        await provider.send("anvil_setStorageAt", [arenaAddr, ownerSlot, ethers.zeroPadValue("0x00", 32)]);
        
        // Also reset equipment slots so previous test runs don't interfere
        const destOffset = 0x8000 + i * 0x1000;
        const pageOffset = destOffset + 0x70 * 256;
        await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(pageOffset), 32), ethers.zeroPadValue("0x00", 32)]);
        await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(pageOffset + 32), 32), ethers.zeroPadValue("0x00", 32)]);
    }
    console.log("✓ Reset Arena Processor storage slots successfully.");
    
    // 4. Test Registration Guard & Access Controls
    // Register Card 0 and Card 1 from deployer (deployer becomes owner)
    await (await arenaContract.registerPlayerYue(0)).wait();
    await (await arenaContract.registerPlayerYue(1)).wait();
    console.log("✓ Registered Card 0 and 1 from deployer.");

    // Tries to double-register Card 1 from attacker (should revert)
    const arenaContractAttacker = arenaContract.connect(attacker);
    await assert.rejects(
        async () => {
            const tx = await arenaContractAttacker.registerPlayerYue(1);
            await tx.wait();
        },
        /revert/,
        "Should revert when unauthorized attacker registers card 1"
    );
    console.log("✓ Checked: Attacker cannot register card 1.");

    // 5. Test Equipment Access Controls
    // Deployer (owner) equips Card 1
    await (await arenaContract.systemEquipQing(1, 0x70, 10, 50)).wait(); // Width: 40
    console.log("✓ Owner equipped Card 1 successfully.");

    // Attacker tries to equip Card 1 (should revert)
    await assert.rejects(
        async () => {
            const tx = await arenaContractAttacker.systemEquipQing(1, 0x70, 0, 100);
            await tx.wait();
        },
        /revert/,
        "Should revert when unauthorized attacker equips card 1"
    );
    console.log("✓ Checked: Attacker cannot equip card 1.");

    // 6. Test Accessor Approval & Equipment
    // Owner approves Accessor for Card 2
    // Register Card 2 from deployer
    await (await arenaContract.registerPlayerYue(2)).wait();
    
    await (await arenaContract.approveAccessorForCard(2, accessor.address, 1)).wait();
    console.log("✓ Approved accessor for Card 2.");

    // Accessor equips Card 2
    const arenaContractAccessor = arenaContract.connect(accessor);
    await (await arenaContractAccessor.systemEquipQing(2, 0x70, 5, 85)).wait(); // Width: 80
    console.log("✓ Accessor equipped Card 2 successfully.");

    // 7. Register and equip rest of players from deployer
    await (await arenaContract.registerPlayerYue(3)).wait();
    await (await arenaContract.systemEquipQing(3, 0x70, 20, 30)).wait(); // Width: 10
    
    await (await arenaContract.registerPlayerYue(4)).wait();
    await (await arenaContract.systemEquipQing(4, 0x70, 0, 95)).wait(); // Width: 95 (Expected Winner)
    console.log("✓ Registered and equipped all 5 players.");

    async function logState(label) {
        const targetQing = BigInt(await provider.getStorage(arenaAddr, "0x100"));
        const cursor = BigInt(await provider.getStorage(arenaAddr, "0x101"));
        const leader = BigInt(await provider.getStorage(arenaAddr, "0x102"));
        const maxW = BigInt(await provider.getStorage(arenaAddr, "0x103"));
        const completed = BigInt(await provider.getStorage(arenaAddr, "0x104"));
        const total = BigInt(await provider.getStorage(arenaAddr, "0x200"));
        console.log(`[STATE ${label}] Target Qing: ${targetQing}, Cursor: ${cursor}, Leader: ${leader}, Max Width: ${maxW}, Completed: ${completed}, Total Players: ${total}`);
    }

    // 8. Run Batch Evaluation
    // Let's call processBatch to ensure winner is computed correctly in batches of 2
    
    await logState("BEFORE BATCH 1");
    const txBatch1 = await arenaContract.processBatch(2, { gasLimit: 300000 });
    const rc1 = await txBatch1.wait();
    console.log(`✓ Batch 1 receipt status: ${rc1.status}, gasUsed: ${rc1.gasUsed}`);
    
    await logState("BEFORE BATCH 2");
    const txBatch2 = await arenaContract.processBatch(2, { gasLimit: 300000 });
    const rc2 = await txBatch2.wait();
    console.log(`✓ Batch 2 receipt status: ${rc2.status}, gasUsed: ${rc2.gasUsed}`);
    
    await logState("BEFORE BATCH 3");
    const txBatch3 = await arenaContract.processBatch(2, { gasLimit: 300000 });
    const rc3 = await txBatch3.wait();
    console.log(`✓ Batch 3 receipt status: ${rc3.status}, gasUsed: ${rc3.gasUsed}`);
    console.log("✓ Ran processBatch in 3 batches.");

    // Query storage slots using provider.getStorage
    // Slot 0x104 (Round Completed)
    const completed = BigInt(await provider.getStorage(arenaAddr, 0x104));
    assert.strictEqual(completed, 1n, "Round should be completed");

    // Slot 0x300 (Winner Card ID)
    const winnerId = BigInt(await provider.getStorage(arenaAddr, 0x300));
    assert.strictEqual(winnerId, 4n, "Winner Card ID should be 4");

    // Slot 0x301 (Max Bar Width)
    const maxWidth = BigInt(await provider.getStorage(arenaAddr, 0x301));
    assert.strictEqual(maxWidth, 95n, "Max bar width should be 95");

    // 9. Test tag and tags relational query logic on-chain
    const urgentQing = "0x00000000000000000000000000000000000003e7"; // 999
    
    // Address representations matching cardIds 1, 2, 3, 4 padded to 20 bytes
    const card1Addr = "0x0000000000000000000000000000000000000001";
    const card2Addr = "0x0000000000000000000000000000000000000002";
    const card3Addr = "0x0000000000000000000000000000000000000003";
    const card4Addr = "0x0000000000000000000000000000000000000004";

    await (await arenaContract.tag(urgentQing, card1Addr)).wait(); // Card 1: width 40
    await (await arenaContract.tag(urgentQing, card2Addr)).wait(); // Card 2: width 80
    await (await arenaContract.tag(urgentQing, card3Addr)).wait(); // Card 3: width 10
    await (await arenaContract.tag(urgentQing, card4Addr)).wait(); // Card 4: width 95
    
    const sortedTags = await arenaContract.tags(urgentQing);
    console.log("On-chain sorted tags:", sortedTags);
    
    // Expected sorted order by width: 4 (95), 2 (80), 1 (40), 3 (10)
    assert.deepStrictEqual(Array.from(sortedTags), [card4Addr, card2Addr, card1Addr, card3Addr], "On-chain tags should be sorted descending by bar width");
    console.log("✓ Step 9: Verified on-chain tag and tags relational mapping sorted by highest bars");

    console.log("★★★ ALL EVM ARENA INTEGRATION TESTS PASSED ★★★");
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
