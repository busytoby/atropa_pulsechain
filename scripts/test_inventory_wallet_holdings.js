const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

// Helper to compile Yul
function compileYul(yulPath) {
    const { execSync } = require("child_process");
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("=== Starting Inventory Wallet Holdings Unit Tests & Benchmarks ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error connecting to local EVM node. Make sure Anvil or Hardhat is running!");
        process.exit(1);
    }
    const deployer = signers[0];
    const player = signers[1];
    console.log("Deployer Address:", deployer.address);
    console.log("Player Address:  ", player.address);

    // 1. Compile and deploy zmachine
    console.log("\n[DEPLOY] Compiling and deploying zmachine.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/zmachine.yul");
    const zmachineBytecode = compileYul(yulPath);
    const tx = await deployer.sendTransaction({ data: zmachineBytecode, gasLimit: 4000000 });
    const receipt = await tx.wait();
    const zmachineAddress = receipt.contractAddress;
    console.log("Z-Machine deployed at:", zmachineAddress);

    const abi = [
        "function parseCommand(address player, bytes cmd) public returns (string)",
        "function getObjectProperty(uint256 objId, uint256 propId, address player) public view returns (uint256)"
    ];
    const contract = new ethers.Contract(zmachineAddress, abi, player);

    // 2. Deploy MockERC20 tokens for Object 50, 51, and 52
    console.log("\n[DEPLOY] Deploying MockERC20 tokens...");
    const erc20Bin = fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.bin"), "utf8");
    const erc20Abi = JSON.parse(fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.abi"), "utf8"));
    const erc20Factory = new ethers.ContractFactory(erc20Abi, "0x" + erc20Bin, deployer);

    const token50 = await erc20Factory.deploy();
    await token50.waitForDeployment();
    const tokenAddr50 = await token50.getAddress();
    console.log("Token 50 (Gold Token) deployed at:", tokenAddr50);

    const token51 = await erc20Factory.deploy();
    await token51.waitForDeployment();
    const tokenAddr51 = await token51.getAddress();
    console.log("Token 51 (Keycard) deployed at:", tokenAddr51);

    const token52 = await erc20Factory.deploy();
    await token52.waitForDeployment();
    const tokenAddr52 = await token52.getAddress();
    console.log("Token 52 (Energy Pack) deployed at:", tokenAddr52);

    // Bind object ids to tokens in ZMachine storage
    console.log("\n[STORAGE] Binding Object IDs to token addresses in zmachine storage...");
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        ethers.toBeHex(2000000 + 50),
        ethers.zeroPadValue(tokenAddr50, 32)
    ]);
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        ethers.toBeHex(2000000 + 51),
        ethers.zeroPadValue(tokenAddr51, 32)
    ]);
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        ethers.toBeHex(2000000 + 52),
        ethers.zeroPadValue(tokenAddr52, 32)
    ]);

    // Force default room setup initialization in Z-Machine
    // By sending an initial dummy command "look"
    console.log("\n[INIT] Initializing room placements...");
    await (await contract.parseCommand(player.address, ethers.toUtf8Bytes("look"), { gasLimit: 500000 })).wait();

    // SCENARIO A: No holdings, no inventory coordinates (items are in rooms 1, 2, 3)
    console.log("\n[TEST] Scenario A: Empty inventory (no ERC-20 balance, items in rooms)");
    let response = await contract.parseCommand.staticCall(player.address, ethers.toUtf8Bytes("inve"));
    console.log("Response:", JSON.stringify(response));
    if (response.includes("Your inventory is empty.")) {
        console.log("  => PASS: Empty inventory correctly handled!");
    } else {
        console.error("  => FAIL: Expected empty inventory message.");
        process.exit(1);
    }

    // SCENARIO B: Coordinate fallback (item 51 Keycard is placed in Room 0 offline inventory)
    console.log("\n[TEST] Scenario B: Coordinate fallback (Keycard at Room 0)");
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        ethers.toBeHex(2000300 + 51),
        ethers.zeroPadValue("0x00", 32)
    ]);
    response = await contract.parseCommand.staticCall(player.address, ethers.toUtf8Bytes("inve"));
    console.log("Response:\n" + response);
    if (response.includes("- Keycard") && !response.includes("- Gold Token") && !response.includes("- Energy Pack")) {
        console.log("  => PASS: Coordinate fallback works correctly!");
    } else {
        console.error("  => FAIL: Coordinate fallback did not output Keycard exclusively.");
        process.exit(1);
    }

    // SCENARIO C: ERC-20 balance dynamic query (transfer token50 to player)
    console.log("\n[TEST] Scenario C: ERC-20 balance query (player holds Gold Token)");
    const mintAmount = ethers.parseEther("10");
    await (await token50.transfer(player.address, mintAmount)).wait();
    console.log(`Transferred ${ethers.formatEther(mintAmount)} Gold Tokens to Player.`);

    response = await contract.parseCommand.staticCall(player.address, ethers.toUtf8Bytes("inve"));
    console.log("Response:\n" + response);
    if (response.includes("- Gold Token (ERC20)") && response.includes("- Keycard")) {
        console.log("  => PASS: ERC-20 balance query correctly combined with coordinate fallback!");
    } else {
        console.error("  => FAIL: Did not detect Gold Token (ERC20).");
        process.exit(1);
    }

    // SCENARIO D: Gas Benchmarks
    console.log("\n[BENCHMARK] Measuring gas used for query commands...");
    const cmdTx = await contract.parseCommand(player.address, ethers.toUtf8Bytes("inve"), { gasLimit: 1000000 });
    const cmdReceipt = await cmdTx.wait();
    console.log(`Gas used for 'inve' transaction execution: ${cmdReceipt.gasUsed.toString()} gas`);

    console.log("\n=== All Tests & Benchmarks Completed Successfully ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
