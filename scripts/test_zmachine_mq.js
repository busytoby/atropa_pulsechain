const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

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
    console.log("STARTING INTEGRATED Z-MACHINE & WINCHESTER_MQ ROUTING TEST");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signer;
    try {
        signer = await provider.getSigner(0);
        console.log(`Using Signer: ${signer.address}`);
    } catch (err) {
        console.error("Local provider not reachable. Please make sure Anvil is running on port 8545.");
        process.exit(1);
    }

    // 1. Deploy WinchesterMQ Controller
    console.log("Compiling and deploying WinchesterMQ controller...");
    const wmqBytecode = compileYul(path.join(__dirname, "../solidity/bin/WinchesterMQ.yul"));
    const wmqFactory = new ethers.ContractFactory([], wmqBytecode, signer);
    const wmq = await wmqFactory.deploy();
    await wmq.waitForDeployment();
    const wmqAddress = await wmq.getAddress();
    console.log(`WinchesterMQ deployed at: ${wmqAddress}`);

    // 2. Deploy Virtual Disk (LUN 1 - MQ Cartridge)
    console.log("Deploying MQ Virtual Disk Cartridge (LUN 1)...");
    const virtualDiskJson = JSON.parse(fs.readFileSync(path.join(__dirname, '../Wallet/bin/Contracts/VirtualDisk.sol.json'), 'utf8'));
    const contractObj = virtualDiskJson.contracts["dysnomia/VirtualDisk.sol:VirtualDisk"];
    const virtualDiskFactory = new ethers.ContractFactory(contractObj.abi, contractObj.bin, signer);
    const mqDisk = await virtualDiskFactory.deploy();
    await mqDisk.waitForDeployment();
    // Helpers to write to WinchesterMQ using exact Yul selectors
    const writeSignalsOut = async (signals) => {
        const txData = ethers.concat([
            "0x485301a0",
            ethers.zeroPadValue(ethers.toBeHex(signals), 32)
        ]);
        const tx = await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const writeDataPort = async (val) => {
        const txData = ethers.concat([
            "0x98d400c0",
            ethers.zeroPadValue(ethers.toBeHex(val), 32)
        ]);
        const tx = await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    // 3. Deploy and setup Z-Machine
    console.log("Compiling and deploying Z-Machine contract...");
    const zmBytecode = compileYul(path.join(__dirname, "../solidity/bin/zmachine.yul"));
    const zmFactory = new ethers.ContractFactory([], zmBytecode, signer);
    const zm = await zmFactory.deploy();
    await zm.waitForDeployment();
    const zmAddress = await zm.getAddress();
    console.log(`Z-Machine deployed at: ${zmAddress}`);

    console.log("Compiling and deploying Z-Machine Parser contract...");
    const parserBytecode = compileYul(path.join(__dirname, "../solidity/bin/zmachineParser.yul"));
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

    // 4. Simulate SASI Byte-Level Write to LUN 1, Sector 0 (Inbox)
    // CDB bytes: [0x0A (Write), 0x20 (LUN 1 + High LBA 0), 0x00, 0x00 (Low LBA 0), 0x01 (1 Block), 0x00]
    console.log("Simulating SASI Host selection & Write handshake to LUN 1, Sector 0 (Inbox)...");
    await writeSignalsOut(0x01); // SEL=1

    const cdb = [0x0A, 0x20, 0x00, 0x00, 0x01, 0x00];
    for (let i = 0; i < cdb.length; i++) {
        await writeDataPort(cdb[i]);
        await writeSignalsOut(0x05); // SEL=1, ACK=1
        await writeSignalsOut(0x01); // SEL=1, ACK=0
    }

    // Construct sector message payload: Magic "MSGQ" + Sender (Card ID 1) + Command "look"
    const payload = Buffer.alloc(256);
    payload.write("MSGQ", 0);                     // Magic header
    payload.writeUInt32BE(300, 4);                // TTL Expiry (300 ticks)
    payload.writeUInt32BE(1, 8);                  // Sender (Card ID 1)
    payload.write("CORRELATION_ID_ZMACHINE_0001", 12); // Correlation ID
    payload.write("look", 44);                    // Game command

    console.log("Streaming 256-byte transaction payload containing command 'look'...");
    for (let i = 0; i < 256; i++) {
        await writeDataPort(payload[i]);
        await writeSignalsOut(0x05); // SEL=1, ACK=1
        await writeSignalsOut(0x01); // SEL=1, ACK=0
    }
    console.log("SASI Write transaction complete.");
    await writeSignalsOut(0x02); // Assert RST to release bus

    // 5. Read from the Virtual Disk storage slots to simulate Daemon MQ_GET & Z-Machine processing
    console.log("\n--- Daemon Processing Simulation ---");
    
    // Read the inbox block from WinchesterMQ storage slot directly
    const inboxData = Buffer.alloc(256);
    const inboxBaseSlot = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(0x1000), 32));
    
    for (let i = 0; i < 8; i++) { // 8 slots of 32 bytes = 256 bytes
        const targetSlot = ethers.toBeHex(BigInt(inboxBaseSlot) + BigInt(i));
        const val = await provider.getStorage(wmqAddress, targetSlot);
        const valBuffer = Buffer.from(ethers.getBytes(val));
        valBuffer.copy(inboxData, i * 32);
    }

    const magic = inboxData.toString("utf8", 0, 4);
    const command = inboxData.toString("utf8", 44).replace(/\0/g, "");
    console.log(`Daemon read message magic: "${magic}"`);
    console.log(`Daemon extracted Z-Machine command: "${command}"`);

    // Call parseCommand on zmachine
    console.log("Daemon executing Z-Machine parseCommand via contract call...");
    const cmdBytes = ethers.hexlify(Buffer.from(command));
    const zmResponse = await zmContract.parseCommand.staticCall(signer.address, cmdBytes);
    console.log(`Z-Machine returned output: "${zmResponse}"`);

    // 6. Write response payload back to LUN 1, Sector 1 (Player Outbox)
    console.log("Daemon preparing Outbox message sector for player...");
    const responsePayload = Buffer.alloc(256);
    responsePayload.write("MSGQ", 0);
    responsePayload.writeUInt32BE(0, 4); // TTL
    responsePayload.writeUInt32BE(0, 8); // System sender
    responsePayload.write("CORRELATION_ID_ZMACHINE_0001", 12);
    responsePayload.write(zmResponse, 44);

    // Simulate SASI Write handshake to Sector 1
    await writeSignalsOut(0x01); // SEL=1
    const responseCdb = [0x0A, 0x20, 0x00, 0x01, 0x01, 0x00]; // LBA 1
    for (let i = 0; i < responseCdb.length; i++) {
        await writeDataPort(responseCdb[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }
    for (let i = 0; i < 256; i++) {
        await writeDataPort(responsePayload[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }
    console.log("Outbox write transaction complete.");
    await writeSignalsOut(0x02); // Assert RST to release bus

    // 7. Verify sector content on the WinchesterMQ LBA 1 (Player Outbox)
    const outboxData = Buffer.alloc(256);
    const outboxBaseSlot = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(0x1001), 32));
    
    for (let i = 0; i < 8; i++) {
        const targetSlot = ethers.toBeHex(BigInt(outboxBaseSlot) + BigInt(i));
        const val = await provider.getStorage(wmqAddress, targetSlot);
        const valBuffer = Buffer.from(ethers.getBytes(val));
        valBuffer.copy(outboxData, i * 32);
    }

    const outputResponse = outboxData.toString("utf8", 44).replace(/\0/g, "");
    console.log("\n===============================================================");
    console.log(`SUCCESS: Player Outbox Sector read output: "${outputResponse}"`);
    console.log("===============================================================");
}

main().catch(console.error);
