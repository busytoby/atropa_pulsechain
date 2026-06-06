const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Array length is stored at Slot 12
const ARRAY_SLOT = 12;

async function main() {
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);

    const tokensToFix = [
        { name: "Genesis PK3", address: "0x5fD11B3E10E0d71Ab05ace92797e517e532E1462", keys: config.saved_keys["0x5fD11B3E10E0d71Ab05ace92797e517e532E1462"] },
        { name: "Genesis PK4", address: "0xb4C9Ed909ecB896586d6ebCfEbA0E97fdCA51310", keys: config.saved_keys["0xb4C9Ed909ecB896586d6ebCfEbA0E97fdCA51310"] },
        { name: "Genesis PK9", address: "0xe40AF675753322508d033CE8EE6135aA76470338", keys: config.saved_keys["0xe40AF675753322508d033CE8EE6135aA76470338"] }
    ];

    for (const token of tokensToFix) {
        console.log(`\nRepopulating state array for ${token.name} (${token.address})...`);
        const length = token.keys.length;

        // 1. Write the length to Slot 12
        const lengthHex = ethers.zeroPadValue(ethers.toBeHex(length), 32);
        console.log(`Setting length slot (12) to: ${length}`);
        await provider.send("anvil_setStorageAt", [token.address, ethers.toBeHex(ARRAY_SLOT), lengthHex]);

        // 2. Calculate starting index slot using keccak256 of the array slot (12)
        const arrayStartSlotHex = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(ARRAY_SLOT), 32));
        const arrayStartSlot = BigInt(arrayStartSlotHex);

        // 3. Write each public key address to its slot
        for (let i = 0; i < length; i++) {
            const wallet = new ethers.Wallet(token.keys[i]);
            const targetSlot = ethers.toBeHex(arrayStartSlot + BigInt(i));
            
            // Pad address to 32 bytes (left-padded with zeros)
            const addressPadded = ethers.zeroPadValue(wallet.address, 32);
            
            console.log(`  Writing index [${i}] (${wallet.address}) to slot: ${targetSlot}`);
            await provider.send("anvil_setStorageAt", [token.address, targetSlot, addressPadded]);
        }
        console.log(`✅ ${token.name} storage state repopulated!`);
    }

    console.log("\nAll contract storage arrays repopulated. Run verify_pk_nonces.js to validate!");
}

main().catch(console.error);
