const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

// Paths to compiled artifacts
const MOCKS_ARTIFACT_PATH = path.join(__dirname, "../Wallet/bin/Contracts/Mocks.sol.json");
const PKMINTER_ARTIFACT_PATH = path.join(__dirname, "../Wallet/bin/Contracts/pkminter.sol.json");

// Hardcoded math library address that TT points to:
const MATH_LIB_ADDRESS = "0xB680F0cc810317933F234f67EB6A9E923407f05D";

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log(`Deployer address: ${deployer.address}`);

    // Load Mocks artifacts
    const mocksArtifact = JSON.parse(fs.readFileSync(MOCKS_ARTIFACT_PATH, "utf8"));
    
    // 1. Deploy MockAtropaMath
    console.log("\n--- Deploying MockAtropaMath ---");
    const mockAtropaMathContracts = mocksArtifact.contracts["dysnomia/Mocks.sol:MockAtropaMath"];
    const mockAtropaMathFactory = new ethers.ContractFactory(mockAtropaMathContracts.abi, mockAtropaMathContracts.bin, deployer);
    const mockAtropaMath = await mockAtropaMathFactory.deploy();
    await mockAtropaMath.waitForDeployment();
    const mockAtropaMathAddress = await mockAtropaMath.getAddress();
    console.log(`MockAtropaMath deployed at: ${mockAtropaMathAddress}`);

    // Get runtime bytecode of MockAtropaMath to inject
    const mockAtropaMathRuntimeBytecode = await provider.getCode(mockAtropaMathAddress);
    console.log(`Injecting MockAtropaMath code at: ${MATH_LIB_ADDRESS}`);
    await provider.send("anvil_setCode", [MATH_LIB_ADDRESS, mockAtropaMathRuntimeBytecode]);
    
    // Verify override worked
    const codeAtMathLib = await provider.getCode(MATH_LIB_ADDRESS);
    if (codeAtMathLib === mockAtropaMathRuntimeBytecode) {
        console.log("SUCCESS: MockAtropaMath code successfully injected!");
    } else {
        console.error("ERROR: Failed to inject MockAtropaMath code!");
        process.exit(1);
    }

    // 2. Deploy MockLau
    console.log("\n--- Deploying MockLau ---");
    const mockLauContracts = mocksArtifact.contracts["dysnomia/Mocks.sol:MockLau"];
    const mockLauFactory = new ethers.ContractFactory(mockLauContracts.abi, mockLauContracts.bin, deployer);
    // Construct MockLau with a dummy address for Xiao (or we can passMATH_LIB_ADDRESS)
    const mockLau = await mockLauFactory.deploy(MATH_LIB_ADDRESS);
    await mockLau.waitForDeployment();
    const mockLauAddress = await mockLau.getAddress();
    console.log(`MockLau deployed at: ${mockLauAddress}`);

    // 3. Deploy MockCho
    console.log("\n--- Deploying MockCho ---");
    const mockChoContracts = mocksArtifact.contracts["dysnomia/Mocks.sol:MockCho"];
    const mockChoFactory = new ethers.ContractFactory(mockChoContracts.abi, mockChoContracts.bin, deployer);
    // Construct MockCho with mockLauAddress which acts as the UserToken address (LAU)
    const mockCho = await mockChoFactory.deploy(mockLauAddress);
    await mockCho.waitForDeployment();
    const mockChoAddress = await mockCho.getAddress();
    console.log(`MockCho deployed at: ${mockChoAddress}`);

    // 4. Deploy PKMinter
    console.log("\n--- Deploying PKMinter ---");
    const pkminterArtifact = JSON.parse(fs.readFileSync(PKMINTER_ARTIFACT_PATH, "utf8"));
    const pkminterContracts = pkminterArtifact.contracts["dysnomia/domain/dan/pkminter.sol:PKMinter"];
    const pkminterFactory = new ethers.ContractFactory(pkminterContracts.abi, pkminterContracts.bin, deployer);
    const pkminter = await pkminterFactory.deploy(mockChoAddress);
    await pkminter.waitForDeployment();
    const pkminterAddress = await pkminter.getAddress();
    console.log(`PKMinter deployed at: ${pkminterAddress}`);

    console.log("\n==================================================");
    console.log("LOCAL DEPLOYMENT SUMMARY:");
    console.log(`MockAtropaMath: ${mockAtropaMathAddress}`);
    console.log(`MockAtropaMath Injected At: ${MATH_LIB_ADDRESS}`);
    console.log(`MockLau:        ${mockLauAddress}`);
    console.log(`MockCho:        ${mockChoAddress}`);
    console.log(`PKMinter:       ${pkminterAddress}`);
    console.log("==================================================");
}

main().catch(console.error);
