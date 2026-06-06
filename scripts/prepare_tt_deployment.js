const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");
const PROVIDER_URL = "https://rpc.pulsechain.com";
const PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B";

const PKMINTER_ABI = [
    "function New(string memory Name, string memory Symbol, uint8 Complexity, address[] memory Signers, bytes32 Nonce, bytes[] memory Signatures) public returns (address)"
];

async function main() {
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found at:", CONFIG_PATH);
        process.exit(1);
    }

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const keys = config.default.keys;
    if (!keys || keys.length !== 8) {
        console.error("Default config does not contain exactly 8 keys.");
        process.exit(1);
    }

    console.log("Deriving signer addresses from the 8 private keys...");
    const wallets = [];
    const signers = [];
    for (let i = 0; i < keys.length; i++) {
        const wallet = new ethers.Wallet(keys[i]);
        wallets.push(wallet);
        signers.push(wallet.address);
        console.log(` Signer [${i}]: ${wallet.address}`);
    }

    // Generate a random 32-byte nonce
    const nonce = ethers.randomBytes(32);
    const nonceHex = ethers.hexlify(nonce);
    console.log(`\nGenerated Nonce: ${nonceHex}`);

    console.log("\nSigning nonce with the 8 private keys...");
    const signatures = [];
    for (let i = 0; i < wallets.length; i++) {
        // Sign the raw nonce hash directly (no EIP-191 prefix)
        const sig = wallets[i].signingKey.sign(nonceHex);
        const flatSig = ethers.concat([sig.r, sig.s, ethers.toBeHex(sig.v)]);
        signatures.push(flatSig);
        console.log(` Signature [${i}]: ${ethers.hexlify(flatSig).substring(0, 30)}...`);
    }

    // Prepare contract interface
    const iface = new ethers.Interface(PKMINTER_ABI);
    const name = "PKI Token 8";
    const symbol = "PKI8";
    const complexity = 8;

    const calldata = iface.encodeFunctionData("New", [
        name,
        symbol,
        complexity,
        signers,
        nonceHex,
        signatures
    ]);

    console.log("\n==================================================");
    console.log("TRANSACTION DETAILS TO DEPLOY NEW TT TOKEN:");
    console.log("==================================================");
    console.log(`To (PKMinter Address): ${PKMINTER_ADDRESS}`);
    console.log(`Data (Calldata):       ${calldata}`);
    console.log("==================================================");

    // If PRIVATE_KEY environment variable is provided, send the transaction
    const userPrivateKey = process.env.PRIVATE_KEY;
    if (userPrivateKey) {
        console.log("\nPrivate key provided in environment. Broadcasting transaction...");
        const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
        const wallet = new ethers.Wallet(userPrivateKey, provider);
        console.log(`Sending from address: ${wallet.address}`);

        try {
            const tx = await wallet.sendTransaction({
                to: PKMINTER_ADDRESS,
                data: calldata,
                gasLimit: 3000000
            });
            console.log(`Transaction broadcasted. Hash: ${tx.hash}`);
            console.log("Waiting for confirmation...");
            const receipt = await tx.wait();
            console.log(`Transaction confirmed in block ${receipt.blockNumber}! Status: ${receipt.status === 1 ? "SUCCESS" : "FAILED"}`);
        } catch (err) {
            console.error("Transaction failed:", err.message);
        }
    } else {
        console.log("\nTo deploy this contract, send a transaction to the 'To' address with the 'Data' payload above using your wallet.");
    }
}

main().catch(console.error);
