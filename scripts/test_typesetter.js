const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing MahoneyTypesetter (BYTE Production Assistant) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/mahoney.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/mahoney.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const mahoneyABI = [
        "function justify(bytes text, uint256 lineLength) external view returns (bytes justified)"
    ];

    const factory = new ethers.ContractFactory(mahoneyABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`MahoneyTypesetter deployed at: ${contractAddr}\n`);

    // Helper to run typesetting verification
    async function testTypeset(inputText, lineLength, expectedOutput) {
        console.log(`Input text: "${inputText}" (Line length: ${lineLength})`);
        const textBytes = ethers.hexlify(ethers.toUtf8Bytes(inputText));
        const resBytes = await contract.justify(textBytes, lineLength);
        const outputText = ethers.toUtf8String(resBytes);
        console.log(`Output text:\n${outputText}---`);
        if (outputText !== expectedOutput) {
            console.error(`FAIL! Expected:\n${expectedOutput}---`);
            process.exit(1);
        }
        console.log("Passed!\n");
    }

    // Test Case 1: Simple perfect fit line length 15
    // "The quick brown" has length 3 + 1 + 5 + 1 + 5 = 15.
    // "fox jumps over" has length 3 + 1 + 5 + 1 + 4 = 14 -> fits in 15, needs 3 spaces total distributed in 2 gaps:
    // gap 1: 2 spaces, gap 2: 1 space -> "fox  jumps over"
    // "the lazy dog" is the last line -> "the lazy dog"
    const inputStr = "The quick brown fox jumps over the lazy dog";
    const expectedStr = "The quick brown\nfox  jumps over\nthe lazy dog\n";
    await testTypeset(inputStr, 15, expectedStr);

    // Test Case 2: Different line length
    const input2 = "BYTE Magazine production typesetting logic";
    // Words:
    // "BYTE" (4), "Magazine" (8), "production" (10), "typesetting" (11), "logic" (5)
    // lineLength = 20
    // Line 1: "BYTE Magazine" -> len 4 + 1 + 8 = 13. next "production" (10) -> 13 + 1 + 10 = 24 > 20.
    // So Line 1 is "BYTE Magazine". Total word chars = 12. Spaces to distribute = 20 - 12 = 8.
    // Gap count = 1. All 8 spaces in the single gap.
    // Line 1: "BYTE        Magazine\n"
    // Line 2: "production" -> len 10. next "typesetting" (11) -> 10 + 1 + 11 = 22 > 20.
    // So Line 2 is "production". Word chars = 10. Spaces = 10. Gap count = 0.
    // Line 2: "production\n"
    // Line 3: "typesetting logic" -> last line. "typesetting logic\n"
    const expected2 = "BYTE        Magazine\nproduction\ntypesetting logic\n";
    await testTypeset(input2, 20, expected2);

    console.log(">>> ALL MAHONEY TYPESETTER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
