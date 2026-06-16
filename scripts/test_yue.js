const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PULSECHAIN_RPC_URL = "https://rpc.pulsechain.com";

// List of YUE contract candidates discovered on PulseChain mainnet
const YUE_CANDIDATES = [
    { name: "godlobster", address: "0x04a9f45db8a32395a31ddb31c7989b2a75e79094" },
    { name: "WIZYUE?", address: "0x4a32b4391db5771a3a3682183a6b109d516f723f" },
    { name: "YUE_TEST", address: "0x4f94f27d9655411927e72799c6e797989a86f2d4" },
    { name: "ℝ◉𝕐𝔸𝕃 𝕄♔ℕ𝕋 𓂀", address: "0xcda0c40f7f27f5e9992af7812538b1c0c16e7645" },
    { name: "tsunigesdiYUE", address: "0xe29aeec5bE7D7d6C45D776EcA36c325FC44b6069" }
];

// Helper to load contract ABI from compiled yue.sol.json
function getContractAbi() {
    const filePath = path.join(__dirname, "../Wallet/bin/Contracts/yue.sol.json");
    const fileContent = JSON.parse(fs.readFileSync(filePath, "utf8"));
    for (const key of Object.keys(fileContent.contracts)) {
        if (key.endsWith(":YUE")) {
            return fileContent.contracts[key].abi;
        }
    }
    throw new Error("YUE contract ABI not found");
}

async function main() {
    console.log("Connecting to live PulseChain RPC...");
    const provider = new ethers.JsonRpcProvider(PULSECHAIN_RPC_URL);
    const abi = getContractAbi();
    
    // List of NoNukes partner tokens to query YUE scores for
    const tokensToQuery = [
        { name: "SⒶV (SⒶVⒶNT)", address: "0x041a80b38d3a5b4dbb30e56440ca8f0c8dfa6412" },
        { name: "GAS Money ⛽", address: "0x042b48a98b37042d58bc8defeeb7ca4ec76e6106" },
        { name: "✪WATER✪🌊", address: "0x1ca9668a72da58c47bec5d15d989f8e65d90db97" },
        { name: "✪WOOD✪🪵", address: "0x6327772dd3158c510069e26606420f88fdcfcb92" },
        { name: "Liquid Resonance ❁", address: "0xfc70068e1d2ab636dddde8c1ee6b22f8b3b00724" }
    ];

    console.log("\n==================================================");
    console.log("QUERYING YUE SCORE POINTS ON PULSECHAIN CANDIDATES:");
    console.log("==================================================");
    
    for (const candidate of YUE_CANDIDATES) {
        console.log(`\nCandidate Contract: ${candidate.name} (${candidate.address})`);
        const yue = new ethers.Contract(candidate.address, abi, provider);
        
        try {
            const origin = await yue.Origin();
            console.log(` -> System Origin: ${origin}`);
            
            for (const token of tokensToQuery) {
                try {
                    const bar = await yue.Bar(token.address);
                    if (bar[0] > 0 || bar[1] > 0) {
                        console.log(` -> [MATCH] Token: ${token.name} (${token.address})`);
                        console.log(`    -> Hypobar (Omega):   ${bar[0].toString()}`);
                        console.log(`    -> Epibar (Omicron):  ${bar[1].toString()}`);
                    } else {
                        console.log(` -> Token: ${token.name} (${token.address}) score is 0, 0`);
                    }
                } catch (err) {
                    console.error(` -> Error querying score for ${token.name}:`, err.message.substring(0, 100));
                }
            }
        } catch (err) {
            console.error(` -> Failed to query candidate contract:`, err.message.substring(0, 100));
        }
    }
    
    console.log("\n==================================================");
    console.log("YUE Live Query Test Complete.");
    console.log("==================================================");
}

main().catch(console.error);
