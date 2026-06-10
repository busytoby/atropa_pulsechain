const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const cpuABI = [
    "function peekUser(address user, uint256 addr) public view returns (uint256)",
    "function pokeUser(address user, uint256 addr, uint256 val) public returns (uint256)"
];

async function runMathTest(cpu, user, op, a, b) {
    // 1. Write Operand A (16-bit) to 55100 (low) and 55101 (high)
    // Convert signed values to 16-bit representation
    const uA = a < 0 ? (65536 + a) : a;
    const a_lo = uA & 0xFF;
    const a_hi = (uA >> 8) & 0xFF;
    await (await cpu.pokeUser(user, 55100, a_lo)).wait();
    await (await cpu.pokeUser(user, 55101, a_hi)).wait();

    // 2. Write Operand B (16-bit) to 55102 (low) and 55103 (high)
    const uB = b < 0 ? (65536 + b) : b;
    const b_lo = uB & 0xFF;
    const b_hi = (uB >> 8) & 0xFF;
    await (await cpu.pokeUser(user, 55102, b_lo)).wait();
    await (await cpu.pokeUser(user, 55103, b_hi)).wait();

    // 3. Write Operator Command to 55104
    await (await cpu.pokeUser(user, 55104, op)).wait();

    // 4. Read 32-bit result from 55105-55108
    const r0 = await cpu.peekUser(user, 55105);
    const r1 = await cpu.peekUser(user, 55106);
    const r2 = await cpu.peekUser(user, 55107);
    const r3 = await cpu.peekUser(user, 55108);
    const status = await cpu.peekUser(user, 55109);

    let resVal = Number(r0) | (Number(r1) << 8) | (Number(r2) << 16) | (Number(r3) << 24);
    return { res: resVal, status: Number(status) };
}

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];
    const userAddr = signer.address;

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpuAddress = config.networks.localhost.cpu6502Address;
    const mathAddress = config.networks.localhost.mathCoprocessorAddress;

    const cpu = new ethers.Contract(cpuAddress, cpuABI, signer);

    console.log("\n=== STEP 2: Verifying Math Coprocessor Address Binding ===");
    const boundMath = await cpu.peekUser(userAddr, 54697);
    const boundMathAddress = ethers.toBeHex(boundMath, 20);
    console.log(`Bound Math Coprocessor Address: ${boundMathAddress}`);
    if (ethers.getAddress(boundMathAddress) !== ethers.getAddress(mathAddress)) {
        throw new Error("FAIL: Math Coprocessor address is not bound correctly in the CPU storage slot.");
    }

    console.log("\n=== STEP 3: Executing Arithmetic Tests ===");

    // Test 1: Add (op = 1) -> 150 + 350 = 500
    const addResult = await runMathTest(cpu, userAddr, 1, 150, 350);
    console.log(`Add: 150 + 350 = ${addResult.res} (Expected: 500), status: ${addResult.status}`);
    if (addResult.res !== 500 || addResult.status !== 0) throw new Error("FAIL: Add test.");

    // Test 2: Subtract (op = 2) -> 50 - 120 = -70
    const subResult = await runMathTest(cpu, userAddr, 2, 50, 120);
    console.log(`Subtract: 50 - 120 = ${subResult.res} (Expected: -70), status: ${subResult.status}`);
    if (subResult.res !== -70 || subResult.status !== 0) throw new Error("FAIL: Subtract test.");

    // Test 3: Multiply (op = 3) -> 25 * -4 = -100
    const mulResult = await runMathTest(cpu, userAddr, 3, 25, -4);
    console.log(`Multiply: 25 * -4 = ${mulResult.res} (Expected: -100), status: ${mulResult.status}`);
    if (mulResult.res !== -100 || mulResult.status !== 0) throw new Error("FAIL: Multiply test.");

    // Test 4: Divide (op = 4) -> 1000 / 25 = 40
    const divResult = await runMathTest(cpu, userAddr, 4, 1000, 25);
    console.log(`Divide: 1000 / 25 = ${divResult.res} (Expected: 40), status: ${divResult.status}`);
    if (divResult.res !== 40 || divResult.status !== 0) throw new Error("FAIL: Divide test.");

    // Test 5: Divide by Zero
    const divZeroResult = await runMathTest(cpu, userAddr, 4, 1000, 0);
    console.log(`Divide by Zero status: ${divZeroResult.status} (Expected: 1)`);
    if (divZeroResult.status !== 1) throw new Error("FAIL: Divide by Zero check.");

    // Test 6: Power (op = 6) -> 2 ^ 10 = 1024
    const powResult = await runMathTest(cpu, userAddr, 6, 2, 10);
    console.log(`Power: 2 ^ 10 = ${powResult.res} (Expected: 1024), status: ${powResult.status}`);
    if (powResult.res !== 1024 || powResult.status !== 0) throw new Error("FAIL: Power test.");

    // Test 7: Square Root (op = 7) -> sqrt(144) = 12
    const sqrtResult = await runMathTest(cpu, userAddr, 7, 144, 0);
    console.log(`Square Root: sqrt(144) = ${sqrtResult.res} (Expected: 12), status: ${sqrtResult.status}`);
    if (sqrtResult.res !== 12 || sqrtResult.status !== 0) throw new Error("FAIL: Square Root test.");

    // Test 8: Sine (op = 8) -> sin(4) = 100
    const sinResult = await runMathTest(cpu, userAddr, 8, 4, 0);
    console.log(`Sine: sin(4) = ${sinResult.res} (Expected: 100), status: ${sinResult.status}`);
    if (sinResult.res !== 100 || sinResult.status !== 0) throw new Error("FAIL: Sine test.");

    // Test 9: Cosine (op = 9) -> cos(4) = 0
    const cosResult = await runMathTest(cpu, userAddr, 9, 4, 0);
    console.log(`Cosine: cos(4) = ${cosResult.res} (Expected: 0), status: ${cosResult.status}`);
    if (cosResult.res !== 0 || cosResult.status !== 0) throw new Error("FAIL: Cosine test.");

    console.log("\n=== ALL MATH COPROCESSOR TESTS PASSED ===");
}

main().catch((err) => {
    console.error(err);
    process.exit(1);
});
