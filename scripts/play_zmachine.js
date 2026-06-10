const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const readline = require("readline");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

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
    console.log("=== On-Chain Z-Machine CLI Console ===");
    
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error: Could not connect to local node. Please start Hardhat/Anvil first.");
        process.exit(1);
    }
    
    const player = signers[0];
    console.log("Player Address:", player.address);

    let zmachineAddress;
    
    // Attempt to load from config
    if (fs.existsSync(CONFIG_PATH)) {
        const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
        zmachineAddress = config.networks.localhost.zmachineAddress;
    }

    if (!zmachineAddress) {
        console.log("No deployed Z-Machine address found in config. Compiling and deploying fresh instance...");
        const yulPath = path.join(__dirname, "../solidity/bin/zmachine.yul");
        const bytecode = compileYul(yulPath);
        const tx = await player.sendTransaction({ data: bytecode, gasLimit: 4000000 });
        const receipt = await tx.wait();
        zmachineAddress = receipt.contractAddress;
        console.log("Fresh Z-Machine deployed at:", zmachineAddress);
    } else {
        console.log("Using registered Z-Machine at:", zmachineAddress);
    }

    const abi = [
        "function parseCommand(address player, bytes cmd) public returns (string)",
        "function getObjectProperty(uint256 objId, uint256 propId, address player) public view returns (uint256)"
    ];
    const contract = new ethers.Contract(zmachineAddress, abi, player);

    // Deploy and bind MockERC20 token for binding Object 50
    console.log("Setting up mock ERC-20 token rewards for Object 50...");
    const erc20Bin = fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.bin"), "utf8");
    const erc20Abi = JSON.parse(fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.abi"), "utf8"));
    const erc20Factory = new ethers.ContractFactory(erc20Abi, "0x" + erc20Bin, player);
    const token = await erc20Factory.deploy();
    await token.waitForDeployment();
    const tokenAddress = await token.getAddress();

    const storageSlot = ethers.toBeHex(2000000 + 50);
    await provider.send("hardhat_setStorageAt", [
        zmachineAddress,
        storageSlot,
        ethers.zeroPadValue(tokenAddress, 32)
    ]);

    // Fund Z-Machine to enable reward transfers
    await (await token.transfer(zmachineAddress, ethers.parseEther("100"))).wait();
    console.log("ERC-20 token rewards active. Object 50 bound to:", tokenAddress);

    // Deploy and bind Target.sol as the default Room Contract for rooms 1-25
    console.log("Deploying Target contract as the default Room Contract...");
    const compileSolidity = (solPath) => {
        const { execSync } = require("child_process");
        const output = execSync(`solc --bin --optimize --optimize-runs=200 --evm-version shanghai "${solPath}"`, { encoding: "utf8" });
        const lines = output.split("\n");
        const binIndex = lines.findIndex(line => line.includes("Binary:"));
        if (binIndex === -1) {
            throw new Error(`Could not find binary representation for ${solPath}`);
        }
        return "0x" + lines[binIndex + 1].trim();
    };

    const targetSolPath = path.join(__dirname, "../solidity/Target.sol");
    const targetBin = compileSolidity(targetSolPath);
    const targetAbi = [
        "function Type() public view returns (string)",
        "function name() public view returns (string)",
        "function symbol() public view returns (string)",
        "function execute(string cmd) public view returns (string)"
    ];
    const targetFactory = new ethers.ContractFactory(targetAbi, targetBin, player);
    const targetContract = await targetFactory.deploy();
    await targetContract.waitForDeployment();
    const targetAddress = await targetContract.getAddress();
    console.log("Target Room Contract deployed at:", targetAddress);

    console.log("Binding Target Room Contract to room IDs 1 through 25...");
    for (let rId = 1; rId <= 25; rId++) {
        await provider.send("hardhat_setStorageAt", [
            zmachineAddress,
            ethers.toBeHex(2000000 + rId),
            ethers.zeroPadValue(targetAddress, 32)
        ]);
    }
    console.log("Room bindings complete.");

    // Initial setup info
    console.log("\nType commands below (e.g. 'look', 'take', or 'exit' to quit).\n");

    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    const promptUser = () => {
        rl.question("> ", async (input) => {
            const cmd = input.trim();
            if (cmd.toLowerCase() === "exit" || cmd.toLowerCase() === "quit") {
                console.log("Goodbye!");
                rl.close();
                return;
            }

            if (!cmd) {
                promptUser();
                return;
            }

            try {
                // We perform a static call to get the return value
                const response = await contract.parseCommand.staticCall(player.address, ethers.toUtf8Bytes(cmd));
                
                // If the command is modifying state, execute it as a transaction
                if (cmd.toLowerCase() === "take" || cmd.toLowerCase() === "look") {
                    const tx = await contract.parseCommand(player.address, ethers.toUtf8Bytes(cmd), { gasLimit: 1000000 });
                    await tx.wait();
                }
                
                console.log(response);
            } catch (err) {
                console.log("Error sending command:", err.message);
            }
            promptUser();
        });
    };

    promptUser();
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
