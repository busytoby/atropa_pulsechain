const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Helper to compile Yul
function compileYul(yulPath) {
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
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
    const receipt = await tx.wait();
    const zmachineAddress = receipt.contractAddress;
    console.log("ZMachine prototype deployed at:", zmachineAddress);

    // Define ABI for selector interaction
    const abi = [
        "function uploadRomChunk(uint256 offset, bytes data) public returns (bool)",
        "function decodeZString(uint256 offset) public view returns (string)",
        "function getObjectProperty(uint256 objId, uint256 propId, address player) public view returns (uint256)",
        "function executeTokenReward(address token, address player, uint256 amount) public returns (bool)",
        "function executeTokenPayment(address token, address player, uint256 amount) public returns (bool)",
        "function triggerZ6Sound(address musicMaker, uint256 note, uint256 voice) public returns (bool)",
        "function decryptInvisiclue(address keySystem, address player, uint256 hintId) public view returns (string)"
    ];
    const contract = new ethers.Contract(zmachineAddress, abi, deployer);

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
    await uploadTx.wait();
    console.log("ROM chunk uploaded successfully.");

    // 4. Query ERC-20 token balance dynamically via object property 31
    console.log("\nQuerying Object ID 50, Property 31 (Token Balance Query)...");
    const balance = await contract.getObjectProperty(50, 31, deployer.address);
    console.log("Returned balance of player:", ethers.formatEther(balance), "ZGC");

    // 5. Test reward transfer
    console.log("\nTesting reward distribution: rewarding player with 100 ZGC...");
    
    // Transfer tokens to ZMachine contract to hold rewards
    const transferTx = await token.transfer(zmachineAddress, ethers.parseEther("100"));
    await transferTx.wait();

    // Trigger reward execution
    const rewardTx = await contract.executeTokenReward(tokenAddress, deployer.address, ethers.parseEther("5"), { gasLimit: 2000000 });
    await rewardTx.wait();
    console.log("Reward successfully distributed!");

    // 6. Test Z6 sound note trigger
    console.log("\nTesting Z6 Sound note triggers (note=60, voice=1)...");
    console.log("Compiling and deploying musicMaker.yul...");
    const musicMakerBytecode = compileYul(path.join(__dirname, "../solidity/bin/musicMaker.yul"));
    const musicMakerTx = await deployer.sendTransaction({ data: musicMakerBytecode, gasLimit: 3000000 });
    const musicMakerReceipt = await musicMakerTx.wait();
    const musicMakerAddress = musicMakerReceipt.contractAddress;
    console.log("MusicMaker deployed at:", musicMakerAddress);

    const soundTx = await contract.triggerZ6Sound(musicMakerAddress, 60, 1, { gasLimit: 2000000 });
    await soundTx.wait();
    console.log("Z6 Note triggered successfully.");

    // 7. Test token-gated Invisiclues hint decrypting
    console.log("\nTesting token-gated Invisiclues hint decryption...");
    console.log("Compiling and deploying keySystem.yul...");
    const keySystemBytecode = compileYul(path.join(__dirname, "../solidity/bin/keySystem.yul"));
    const keySystemTx = await deployer.sendTransaction({ data: keySystemBytecode, gasLimit: 3000000 });
    const keySystemReceipt = await keySystemTx.wait();
    const keySystemAddress = keySystemReceipt.contractAddress;
    console.log("KeySystem deployed at:", keySystemAddress);
    
    // Inject key verification setup on keysystem first
    const keySystemABI = ["function setKey256(address user, bytes32 key) external returns (uint256)"];
    const keySystem = new ethers.Contract(keySystemAddress, keySystemABI, deployer);
    await (await keySystem.setKey256(deployer.address, ethers.id("TSFi2_Secure_Key_Seed"))).wait();

    // Bind Object ID 99 to the ERC-20 token address in Z-machine storage mapping for Invisiclues pull payments
    const hintStorageSlot = ethers.toBeHex(2000000 + 99);
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        hintStorageSlot,
        ethers.zeroPadValue(tokenAddress, 32)
    ]);

    // Approve Z-machine contract to collect fee of 1 ZGC from player
    await (await token.approve(zmachineAddress, ethers.parseEther("1"))).wait();

    const hintStr = await contract.decryptInvisiclue(keySystemAddress, deployer.address, 1, { gasLimit: 3000000 });
    console.log("Decrypted hint payload:", hintStr);

    // 8. Test Gauntlet Stat Bridge
    console.log("\nTesting Gauntlet/Folklore CPU Stat Bridge...");
    console.log("Compiling and deploying folklore.yul...");
    const folkloreBytecode = compileYul(path.join(__dirname, "../solidity/bin/folklore.yul"));
    const folkloreTx = await deployer.sendTransaction({ data: folkloreBytecode, gasLimit: 15000000 });
    const folkloreReceipt = await folkloreTx.wait();
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
    await (await extendedContract.bindFolkloreAddress(folkloreAddress)).wait();

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
    await (await folkloreContract.poke(55050, 1)).wait();
    await (await folkloreContract.poke(55053, 1750)).wait();
    await (await folkloreContract.poke(55054, 4)).wait();

    // Verify property values queried on Object ID 80
    console.log("Querying Object 80 bridged properties:");
    const activeVal = await extendedContract.getObjectProperty(80, 36, deployer.address);
    const healthVal = await extendedContract.getObjectProperty(80, 32, deployer.address);
    const keysVal = await extendedContract.getObjectProperty(80, 33, deployer.address);
    console.log(`  - Active flag (Prop 36): ${activeVal}`);
    console.log(`  - Health (Prop 32): ${healthVal}`);
    console.log(`  - Keys (Prop 33): ${keysVal}`);

    if (activeVal.toString() !== "1" || healthVal.toString() !== "1750" || keysVal.toString() !== "4") {
        throw new Error("Gauntlet bridge property mismatch!");
    }

    // Verify Z-Machine Inventory command outputs the bridged values
    console.log("Executing 'inventory' command on Z-Machine parser...");
    const cmdBytes = ethers.Buffer.from("inventory");
    const invResponse = await extendedContract.parseCommand(deployer.address, cmdBytes);
    console.log("Z-Machine output:\n", invResponse);

    if (!invResponse.includes("Gauntlet Health: 1750") || !invResponse.includes("Gauntlet Keys: 4")) {
        throw new Error("Bridge inventory output does not contain mocked stats!");
    }

    console.log("Gauntlet Stat Bridge verification passed!");
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
