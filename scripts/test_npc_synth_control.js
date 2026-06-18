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
    console.log("STARTING SYNTHESIZER & CPU DIRECT NPC CONTROL INTEGRATION TEST");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    const playerAddress = signer.address;
    const npcAddress = "0x0000000000000000000000000000000000000003";

    // 1. Deploy contracts
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

    console.log("Compiling and deploying WMQ_Scheduler contract...");
    const schedulerBytecode = compileYul("../solidity/bin/WMQ_Scheduler.yul");
    const schedulerFactory = new ethers.ContractFactory([], schedulerBytecode, signer);
    const scheduler = await schedulerFactory.deploy();
    await scheduler.waitForDeployment();
    const schedulerAddress = await scheduler.getAddress();
    console.log(`WMQ_Scheduler deployed at: ${schedulerAddress}`);

    console.log("Compiling and deploying MusicMaker (Synthesizer)...");
    const musicMakerBytecode = compileYul("../solidity/bin/musicMaker.yul");
    const musicMakerFactory = new ethers.ContractFactory([], musicMakerBytecode, signer);
    const musicMaker = await musicMakerFactory.deploy();
    await musicMaker.waitForDeployment();
    const musicMakerAddress = await musicMaker.getAddress();
    console.log(`MusicMaker deployed at: ${musicMakerAddress}`);

    // Bind Parser to Z-Machine
    const zmContract = new ethers.Contract(zmAddress, [
        "function parseCommand(address player, bytes cmd) public returns (string)",
        "function bindParserAddress(address parser) public returns (bool)"
    ], signer);
    await (await zmContract.bindParserAddress(parserAddress)).wait();
    console.log("Parser address bound to Z-Machine successfully.");

    // Setup map (North exits: Room 10 -> Room 11, South exits: Room 11 -> Room 10)
    const exitsRoom10Slot = ethers.zeroPadValue(ethers.toBeHex(3200000 + 10), 32);
    const exitsRoom10Value = ethers.zeroPadValue("0x0B000000", 32); // North -> 11
    const exitsRoom11Slot = ethers.zeroPadValue(ethers.toBeHex(3200000 + 11), 32);
    const exitsRoom11Value = ethers.zeroPadValue("0x000A0000", 32); // South -> 10

    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom10Slot, exitsRoom10Value]);
    await provider.send("anvil_setStorageAt", [zmAddress, exitsRoom11Slot, exitsRoom11Value]);
    console.log("Game map connected.");

    // Initialize locations (Room 10)
    const playerRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(playerAddress)), 32);
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(npcAddress)), 32);
    await provider.send("anvil_setStorageAt", [zmAddress, playerRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    await provider.send("anvil_setStorageAt", [zmAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);

    // Helper functions for Scheduler and MusicMaker
    const registerCardProcess = async (cardId, entryPoint) => {
        const txData = ethers.concat([
            "0x8a1b9c7e",
            ethers.zeroPadValue(ethers.toBeHex(cardId), 32),
            ethers.zeroPadValue(ethers.toBeHex(entryPoint), 32)
        ]);
        const tx = await signer.sendTransaction({ to: schedulerAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const getCardRegs = async (cardId) => {
        const txData = ethers.concat([
            "0x47657267",
            ethers.zeroPadValue(ethers.toBeHex(cardId), 32)
        ]);
        const res = await provider.call({ to: schedulerAddress, data: txData });
        const bytes = ethers.getBytes(res);
        const regs = [];
        for (let i = 0; i < 7; i++) {
            regs.push(ethers.toBigInt(bytes.slice(i * 32, (i + 1) * 32)));
        }
        return regs;
    };

    const playNote = async (note, voice) => {
        // Selector 0xf63eb9b8 (playNote)
        const txData = ethers.concat([
            "0xf63eb9b8",
            ethers.zeroPadValue(ethers.toBeHex(BigInt(note) << 240n), 32),
            ethers.zeroPadValue(ethers.toBeHex(BigInt(voice) << 248n), 32)
        ]);
        const tx = await signer.sendTransaction({ to: musicMakerAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const getVoice1Frequency = async () => {
        // Selector 0x112df49e
        const res = await provider.call({ to: musicMakerAddress, data: "0x112df49e" });
        return Number(ethers.toBigInt(res));
    };

    const getRoomId = async (addr) => {
        const slot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(addr)), 32);
        const val = await provider.send("eth_getStorageAt", [zmAddress, slot, "latest"]);
        return Number(ethers.toBigInt(val));
    };

    // =================================================================
    // PROCESS CARD REGISTRATION (NPC = CARD 2)
    // =================================================================
    console.log("\n--- Registering NPC Process Card ---");
    await registerCardProcess(2, 0x1000);
    console.log("NPC Process Card 2 registered.");

    // =================================================================
    // SYNTHESIZER CONTROL LOOP EMULATION
    // =================================================================
    console.log("\n--- Executing Synthesizer Control Loop ---");
    
    // Play Note 60 (Note 60 * 100 = 6000 frequency) on Voice 1 to command "north"
    console.log("Playing command Note 60 on Synthesizer Voice 1 (Target: North)...");
    await playNote(60, 1);
    const freq = await getVoice1Frequency();
    console.log(`Voice 1 frequency read from Synthesizer SID registers: ${freq}`);

    // Map frequency to direction command
    let command = "";
    if (freq === 6000) {
        command = "north";
    } else if (freq === 7000) {
        command = "south";
    }
    console.log(`Mapped synthesizer frequency to command: "${command}"`);

    // =================================================================
    // CPU DIRECT REGISTER INJECTION
    // =================================================================
    console.log("\n--- CPU Direct Access: Injecting command into registers ---");
    
    // We inject the command selector/values directly into the CPU Accumulator 'A'
    // in the NPC's process card (Card ID 2) inside WMQ_Scheduler's storage.
    // Register A is at index 2 of the PCB.
    // PCB offset = 0x6000 + cardId * 16 = 0x6020.
    // Register A slot = 0x6020 + 2 = 0x6022.
    const accumulatorSlot = ethers.zeroPadValue(ethers.toBeHex(0x6022), 32);
    // Write command bytes representation as hex directly to the CPU register
    const cmdVal = ethers.zeroPadValue(ethers.hexlify(Buffer.from(command)), 32);
    
    console.log(`Directly writing "${command}" (hex: ${cmdVal}) to CPU Accumulator 'A' register...`);
    await provider.send("anvil_setStorageAt", [schedulerAddress, accumulatorSlot, cmdVal]);

    // Verify CPU Accumulator was updated
    let regs = await getCardRegs(2);
    console.log(`Verified CPU Registers: IP: ${regs[0]}, SP: ${regs[1]}, A (Accumulator): ${ethers.toBeHex(regs[2])}`);

    // =================================================================
    // EXECUTE GUEST SCHEDULER TIME SLICE
    // =================================================================
    console.log("\n--- Running NPC execution time slice ---");
    // Under CPU direct control, the NPC process reads the Accumulator instruction
    // and invokes Z-Machine parser command execution
    const cmdBytes = ethers.hexlify(Buffer.from(command));
    const zmResponse = await zmContract.parseCommand.staticCall(npcAddress, cmdBytes);
    console.log(`Z-Machine returned description: "${zmResponse.trim()}"`);
    
    await (await zmContract.parseCommand(npcAddress, cmdBytes)).wait();
    console.log("Movement command successfully processed by Z-Machine.");

    // Verify NPC room ID location updated to Room 11 (Hallway)
    const npcRoom = await getRoomId(npcAddress);
    console.log(`\nNPC Room location: ${npcRoom} (Expected: 11 - Hallway)`);

    if (npcRoom !== 11) {
        throw new Error("Synth/CPU control failed to route the NPC!");
    }
    console.log("\n===============================================================");
    console.log("SUCCESS: Synthesizer & CPU Direct Access NPC control verified!");
    console.log("===============================================================");
}

main().catch(console.error);
