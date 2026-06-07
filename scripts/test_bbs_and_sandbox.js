const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const diskABI = [
    "function executeDiskCommand(bytes calldata cmd) public returns (bytes)"
];

function buildWriteCalldata(filename, content) {
    const cmdHeader = `W0:${filename}\x00`;
    const cmdHeaderBytes = Buffer.from(cmdHeader, "utf-8");
    const contentBytes = Buffer.from(content, "utf-8");
    const fullCmd = Buffer.concat([cmdHeaderBytes, contentBytes]);

    const executeSelector = "0x9812a4df";
    const offsetSlot = ethers.zeroPadValue(ethers.toBeHex(32), 32);
    const lengthSlot = ethers.zeroPadValue(ethers.toBeHex(fullCmd.length), 32);
    const paddedBytes = ethers.concat([fullCmd, ethers.zeroPadValue("0x", (32 - (fullCmd.length % 32)) % 32)]);
    return ethers.concat([executeSelector, offsetSlot, lengthSlot, paddedBytes]);
}

function buildReadCalldata(filename) {
    const cmdHeader = `R0:${filename}\x00`;
    const cmdHeaderBytes = Buffer.from(cmdHeader, "utf-8");

    const executeSelector = "0x9812a4df";
    const offsetSlot = ethers.zeroPadValue(ethers.toBeHex(32), 32);
    const lengthSlot = ethers.zeroPadValue(ethers.toBeHex(cmdHeaderBytes.length), 32);
    const paddedBytes = ethers.concat([cmdHeaderBytes, ethers.zeroPadValue("0x", (32 - (cmdHeaderBytes.length % 32)) % 32)]);
    return ethers.concat([executeSelector, offsetSlot, lengthSlot, paddedBytes]);
}

