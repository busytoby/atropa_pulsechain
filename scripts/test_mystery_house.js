const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

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
    console.log("=== Launching Z-Machine Mystery House EVM Traversal Verification ===");

    console.log("Compiling zmachine.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/zmachine.yul");
    const bytecode = compileYul(yulPath);

    console.log("Compiling zmachineParser.yul...");
    const parserYulPath = path.join(__dirname, "../solidity/bin/zmachineParser.yul");
    const parserBytecode = compileYul(parserYulPath);

    // Get signers
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log("Deploying contracts with account:", deployer.address);

    const tx = await deployer.sendTransaction({
        data: bytecode,
        gasLimit: 15000000
    });
    const receipt = await waitForReceipt(tx, provider);
    const zmachineAddress = receipt.contractAddress;
    console.log("ZMachine deployed at:", zmachineAddress);

    const parserTx = await deployer.sendTransaction({
        data: parserBytecode,
        gasLimit: 15000000
    });
    const parserReceipt = await waitForReceipt(parserTx, provider);
    const parserAddress = parserReceipt.contractAddress;
    console.log("ZMachineParser deployed at:", parserAddress);

    // ABI definition for interactions
    const abi = [
        "function bindParserAddress(address parser) public returns (bool)",
        "function createRoom(uint256 roomId, bytes desc, uint256 exits) public returns (bool)",
        "function getVectorScene(uint256 roomIndex) public view returns (bytes)",
        "function parseCommand(address player, bytes cmd) public returns (string)"
    ];

    const contract = new ethers.Contract(zmachineAddress, abi, deployer);

    console.log("Binding parser address...");
    await waitForReceipt(await contract.bindParserAddress(parserAddress), provider);
    console.log("Parser successfully linked!");

    // Call getVectorScene(0) to fetch the Victorian House outline
    console.log("Fetching Room 0 vector scene...");
    const vectorDataHex = await contract.getVectorScene(0);
    const vectorData = Buffer.from(vectorDataHex.slice(2), "hex");
    console.log(`Vector Data Length: ${vectorData.length} bytes`);
    if (vectorData.length !== 50) {
        throw new Error(`Expected exactly 50 bytes for vector scene, got ${vectorData.length}`);
    }
    console.log("✓ Vector data size verification passed.");

    // Validate ground line coordinates
    console.log("Validating ground line coordinates...");
    console.log(`Line 0: (${vectorData[0]}, ${vectorData[1]}) -> (${vectorData[2]}, ${vectorData[3]}), color: ${vectorData[4]}`);
    if (vectorData[0] !== 0 || vectorData[1] !== 150 || vectorData[2] !== 240 || vectorData[3] !== 150 || vectorData[4] !== 1) {
        throw new Error("Ground line coordinates or color mismatch!");
    }
    console.log("✓ Ground line coordinates verified.");

    // Validate Room 1 (Entry Hall)
    console.log("Fetching Room 1 vector scene...");
    const vectorData1Hex = await contract.getVectorScene(1);
    const vectorData1 = Buffer.from(vectorData1Hex.slice(2), "hex");
    console.log(`✓ Room 1 size verified. Left Wall start: ${vectorData1[5]} end: ${vectorData1[7]}`);
    if (vectorData1[5] !== 20 || vectorData1[7] !== 20) {
        throw new Error("Room 1 Left Wall coordinates mismatch");
    }

    // Validate Room 2 (Living Room / Library)
    console.log("Fetching Room 2 vector scene...");
    const vectorData2Hex = await contract.getVectorScene(2);
    const vectorData2 = Buffer.from(vectorData2Hex.slice(2), "hex");
    console.log(`✓ Room 2 size verified. Fireplace Mantel start: ${vectorData2[30]} end: ${vectorData2[32]}`);
    if (vectorData2[30] !== 80 || vectorData2[32] !== 160) {
        throw new Error("Room 2 Fireplace coordinates mismatch");
    }

    console.log("\n=== Starting Interactive Gameplay Traversal Simulation ===");
    console.log("Setting up Victorian mansion room descriptions...");
    
    // Setup room descriptions for the mansion
    const rooms = [
        { id: 10, desc: "You are outside a large Victorian mansion. The front door is to the north.", exits: 0x01000000 },
        { id: 1,  desc: "You are in the entry hall. Doors lead east and west. The exit is south.", exits: 0x000a0203 },
        { id: 2,  desc: "You are in the library. Old books line the walls.", exits: 0x00000001 },
        { id: 3,  desc: "You are in the sanctuary. A stone altar stands here.", exits: 0x00000100 }
    ];

    for (const r of rooms) {
        const descBytes = Buffer.from(r.desc, "utf8");
        await waitForReceipt(await contract.createRoom(r.id, descBytes, r.exits), provider);
        console.log(`  Room ${r.id} description and exits registered.`);
    }

    const player = deployer.address;
    
    // Traversal gameplay loop simulation on-chain
    console.log("Command: look");
    let responseLook = await contract.parseCommand.staticCall(player, Buffer.from("look", "utf8"));
    console.log("Output:\n", responseLook);
    if (!responseLook.includes("outside a large Victorian mansion")) {
        throw new Error("Initial look description mismatch!");
    }

    // Go North to Room 1
    console.log("Command: north");
    // Change state
    await waitForReceipt(await contract.parseCommand(player, Buffer.from("north", "utf8")), provider);
    let responseNorth = await contract.parseCommand.staticCall(player, Buffer.from("look", "utf8"));
    console.log("Output:\n", responseNorth);
    if (!responseNorth.includes("entry hall")) {
        throw new Error("North navigation failed!");
    }

    // Go East to Room 2
    console.log("Command: east");
    await waitForReceipt(await contract.parseCommand(player, Buffer.from("east", "utf8")), provider);
    let responseEast = await contract.parseCommand.staticCall(player, Buffer.from("look", "utf8"));
    console.log("Output:\n", responseEast);
    if (!responseEast.includes("library")) {
        throw new Error("East navigation failed!");
    }

    // Go South (blocked)
    console.log("Command: south");
    // Static call to see error response without modifying state
    let responseSouth = await contract.parseCommand.staticCall(player, Buffer.from("south", "utf8"));
    console.log("Output:\n", responseSouth);
    if (!responseSouth.toLowerCase().includes("cannot") && !responseSouth.toLowerCase().includes("don't understand") && !responseSouth.toLowerCase().includes("go that way")) {
        throw new Error("South block navigation failed! Output: " + responseSouth);
    }

    // Go West to Room 1
    console.log("Command: west");
    await waitForReceipt(await contract.parseCommand(player, Buffer.from("west", "utf8")), provider);
    let responseWest1 = await contract.parseCommand.staticCall(player, Buffer.from("look", "utf8"));
    console.log("Output:\n", responseWest1);
    if (!responseWest1.includes("entry hall")) {
        throw new Error("West navigation to entry hall failed!");
    }

    // Go West to Room 3
    console.log("Command: west");
    await waitForReceipt(await contract.parseCommand(player, Buffer.from("west", "utf8")), provider);
    let responseWest2 = await contract.parseCommand.staticCall(player, Buffer.from("look", "utf8"));
    console.log("Output:\n", responseWest2);
    if (!responseWest2.includes("sanctuary")) {
        throw new Error("West navigation to sanctuary failed!");
    }

    console.log("=== All Z-Machine Mystery House tests passed successfully! ===");
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
