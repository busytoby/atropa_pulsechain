const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Aunt Sally algebraic PEMDAS Parser (auntsally.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/auntsally.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/auntsally.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const sallyABI = [
        "function evaluate(bytes expr) external view returns (int256 result)"
    ];

    const factory = new ethers.ContractFactory(sallyABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`AuntSallyParser contract deployed at: ${contractAddr}\n`);

    // Helper to evaluate expressions
    async function testExpr(exprStr, expectedVal) {
        console.log(`Evaluating expression: "${exprStr}"`);
        const exprBytes = ethers.hexlify(ethers.toUtf8Bytes(exprStr));
        const result = await contract.evaluate(exprBytes);
        console.log(`Result: ${result.toString()} (Expected: ${expectedVal})`);
        if (result.toString() !== expectedVal.toString()) {
            console.error(`FAIL: Expected ${expectedVal}, got ${result}`);
            process.exit(1);
        }
        console.log("Passed!\n");
    }

    // Test Cases
    await testExpr("3+4", 7);
    await testExpr("10-2*3", 4);
    await testExpr("(3+4)*2", 14);
    await testExpr("12/3-1", 3);
    await testExpr("2*(3+4*(5-2))", 30);

    console.log(">>> ALL AUNT SALLY ALGEBRAIC PARSER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
