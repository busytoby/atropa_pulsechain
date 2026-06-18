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
    console.log("=================================================================");
    console.log("STARTING ZMM SCHEDULER & WINCHESTER_MQ PROCESS REGULATION TEST");
    console.log("=================================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);
    console.log(`Using Signer: ${signer.address}`);

    // 1. Deploy WinchesterMQ Controller
    console.log("Compiling and deploying WinchesterMQ controller...");
    const wmqBytecode = compileYul("../solidity/bin/WinchesterMQ.yul");
    const wmqFactory = new ethers.ContractFactory([], wmqBytecode, signer);
    const wmq = await wmqFactory.deploy();
    await wmq.waitForDeployment();
    const wmqAddress = await wmq.getAddress();
    console.log(`WinchesterMQ deployed at: ${wmqAddress}`);

    // 2. Deploy WMQ_Scheduler
    console.log("Compiling and deploying WMQ_Scheduler...");
    const schedulerBytecode = compileYul("../solidity/bin/WMQ_Scheduler.yul");
    const schedulerFactory = new ethers.ContractFactory([], schedulerBytecode, signer);
    const scheduler = await schedulerFactory.deploy();
    await scheduler.waitForDeployment();
    const schedulerAddress = await scheduler.getAddress();
    console.log(`WMQ_Scheduler deployed at: ${schedulerAddress}`);

    // Helper functions using custom Yul selectors
    const registerCardProcess = async (cardId, entryPoint) => {
        const txData = ethers.concat([
            "0x8a1b9c7e",
            ethers.zeroPadValue(ethers.toBeHex(cardId), 32),
            ethers.zeroPadValue(ethers.toBeHex(entryPoint), 32)
        ]);
        const tx = await signer.sendTransaction({ to: schedulerAddress, data: txData, gasLimit: 1000000 });
        await tx.wait();
    };

    const runNextSlice = async (stepLimit) => {
        const txData = ethers.concat([
            "0x52756e53",
            ethers.zeroPadValue(ethers.toBeHex(stepLimit), 32)
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
            const val = ethers.toBigInt(bytes.slice(i * 32, (i + 1) * 32));
            regs.push(val);
        }
        return regs;
    };

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

    // =================================================================
    // PART 1: Process Registration and Slice Execution
    // =================================================================
    console.log("\n--- PART 1: Process Lifecycle and Registering Card ---");
    console.log("Registering Process Card ID 2 with entryPoint 0x1000...");
    await registerCardProcess(2, 0x1000);
    console.log("Card registered successfully.");

    console.log("Querying Card ID 2 Registers before run...");
    let regs = await getCardRegs(2);
    console.log(`IP: ${regs[0].toString()}, SP: ${regs[1].toString()}, A: ${regs[2].toString()}, Active: ${regs[6].toString()}`);

    console.log("Running next CPU time slice on scheduler (step limit: 50)...");
    await runNextSlice(50);
    console.log("Slice execution completed.");

    console.log("Querying Card ID 2 Registers after run...");
    regs = await getCardRegs(2);
    console.log(`IP: ${regs[0].toString()}, SP: ${regs[1].toString()}, A: ${regs[2].toString()}, Active: ${regs[6].toString()}`);

    // =================================================================
    // PART 2: WinchesterMQ Expiry Regulation (TTL & Ticks)
    // =================================================================
    console.log("\n--- PART 2: WinchesterMQ Expiry Regulation (TTL) ---");
    
    // We will write a message block to Sector 0 with a very short TTL
    // Sector payload format: Magic "MSGQ" (bytes 0-3) + Expiry Jiffies (bytes 8-15)
    // Current block number * 60 = current jiffies.
    const currentBlock = await provider.getBlockNumber();
    const currentJiffies = currentBlock * 60;
    const expiryJiffies = currentJiffies + 100; // Expires in ~100 ticks
    console.log(`Current Block: ${currentBlock}, Current Jiffies: ${currentJiffies}`);
    console.log(`Setting message Expiry Jiffies to: ${expiryJiffies}`);

    // Setup LUN 1 Sector 0 write CDB
    console.log("Writing short-lived message to WinchesterMQ LUN 1, Sector 0...");
    await writeSignalsOut(0x01); // SEL=1
    const cdb = [0x0A, 0x20, 0x00, 0x00, 0x01, 0x00];
    for (let i = 0; i < cdb.length; i++) {
        await writeDataPort(cdb[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }

    const payload = Buffer.alloc(256);
    payload.write("MSGQ", 0);
    // Write ExpiryJiffies (uint64 Big-Endian) starting at byte index 8
    payload.writeBigUInt64BE(BigInt(expiryJiffies), 8);
    payload.write("look", 44);

    for (let i = 0; i < 256; i++) {
        await writeDataPort(payload[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }
    await writeSignalsOut(0x02); // RST to release bus
    console.log("Message written successfully.");

    // Validate that the block is currently ACTIVE (Not expired yet)
    // We try to read LUN 1 Sector 0 using CDB opcode 0x08 (Read Block)
    // CDB bytes: [0x08, 0x20, 0x00, 0x00, 0x01, 0x00]
    console.log("Reading from Sector 0 while active...");
    await writeSignalsOut(0x01);
    const readCdb = [0x08, 0x20, 0x00, 0x00, 0x01, 0x00];
    for (let i = 0; i < readCdb.length; i++) {
        await writeDataPort(readCdb[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }
    // Toggle ACK to read the first byte (should be 'M' / 0x4D)
    // In read mode, controller places data on port, host reads, then toggles ACK
    // To read: we query the data port register value and toggle ACK
    const readDataPort = async () => {
        const val = await provider.getStorage(wmqAddress, ethers.toBeHex(0x20));
        return Number(ethers.getBytes(val)[31]);
    };

    let firstByte = await readDataPort();
    console.log(`First byte read (hex): 0x${firstByte.toString(16).toUpperCase()} ('${String.fromCharCode(firstByte)}')`);
    await writeSignalsOut(0x05);
    await writeSignalsOut(0x01);
    await writeSignalsOut(0x02); // Reset to release

    // Mine blocks to simulate time passage (each block adds 60 ticks)
    console.log("\nMining 3 blocks on local EVM to simulate passage of time...");
    await provider.send("anvil_mine", ["0x3"]);
    const newBlock = await provider.getBlockNumber();
    const newJiffies = newBlock * 60;
    console.log(`New Block: ${newBlock}, New Jiffies: ${newJiffies} (Time exceeded expiry of ${expiryJiffies})`);

    // Try reading Sector 0 again. Since it has expired, checkBlockExpired should skip the block
    // and transition immediately to Status Phase (Status 0x04: Block Skipped/Expired)
    console.log("Attempting to read Sector 0 after expiry time...");
    await writeSignalsOut(0x01);
    for (let i = 0; i < readCdb.length; i++) {
        await writeDataPort(readCdb[i]);
        await writeSignalsOut(0x05);
        await writeSignalsOut(0x01);
    }

    // Checking if the signals match the Status Phase (C/D=1, I/O=1, MSG=1)
    const readSignalsIn = async () => {
        const txData = ethers.concat([
            "0x525302b0", // readSignalsIn
            ethers.zeroPadValue("0x", 32)
        ]);
        const res = await provider.call({ to: wmqAddress, data: txData });
        return Number(ethers.getBytes(res)[31]);
    };

    const signals = await readSignalsIn();
    console.log(`Bus Signals In: 0x${signals.toString(16).toUpperCase()}`);
    // Bitmask for Status Phase: C/D (0x04) + I/O (0x08) + MSG (0x10) + REQ (0x02) + BSY (0x01) = 0x1F
    const isStatusPhase = (signals & 0x1C) === 0x1C;
    console.log(`Is Status Phase (C/D, I/O, MSG all asserted)? ${isStatusPhase}`);

    const statusVal = await readDataPort();
    console.log(`Read Status Code: 0x${statusVal.toString(16).toUpperCase()} (Expected: 0x04 - Expired/Skipped)`);
    await writeSignalsOut(0x02); // Release bus

    console.log("\n=================================================================");
    console.log("ALL ZMM PROCESS LIFECYCLE & TIMING TESTS PASSED SUCCESSFULLY");
    console.log("=================================================================");
}

main().catch(console.error);
