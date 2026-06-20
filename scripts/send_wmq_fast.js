const { ethers } = require('ethers');

async function main() {
    const command = process.argv[2];
    const contractAddress = process.argv[3];
    
    if (!command || !contractAddress) {
        console.log("Usage: node scripts/send_wmq_fast.js <command> <wmq_contract_address>");
        process.exit(1);
    }

    console.log(`=== Auncient WMQ Fast Sender ===`);
    console.log(`Command: "${command}"`);
    console.log(`Target Contract: ${contractAddress}`);

    const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
    const signer = await provider.getSigner(0);

    const rawBytes = ethers.toUtf8Bytes(command);
    const paddedBytes = new Uint8Array(32);
    paddedBytes.set(rawBytes.slice(0, 32));

    const txData = ethers.concat([
        "0xccb077a0",
        ethers.hexlify(paddedBytes)
    ]);

    console.log("Sending manual transaction with data:", txData);
    const tx = await signer.sendTransaction({
        to: contractAddress,
        data: txData,
        gasLimit: 1000000
    });
    await tx.wait();
    console.log("[WMQ] Fast event sent successfully.");
}

main().catch(err => {
    console.error("Fatal Error:", err);
});
