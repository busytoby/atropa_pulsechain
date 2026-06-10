const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Intel 8080 CPU Emulator (i8080.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/i8080.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/i8080.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const i8080ABI = [
        "function execute(bytes bytecode, uint256 cycles) external view returns (uint8 a, uint8 b, uint8 c, uint8 d, uint8 e, uint8 h, uint8 l, uint16 pc, uint16 sp, uint8 flags)"
    ];

    const factory = new ethers.ContractFactory(i8080ABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Intel8080 deployed at: ${contractAddr}\n`);

    // Helper to run code blocks
    async function runTest(hexCode, cycles, expectedRegs) {
        console.log(`Executing bytecode: ${hexCode} for ${cycles} cycles`);
        const [a, b, c, d, e, h, l, pc, sp, flags] = await contract.execute(hexCode, cycles);
        console.log(`Result registers -> A: ${a}, B: ${b}, PC: ${pc}, Flags: 0x${flags.toString(16)}`);
        
        for (const [regName, expVal] of Object.entries(expectedRegs)) {
            let actualVal;
            if (regName === "a") actualVal = a;
            else if (regName === "b") actualVal = b;
            else if (regName === "pc") actualVal = pc;
            else if (regName === "flags") actualVal = flags;
            
            if (Number(actualVal) !== expVal) {
                console.error(`FAIL: Register ${regName} mismatched. Expected ${expVal}, got ${actualVal}`);
                process.exit(1);
            }
        }
        console.log("Passed!\n");
    }

    // Test 1: Simple registers moves and addition:
    // MVI A, 10 (3e 0a)
    // MVI B, 20 (06 14)
    // ADD B     (80)
    // Bytecode: 0x3e0a061480
    // Expected final: A = 30, B = 20, PC = 5.
    // Flags: binary of 30 is 00011110. Parity should be 1 (even). Carry=0, Zero=0, Sign=0.
    // Flags byte = 2 (default) | 0x04 (parity) = 0x06
    await runTest("0x3e0a061480", 3, { a: 30, b: 20, pc: 5, flags: 0x06 });

    // Test 2: Subtraction and sign/carry check:
    // MVI A, 5 (3e 05)
    // MVI B, 8 (06 08)
    // SUB B    (90)
    // Bytecode: 0x3e05060890
    // Result of 5 - 8 = -3 => 253 (0xfd)
    // Flags: Sign=1 (0x80), Carry=1 (0x01), Parity=0 (0xfd has 7 ones), Zero=0.
    // Flags byte = 2 | 0x80 | 0x01 = 0x83
    await runTest("0x3e05060890", 3, { a: 253, b: 8, pc: 5, flags: 0x83 });

    // Test 3: Jump instruction:
    // MVI A, 1  (3e 01)
    // JMP 0x0007 (c3 07 00)
    // MVI A, 2  (3e 02) -> skipped
    // MVI B, 5  (06 05) -> executed at address 7
    // Bytecode: 0x3e01c307003e020605
    // Cycles = 3. PC should land at 9. A = 1, B = 5.
    await runTest("0x3e01c307003e020605", 3, { a: 1, b: 5, pc: 9 });

    console.log(">>> ALL INTEL 8080 CPU TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
