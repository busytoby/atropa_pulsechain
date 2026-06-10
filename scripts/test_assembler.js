const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing On-Chain Assembler (assembler.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/assembler.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/assembler.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const asmABI = [
        "function assemble(string calldata code) external view returns (bytes memory)"
    ];

    const factory = new ethers.ContractFactory(asmABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Assembler contract deployed at: ${contractAddr}\n`);

    const code = `
        MVI A 5
        MVI B 3
        ADD B
        STA 16
        HLT
    `;

    console.log("Assembling code...");
    console.log(`--- Source Code ---\n${code}\n-------------------`);
    
    const outputHex = await contract.assemble(code);
    const outputBytes = Array.from(ethers.getBytes(outputHex));
    console.log(`Assembled Output bytes: [${outputBytes.map(b => "0x" + b.toString(16).toUpperCase().padStart(2, "0")).join(", ")}]`);

    const expected = [0x01, 0x05, 0x02, 0x03, 0x03, 0x08, 0x00, 0x10, 0xFF];
    console.log(`Expected Output bytes:  [${expected.map(b => "0x" + b.toString(16).toUpperCase().padStart(2, "0")).join(", ")}]`);

    if (JSON.stringify(outputBytes) !== JSON.stringify(expected)) {
        throw new Error("Assembled output mismatched expected byte representations!");
    }

    console.log("Passed!\n");
    console.log(">>> ALL ASSEMBLER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
