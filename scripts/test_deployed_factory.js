const { ethers } = require("ethers");

const FACTORY_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
const PROVIDER_URL = "http://127.0.0.1:8545";

// Simple test contract initcode (returns runtime returning 0x42)
const DUMMY_INITCODE = "0x600a80600b6000396000f3604260005260206000f3";

async function main() {
    console.log(`Connecting to local EVM at ${PROVIDER_URL}...`);
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    
    // Setup signers
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    const masterKeyWallet = ethers.Wallet.createRandom().connect(provider);

    // Fund master key
    await deployer.sendTransaction({
        to: masterKeyWallet.address,
        value: ethers.parseEther("1")
    });

    console.log(`Deployer: ${deployer.address}`);
    console.log(`Master Key: ${masterKeyWallet.address}`);
    console.log(`Factory Address: ${FACTORY_ADDRESS}`);

    const salt = ethers.randomBytes(32);
    const bytecodeHash = ethers.keccak256(DUMMY_INITCODE);

    // ----------------------------------------------------
    // Test 1: resolve(bytes32,bytes32) -> address
    // Selector: 0x05de9943
    // ----------------------------------------------------
    console.log("\n--- Testing resolve() ---");
    const resolveSelector = "0x05de9943";
    const resolveCalldata = ethers.concat([
        resolveSelector,
        salt,
        bytecodeHash
    ]);
    const predictedAddressRaw = await provider.call({
        to: FACTORY_ADDRESS,
        data: resolveCalldata
    });
    const predictedAddress = ethers.getAddress(predictedAddressRaw);
    const expectedAddress = ethers.getCreate2Address(FACTORY_ADDRESS, salt, bytecodeHash);
    console.log(`Predicted Address: ${predictedAddress}`);
    console.log(`Expected (ethers): ${expectedAddress}`);
    if (predictedAddress === expectedAddress) {
        console.log("SUCCESS: resolve() calculated CREATE2 address correctly!");
    } else {
        console.log("FAILED: Address mismatch.");
    }

    // ----------------------------------------------------
    // Test 2: verifySignature(bytes32,bytes32,bytes32,uint8,address) -> bool
    // Selector: 0x489069d1
    // ----------------------------------------------------
    console.log("\n--- Testing verifySignature() ---");
    const signature = masterKeyWallet.signingKey.sign(bytecodeHash);
    const r = signature.r;
    const s = signature.s;
    const v = signature.v;

    const vLeftAligned = ethers.concat([ethers.toBeHex(v), ethers.zeroPadValue("0x", 31)]);
    const masterKeyLeftAligned = ethers.concat([masterKeyWallet.address, ethers.zeroPadValue("0x", 12)]);

    const verifySigCalldata = ethers.concat([
        "0x489069d1",
        bytecodeHash,
        r,
        s,
        vLeftAligned,
        masterKeyLeftAligned
    ]);
    const verifyResult = await provider.call({
        to: FACTORY_ADDRESS,
        data: verifySigCalldata
    });
    const isValid = ethers.toNumber(verifyResult) === 1;
    console.log(`verifySignature returned: ${isValid}`);
    if (isValid) {
        console.log("SUCCESS: verifySignature() passed!");
    } else {
        console.log("FAILED: verifySignature() rejected valid signature.");
    }

    // ----------------------------------------------------
    // Test 3: verifyIdempotency(address,bytes32) -> bool/address
    // Selector: 0x4b9e903d
    // ----------------------------------------------------
    console.log("\n--- Testing verifyIdempotency() (when not deployed) ---");
    const idemCalldata = ethers.concat([
        "0x4b9e903d",
        ethers.concat([predictedAddress, ethers.zeroPadValue("0x", 12)]),
        salt
    ]);
    const idemResultRaw = await provider.call({
        to: FACTORY_ADDRESS,
        data: idemCalldata
    });
    const idemResult = ethers.toNumber(idemResultRaw);
    console.log(`verifyIdempotency returned: ${idemResult}`);
    if (idemResult === 1) {
        console.log("SUCCESS: verifyIdempotency() correctly returns 1 when contract does not exist.");
    } else {
        console.log("FAILED.");
    }

    // ----------------------------------------------------
    // Test 4: executeDeployment(bytes,bytes32) -> address
    // Selector: 0x93d9b8f0
    // ----------------------------------------------------
    console.log("\n--- Testing executeDeployment() ---");
    // Layout: selector (4) + offset (32) + salt (32) + length (32) + bytecode
    const bytecodeOffset = ethers.zeroPadValue(ethers.toBeHex(64), 32);
    const bytecodeLength = ethers.zeroPadValue(ethers.toBeHex((DUMMY_INITCODE.length - 2) / 2), 32);
    
    const executeCalldata = ethers.concat([
        "0x93d9b8f0",
        bytecodeOffset,
        salt,
        bytecodeLength,
        DUMMY_INITCODE
    ]);

    const deployTx = await deployer.sendTransaction({
        to: FACTORY_ADDRESS,
        data: executeCalldata
    });
    const deployReceipt = await deployTx.wait();
    console.log(`executeDeployment status: ${deployReceipt.status === 1 ? "SUCCESS" : "FAILED"}`);

    const code = await provider.getCode(predictedAddress);
    console.log(`Deployed Code: ${code}`);
    if (code === "0x604260005260206000f3") {
        console.log("SUCCESS: Contract successfully deployed at CREATE2 address via executeDeployment()!");
    } else {
        console.log("FAILED.");
    }

    // ----------------------------------------------------
    // Test 5: verifyIdempotency() (when already deployed)
    // ----------------------------------------------------
    console.log("\n--- Testing verifyIdempotency() (when already deployed) ---");
    const idemResultRaw2 = await provider.call({
        to: FACTORY_ADDRESS,
        data: idemCalldata
    });
    const idemAddress = ethers.getAddress(ethers.dataSlice(idemResultRaw2, 12, 32));
    console.log(`verifyIdempotency returned address: ${idemAddress}`);
    if (idemAddress === predictedAddress) {
        console.log("SUCCESS: verifyIdempotency() returns deployed address when contract already exists!");
    } else {
        console.log("FAILED.");
    }

    // ----------------------------------------------------
    // Test 6: createAuthorized() (main flow)
    // Selector: 0xb5ba0c68
    // ----------------------------------------------------
    console.log("\n--- Testing createAuthorized() ---");
    const newSalt = ethers.randomBytes(32);
    const newPredictedAddress = ethers.getCreate2Address(FACTORY_ADDRESS, newSalt, bytecodeHash);
    const newSignature = masterKeyWallet.signingKey.sign(bytecodeHash);
    
    const newVLeftAligned = ethers.concat([ethers.toBeHex(newSignature.v), ethers.zeroPadValue("0x", 31)]);
    
    const createCalldata = ethers.concat([
        "0xb5ba0c68",
        newSalt,
        newSignature.r,
        newSignature.s,
        newVLeftAligned,
        masterKeyLeftAligned,
        DUMMY_INITCODE
    ]);

    const createTx = await deployer.sendTransaction({
        to: FACTORY_ADDRESS,
        data: createCalldata
    });
    const createReceipt = await createTx.wait();
    console.log(`createAuthorized status: ${createReceipt.status === 1 ? "SUCCESS" : "FAILED"}`);
    
    const newCode = await provider.getCode(newPredictedAddress);
    if (newCode === "0x604260005260206000f3") {
        console.log("SUCCESS: Authorized contract successfully deployed at predicted address!");
    } else {
        console.log("FAILED.");
    }
}

main().catch(console.error);
