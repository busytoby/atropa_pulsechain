const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Equational Logic Monitor (elm.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/elm.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/elm.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const elmABI = [
        "function getVariable(string calldata name) external view returns (int256)",
        "function evalStatement(string calldata statement) external returns (string memory name, int256 value, bool isEquation)"
    ];

    const factory = new ethers.ContractFactory(elmABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`ELM contract deployed at: ${contractAddr}\n`);

    const gasLimit = 150000;

    // 1. Assign B = 10
    console.log("Evaluating 'B = 10'...");
    let tx1 = await contract.evalStatement.staticCall("B = 10");
    console.log(`staticCall returns -> name: "${tx1.name}", value: ${tx1.value}, isEquation: ${tx1.isEquation}`);
    await (await contract.evalStatement("B = 10", { gasLimit })).wait();

    // 2. Assign C = 20
    console.log("Evaluating 'C = 20'...");
    await (await contract.evalStatement("C = 20", { gasLimit })).wait();

    // 3. Evaluate equation A = B + C
    console.log("Evaluating 'A = B + C'...");
    let tx3 = await contract.evalStatement.staticCall("A = B + C");
    console.log(`staticCall returns -> name: "${tx3.name}", value: ${tx3.value}, isEquation: ${tx3.isEquation}`);
    if (tx3.name !== "A" || Number(tx3.value) !== 30 || !tx3.isEquation) {
        throw new Error("Equation evaluation failed for B + C!");
    }
    await (await contract.evalStatement("A = B + C", { gasLimit })).wait();

    // Verify variable A directly
    let valA = await contract.getVariable("A");
    console.log(`Variable A value: ${valA} (Expected: 30)`);
    if (Number(valA) !== 30) {
        throw new Error("Variable A stored value mismatched!");
    }
    console.log("Passed!\n");

    // 4. Test multiplication: X = A * B -> 30 * 10 = 300
    console.log("Evaluating 'X = A * B'...");
    let tx4 = await contract.evalStatement.staticCall("X = A * B");
    console.log(`staticCall returns -> name: "${tx4.name}", value: ${tx4.value}`);
    if (Number(tx4.value) !== 300) {
        throw new Error("Multiplication output mismatched!");
    }
    await (await contract.evalStatement("X = A * B", { gasLimit })).wait();

    // 5. Test division: Y = X / C -> 300 / 20 = 15
    console.log("Evaluating 'Y = X / C'...");
    let tx5 = await contract.evalStatement.staticCall("Y = X / C");
    console.log(`staticCall returns -> name: "${tx5.name}", value: ${tx5.value}`);
    if (Number(tx5.value) !== 15) {
        throw new Error("Division output mismatched!");
    }
    await (await contract.evalStatement("Y = X / C", { gasLimit })).wait();

    // 6. Test subtraction: Z = Y - 5 -> 15 - 5 = 10
    console.log("Evaluating 'Z = Y - 5'...");
    let tx6 = await contract.evalStatement.staticCall("Z = Y - 5");
    console.log(`staticCall returns -> name: "${tx6.name}", value: ${tx6.value}`);
    if (Number(tx6.value) !== 10) {
        throw new Error("Subtraction output mismatched!");
    }
    await (await contract.evalStatement("Z = Y - 5", { gasLimit })).wait();

    // 7. Evaluate expression without assignment: B * C -> 10 * 20 = 200
    console.log("Evaluating 'B * C' (no assignment)...");
    let tx7 = await contract.evalStatement.staticCall("B * C");
    console.log(`staticCall returns -> name: "${tx7.name}", value: ${tx7.value}, isEquation: ${tx7.isEquation}`);
    if (tx7.name !== "" || Number(tx7.value) !== 200 || tx7.isEquation) {
        throw new Error("Fell back or mis-parsed non-assignment expression!");
    }

    console.log("Passed!\n");
    console.log(">>> ALL EQUATIONAL LOGIC MONITOR TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
