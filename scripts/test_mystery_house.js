const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

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
    console.log("=== Launching Z-Machine Mystery House Vector Verification ===");
    
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    console.log("Compiling and deploying zmachine.yul...");
    const zmBytecode = compileYul(path.join(__dirname, "../solidity/bin/zmachine.yul"));
    const zmTx = await deployer.sendTransaction({ data: zmBytecode, gasLimit: 15000000 });
    const zmReceipt = await zmTx.wait();
    const zmAddress = zmReceipt.contractAddress;
    console.log("Z-Machine contract deployed at:", zmAddress);

    console.log("Compiling and deploying zmachineParser.yul...");
    const parserBytecode = compileYul(path.join(__dirname, "../solidity/bin/zmachineParser.yul"));
    const parserTx = await deployer.sendTransaction({ data: parserBytecode, gasLimit: 15000000 });
    const parserReceipt = await parserTx.wait();
    const parserAddress = parserReceipt.contractAddress;
    console.log("Z-Machine Parser contract deployed at:", parserAddress);

    const zmAbi = [
        "function getVectorScene(uint256 roomIndex) public view returns (bytes)",
        "function bindParserAddress(address parser) public returns (bool)"
    ];
    const zm = new ethers.Contract(zmAddress, zmAbi, deployer);

    console.log("Linking Parser Address...");
    await (await zm.bindParserAddress(parserAddress)).wait();

    // Call getVectorScene(0) to fetch the Victorian House outline
    console.log("Fetching Room 0 vector scene...");
    const vectorData = await zm.getVectorScene(0);
    console.log(`Vector Data Length: ${ethers.getBytes(vectorData).length} bytes`);
    
    // Convert to byte array
    const bytes = ethers.getBytes(vectorData);
    if (bytes.length !== 50) {
        throw new Error(`Expected exactly 50 bytes for vector scene, got ${bytes.length}`);
    }
    console.log("✓ Vector data size verification passed.");

    // Validate the first line command: Ground line (0, 150) to (240, 150)
    console.log("Validating ground line coordinates...");
    const x0 = bytes[0];
    const y0 = bytes[1];
    const x1 = bytes[2];
    const y1 = bytes[3];
    const color = bytes[4];
    console.log(`Line 0: (${x0}, ${y0}) -> (${x1}, ${y1}), color: ${color}`);
    if (x0 !== 0 || y0 !== 150 || x1 !== 240 || y1 !== 150 || color !== 1) {
        throw new Error("Ground line coordinates or color mismatch!");
    }
    console.log("✓ Ground line coordinates verified.");

    // Validate Room 1 (Entry Hall)
    console.log("Fetching Room 1 vector scene...");
    const vectorData1 = await zm.getVectorScene(1);
    const bytes1 = ethers.getBytes(vectorData1);
    if (bytes1.length !== 50) {
        throw new Error(`Expected 50 bytes for Room 1, got ${bytes1.length}`);
    }
    console.log("✓ Room 1 size verified. Left Wall start:", bytes1[5], "end:", bytes1[7]);
    if (bytes1[5] !== 20 || bytes1[7] !== 20) {
        throw new Error("Room 1 Left Wall coordinates mismatch");
    }

    // Validate Room 2 (Living Room)
    console.log("Fetching Room 2 vector scene...");
    const vectorData2 = await zm.getVectorScene(2);
    const bytes2 = ethers.getBytes(vectorData2);
    if (bytes2.length !== 50) {
        throw new Error(`Expected 50 bytes for Room 2, got ${bytes2.length}`);
    }
    console.log("✓ Room 2 size verified. Fireplace Mantel start:", bytes2[30], "end:", bytes2[32]);
    if (bytes2[30] !== 80 || bytes2[32] !== 160) {
        throw new Error("Room 2 Fireplace coordinates mismatch");
    }

    // 3. Define extended ABI for interactive gameplay simulation
    console.log("\n=== Starting Interactive Gameplay Traversal Simulation ===");
    const zmExtendedAbi = [
        ...zmAbi,
        "function createRoom(uint256 roomId, bytes desc, uint256 exits) public returns (bool)",
        "function parseCommand(address player, bytes cmd) public returns (string)"
    ];
    const zmExtended = new ethers.Contract(zmAddress, zmExtendedAbi, deployer);

    // Setup room descriptions for the mansion
    console.log("Setting up Victorian mansion room descriptions...");
    const rooms = [
        { id: 10, desc: "You are outside a large Victorian mansion. The front door is to the north." },
        { id: 1,  desc: "You are in the entry hall. Doors lead east and west. The exit is south." },
        { id: 2,  desc: "You are in the library. Old books line the walls." },
        { id: 3,  desc: "You are in the sanctuary. A stone altar stands here." }
    ];

    for (const r of rooms) {
        const descBytes = Buffer.from(r.desc);
        await (await zmExtended.createRoom(r.id, descBytes, 0)).wait();
        console.log(`  Room ${r.id} description registered.`);
    }

    // Traversal gameplay loop simulation
    const playerWallet = signers[1];
    const playerContract = new ethers.Contract(zmAddress, zmExtendedAbi, playerWallet);

    async function executeCommand(cmd) {
        const response = await playerContract.parseCommand.staticCall(playerWallet.address, Buffer.from(cmd));
        const tx = await playerContract.parseCommand(playerWallet.address, Buffer.from(cmd), { gasLimit: 2000000 });
        await tx.wait();
        return response;
    }

    console.log("\nSimulating player traversal commands:");

    // Look (starts at Room 10)
    console.log("Command: look");
    const responseLook = await executeCommand("look");
    console.log("Output:\n", responseLook);
    if (!responseLook.includes("outside a large Victorian mansion")) {
        throw new Error("Initial look description mismatch!");
    }

    // Go North to Room 1
    console.log("Command: north");
    const responseNorth = await executeCommand("north");
    console.log("Output:\n", responseNorth);
    if (!responseNorth.includes("entry hall")) {
        throw new Error("North navigation failed!");
    }

    // Go East to Room 2
    console.log("Command: east");
    const responseEast = await executeCommand("east");
    console.log("Output:\n", responseEast);
    if (!responseEast.includes("library")) {
        throw new Error("East navigation failed!");
    }

    // Attempt invalid South direction from Room 2
    console.log("Command: south");
    const responseInvalid = await executeCommand("south");
    console.log("Output:\n", responseInvalid);
    if (!responseInvalid.includes("cannot go that way")) {
        throw new Error("Invalid exit validation failed!");
    }

    // Go West back to Room 1
    console.log("Command: west");
    const responseWest = await executeCommand("west");
    console.log("Output:\n", responseWest);
    if (!responseWest.includes("entry hall")) {
        throw new Error("West navigation back to Room 1 failed!");
    }

    // Go West to Room 3
    console.log("Command: west");
    const responseSanctuary = await executeCommand("west");
    console.log("Output:\n", responseSanctuary);
    if (!responseSanctuary.includes("sanctuary")) {
        throw new Error("West navigation to Sanctuary failed!");
    }

    console.log("=== All Z-Machine Mystery House tests passed successfully! ===");
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
