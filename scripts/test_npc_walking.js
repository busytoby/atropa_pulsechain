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
    console.log("STARTING WALKING NPC & MULTI-CHARACTER ROOM STATE TEST");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    const playerAddress = signer.address;
    const npcAddress = "0x0000000000000000000000000000000000000003";

    console.log(`Player Address: ${playerAddress}`);
    console.log(`NPC Address:    ${npcAddress}`);

    // 1. Deploy Z-Machine and Parser
    console.log("Compiling and deploying Z-Machine contract...");
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
    // We create a lobby (Room 10) connected to Hallway (Room 11) to the North
    // Room 10 exits: slot 3200000 + 10 = 3200010. Exits value: 11 to North (0x0B000000)
    const exitsRoom10Slot = ethers.toBeHex(3200000 + 10);
    const exitsRoom10Value = ethers.zeroPadValue("0x0B000000", 32); // North -> 11 (0x0b)
    
    // Room 11 exits: slot 3200000 + 11 = 3200011. Exits value: 10 to South (0x000A0000)
    const exitsRoom11Slot = ethers.toBeHex(3200000 + 11);
    const exitsRoom11Value = ethers.zeroPadValue("0x000A0000", 32); // South -> 10 (0x0a)

    console.log("\nSetting up game map on EVM...");
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom10Slot, exitsRoom10Value]);
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom11Slot, exitsRoom11Value]);
    console.log("Room 10 (Lobby) and Room 11 (Hallway) connected.");

    // 3. Set Initial room locations: Both player and NPC start in Room 10
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcAddress)), 32);

    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    console.log("Initialized Player and NPC in Room 10.");

    // Helper to query location
    const getRoomId = async (addr) => {
        const slot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(addr)), 32);
        const val = await provider.send("eth_getStorageAt", [zmAddress, slot, "latest"]);
        return Number(ethers.toBigInt(val));
    };

    console.log(`\n--- Initial State Verification ---`);
    console.log(`Player Room ID: ${await getRoomId(playerAddress)}`);
    console.log(`NPC Room ID:    ${await getRoomId(npcAddress)}`);

    // 4. Simulate NPC walking North
    console.log(`\n--- Simulating NPC walking North ---`);
    const walkNorthCmd = ethers.hexlify(Buffer.from("north"));
    
    // NPC executes command
    const npcResult = await zmContract.parseCommand.staticCall(npcAddress, walkNorthCmd);
    console.log(`NPC command output: "${npcResult.trim()}"`);
    const npcTx = await zmContract.parseCommand(npcAddress, walkNorthCmd);
    console.log("NPC submitted 'north' command. Hash:", npcTx.hash);
    const npcReceipt = await npcTx.wait();

    // Verify locations
    const playerRoomAfterNPCWalk = await getRoomId(playerAddress);
    const npcRoomAfterNPCWalk = await getRoomId(npcAddress);
    console.log(`Player Room ID: ${playerRoomAfterNPCWalk}`);
    console.log(`NPC Room ID:    ${npcRoomAfterNPCWalk} (Expected: 11 - Hallway)`);

    if (npcRoomAfterNPCWalk !== 11) {
        throw new Error("NPC failed to walk North to Room 11!");
    }
    console.log("NPC successfully moved to Room 11.");

    // 5. Simulate NPC walking back South
    console.log(`\n--- Simulating NPC walking back South ---`);
    const walkSouthCmd = ethers.hexlify(Buffer.from("south"));
    
    const npcResult2 = await zmContract.parseCommand.staticCall(npcAddress, walkSouthCmd);
    console.log(`NPC command output: "${npcResult2.trim()}"`);
    const npcTx2 = await zmContract.parseCommand(npcAddress, walkSouthCmd);
    await npcTx2.wait();
    console.log("NPC submitted 'south' command.");

    // Verify locations
    const playerRoomFinal = await getRoomId(playerAddress);
    const npcRoomFinal = await getRoomId(npcAddress);
    console.log(`Player Room ID: ${playerRoomFinal}`);
    console.log(`NPC Room ID:    ${npcRoomFinal} (Expected: 10 - Lobby)`);

    if (npcRoomFinal !== 10) {
        throw new Error("NPC failed to walk South back to Room 10!");
    }
    console.log("NPC successfully returned to Room 10.");

    console.log("\n===============================================================");
    console.log("SUCCESS: Walking NPC and Multi-Character Room routing Verified!");
    console.log("===============================================================");
}

main().catch(console.error);
