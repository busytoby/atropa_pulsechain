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
    console.log("       DYSNOMIA / Z-MACHINE ADVANCED ASYNCHRONOUS NPC DEMO       ");
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

    // 2. Configure game map exits
    const exits = {
        10: "0x0B0D0C00", // Grand Lobby: N->11, S->13, E->12
        11: "0x000A0E00", // Dusty Library: S->10, E->14
        12: "0x0000000A", // Royal Garden: W->10
        13: "0x0A000000", // Cold Dungeon: N->10
        14: "0x0000000B"  // Golden Treasury: W->11
    };

    console.log("Configuring game map layout on EVM...");
    for (const [roomId, hexExits] of Object.entries(exits)) {
        const exitSlot = ethers.toBeHex(3200000 + Number(roomId));
        await provider.send("anvil_setStorageAt", [zmAddress, exitSlot, ethers.zeroPadValue(hexExits, 32)]);
    }

    // 3. Initialize room locations: both start in Room 10
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcCardId)), 32);
    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);

    // 4. Set custom room descriptions
    await bindRoomDnaCustom(10, "You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling.");
    await bindRoomDnaCustom(11, "You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with Auncient spellbooks.");
    await bindRoomDnaCustom(12, "You step into the Royal Garden. Exotic flowers bloom under the sunshine.");
    await bindRoomDnaCustom(13, "You creep into the Cold Dungeon. Chains hang from stone walls, and it smells damp.");
    await bindRoomDnaCustom(14, "You discover the Golden Treasury! Shimmering coins and glowing gems pile high.");

    // Helper to query active room ID of player or NPC
    const getRoomId = async (slotHex) => {
        const val = await provider.send("eth_getStorageAt", [zmAddress, slotHex, "latest"]);
        return Number(ethers.toBigInt(val));
    };

    // Helper to get exits of a room
    const getRoomExits = async (roomId) => {
        const raw = await provider.send("eth_getStorageAt", [zmAddress, ethers.toBeHex(3200000 + roomId), "latest"]);
        const bytes = ethers.getBytes(raw);
        return {
            north: bytes[0],
            south: bytes[1],
            east: bytes[2],
            west: bytes[3]
        };
    };

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
            const rId = await getRoomId(npcRoomSlot);
            return `NPC Responds: My storage slot 4000002 indicates we are currently inside Room ${rId}.`;
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

    await displayRoom();

    // Set up interactive terminal prompt
    rl.setPrompt("\nEnter command ('look', 'north', 'south', 'east', 'west', 'Chat ...'): ");
    rl.prompt();

    // 5. Asynchronous Background Process for NPC Wandering
    // Every 4 seconds, the NPC ticks and has a 50% chance of walking to an adjacent room
    const npcInterval = setInterval(async () => {
        try {
            const playerRoom = await getRoomId(playerRoomSlot);
            const npcRoom = await getRoomId(npcRoomSlot);

            if (Math.random() < 0.5) {
                const npcExits = await getRoomExits(npcRoom);
                const validExits = Object.values(npcExits).filter(id => id !== 0);
                if (validExits.length > 0) {
                    const nextNpcRoom = validExits[Math.floor(Math.random() * validExits.length)];
                    
                    // Update NPC location slot in storage asynchronously
                    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(nextNpcRoom), 32)]);
                    
                    // Alert user if NPC enters/leaves their room
                    if (nextNpcRoom === playerRoom && npcRoom !== playerRoom) {
                        readline.clearLine(process.stdout, 0);
                        readline.cursorTo(process.stdout, 0);
                        console.log("\n[AI NPC] Active Process Card 2 has entered the room.");
                        rl.prompt(true);
                    } else if (npcRoom === playerRoom && nextNpcRoom !== playerRoom) {
                        readline.clearLine(process.stdout, 0);
                        readline.cursorTo(process.stdout, 0);
                        console.log("\n[AI NPC] Active Process Card 2 has walked away.");
                        rl.prompt(true);
                    }
                }
            }
        } catch (err) {
            // Quietly ignore network or block delays
        }
    }, 4000);

    // 6. Non-blocking readline line listener
    rl.on("line", async (line) => {
        const trimmed = line.trim();
        if (trimmed.toLowerCase() === "/exit") {
            clearInterval(npcInterval);
            rl.close();
            process.exit(0);
        }

        if (!trimmed) {
            rl.prompt();
            return;
        }

        try {
            const isChat = trimmed.toLowerCase().startsWith("chat");
            const isMovement = ["north", "south", "east", "west"].includes(trimmed.toLowerCase());
            const isLook = trimmed.toLowerCase() === "look";

            const playerRoom = await getRoomId(playerRoomSlot);
            const npcRoom = await getRoomId(npcRoomSlot);

            if (isChat) {
                if (playerRoom !== npcRoom) {
                    console.log("\n[SYSTEM] You call out, but the NPC is not in this room to hear you.");
                } else {
                    const cmdBytes = ethers.hexlify(Buffer.from(trimmed));
                    await (await zmContract.parseCommand(playerAddress, cmdBytes)).wait();

                    const promptText = trimmed.slice(5).trim();
                    console.log("\n[AI NPC] Processing your message...");
                    const response = await getAiResponse(promptText);
                    await bindRoomDnaCustom(playerRoom, response);
                }
            } else if (isMovement || isLook) {
                const cmdBytes = ethers.hexlify(Buffer.from(trimmed));
                await (await zmContract.parseCommand(playerAddress, cmdBytes)).wait();
            } else {
                console.log("\n[SYSTEM] Unknown command.");
            }

            await displayRoom();
        } catch (err) {
            console.error("Error executing command:", err.message);
        }

        rl.prompt();
    });
}

main().catch(console.error);
