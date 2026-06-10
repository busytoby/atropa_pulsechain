const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Zarrella Double-Entry Accounting Ledger (ledger.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/ledger.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/ledger.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const ledgerABI = [
        "function postTransaction(uint256 debitAccount, uint256 creditAccount, uint256 amount) external returns (uint256)",
        "function getBalanceDirect(uint256 accountId) external view returns (int256)",
        "function setPointer(uint256 pointerId, uint256 targetAccountId) external returns (uint256)",
        "function getBalanceIndirect(uint256 pointerId) external view returns (int256)",
        "function getBalanceIndexed(uint256 baseAccountId, uint256 index) external view returns (int256)"
    ];

    const factory = new ethers.ContractFactory(ledgerABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Ledger contract deployed at: ${contractAddr}\n`);

    const gasLimit = 150000;

    // 1. Post transaction: Debit Account 2, Credit Account 1, Amount 500
    console.log("Posting transaction: Debit 2, Credit 1, Amount 500...");
    await (await contract.postTransaction(2, 1, 500, { gasLimit })).wait();

    // 2. Direct Address Mode validation
    console.log("Reading balance of Account 2 (Direct)...");
    const bal2 = await contract.getBalanceDirect(2);
    console.log(`Balance Account 2: ${bal2} (Expected: 500)`);
    
    console.log("Reading balance of Account 1 (Direct)...");
    const bal1 = await contract.getBalanceDirect(1);
    console.log(`Balance Account 1: ${bal1} (Expected: -500)`);

    if (Number(bal2) !== 500 || Number(bal1) !== -500) {
        throw new Error("Direct addressing balance mismatch!");
    }
    console.log("Direct addressing mode: Passed!\n");

    // 3. Indirect Address Mode validation
    console.log("Setting Pointer 10 -> Account 2...");
    await (await contract.setPointer(10, 2, { gasLimit })).wait();

    console.log("Reading balance through Pointer 10 (Indirect)...");
    const balIndirect = await contract.getBalanceIndirect(10);
    console.log(`Balance via Pointer 10: ${balIndirect} (Expected: 500)`);
    if (Number(balIndirect) !== 500) {
        throw new Error("Indirect addressing balance mismatch!");
    }
    console.log("Indirect addressing mode: Passed!\n");

    // 4. Indexed Address Mode validation
    console.log("Posting transaction to Account 5 (Base 2 + Offset 3): Debit 5, Credit 1, Amount 100...");
    await (await contract.postTransaction(5, 1, 100, { gasLimit })).wait();

    console.log("Reading balance of Base 2, Index 3 (Indexed)...");
    const balIndexed = await contract.getBalanceIndexed(2, 3);
    console.log(`Balance of Account 5 via Indexed mode: ${balIndexed} (Expected: 100)`);
    if (Number(balIndexed) !== 100) {
        throw new Error("Indexed addressing balance mismatch!");
    }
    console.log("Indexed addressing mode: Passed!\n");

    console.log(">>> ALL ACCOUNTING & ADDRESSING TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
