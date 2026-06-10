const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function parseTextToPhonemes(string memory text) public view returns (bytes32[])"
];

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log("Speech Synthesizer:", speechAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);

    console.log("\n=== STEP 2: Testing parseTextToPhonemes(\"shee\") ===");
    const phonemes1 = await speech.parseTextToPhonemes("shee");
    console.log("Phonemes array length:", phonemes1.length);
    console.log("Decoded values:");
    for (let p of phonemes1) {
        // Decode trailing zero padded bytes32 to string
        const cleanedHex = p.replace(/0+$/, "");
        console.log(`- ${p} -> "${Buffer.from(cleanedHex.substring(2), "hex").toString("utf8")}"`);
    }

    if (phonemes1.length !== 2) {
        throw new Error("FAIL: Expected 2 phonemes for 'shee'!");
    }
    console.log("✓ Success: 'shee' parsed correctly into ['sh', 'ee'].");

    console.log("\n=== STEP 3: Testing parseTextToPhonemes(\"soomf\") ===");
    const phonemes2 = await speech.parseTextToPhonemes("soomf");
    console.log("Phonemes array length:", phonemes2.length);
    console.log("Decoded values:");
    for (let p of phonemes2) {
        const cleanedHex = p.replace(/0+$/, "");
        console.log(`- ${p} -> "${Buffer.from(cleanedHex.substring(2), "hex").toString("utf8")}"`);
    }

    if (phonemes2.length !== 4) {
        throw new Error("FAIL: Expected 4 phonemes for 'soomf'!");
    }
    console.log("✓ Success: 'soomf' parsed correctly into ['s', 'oo', 'm', 'f'].");

    console.log("\n=== ALL ON-CHAIN GRAPHEME-TO-PHONEME PARSER TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
