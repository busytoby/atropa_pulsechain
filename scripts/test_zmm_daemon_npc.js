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
    console.log("STARTING AUTONOMOUS DAEMON NPC SCHEDULER TEST");
    console.log("===============================================================");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signer = await provider.getSigner(0);

    // 1. Deploy WMQ_Scheduler
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
            regs.push(ethers.toBigInt(bytes.slice(i * 32, (i + 1) * 32)));
        }
        return regs;
    };

    const getRoomId = async (cardId) => {
        const slot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(cardId)), 32);
        const val = await provider.send("eth_getStorageAt", [schedulerAddress, slot, "latest"]);
        return Number(ethers.toBigInt(val));
    };

    // 2. Setup game map exits in Scheduler storage directly
    // Room 10: Lobby. Exits: North -> 11, South -> 12. Val: 0x0B0C0000
    const exitsRoom10Slot = ethers.zeroPadValue(ethers.toBeHex(3200000 + 10), 32);
    const exitsRoom10Value = ethers.zeroPadValue("0x0B0C0000", 32);
    
    // Room 11: Hallway 1. Exits: South -> 10. Val: 0x000A0000
    const exitsRoom11Slot = ethers.zeroPadValue(ethers.toBeHex(3200000 + 11), 32);
    const exitsRoom11Value = ethers.zeroPadValue("0x000A0000", 32);

    // Room 12: Hallway 2. Exits: North -> 10. Val: 0x0A000000
    const exitsRoom12Slot = ethers.zeroPadValue(ethers.toBeHex(3200000 + 12), 32);
    const exitsRoom12Value = ethers.zeroPadValue("0x0A000000", 32);

    console.log("Configuring room layout exits in Scheduler storage...");
    await provider.send("anvil_setStorageAt", [schedulerAddress, exitsRoom10Slot, exitsRoom10Value]);
    await provider.send("anvil_setStorageAt", [schedulerAddress, exitsRoom11Slot, exitsRoom11Value]);
    await provider.send("anvil_setStorageAt", [schedulerAddress, exitsRoom12Slot, exitsRoom12Value]);

    // 3. Register Daemon NPC Process Card ID 2
    console.log("\nRegistering Daemon NPC Process (Card ID 2) at entry point 0x1000...");
    await registerCardProcess(2, 0x1000);

    // Set NPC's initial room location in Scheduler storage to Room 10
    const npcRoomSlot = ethers.zeroPadValue(ethers.toBeHex(BigInt(4000000) + BigInt(2)), 32);
    await provider.send("anvil_setStorageAt", [schedulerAddress, npcRoomSlot, ethers.zeroPadValue(ethers.toBeHex(10), 32)]);
    console.log("NPC placed in Room 10.");

    // Verify initial values
    console.log(`Initial Room ID: ${await getRoomId(2)}`);
    let regs = await getCardRegs(2);
    console.log(`Initial Register X (stepCount): ${regs[3].toString()}`);

    // 4. Run CPU execution slices and observe state updates
    console.log("\n--- Dispatching CPU Slices for Autonomous Movement ---");
    const expectedRoute = [11, 10, 11, 10, 11]; // Predicted cyclic walking path based on X-reg mod choice

    for (let i = 0; i < 5; i++) {
        console.log(`\nRunning slice ${i + 1} (mock step: 50)...`);
        await runNextSlice(50);
        
        const currentRoom = await getRoomId(2);
        regs = await getCardRegs(2);
        const stepCount = regs[3].toString();
        
        console.log(`NPC Location after slice ${i + 1}: Room ${currentRoom} (Expected: ${expectedRoute[i]})`);
        console.log(`NPC Register X (stepCount):    ${stepCount}`);

        if (currentRoom !== expectedRoute[i]) {
            throw new Error(`State mismatch! NPC moved to Room ${currentRoom} but expected ${expectedRoute[i]}`);
        }
    }

    console.log("\n===============================================================");
    console.log("SUCCESS: ZMM Scheduler Autonomous NPC Daemon Walking verified!");
    console.log("===============================================================");
}

main().catch(console.error);