async function main() {
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const diskAddress = config.networks.localhost.diskSystemAddress;
    console.log(`Using DiskSystem at: ${diskAddress}`);

    const mineBlock = async () => {
        await provider.send("evm_mine", []);
    };

    const signers = await provider.listAccounts();
    const alice = signers[0];
    const bob = signers[1];
    console.log(`Alice (User 0): ${alice.address}`);
    console.log(`Bob   (User 1): ${bob.address}`);

    const aliceDisk = new ethers.Contract(diskAddress, diskABI, alice);
    const bobDisk = new ethers.Contract(diskAddress, diskABI, bob);

    console.log("\n=== 1. Testing BBS Global Append-Only Board ===");
    
    // Clear/reset BBS if needed by writing a script/using scratch to ensure it's fresh?
    // Let's write to "bbs.txt" with Alice
    console.log("Alice posting to BBS...");
    const txAliceBbs = await alice.sendTransaction({
        to: diskAddress,
        data: buildWriteCalldata("bbs.txt", "Hello BBS from Alice!")
    });
    await txAliceBbs.wait();
    await mineBlock();

    // Let's write to "bbs.txt" with Bob
    console.log("Bob posting to BBS...");
    const txBobBbs = await bob.sendTransaction({
        to: diskAddress,
        data: buildWriteCalldata("bbs.txt", "Greetings from Bob!")
    });
    await txBobBbs.wait();
    await mineBlock();

    // Read back BBS content (with Alice or Bob, it's global so results should be identical)
    console.log("Reading BBS board...");
    const readBbsResult = await provider.call({
        to: diskAddress,
        data: buildReadCalldata("bbs.txt")
    });

    // Extract raw string from returned bytes (DiskSystem returns raw data slots, not ABI-encoded bytes)
    const bbsText = Buffer.from(ethers.getBytes(readBbsResult)).toString("utf-8");

    console.log("--- BBS.TXT Content ---");
    console.log(bbsText);
    console.log("-----------------------");

    // Validate that Alice and Bob's postings are both in there with the addresses formatted
    const aliceHexPart = alice.address.toLowerCase().slice(2);
    const bobHexPart = bob.address.toLowerCase().slice(2);

    if (!bbsText.toLowerCase().includes(aliceHexPart) || !bbsText.toLowerCase().includes(bobHexPart)) {
        throw new Error("BBS content does not include both users' addresses in prefix!");
    }
    if (!bbsText.includes("Hello BBS from Alice!") || !bbsText.includes("Greetings from Bob!")) {
        throw new Error("BBS content is missing written messages!");
    }
    console.log("BBS Verification: SUCCESS");

    console.log("\n=== 2. Testing Sandbox Separation ===");
    console.log("Alice writing private file 'secret.txt'...");
    const txAliceSecret = await alice.sendTransaction({
        to: diskAddress,
        data: buildWriteCalldata("secret.txt", "Alice Private Key: 12345")
    });
    await txAliceSecret.wait();
    await mineBlock();

    console.log("Bob writing private file 'secret.txt'...");
    const txBobSecret = await bob.sendTransaction({
        to: diskAddress,
        data: buildWriteCalldata("secret.txt", "Bob Private Key: 54321")
    });
    await txBobSecret.wait();
    await mineBlock();

    // Alice reads "secret.txt"
    console.log("Alice reading 'secret.txt'...");
    const aliceReadResult = await provider.call({
        from: alice.address,
        to: diskAddress,
        data: buildReadCalldata("secret.txt")
    });
    const aliceText = Buffer.from(ethers.getBytes(aliceReadResult)).toString("utf-8").replace(/\u0000/g, "").trim();
    console.log(`  Alice got: "${aliceText}"`);

    // Bob reads "secret.txt"
    console.log("Bob reading 'secret.txt'...");
    const bobReadResult = await provider.call({
        from: bob.address,
        to: diskAddress,
        data: buildReadCalldata("secret.txt")
    });
    const bobText = Buffer.from(ethers.getBytes(bobReadResult)).toString("utf-8").replace(/\u0000/g, "").trim();
    console.log(`  Bob got: "${bobText}"`);

    if (aliceText !== "Alice Private Key: 12345" || bobText !== "Bob Private Key: 54321") {
        throw new Error("Sandbox data leaked or crossed over!");
    }
    console.log("Sandbox Isolation: SUCCESS");

    console.log("\n=== 3. Testing Directory '$' Listings ===");
    // Read Alice directory
    console.log("Alice reading directory '$'...");
    const aliceDirResult = await provider.call({
        from: alice.address,
        to: diskAddress,
        data: buildReadCalldata("$")
    });
    const aliceDirList = parseDirectory(aliceDirResult);
    console.log("  Alice Directory:", aliceDirList);

    // Read Bob directory
    console.log("Bob reading directory '$'...");
    const bobDirResult = await provider.call({
        from: bob.address,
        to: diskAddress,
        data: buildReadCalldata("$")
    });
    const bobDirList = parseDirectory(bobDirResult);
    console.log("  Bob Directory:", bobDirList);

    if (!aliceDirList.includes("secret.txt") || aliceDirList.includes("bbs.txt")) {
        throw new Error("Alice directory listing incorrect!");
    }
    if (!bobDirList.includes("secret.txt") || bobDirList.includes("bbs.txt")) {
        throw new Error("Bob directory listing incorrect!");
    }
    console.log("Directory Listings: SUCCESS");

    console.log("\n=== ALL BBS AND SANDBOX TESTS PASSED SUCCESSFULLY! ===");
}

function parseDirectory(hexResult) {
    const list = [];
    const hex = hexResult.startsWith("0x") ? hexResult.slice(2) : hexResult;
    // The format returned is 8 words of 32 bytes (256 bytes)
    for (let i = 0; i < 8; i++) {
        const word = hex.substr(i * 64, 64);
        if (!word || word.length < 64) break;
        const buf = Buffer.from(word, "hex");
        const name = buf.toString("utf-8").replace(/\u0000/g, "").trim();
        if (name) {
            list.push(name);
        }
    }
    return list;
}

main().catch(err => {
    console.error("Test failed:", err);
    process.exit(1);
});
