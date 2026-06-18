const { ethers } = require("ethers");
const readline = require("readline");
const { execSync } = require("child_process");
const path = require("path");

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
    console.clear();
    console.log("=================================================================");
    console.log("          DYSNOMIA / Z-MACHINE INTERACTIVE AI NPC CHAT           ");
    console.log("=================================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    const playerAddress = signer.address;
    const npcCardId = 2;
    const npcAddress = ethers.zeroPadValue(ethers.toBeHex(npcCardId), 20);

    // 1. Deploy Z-Machine and Parser
    console.log("\nCompiling and deploying Z-Machine environment...");
    const zmBytecode = compileYul("../solidity/bin/zmachine.yul");
    const zmFactory = new ethers.ContractFactory([], zmBytecode, signer);
    const zm = await zmFactory.deploy();
    await zm.waitForDeployment();
    const zmAddress = await zm.getAddress();

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

    // Helper to bind Room DNA using the custom selector 0xb23e800d
    const bindRoomDnaCustom = async (roomId, dnaText) => {
        const dummyInterface = new ethers.Interface([
            "function dummy(uint256, bytes) public"
        ]);
        const dnaBytes = ethers.toUtf8Bytes(dnaText);
        const encoded = dummyInterface.encodeFunctionData("dummy", [roomId, dnaBytes]);
        const customData = "0xb23e800d" + encoded.slice(10);
        const tx = await signer.sendTransaction({
            to: zmAddress,
            data: customData,
            gasLimit: 2000000
        });
        await tx.wait();
    };

    // 2. Setup game map (Room 12 is ZMM room)
    const exitsRoom12Slot = ethers.toBeHex(3200000 + 12);
    const exitsRoom12Value = ethers.zeroPadValue("0x00000000", 32); // Isolated test chamber
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom12Slot, exitsRoom12Value]);

    // 3. Initialize room locations to Room 12
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcCardId)), 32);
    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(12), 32)]);

    console.log(`\nPlayer and NPC spawned in Room 12.`);
    console.log("Ready for interactive testing. Type '/exit' to quit.");

    const countRooms = async () => {
        let count = 0;
        for (let r = 0; r <= 50; r++) {
            const slot = ethers.toBeHex(3200000 + r);
            const val = await provider.send("eth_getStorageAt", [zmAddress, slot, "latest"]);
            if (ethers.toBigInt(val) !== 0n) {
                count++;
            }
        }
        return count;
    };

    const getAiResponse = async (msg) => {
        const query = msg.toLowerCase();
        if (query.includes("how many rooms") || query.includes("room count")) {
            const numRooms = await countRooms();
            return `NPC Responds: Based on my realtime EVM scan of layout slots (3200000 to 3200050), there are currently ${numRooms} active rooms configured with exit mappings in this Z-Machine instance.`;
        }
        if (query.includes("identity") || query.includes("who are you")) {
            return "NPC Responds: I am the daemon process running on Card ID 2. I regulate time-sliced ZMM activity.";
        }
        if (query.includes("where") || query.includes("position") || query.includes("room")) {
            return "NPC Responds: My storage slot 4000002 indicates we are currently inside Room 12.";
        }
        if (query.includes("capability") || query.includes("what can you do")) {
            return "NPC Responds: I can walk, take items, and process chat commands under my registered card identity.";
        }
        if (query.includes("hello") || query.includes("hi")) {
            return "NPC Responds: Greetings traveler! Ask me about my identity, position, capabilities, or how many rooms there are.";
        }
        return `NPC Responds: Processing prompt "${msg}"... My internal states are fully synchronized.`;
    };

    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    const displayRoom = async () => {
        const lookCmd = ethers.hexlify(Buffer.from("look"));
        const lookResult = await zmContract.parseCommand.staticCall(playerAddress, lookCmd);
        console.log("\n=================== Z-MACHINE VIEW ===================");
        console.log(lookResult.trim());
        console.log("======================================================");
    };

    // Print initial state
    await displayRoom();

    const askPrompt = () => {
        rl.question("\nEnter chat prompt (e.g. 'hello npc'): ", async (input) => {
            const trimmed = input.trim();
            if (trimmed.toLowerCase() === "/exit") {
                rl.close();
                process.exit(0);
            }

            if (!trimmed) {
                askPrompt();
                return;
            }

            const chatCommand = `Chat ${trimmed}`;
            console.log(`\nExecuting: "${chatCommand}"...`);
            
            try {
                // Submit Player's Chat command to Z-Machine
                const cmdBytes = ethers.hexlify(Buffer.from(chatCommand));
                const tx = await zmContract.parseCommand(playerAddress, cmdBytes);
                await tx.wait();

                // Mocking the background AI Daemon detecting the event & responding
                console.log("[AI DAEMON] Intercepted prompt. Simulating LLM response...");
                const response = await getAiResponse(trimmed);
                console.log(`[AI DAEMON] NPC Response Generated: "${response}"`);

                // Write response to Room DNA
                await bindRoomDnaCustom(12, response);

                // Show updated room output
                await displayRoom();
            } catch (err) {
                console.error("Error executing command:", err.message);
            }

            askPrompt();
        });
    };

    askPrompt();
}

main().catch(console.error);
