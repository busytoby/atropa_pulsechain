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
    console.log("STARTING MULTI-USER CONCURRENT HANDSHAKE TEST ON WINCHESTER_MQ");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signer1, signer2;
    try {
        signer1 = await provider.getSigner(0);
        signer2 = await provider.getSigner(1);
        console.log(`Player 1 Wallet: ${signer1.address}`);
        console.log(`Player 2 Wallet: ${signer2.address}`);
    } catch (err) {
        console.error("Local provider not reachable. Please make sure Anvil is running on port 8545.");
        process.exit(1);
    }

    // 1. Deploy WinchesterMQ Controller
    console.log("Compiling and deploying WinchesterMQ...");
    const wmqBytecode = compileYul(path.join(__dirname, "../solidity/bin/WinchesterMQ.yul"));
    const wmqFactory = new ethers.ContractFactory([], wmqBytecode, signer1);
    const wmq = await wmqFactory.deploy();
    await wmq.waitForDeployment();
    const wmqAddress = await wmq.getAddress();
    console.log(`WinchesterMQ deployed at: ${wmqAddress}`);

    // Helper functions for sending signal and data transactions
    const writeSignalsOut = async (signer, signals) => {
        const txData = ethers.concat([
            "0x485301a0",
            ethers.zeroPadValue(ethers.toBeHex(signals), 32)
        ]);
        const tx = await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const writeDataPort = async (signer, val) => {
        const txData = ethers.concat([
            "0x98d400c0",
            ethers.zeroPadValue(ethers.toBeHex(val), 32)
        ]);
        const tx = await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const readSignalsIn = async (signer) => {
        const txData = "0x525302b0";
        const result = await provider.call({
            to: wmqAddress,
            from: signer.address,
            data: txData
        });
        return Number(ethers.toBigInt(result));
    };

    // 2. Simulate Concurrent/Interleaved Handshakes
    // In a globally shared state system, interleaving these calls would cause CDB bytes
    // and bus phase transitions to collide, corrupting both operations.
    // With virtualization, each signer manages its own independent bus state.

    console.log("\n--- Initiating Interleaved SASI Selection Handshakes ---");
    
    console.log("Player 1 asserts SEL = 1...");
    await writeSignalsOut(signer1, 0x01);
    
    console.log("Player 2 asserts SEL = 1...");
    await writeSignalsOut(signer2, 0x01);

    // Verify both have BSY = 1, C/D = 1, REQ = 1 in their isolated views
    const sigs1 = await readSignalsIn(signer1);
    const sigs2 = await readSignalsIn(signer2);
    console.log(`Player 1 Bus Signals: 0x${sigs1.toString(16)} (Expected BSY | C/D | REQ = 0x07)`);
    console.log(`Player 2 Bus Signals: 0x${sigs2.toString(16)} (Expected BSY | C/D | REQ = 0x07)`);

    if (sigs1 !== 0x07 || sigs2 !== 0x07) {
        throw new Error("Initial bus state incorrect or corrupted by concurrent selection!");
    }

    // CDB: Write Block, LUN 1, LBA 0 (Inbox), 1 Block, 0 Control
    const cdb1 = [0x0A, 0x20, 0x00, 0x00, 0x01, 0x00];
    // CDB: Write Block, LUN 1, LBA 2 (Custom LBA for Player 2), 1 Block, 0 Control
    const cdb2 = [0x0A, 0x20, 0x00, 0x02, 0x01, 0x00];

    console.log("\n--- Interleaving CDB command byte transfers ---");
    for (let i = 0; i < 6; i++) {
        // Send byte for Player 1
        await writeDataPort(signer1, cdb1[i]);
        await writeSignalsOut(signer1, 0x05); // SEL=1, ACK=1
        await writeSignalsOut(signer1, 0x01); // SEL=1, ACK=0

        // Send byte for Player 2
        await writeDataPort(signer2, cdb2[i]);
        await writeSignalsOut(signer2, 0x05); // SEL=1, ACK=1
        await writeSignalsOut(signer2, 0x01); // SEL=1, ACK=0
    }
    console.log("CDB transfers complete for both players.");

    // Construct different sector payloads
    const payload1 = Buffer.alloc(256);
    payload1.write("MSGQ", 0);
    payload1.writeUInt32BE(300, 4);
    payload1.writeUInt32BE(1, 8); // Sender Card 1
    payload1.write("PLAYER_1_MESSAGE_CONTENT_LOOK", 44);

    const payload2 = Buffer.alloc(256);
    payload2.write("MSGQ", 0);
    payload2.writeUInt32BE(300, 4);
    payload2.writeUInt32BE(2, 8); // Sender Card 2
    payload2.write("PLAYER_2_MESSAGE_CONTENT_LOOK", 44);

    console.log("\n--- Interleaving 256-byte sector payload streaming ---");
    for (let i = 0; i < 256; i++) {
        // Stream 1 byte for Player 1
        await writeDataPort(signer1, payload1[i]);
        await writeSignalsOut(signer1, 0x05); // ACK=1
        await writeSignalsOut(signer1, 0x01); // ACK=0

        // Stream 1 byte for Player 2
        await writeDataPort(signer2, payload2[i]);
        await writeSignalsOut(signer2, 0x05); // ACK=1
        await writeSignalsOut(signer2, 0x01); // ACK=0
    }
    console.log("Sector data streams complete.");

    // Assert resets to release bus per caller
    await writeSignalsOut(signer1, 0x02);
    await writeSignalsOut(signer2, 0x02);

    // 3. Verify that both sectors were written successfully to their respective LBAs in storage
    console.log("\n--- Verification of Written Storage Sectors ---");
    
    // Player 1 wrote to LBA 0 -> hashKey(0x1000, 0)
    const baseSlot1 = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(0x1000), 32));
    const sector1Data = Buffer.alloc(256);
    for (let i = 0; i < 8; i++) {
        const slot = ethers.toBeHex(BigInt(baseSlot1) + BigInt(i));
        const val = await provider.getStorage(wmqAddress, slot);
        Buffer.from(ethers.getBytes(val)).copy(sector1Data, i * 32);
    }
    const msg1 = sector1Data.toString("utf8", 44).replace(/\0/g, "");
    console.log(`LBA 0 Sector Content: "${msg1}"`);

    // Player 2 wrote to LBA 2 -> hashKey(0x1000, 2)
    const baseSlot2 = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(0x1002), 32));
    const sector2Data = Buffer.alloc(256);
    for (let i = 0; i < 8; i++) {
        const slot = ethers.toBeHex(BigInt(baseSlot2) + BigInt(i));
        const val = await provider.getStorage(wmqAddress, slot);
        Buffer.from(ethers.getBytes(val)).copy(sector2Data, i * 32);
    }
    const msg2 = sector2Data.toString("utf8", 44).replace(/\0/g, "");
    console.log(`LBA 2 Sector Content: "${msg2}"`);

    if (msg1 === "PLAYER_1_MESSAGE_CONTENT_LOOK" && msg2 === "PLAYER_2_MESSAGE_CONTENT_LOOK") {
        console.log("\n===============================================================");
        console.log("SUCCESS: CONCURRENT HANDSHAKES ISOLATED SUCCESSFULLY BY CALLER");
        console.log("===============================================================");
    } else {
        throw new Error("State corruption occurred! Messages were not isolated correctly.");
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
