const { ethers } = require('ethers');

async function main() {
    const command = process.argv[2];
    const contractAddress = process.argv[3];
    
    if (!command || !contractAddress) {
        console.log("Usage: node scripts/send_wmq_command.js <command> <wmq_contract_address>");
        process.exit(1);
    }

    console.log(`=== Auncient WMQ Sender ===`);
    console.log(`Command: "${command}"`);
    console.log(`Target Contract: ${contractAddress}`);

    const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
    const signer = await provider.getSigner(0);

    // Helpers to make transactions using WinchesterMQ custom selectors
    async function writeSignals(sigs) {
        // writeSignalsOut selector is 0x485301a0
        const txData = ethers.concat([
            "0x485301a0",
            ethers.zeroPadValue(ethers.toBeHex(sigs), 32)
        ]);
        const tx = await signer.sendTransaction({
            to: contractAddress,
            data: txData,
            gasLimit: 1000000 // 1 Million gas limit to prevent out-of-gas errors
        });
        await tx.wait();
    }

    async function writeData(val) {
        // writeDataPort selector is 0x98d400c0
        const txData = ethers.concat([
            "0x98d400c0",
            ethers.zeroPadValue(ethers.toBeHex(val), 32)
        ]);
        const tx = await signer.sendTransaction({
            to: contractAddress,
            data: txData,
            gasLimit: 1000000
        });
        await tx.wait();
    }

    async function readSignals() {
        // readSignalsIn selector is 0x525302b0
        const sigsVal = await provider.call({
            to: contractAddress,
            from: signer.address, // Match caller() to align transient state slots
            data: "0x525302b0"
        });
        return Number(ethers.toBigInt(sigsVal));
    }

    // Prepare 6-byte CDB for Write Block (Opcode 0x0A)
    // LUN 0, LBA 0 (target queue block index)
    const cdb = [0x0A, 0x00, 0x00, 0x00, 0x01, 0x00];

    // Prepare 256-byte payload buffer containing the command string
    const payload = Buffer.alloc(256);
    payload.write(command, 0, 'utf8');

    console.log("[WMQ] Initiating Select Phase (SEL = 1)...");
    await writeSignals(0x01); // SEL = 1, BSY = 0, RST = 0, ACK = 0

    // Send 6 CDB bytes
    for (let i = 0; i < 6; i++) {
        const sigs = await readSignals();
        // Verify C/D = 1, I/O = 0 (Command phase)
        if ((sigs & 0x04) === 0 || (sigs & 0x08) !== 0) {
            console.error(`[WMQ ERROR] Invalid signals state during CDB index ${i}: 0x${sigs.toString(16)}`);
            process.exit(1);
        }
        await writeData(cdb[i]);
        await writeSignals(0x05); // Assert ACK (SEL=1, BSY=1, ACK=1)
        await writeSignals(0x01); // Deassert ACK (ACK=0)
    }

    console.log("[WMQ] CDB phase complete. Writing 256-byte payload...");

    // Send 256 payload bytes
    for (let i = 0; i < 256; i++) {
        await writeData(payload[i]);
        await writeSignals(0x05); // Assert ACK
        await writeSignals(0x01); // Deassert ACK
    }

    console.log("[WMQ] Payload transfer complete. Reading Status...");
    
    // Read Status Phase
    const statusSigs = await readSignals();
    // Verify C/D = 1, I/O = 1 (Status phase)
    if ((statusSigs & 0x04) === 0 || (statusSigs & 0x08) === 0) {
        console.error(`[WMQ ERROR] Invalid status phase signals: 0x${statusSigs.toString(16)}`);
    }
    
    // Assert ACK to complete status phase
    await writeSignals(0x05);
    // Release bus (SEL=0, ACK=0)
    await writeSignals(0x00);

    console.log("[WMQ] Handshake finished. Event successfully queued on WinchesterMQ.");
}

main().catch(err => {
    console.error("Fatal Error:", err);
});
