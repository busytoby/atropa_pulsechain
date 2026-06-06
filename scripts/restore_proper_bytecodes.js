const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const PKMINTER_ABI = [
    "function New(string memory Name, string memory Symbol, uint8 Complexity, address[] memory Signers, bytes32 Nonce, bytes[] memory Signatures) public returns (address)"
];

async function main() {
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const pkMinterAddress = config.networks.localhost.pkminterAddress;
    const pkMinter = new ethers.Contract(pkMinterAddress, PKMINTER_ABI, deployer);

    const tokensToRestore = [
        { name: "Genesis PK3", address: "0x5fD11B3E10E0d71Ab05ace92797e517e532E1462", keys: config.saved_keys["0x5fD11B3E10E0d71Ab05ace92797e517e532E1462"] },
        { name: "Genesis PK4", address: "0xb4C9Ed909ecB896586d6ebCfEbA0E97fdCA51310", keys: config.saved_keys["0xb4C9Ed909ecB896586d6ebCfEbA0E97fdCA51310"] },
        { name: "Genesis PK9", address: "0xe40AF675753322508d033CE8EE6135aA76470338", keys: config.saved_keys["0xe40AF675753322508d033CE8EE6135aA76470338"] }
    ];

    for (const token of tokensToRestore) {
        console.log(`\nGenerating proper bytecode for ${token.name} (Complexity: ${token.keys.length})...`);

        const complexity = token.keys.length;
        const generatedAddresses = [];
        const signatures = [];
        const nonce = ethers.hexlify(ethers.randomBytes(32));

        for (let i = 0; i < complexity; i++) {
            const wallet = new ethers.Wallet(token.keys[i]);
            generatedAddresses.push(wallet.address);
            
            const sig = wallet.signingKey.sign(nonce);
            const flatSig = ethers.concat([sig.r, sig.s, ethers.toBeHex(sig.v)]);
            signatures.push(flatSig);
        }

        console.log("Deploying temporary contract to extract bytecode...");
        const tx = await pkMinter.New(
            token.name,
            "GPK",
            complexity,
            generatedAddresses,
            nonce,
            signatures
        );
        const receipt = await tx.wait();

        // Find the deployed contract address from logs
        let deployedAddress = null;
        for (const log of receipt.logs) {
            if (log.address && log.address.toLowerCase() !== pkMinterAddress.toLowerCase()) {
                deployedAddress = log.address;
                break;
            }
        }

        if (!deployedAddress) {
            console.error(`❌ Failed to find deployed address for ${token.name}`);
            continue;
        }

        console.log(`Deployed temp contract at: ${deployedAddress}`);

        // Fetch runtime bytecode
        const bytecode = await provider.getCode(deployedAddress);
        console.log(`Bytecode length: ${bytecode.length} hex chars`);

        // Inject bytecode into target address
        console.log(`Injecting bytecode into target: ${token.address}...`);
        await provider.send("anvil_setCode", [token.address, bytecode]);
        console.log(`✅ ${token.name} bytecode successfully restored!`);
    }

    console.log("\nAll bytecodes restored successfully. Run verify_pk_nonces.js to validate!");
}

main().catch(console.error);
