const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing HP-65 RPN Stack Calculator (NelsonRPN) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/nelson.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/nelson.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const nelsonABI = [
        "function execute(bytes expr) external view returns (uint256 x, uint256 y, uint256 z, uint256 t)"
    ];

    const factory = new ethers.ContractFactory(nelsonABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`NelsonRPN deployed at: ${contractAddr}\n`);

    // Helper to run expressions
    async function testExpr(exprStr, expectedX, expectedY = 0, expectedZ = 0, expectedT = 0) {
        console.log(`Running expression: "${exprStr}"`);
        const exprBytes = ethers.hexlify(ethers.toUtf8Bytes(exprStr));
        const [x, y, z, t] = await contract.execute(exprBytes);
        console.log(`Result -> X: ${x.toString()}, Y: ${y.toString()}, Z: ${z.toString()}, T: ${t.toString()}`);
        if (
            x.toString() !== expectedX.toString() ||
            y.toString() !== expectedY.toString() ||
            z.toString() !== expectedZ.toString() ||
            t.toString() !== expectedT.toString()
        ) {
            console.error(`Mismatch! Expected -> X: ${expectedX}, Y: ${expectedY}, Z: ${expectedZ}, T: ${expectedT}`);
            process.exit(1);
        }
        console.log("Passed!\n");
    }

    // Test cases:
    // 1. Basic space-separated addition: "12 34 +"
    //    '1' & '2' -> lifts stack (all 0s), insideNumber=1, X=12
    //    ' ' -> insideNumber=0
    //    '3' & '4' -> lifts stack (Y=12), insideNumber=1, X=34
    //    ' ' -> insideNumber=0
    //    '+' -> X = Y + X = 12 + 34 = 46. Y = Z = 0, Z = T = 0.
    await testExpr("12 34 +", 46);

    // 2. Continuous expression: "3e4+"
    //    '3' -> lifts stack (all 0s), X=3
    //    'e' -> ENTER: T=Z=0, Z=Y=0, Y=X=3. Stack is: X=3, Y=3, Z=0, T=0
    //    '4' -> digit: lifts stack -> T=Z=0, Z=Y=3, Y=X=3, X=4. Stack is: X=4, Y=3, Z=3, T=0
    //    '+' -> X = Y + X = 3 + 4 = 7. Stack drops: Y=Z=3, Z=T=0. Stack is: X=7, Y=3, Z=0, T=0
    await testExpr("3e4+", 7, 3, 0, 0);

    // 3. Roll down test: "1e2e3e4er"
    //    Initial: 1 Enter 2 Enter 3 Enter 4 Enter
    //    Let's trace:
    //    - '1' -> X=1
    //    - 'e' -> Y=1, X=1
    //    - '2' -> Y=1, X=2 (actually digits lift stack because insideNumber=0: Z=1, Y=2 but wait: digit check does:
    //             t:=z, z:=y, y:=x, x:=0. Since X was 1, Y=1, Z=0, T=0:
    //             lift makes: t=0, z=1, y=1, x=0, then x gets 2. So stack: X=2, Y=1, Z=1, T=0.
    //    - 'e' -> T=0, Z=1, Y=1, X=2 -> enter: T=1, Z=1, Y=2, X=2.
    //    - '3' -> digit lifts: T=1, Z=2, Y=2, X=3.
    //    - 'e' -> enter: T=2, Z=2, Y=3, X=3.
    //    - '4' -> digit lifts: T=2, Z=3, Y=3, X=4.
    //    - 'e' -> enter: T=3, Z=3, Y=4, X=4.
    //    - 'r' -> roll down: temp=x=4, X=Y=4, Y=Z=3, Z=T=3, T=temp=4. Stack: X=4, Y=3, Z=3, T=4.
    await testExpr("1e2e3e4er", 4, 3, 3, 4);

    // 4. Subtraction, Multiplication, Division: "20 5 - 3 * 9 /"
    //    - "20 5 -" -> 20 - 5 = 15
    //    - "3 *" -> 15 * 3 = 45
    //    - "9 /" -> 45 / 9 = 5
    await testExpr("20 5 - 3 * 9 /", 5);

    // 5. Swap test: "5e6s"
    //    - "5" -> X=5
    //    - "e" -> Y=5, X=5
    //    - "6" -> lift -> Z=5, Y=5, X=6
    //    - "s" -> swap X and Y -> Z=5, Y=6, X=5
    await testExpr("5e6s", 5, 6, 5, 0);

    // 6. Clear stack test: "10e20+x"
    //    - "10e20+" -> 30
    //    - "x" -> all stack registers cleared to 0
    await testExpr("10e20+x", 0, 0, 0, 0);

    console.log(">>> ALL HP-65 RPN CALCULATOR TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
