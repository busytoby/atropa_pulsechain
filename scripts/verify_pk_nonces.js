const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Minimal ABI for validation testing
const TT_ABI = [
    "function Complexity() public view returns (uint8)",
    "function PublicKeys(uint256) public view returns (address)",
    "function ValidateSignatures(bytes32 nonce, bytes[] memory signatures) public returns (bool)",
    "function balanceOf(address) public view returns (uint256)"
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

    // Find all localhost keys that match "Genesis PK..."
    const localhostContracts = config.networks.localhost;
    const savedKeysMap = config.saved_keys || {};

    const pkTokens = [];
    for (const [name, address] of Object.entries(localhostContracts)) {
        if (name.toLowerCase().startsWith("genesis pk") && savedKeysMap[address]) {
            pkTokens.push({ name, address, keys: savedKeysMap[address] });
        }
    }

    if (pkTokens.length === 0) {
        console.log("No Genesis PK tokens found in config.");
        return;
    }

    console.log(`Found ${pkTokens.length} PK tokens to verify on local EVM.\n`);

    for (const token of pkTokens) {
        console.log(`==================================================`);
        console.log(`VERIFYING TOKEN: ${token.name} (${token.address})`);
        console.log(`==================================================`);

        const contract = new ethers.Contract(token.address, TT_ABI, deployer);

        try {
            const complexity = await contract.Complexity();
            console.log(`Contract Complexity: ${complexity}`);
            console.log(`Configured Keys Count: ${token.keys.length}`);

            // Fetch stored public keys in the contract to double check alignment
            const contractPublicKeys = [];
            for (let i = 0; i < complexity; i++) {
                contractPublicKeys.push(await contract.PublicKeys(i));
            }

            // Verify they match our configured key addresses
            let keyMatch = true;
            for (let i = 0; i < complexity; i++) {
                const configWallet = new ethers.Wallet(token.keys[i]);
                if (configWallet.address.toLowerCase() !== contractPublicKeys[i].toLowerCase()) {
                    console.error(`❌ Mismatch at Key #${i}! Config: ${configWallet.address}, Contract: ${contractPublicKeys[i]}`);
                    keyMatch = false;
                }
            }

            if (!keyMatch) {
                console.log("Skipping verification due to key mismatch.");
                continue;
            }
            console.log("✅ Configured keys match contract public keys.");

            // 1. Generate new nonce
            const newNonce = ethers.hexlify(ethers.randomBytes(32));
            console.log(`Generated testing Nonce: ${newNonce}`);

            // 2. Sign nonce with all keys
            const signatures = [];
            for (let i = 0; i < complexity; i++) {
                const wallet = new ethers.Wallet(token.keys[i]);
                const sig = wallet.signingKey.sign(newNonce);
                const flatSig = ethers.concat([sig.r, sig.s, ethers.toBeHex(sig.v)]);
                signatures.push(flatSig);
            }
            console.log(`Generated all signatures.`);

            // Get balance before
            const balBefore = await contract.balanceOf(deployer.address);
            console.log(`Balance before: ${ethers.formatUnits(balBefore, 18)}`);

            // 3. Call ValidateSignatures
            console.log("Broadcasting ValidateSignatures transaction...");
            const tx = await contract.ValidateSignatures(newNonce, signatures);
            const receipt = await tx.wait();
            
            console.log(`Tx Status: ${receipt.status === 1 ? "SUCCESS" : "FAILED"}`);

            // Get balance after
            const balAfter = await contract.balanceOf(deployer.address);
            console.log(`Balance after:  ${ethers.formatUnits(balAfter, 18)}`);

            if (receipt.status === 1 && balAfter > balBefore) {
                console.log(`✅ SUCCESS: Token successfully verified with new nonce!`);
            } else {
                console.log(`❌ FAILURE: Validation transaction succeeded but no tokens minted or transaction failed.`);
            }

        } catch (e) {
            console.error(`❌ Error verifying ${token.name}:`, e.message);
        }
        console.log(`\n`);
    }
}

main().catch(console.error);
