const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Selector for verifySignature(bytes32,bytes32,bytes32,uint8,address)
const VERIFY_SIG_SELECTOR = "0x489069d1";

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const keys = config.default.keys;
    const factoryAddress = config.networks.localhost.immutableFactoryAddress;

    if (!keys || keys.length !== 8) {
        console.error("Exactly 8 keys expected in config.");
        process.exit(1);
    }
    if (!factoryAddress) {
        console.error("Yul ImmutableFactory address not found in config.");
        process.exit(1);
    }

    console.log(`Using factory at: ${factoryAddress}`);

    // Create a dummy message hash to sign
    const dummyMessage = ethers.id("Dysnomia Signature Test");
    console.log(`Dummy Message Hash: ${dummyMessage}`);

    console.log("\n==================================================");
    console.log("TESTING VERIFY SIGNATURE FOR ALL 8 KEYS:");
    console.log("==================================================");

    for (let i = 0; i < keys.length; i++) {
        const wallet = new ethers.Wallet(keys[i]);
        
        // Sign raw hash (not EIP-191)
        const signature = wallet.signingKey.sign(dummyMessage);
        const r = signature.r;
        const s = signature.s;
        const v = signature.v;

        // Yul factory expects parameters left-aligned to 32 bytes
        const vLeftAligned = ethers.concat([ethers.toBeHex(v), ethers.zeroPadValue("0x", 31)]);
        const walletAddressLeftAligned = ethers.concat([wallet.address, ethers.zeroPadValue("0x", 12)]);

        // verifySignature(hash, r, s, v_aligned, address_aligned)
        const verifySigCalldata = ethers.concat([
            VERIFY_SIG_SELECTOR,
            dummyMessage,
            r,
            s,
            vLeftAligned,
            walletAddressLeftAligned
        ]);

        const resultRaw = await provider.call({
            to: factoryAddress,
            data: verifySigCalldata
        });
        const isValid = ethers.toNumber(resultRaw) === 1;

        console.log(`Key [${i}] (${wallet.address}): ${isValid ? "✅ VALID" : "❌ INVALID"}`);
    }
    console.log("==================================================");
}

main().catch(console.error);
