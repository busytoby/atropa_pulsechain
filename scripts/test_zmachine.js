const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

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
        gasLimit: 3000000
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
        "function executeTokenPayment(address token, address player, uint256 amount) public returns (bool)"
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
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
