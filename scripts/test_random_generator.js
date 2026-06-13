const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const FACTORY_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa3";

async function main() {
    console.log("=== STEP 1: Compiling Contracts ===");
    let mockBin, generatorBin;
    try {
        console.log("Compiling Mocks...");
        const mocksOutput = JSON.parse(execSync(
            `solc --standard-json`,
            {
                input: JSON.stringify({
                    language: "Solidity",
                    sources: { "Mocks.sol": { content: require("fs").readFileSync(path.join(__dirname, "../solidity/dysnomia/Mocks.sol"), "utf8") } },
                    settings: { outputSelection: { "*": { "*": ["abi", "evm.bytecode"] } } }
                }),
                encoding: "utf8"
            }
        ));
        
        console.log("Compiling RandomGenerator...");
        const generatorOutput = JSON.parse(execSync(
            `solc --standard-json`,
            {
                input: JSON.stringify({
                    language: "Solidity",
                    sources: { "RandomGenerator.sol": { content: require("fs").readFileSync(path.join(__dirname, "../solidity/dysnomia/RandomGenerator.sol"), "utf8") } },
                    settings: { outputSelection: { "*": { "*": ["abi", "evm.bytecode"] } } }
                }),
                encoding: "utf8"
            }
        ));

        mockBin = mocksOutput.contracts["Mocks.sol"];
        generatorBin = generatorOutput.contracts["RandomGenerator.sol"]["RandomGenerator"];
        console.log("Compilation successful.");
    } catch (err) {
        console.error("Compilation failed:", err.message);
        process.exit(1);
    }

    console.log(`\n=== STEP 2: Connecting to local EVM ===`);
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    const masterKeyWallet = ethers.Wallet.createRandom().connect(provider);

    // Fund master key
    await deployer.sendTransaction({ to: masterKeyWallet.address, value: ethers.parseEther("1") });

    console.log(`Deployer: ${deployer.address}`);
    console.log(`Master Key: ${masterKeyWallet.address}`);

    console.log(`\n=== STEP 3: Deploying Mock Contracts ===`);
    // Deploy MockAtropaMath
    const mathFactory = new ethers.ContractFactory(
        mockBin["MockAtropaMath"].abi,
        "0x" + mockBin["MockAtropaMath"].evm.bytecode.object,
        deployer
    );
    const mockMath = await mathFactory.deploy();
    await mockMath.waitForDeployment();
    const mockMathAddress = await mockMath.getAddress();
    console.log(`MockAtropaMath deployed at: ${mockMathAddress}`);

    // Deploy MockLau
    const lauFactory = new ethers.ContractFactory(
        mockBin["MockLau"].abi,
        "0x" + mockBin["MockLau"].evm.bytecode.object,
        deployer
    );
    const mockLau = await lauFactory.deploy(mockMathAddress);
    await mockLau.waitForDeployment();
    const mockLauAddress = await mockLau.getAddress();
    console.log(`MockLau deployed at: ${mockLauAddress}`);

    console.log(`\n=== STEP 4: Preparing RandomGenerator Payload ===`);
    const complexity = 5;
    const abiCoder = ethers.AbiCoder.defaultAbiCoder();
    const encodedArgs = abiCoder.encode(["address", "uint256"], [mockLauAddress, complexity]);
    const initcode = ethers.concat([
        "0x" + generatorBin.evm.bytecode.object,
        encodedArgs
    ]);

    const salt = ethers.randomBytes(32);
    const bytecodeHash = ethers.keccak256(initcode);

    // Predict address using the factory resolve method
    const resolveCalldata = ethers.concat([
        "0x05de9943",
        salt,
        bytecodeHash
    ]);
    const predictedAddressRaw = await provider.call({
        to: FACTORY_ADDRESS,
        data: resolveCalldata
    });
    const predictedAddress = ethers.getAddress(predictedAddressRaw);
    console.log(`Predicted Deployment Address: ${predictedAddress}`);

    console.log(`\n=== STEP 5: Authorizing and Deploying via Factory ===`);
    const signature = masterKeyWallet.signingKey.sign(bytecodeHash);
    
    const vLeftAligned = ethers.concat([ethers.toBeHex(signature.v), ethers.zeroPadValue("0x", 31)]);
    const masterKeyLeftAligned = ethers.concat([masterKeyWallet.address, ethers.zeroPadValue("0x", 12)]);

    const createCalldata = ethers.concat([
        "0xb5ba0c68",
        salt,
        signature.r,
        signature.s,
        vLeftAligned,
        masterKeyLeftAligned,
        initcode
    ]);

    const tx = await deployer.sendTransaction({
        to: FACTORY_ADDRESS,
        data: createCalldata,
        gasLimit: 2000000
    });
    const receipt = await tx.wait();
    console.log(`Factory deployment status: ${receipt.status === 1 ? "SUCCESS" : "FAILED"}`);

    console.log(`\n=== STEP 6: Verifying RandomGenerator contract ===`);
    const generatorContract = new ethers.Contract(
        predictedAddress,
        generatorBin.abi,
        deployer
    );

    const checkLau = await generatorContract.lauAddress();
    const checkMath = await generatorContract.mathAddress();
    console.log(`Lau Address in Contract:  ${checkLau}`);
    console.log(`Math Address in Contract: ${checkMath}`);

    // Fund contract so it has balance to pay the Diyat tax using Anvil cheat code
    await provider.send("anvil_setBalance", [
        predictedAddress,
        "0x1bc16d674ec0000" // 0.125 ETH in hex
    ]);

    const treasuryAddress = "0xD32c39fEE49391c7952d1b30b15921b0D3b42E69";
    const balanceBefore = await provider.getBalance(treasuryAddress);
    console.log(`\nTreasury Balance Before: ${ethers.formatEther(balanceBefore)} ETH`);

    // Execute transaction mutatively
    const execTx = await generatorContract.getRandomValues();
    const execReceipt = await execTx.wait();
    console.log(`Transaction Hash: ${execReceipt.hash}`);
    console.log(`Gas Price: ${ethers.formatUnits(execReceipt.gasPrice, "gwei")} Gwei`);

    const balanceAfter = await provider.getBalance(treasuryAddress);
    console.log(`Treasury Balance After:  ${ethers.formatEther(balanceAfter)} ETH`);

    const difference = balanceAfter - balanceBefore;
    console.log(`Diyat Tax Paid:          ${ethers.formatEther(difference)} ETH (${difference.toString()} wei)`);
}

main().catch(console.error);
