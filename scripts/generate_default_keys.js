const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const FACTORY_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
const MOCK_LAU_ADDRESS = "0x0165878A594ca255338adfa4d48449f69242Eb8F";

async function main() {
    console.log("Compiling RandomGenerator...");
    let generatorBin;
    try {
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
        generatorBin = generatorOutput.contracts["RandomGenerator.sol"]["RandomGenerator"];
    } catch (err) {
        console.error("Compilation failed:", err.message);
        process.exit(1);
    }

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    
    // We can use a random wallet or one of the signers as the master key
    // Let's use account 1 as the master key for easy signing
    const masterKeyWallet = new ethers.Wallet("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d").connect(provider);

    const complexity = 8;
    const abiCoder = ethers.AbiCoder.defaultAbiCoder();
    const encodedArgs = abiCoder.encode(["address", "uint256"], [MOCK_LAU_ADDRESS, complexity]);
    const initcode = ethers.concat([
        "0x" + generatorBin.evm.bytecode.object,
        encodedArgs
    ]);

    const salt = ethers.randomBytes(32);
    const bytecodeHash = ethers.keccak256(initcode);

    // Predict address
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

    // Sign payload
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

    // Send deploy tx (skip gas estimation using manual gasLimit)
    const tx = await deployer.sendTransaction({
        to: FACTORY_ADDRESS,
        data: createCalldata,
        gasLimit: 3000000
    });
    await tx.wait();

    // Query contract for the 8 random values
    const generatorContract = new ethers.Contract(
        predictedAddress,
        generatorBin.abi,
        provider
    );
    const randomValues = await generatorContract.getRandomValues();
    
    console.log("\n--- Generated 8 Random Keys ---");
    randomValues.forEach((val, i) => {
        console.log(`Key [${i}]: ${val}`);
    });
}

main().catch(console.error);
