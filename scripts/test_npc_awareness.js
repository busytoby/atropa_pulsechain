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
    console.log("NPC STATE & CAPABILITIES AWARENESS GENERATOR");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    
    // NPC is represented on-chain by Card ID 2
    const npcCardId = 2;
    const npcAddress = ethers.zeroPadValue(ethers.toBeHex(npcCardId), 20);

    // 1. Deploy Z-Machine and Parser
    console.log("\nCompiling and deploying Z-Machine contract...");
    const zmBytecode = compileYul("../solidity/bin/zmachine.yul");
    const zmFactory = new ethers.ContractFactory([], zmBytecode, signer);
    const zm = await zmFactory.deploy();
    await zm.waitForDeployment();
    const zmAddress = await zm.getAddress();

    console.log("Compiling and deploying Z-Machine Parser contract...");
    const parserBytecode = compileYul("../solidity/bin/zmachineParser.yul");
    const parserFactory = new ethers.ContractFactory([], parserBytecode, signer);
    const parser = await parserFactory.deploy();
    await parser.waitForDeployment();
    const parserAddress = await parser.getAddress();

    const zmContract = new ethers.Contract(zmAddress, [
        "function parseCommand(address player, bytes cmd) public returns (string)",
        "function bindParserAddress(address parser) public returns (bool)"
    ], signer);

    await (await zmContract.bindParserAddress(parserAddress)).wait();

    // 2. Setup room layout
    // Room 12 exits: North -> 13 (0x0D000000), East -> 14 (0x000E0000)
    const exitsRoom12Slot = ethers.toBeHex(3200000 + 12);
    const exitsRoom12Value = ethers.zeroPadValue("0x0D0E0000", 32); 
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom12Slot, exitsRoom12Value]);

    // Place NPC Card 2 in Room 12
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcCardId)), 32);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);

    // Bind a mock ERC20/QING token contract to Room 12 to show dynamic capabilities
    const mockTokenAddress = "0x8888888888888888888888888888888888888888";
    const tokenBindingSlot = ethers.toBeHex(2000000 + 12);
    await provider.send("anvil_setStorageAt", [zmAddress, tokenBindingSlot, ethers.zeroPadValue(mockTokenAddress, 32)]);

    // Also place an item (e.g. Magic Sword, ID 60) in Room 12
    const swordSlot = ethers.toBeHex(2000300 + 60);
    await provider.send("anvil_setStorageAt", [zmAddress, swordSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);

    console.log("\n[AWARENESS DAEMON] Querying EVM State for NPC...");

    // 3. Extract position
    const rawRoom = await provider.send("eth_getStorageAt", [zmAddress, npcRoomSlot, "latest"]);
    const currentRoomId = Number(ethers.toBigInt(rawRoom));

    // 4. Extract layout exits
    const rawExits = await provider.send("eth_getStorageAt", [zmAddress, ethers.toBeHex(3200000 + currentRoomId), "latest"]);
    const exitsVal = ethers.getBytes(rawExits);
    const exits = [];
    if (exitsVal[0] !== 0) exits.push(`North (Room ${exitsVal[0]})`);
    if (exitsVal[1] !== 0) exits.push(`South (Room ${exitsVal[1]})`);
    if (exitsVal[2] !== 0) exits.push(`East (Room ${exitsVal[2]})`);
    if (exitsVal[3] !== 0) exits.push(`West (Room ${exitsVal[3]})`);

    // 5. Call static look under NPC's identity to discover room content & capabilities
    const lookCmd = ethers.hexlify(Buffer.from("look"));
    const roomDescription = await zmContract.parseCommand.staticCall(npcAddress, lookCmd);

    // 6. Assemble AI Prompt Context
    console.log("\n===============================================================");
    console.log("ASSEMBLED LLM PROMPT FOR AI NPC:");
    console.log("===============================================================");

    const llmPrompt = {
        role: "system",
        content: `You are an autonomous AI daemon NPC process running in the Z-Machine game.
Your current location, surroundings, and capabilities have been extracted from EVM storage:

[EVM REALTIME STATE]
- NPC Identity Card ID: ${npcCardId}
- Current Room ID: ${currentRoomId}
- Available Exits: ${exits.length > 0 ? exits.join(", ") : "None"}
- Room Description:
"""
${roomDescription.trim()}
"""

[CAPABILITIES]
You can respond with any standard Z-Machine command matching the room's available exits or the listed available commands (e.g., "north", "east", "take", "look", "Chat hello traveler").

Decide on your next action.`
    };

    console.log(JSON.stringify(llmPrompt, null, 2));
    console.log("===============================================================");
}

main().catch(console.error);
