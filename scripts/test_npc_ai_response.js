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
    console.log("STARTING AI NPC PROMPT-RESPONSE TEST IN Z-MACHINE");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    const playerAddress = signer.address;
    const npcCardId = 2;

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

    // Helper to bind Room DNA using the custom selector 0xb23e800d
    const bindRoomDnaCustom = async (roomId, dnaText) => {
        const dummyInterface = new ethers.Interface([
            "function dummy(uint256, bytes) public"
        ]);
        const dnaBytes = ethers.toUtf8Bytes(dnaText);
        const encoded = dummyInterface.encodeFunctionData("dummy", [roomId, dnaBytes]);
        // Swap dummy selector with custom selector 0xb23e800d
        const customData = "0xb23e800d" + encoded.slice(10);
        const tx = await signer.sendTransaction({
            to: zmAddress,
            data: customData,
            gasLimit: 2000000
        });
        await tx.wait();
    };

    // 2. Setup game map (Room 12 is ZMM room)
    // Room 12 exits: South -> 10 (0x000A0000)
    const exitsRoom12Slot = ethers.toBeHex(3200000 + 12);
    const exitsRoom12Value = ethers.zeroPadValue("0x000A0000", 32);
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom12Slot, exitsRoom12Value]);

    // 3. Initialize room locations to Room 12
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcCardId)), 32);
    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);

    // 4. Simulate player typing Chat prompt
    const promptMessage = "Chat hello npc";
    console.log(`\nPlayer sends prompt: "${promptMessage}"`);
    const promptCmd = ethers.hexlify(Buffer.from(promptMessage));
    const promptResult = await zmContract.parseCommand.staticCall(playerAddress, promptCmd);
    console.log(`Z-Machine output: "${promptResult.trim()}"`);

    // Execute state transaction for Chat
    const tx = await zmContract.parseCommand(playerAddress, promptCmd);
    await tx.wait();

    // 5. AI Daemon Listener Loop simulation
    console.log("\n[AI DAEMON] Prompt detected. Generating AI response...");
    
    const getAiResponse = (prompt) => {
        if (prompt.toLowerCase().includes("hello")) {
            return "NPC Responds: Hello Traveler! I am the daemon process of Card 2. How can I assist you today?";
        }
        return "NPC Responds: I hear you, but my model parameters are currently optimizing.";
    };

    const responseText = getAiResponse(promptMessage);
    console.log(`[AI DAEMON] Generated response: "${responseText}"`);

    // The AI daemon updates Room 12's DNA/description dynamically
    console.log("[AI DAEMON] Updating Room DNA with the response...");
    await bindRoomDnaCustom(12, responseText);
    console.log("Room DNA successfully bound.");

    // 6. Player calls "look" and observes the AI NPC response
    console.log("\nExecuting 'look' command...");
    const lookCmd = ethers.hexlify(Buffer.from("look"));
    const lookResult = await zmContract.parseCommand.staticCall(playerAddress, lookCmd);
    console.log("Z-Machine output:\n------------------------------------");
    console.log(lookResult);
    console.log("------------------------------------");

    // Verification
    if (!lookResult.includes(responseText)) {
        throw new Error("AI response not present in the room description!");
    }
    const npcPresenceString = "[NPC] Active Process Card2\0 standing here.";
    if (!lookResult.includes(npcPresenceString)) {
        throw new Error("NPC presence string not found!");
    }

    console.log("\n===============================================================");
    console.log("SUCCESS: AI NPC prompt-response loop successfully verified!");
    console.log("===============================================================");
}

main().catch(console.error);
