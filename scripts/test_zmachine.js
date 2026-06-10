const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Helper to compile Yul
function compileYul(yulPath) {
    try {
        const cachePath = path.join(__dirname, "../frontend/compiled_yul.json");
        if (fs.existsSync(cachePath)) {
            const cache = JSON.parse(fs.readFileSync(cachePath, "utf8"));
            const filename = path.basename(yulPath, ".yul");
            if (cache[filename]) {
                return cache[filename];
            }
        }
    } catch (e) {
        // Fall back to compilation
    }
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function waitForReceipt(tx, provider) {
    if (!tx || !tx.hash) return tx;
    for (let i = 0; i < 30; i++) {
        const receipt = await provider.getTransactionReceipt(tx.hash);
        if (receipt) return receipt;
        await new Promise(resolve => setTimeout(resolve, 300));
    }
    return tx.wait();
}

async function main() {
    console.log("Loading registry config...");
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));

    console.log("Compiling zmachine.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/zmachine.yul");
    const bytecode = compileYul(yulPath);

    console.log("Bytecode generated successfully. Length:", bytecode.length);
    
    // Get signers
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log("Deploying contract with account:", deployer.address);

    // Deploy using Ethers raw transaction factory
    const tx = await deployer.sendTransaction({
        data: bytecode,
        gasLimit: 15000000
    });
    const receipt = await waitForReceipt(tx, provider);
    const zmachineAddress = receipt.contractAddress;
    console.log("ZMachine prototype deployed at:", zmachineAddress);

    console.log("Compiling zmachineParser.yul...");
    const parserBytecode = compileYul(path.join(__dirname, "../solidity/bin/zmachineParser.yul"));
    const parserTx = await deployer.sendTransaction({
        data: parserBytecode,
        gasLimit: 15000000
    });
    const parserReceipt = await waitForReceipt(parserTx, provider);
    const parserAddress = parserReceipt.contractAddress;
    console.log("ZMachineParser deployed at:", parserAddress);

    // Define ABI for selector interaction
    const abi = [
        "function uploadRomChunk(uint256 offset, bytes data) public returns (bool)",
        "function decodeZString(uint256 offset) public view returns (string)",
        "function getObjectProperty(uint256 objId, uint256 propId, address player) public view returns (uint256)",
        "function executeTokenReward(address token, address player, uint256 amount) public returns (bool)",
        "function executeTokenPayment(address token, address player, uint256 amount) public returns (bool)",
        "function triggerZ6Sound(address musicMaker, uint256 note, uint256 voice) public returns (bool)",
        "function decryptInvisiclue(address keySystem, address player, uint256 hintId) public view returns (string)",
        "function bindParserAddress(address parser) public returns (bool)"
    ];
    const contract = new ethers.Contract(zmachineAddress, abi, deployer);

    console.log("Binding ZMachineParser address...");
    await waitForReceipt(await contract.bindParserAddress(parserAddress), provider);

    // 1. Deploy MockERC20 token
    console.log("\nDeploying MockERC20 token for binding...");
    const erc20Bin = fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.bin"), "utf8");
    const erc20Abi = JSON.parse(fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.abi"), "utf8"));
    const erc20Factory = new ethers.ContractFactory(erc20Abi, "0x" + erc20Bin, deployer);
    const token = await erc20Factory.deploy();
    await token.waitForDeployment();
    const tokenAddress = await token.getAddress();
    console.log("MockERC20 deployed at:", tokenAddress);

    // 2. Bind Object ID 50 (e.g. "Gold Coin") to the ERC-20 token address in Z-machine storage mapping
    // storage location: sstore(add(2000000, objId), tokenAddr)
    const storageSlot = ethers.toBeHex(2000000 + 50);
    console.log(`Binding Object ID 50 to Token Address in storage slot ${storageSlot}...`);
    
    // Simulating setting binding mapping (direct storage injection)
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        storageSlot,
        ethers.zeroPadValue(tokenAddress, 32)
    ]);

    // 3. Simulating chunked game ROM upload
    console.log("\nSimulating game ROM upload (uploading 64 bytes of Zork header data)...");
    const testHeader = Buffer.alloc(1024);
    testHeader[0] = 3; // Z-Machine Version 3
    testHeader.writeUInt16BE(0x0800, 6); // Initial PC
    testHeader.writeUInt16BE(0x0040, 10); // Object Table Base address at 0x0040 offset

    // Object tree base defaults table layout: 31 defaults (62 bytes) + Object ID 50 base offset
    // Write properties pointer offset 0x0100 for Object ID 50 (objPtr index 50: objTableBase + 62 + (49 * 9) = 62 + 441 = 503 byte offset)
    const objOffset = 62 + (49 * 9);
    testHeader.writeUInt16BE(0x0100, objOffset + 7); // properties ptr offset

    const uploadTx = await contract.uploadRomChunk(0, testHeader, { gasLimit: 500000 });
    await waitForReceipt(uploadTx, provider);
    console.log("ROM chunk uploaded successfully.");

    // 4. Query ERC-20 token balance dynamically via object property 31
    console.log("\nQuerying Object ID 50, Property 31 (Token Balance Query)...");
    const balance = await contract.getObjectProperty(50, 31, deployer.address);
    console.log("Returned balance of player:", ethers.formatEther(balance), "ZGC");

    // 5. Test reward transfer
    console.log("\nTesting reward distribution: rewarding player with 100 ZGC...");
    
    // Transfer tokens to ZMachine contract to hold rewards
    const transferTx = await token.transfer(zmachineAddress, ethers.parseEther("100"));
    await waitForReceipt(transferTx, provider);

    // Trigger reward execution
    const rewardTx = await contract.executeTokenReward(tokenAddress, deployer.address, ethers.parseEther("5"), { gasLimit: 2000000 });
    await waitForReceipt(rewardTx, provider);
    console.log("Reward successfully distributed!");

    // 6. Test Z6 sound note trigger
    console.log("\nTesting Z6 Sound note triggers (note=60, voice=1)...");
    console.log("Compiling and deploying musicMaker.yul...");
    const musicMakerBytecode = compileYul(path.join(__dirname, "../solidity/bin/musicMaker.yul"));
    const musicMakerTx = await deployer.sendTransaction({ data: musicMakerBytecode, gasLimit: 3000000 });
    const musicMakerReceipt = await waitForReceipt(musicMakerTx, provider);
    const musicMakerAddress = musicMakerReceipt.contractAddress;
    console.log("MusicMaker deployed at:", musicMakerAddress);

    const soundTx = await contract.triggerZ6Sound(musicMakerAddress, 60, 1, { gasLimit: 2000000 });
    await waitForReceipt(soundTx, provider);
    console.log("Z6 Note triggered successfully.");

    // 7. Test token-gated Invisiclues hint decrypting
    console.log("\nTesting token-gated Invisiclues hint decryption...");
    console.log("Compiling and deploying keySystem.yul...");
    const keySystemBytecode = compileYul(path.join(__dirname, "../solidity/bin/keySystem.yul"));
    const keySystemTx = await deployer.sendTransaction({ data: keySystemBytecode, gasLimit: 3000000 });
    const keySystemReceipt = await waitForReceipt(keySystemTx, provider);
    const keySystemAddress = keySystemReceipt.contractAddress;
    console.log("KeySystem deployed at:", keySystemAddress);
    
    // Inject key verification setup on keysystem first
    const keySystemABI = ["function setKey256(address user, bytes32 key) external returns (uint256)"];
    const keySystem = new ethers.Contract(keySystemAddress, keySystemABI, deployer);
    await waitForReceipt(await keySystem.setKey256(deployer.address, ethers.id("TSFi2_Secure_Key_Seed")), provider);

    // Bind Object ID 99 to the ERC-20 token address in Z-machine storage mapping for Invisiclues pull payments
    const hintStorageSlot = ethers.toBeHex(2000000 + 99);
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        hintStorageSlot,
        ethers.zeroPadValue(tokenAddress, 32)
    ]);

    // Approve Z-machine contract to collect fee of 1 ZGC from player
    await waitForReceipt(await token.approve(zmachineAddress, ethers.parseEther("1")), provider);

    const hintStr = await contract.decryptInvisiclue(keySystemAddress, deployer.address, 1, { gasLimit: 3000000 });
    console.log("Decrypted hint payload:", hintStr);

    // 8. Test Gauntlet Stat Bridge
    console.log("\nTesting Gauntlet/Folklore CPU Stat Bridge...");
    console.log("Compiling and deploying folklore.yul...");
    const folkloreBytecode = compileYul(path.join(__dirname, "../solidity/bin/folklore.yul"));
    const folkloreTx = await deployer.sendTransaction({ data: folkloreBytecode, gasLimit: 15000000 });
    console.log("Folklore tx sent. Hash:", folkloreTx.hash);
    const folkloreReceipt = await waitForReceipt(folkloreTx, provider);
    const folkloreAddress = folkloreReceipt.contractAddress;
    console.log("Folklore contract deployed at:", folkloreAddress);

    // Bind folklore contract inside Z-Machine
    console.log("Binding Folklore CPU contract inside Z-Machine...");
    const extendedAbi = [
        ...abi,
        "function bindFolkloreAddress(address folklore) public returns (bool)",
        "function parseCommand(address player, bytes cmd) public returns (string)"
    ];
    const extendedContract = new ethers.Contract(zmachineAddress, extendedAbi, deployer);
    await waitForReceipt(await extendedContract.bindFolkloreAddress(folkloreAddress), provider);

    // Mock Gauntlet state variables on Folklore CPU contract
    console.log("Mocking active Gauntlet game state on Folklore...");
    const folkloreAbi = [
        "function poke(uint256 addr, uint256 val) public returns (bool)",
        "function peek(uint256 addr) public view returns (uint256)"
    ];
    const folkloreContract = new ethers.Contract(folkloreAddress, folkloreAbi, deployer);
    
    // 55050: isGauntletActive = 1
    // 55053: gauntletHealth = 1750
    // 55054: gauntletKeys = 4
    // 55061: gauntletWeapon = 1 (Battle Axe)
    // 55062: gauntletArmor = 1 (Leather Armor)
    await waitForReceipt(await folkloreContract.poke(55050, 1), provider);
    await waitForReceipt(await folkloreContract.poke(55053, 1750), provider);
    await waitForReceipt(await folkloreContract.poke(55054, 4), provider);
    await waitForReceipt(await folkloreContract.poke(55061, 1), provider);
    await waitForReceipt(await folkloreContract.poke(55062, 1), provider);

    // Verify property values queried on Object ID 80
    console.log("Querying Object 80 bridged properties:");
    const activeVal = await extendedContract.getObjectProperty(80, 36, deployer.address);
    const healthVal = await extendedContract.getObjectProperty(80, 32, deployer.address);
    const keysVal = await extendedContract.getObjectProperty(80, 33, deployer.address);
    const weaponVal = await extendedContract.getObjectProperty(80, 37, deployer.address);
    const armorVal = await extendedContract.getObjectProperty(80, 38, deployer.address);
    console.log(`  - Active flag (Prop 36): ${activeVal}`);
    console.log(`  - Health (Prop 32): ${healthVal}`);
    console.log(`  - Keys (Prop 33): ${keysVal}`);
    console.log(`  - Weapon (Prop 37): ${weaponVal}`);
    console.log(`  - Armor (Prop 38): ${armorVal}`);

    if (activeVal.toString() !== "1" || healthVal.toString() !== "1750" || keysVal.toString() !== "4" || weaponVal.toString() !== "1" || armorVal.toString() !== "1") {
        throw new Error("Gauntlet bridge property mismatch!");
    }

    // Verify Z-Machine Inventory command outputs the bridged values
    console.log("Executing 'inventory' command on Z-Machine parser...");
    const cmdBytes = Buffer.from("inventory");
    const invResponse = await extendedContract.parseCommand.staticCall(deployer.address, cmdBytes);
    console.log("Z-Machine output:\n", invResponse);

    if (!invResponse.includes("Gauntlet Health: 1750") || !invResponse.includes("Gauntlet Keys: 4") || !invResponse.includes("Equipped Weapon: Battle Axe") || !invResponse.includes("Equipped Armor: Leather Armor")) {
        throw new Error("Bridge inventory output does not contain mocked stats!");
    }

    console.log("Gauntlet Stat Bridge verification passed!");

    console.log("\nTesting Z-Machine Interactive Combat Loop...");

    // Move player from Room 10 (outside) to Room 1 (Entry Hall)
    console.log("Command: north");
    const moveRes = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("north"));
    await waitForReceipt(await extendedContract.parseCommand(deployer.address, Buffer.from("north")), provider);

    // Call look to check room and active enemy description
    console.log("Command: look");
    const lookRes = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("look"));
    console.log("Z-Machine output:\n", lookRes);
    if (!lookRes.includes("A Troll (Health: 50) stands here")) {
        throw new Error("Troll not present or health incorrect!");
    }

    // Try to take the Gold Token (should be blocked)
    console.log("Command: take (blocked by Troll)");
    const takeBlockedRes = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("take"));
    console.log("Z-Machine output:\n", takeBlockedRes);
    if (!takeBlockedRes.includes("The Troll blocks you from taking the Gold Token")) {
        throw new Error("Troll did not block taking the Gold Token!");
    }

    // Execute first attack transaction to deal 25 damage (Mocked weapon Battle Axe)
    console.log("Command: attack (hit 1)");
    await waitForReceipt(await extendedContract.parseCommand(deployer.address, Buffer.from("attack")), provider);
    const attackRes1 = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("look"));
    console.log("Z-Machine output after attack 1:\n", attackRes1);
    if (!attackRes1.includes("A Troll (Health: 25) stands here")) {
        throw new Error("Troll health did not decrease correctly!");
    }

    // Attack again to defeat the Troll
    console.log("Command: attack (hit 2 - defeat)");
    await waitForReceipt(await extendedContract.parseCommand(deployer.address, Buffer.from("attack")), provider);

    // Call look to check room description after Troll is defeated
    const lookDefeatedRes = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("look"));
    console.log("Z-Machine output after Troll is defeated:\n", lookDefeatedRes);
    if (lookDefeatedRes.includes("A Troll")) {
        throw new Error("Troll should be defeated and absent from room description!");
    }

    // Try to take the Gold Token now that Troll is defeated
    console.log("Command: take (unlocked)");
    await waitForReceipt(await extendedContract.parseCommand(deployer.address, Buffer.from("take")), provider);
    const lookFinalRes = await extendedContract.parseCommand.staticCall(deployer.address, Buffer.from("look"));
    console.log("Z-Machine output after taking Gold Token:\n", lookFinalRes);
    if (lookFinalRes.includes("You see a Gold Token here")) {
        throw new Error("Gold Token should have been taken!");
    }

    console.log("Z-Machine Interactive Combat Loop verification passed successfully!");
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
