const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Sphere 1 PDS Monitor & Video RAM Emulator (sphere.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/sphere.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/sphere.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const sphereABI = [
        "function writeDisplay(uint256 offset, uint8 char) external returns (uint256)",
        "function executeMonitorCommand(string calldata cmd) external returns (string memory)"
    ];

    const factory = new ethers.ContractFactory(sphereABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`SphereSystem contract deployed at: ${contractAddr}\n`);

    const gasLimit = 150000;

    // 1. Write characters to Video RAM
    console.log("Writing 'SPHERE' to video RAM display...");
    const chars = "SPHERE";
    for (let i = 0; i < chars.length; i++) {
        await (await contract.writeDisplay(i, chars.charCodeAt(i), { gasLimit })).wait();
    }

    // 2. View Video RAM via PDS 'V' command
    console.log("Executing view command 'V'...");
    let screen = await contract.executeMonitorCommand.staticCall("V");
    console.log(`Screen line 0 view: "${screen.slice(0, 10)}"`);
    if (screen.slice(0, 6) !== "SPHERE") {
        throw new Error("Video display characters mismatched!");
    }
    console.log("Passed!\n");

    // 3. Deposit value into memory: "D 1000 FF"
    console.log("Executing deposit command 'D 1000 FF'...");
    let depRes = await contract.executeMonitorCommand.staticCall("D 1000 FF");
    console.log(`Deposit status: "${depRes}" (Expected: "OK")`);
    if (depRes !== "OK") {
        throw new Error("Deposit command failed!");
    }
    await (await contract.executeMonitorCommand("D 1000 FF", { gasLimit })).wait();

    // 4. Load/examine memory: "L 1000"
    console.log("Executing examine command 'L 1000'...");
    let loadRes = await contract.executeMonitorCommand.staticCall("L 1000");
    console.log(`Load result: "${loadRes}" (Expected: "1000: FF")`);
    if (loadRes !== "1000: FF") {
        throw new Error("Examine memory command failed!");
    }
    console.log("Passed!\n");

    // 5. Jump execution: "G 10A0"
    console.log("Executing go command 'G 10A0'...");
    let goRes = await contract.executeMonitorCommand.staticCall("G 10A0");
    console.log(`Go result: "${goRes}" (Expected: "STARTING AT 10A0")`);
    if (goRes !== "STARTING AT 10A0") {
        throw new Error("Go memory command failed!");
    }
    await (await contract.executeMonitorCommand("G 10A0", { gasLimit })).wait();
    console.log("Passed!\n");

    console.log(">>> ALL SPHERE EMULATOR TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
