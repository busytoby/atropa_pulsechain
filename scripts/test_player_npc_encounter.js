const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

function compileYul(yulPath) {
    const absolutePath = path.resolve(__dirname, yulPath);
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("===============================================================");
    console.log("STARTING PLAYER-NPC ENCOUNTER TEST IN Z-MACHINE");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    const playerAddress = signer.address;
    const npcCardId = 2;

    console.log(`Player Address: ${playerAddress}`);
    console.log(`NPC Card ID:    ${npcCardId}`);

    // 1. Deploy Z-Machine and Parser
    console.log("\nCompiling and deploying Z-Machine contract...");
    const zmBytecode = compileYul("../solidity/bin/zmachine.yul");
    const zmFactory = new ethers.ContractFactory([], zmBytecode, signer);
    const zm = await zmFactory.deploy();
    await zm.waitForDeployment();
    const zmAddress = await zm.getAddress();
    console.log(`Z-Machine deployed at: ${zmAddress}`);

    console.log("Compiling and deploying Z-Machine Parser contract...");
    const parserBytecode = compileYul("../solidity/bin/zmachineParser.yul");
    const parserFactory = new ethers.ContractFactory([], parserBytecode, signer);
    const parser = await parserFactory.deploy();
    await parser.waitForDeployment();
    const parserAddress = await parser.getAddress();
    console.log(`Z-Machine Parser deployed at: ${parserAddress}`);

    const zmContract = new ethers.Contract(zmAddress, [
        "function parseCommand(address player, bytes cmd) public returns (string)",
        "function bindParserAddress(address parser) public returns (bool)"
    ], signer);

    console.log("Binding Parser address to Z-Machine...");
    await (await zmContract.bindParserAddress(parserAddress)).wait();
    console.log("Parser address bound successfully.");

    // 2. Setup Game Map using direct storage injection on Anvil
    // Room 10 exits: North -> 11 (0x0B000000)
    const exitsRoom10Slot = ethers.toBeHex(3200000 + 10);
    const exitsRoom10Value = ethers.zeroPadValue("0x0B000000", 32);
    
    // Room 11 exits: South -> 10 (0x000A0000)
    const exitsRoom11Slot = ethers.toBeHex(3200000 + 11);
    const exitsRoom11Value = ethers.zeroPadValue("0x000A0000", 32);

    console.log("\nSetting up game map exits on EVM...");
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom10Slot, exitsRoom10Value]);
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom11Slot, exitsRoom11Value]);
    console.log("Room 10 (Lobby) and Room 11 (Hallway) connected.");

    // 3. Initialize room locations
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcCardId)), 32);

    // Set initial locations: both start in Room 10
    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    console.log("Player and NPC initialized in Room 10.");

    // Verification helper
    const getRoomId = async (slotHex) => {
        const val = await provider.send("eth_getStorageAt", [zmAddress, slotHex, "latest"]);
        return Number(ethers.toBigInt(val));
    };

    console.log(`Player starting Room: ${await getRoomId(playerRoomSlot)}`);
    console.log(`NPC starting Room:    ${await getRoomId(npcRoomSlot)}`);

    // 4. Move NPC autonomously to Room 11 (simulating process slice scheduler execution)
    console.log("\nMoving NPC (Card 2) to Room 11...");
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(11), 32)]);
    console.log(`NPC new Room: ${await getRoomId(npcRoomSlot)}`);

    // 5. Let the player walk North to Room 11
    console.log("\nPlayer walking North to Room 11...");
    const walkNorthCmd = ethers.hexlify(Buffer.from("north"));
    
    // Check static call response
    const walkResult = await zmContract.parseCommand.staticCall(playerAddress, walkNorthCmd);
    console.log(`Walk output (static): "${walkResult.trim()}"`);
    
    // Execute state change transaction
    const tx = await zmContract.parseCommand(playerAddress, walkNorthCmd);
    await tx.wait();
    
    console.log(`Player current Room: ${await getRoomId(playerRoomSlot)} (Expected: 11)`);
    if (await getRoomId(playerRoomSlot) !== 11) {
        throw new Error("Player failed to move to Room 11!");
    }

    // 6. Execute "look" command and assert NPC presence description is appended
    console.log("\nExecuting 'look' command...");
    const lookCmd = ethers.hexlify(Buffer.from("look"));
    const lookResult = await zmContract.parseCommand.staticCall(playerAddress, lookCmd);
    console.log("Z-Machine output:\n------------------------------------");
    console.log(lookResult);
    console.log("------------------------------------");

    // Check for NPC presence text
    console.log("lookResult Hex length:", lookResult.length);
    console.log("lookResult Hex bytes of NPC string section:");
    const index = lookResult.indexOf("[NPC]");
    if (index !== -1) {
        const slice = lookResult.substring(index, index + 50);
        console.log(`Slice: "${slice}"`);
        console.log("Bytes:", Buffer.from(slice).toString("hex"));
    }
    const expectedString = "[NPC] Active Process Card2\0 standing here.";
    if (!lookResult.includes(expectedString)) {
        throw new Error(`Encounter assertion failed! Output does not contain: "${expectedString}"`);
    }

    console.log("\n===============================================================");
    console.log("SUCCESS: Encounter verified! Player meets the NPC in the room.");
    console.log("===============================================================");
}

main().catch(console.error);
