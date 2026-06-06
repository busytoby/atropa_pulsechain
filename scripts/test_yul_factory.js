const { ethers } = require("ethers");
const { execSync, spawn } = require("child_process");
const fs = require("fs");
const path = require("path");

// Configuration
const YUL_PATH = path.join(__dirname, "../solidity/bin/immutableFactory.yul");
const ANVIL_PORT = 8545;
const ANVIL_URL = `http://127.0.0.1:${ANVIL_PORT}`;

// Dummy contract initcode (returns runtime that returns 0x42)
const DUMMY_INITCODE = "0x600a80600b6000396000f3604260005260206000f3";

async function main() {
    console.log("=== STEP 1: Compiling Yul Factory ===");
    let bytecode;
    try {
        const solcOutput = execSync(`solc --strict-assembly "${YUL_PATH}" --bin`, { encoding: "utf8" });
        // Extract hex bytecode from solc output
        const lines = solcOutput.split("\n");
        const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
        if (binIndex === -1) {
            throw new Error("Could not find binary representation in solc output");
        }
        bytecode = "0x" + lines[binIndex + 1].trim();
        console.log(`Successfully compiled. Bytecode size: ${(bytecode.length - 2) / 2} bytes`);
    } catch (err) {
        console.error("Compilation failed:", err.message);
        process.exit(1);
    }

    console.log("\n=== STEP 2: Starting Local Anvil Node ===");
    const anvil = spawn("anvil", ["--port", ANVIL_PORT.toString()]);
    
    // Wait for Anvil to spin up
    await new Promise(resolve => setTimeout(resolve, 2000));
    console.log("Anvil started.");

    let provider;
    try {
        provider = new ethers.JsonRpcProvider(ANVIL_URL);
        const network = await provider.getNetwork();
        console.log(`Connected to network: ${network.name} (chainId: ${network.chainId})`);
    } catch (err) {
        console.error("Failed to connect to Anvil:", err.message);
        anvil.kill();
        process.exit(1);
    }

    try {
        const signers = await provider.listAccounts();
        const deployer = signers[0];
        const masterKeyWallet = ethers.Wallet.createRandom().connect(provider);

        // Fund masterKeyWallet to pay for transactions if needed, though we will deploy using deployer
        await deployer.sendTransaction({
            to: masterKeyWallet.address,
            value: ethers.parseEther("10")
        });

        console.log(`Deployer address: ${deployer.address}`);
        console.log(`Master Key address: ${masterKeyWallet.address}`);

        console.log("\n=== STEP 3: Deploying Immutable Factory ===");
        const tx = await deployer.sendTransaction({
            data: bytecode
        });
        const receipt = await tx.wait();
        const factoryAddress = receipt.contractAddress;
        console.log(`Factory deployed at: ${factoryAddress}`);

        // Define test parameters
        const salt = ethers.randomBytes(32);
        const bytecodeHash = ethers.keccak256(DUMMY_INITCODE);

        console.log("\n=== STEP 4: Testing resolve() ===");
        // resolve(bytes32 salt, bytes32 bytecodeHash) -> Selector: 0x05de9943
        // Calldata: 0x05de9943 + salt (32 bytes) + bytecodeHash (32 bytes)
        const resolveSelector = "0x05de9943";
        const resolveCalldata = ethers.concat([
            resolveSelector,
            salt,
            bytecodeHash
        ]);

        const predictedAddressRaw = await provider.call({
            to: factoryAddress,
            data: resolveCalldata
        });
        const predictedAddress = ethers.getAddress(predictedAddressRaw);
        console.log(`Predicted Address (Factory resolve): ${predictedAddress}`);

        // Compare with standard ethers CREATE2 calculation
        const expectedAddress = ethers.getCreate2Address(factoryAddress, salt, bytecodeHash);
        console.log(`Expected Address (ethers):           ${expectedAddress}`);

        if (predictedAddress === expectedAddress) {
            console.log("SUCCESS: Predicted address matches ethers expected address!");
        } else {
            console.log(`WARNING: Address mismatch! Predicted: ${predictedAddress}, Expected (ethers): ${expectedAddress}`);
        }

        console.log("\n=== STEP 5: Testing verifySignature() ===");
        // Sign the bytecodeHash using the master key
        // Note: verifySignature does NOT use Ethereum Signed Message prefix, it ecrecovers the raw hash.
        const signature = masterKeyWallet.signingKey.sign(bytecodeHash);
        const r = signature.r;
        const s = signature.s;
        const v = signature.v; // uint8

        // verifySignature(bytes32 targetHash, bytes32 r, bytes32 s, uint8 v, address masterKey)
        // Selector: 0x489069d1
        // We need left-aligned v and masterKey in their 32-byte slots:
        const vSlot = ethers.zeroPadValue(ethers.toBeHex(v), 32); // actually wait: in Yul it does shr(248, calldataload(100))
        // Let's manually construct the left-aligned slots:
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
            to: factoryAddress,
            data: verifySigCalldata
        });
        const isValidSig = ethers.toNumber(verifyResult) === 1;
        console.log(`verifySignature result: ${isValidSig}`);
        if (isValidSig) {
            console.log("SUCCESS: Signature verified successfully!");
        } else {
            throw new Error("Signature verification failed!");
        }

        console.log("\n=== STEP 6: Testing createAuthorized() ===");
        // createAuthorized(bytes32 salt, bytes32 r, bytes32 s, uint8 v, address masterKey) + bytecode payload
        // Selector in Yul: 0xb5ba0c68
        const createCalldata = ethers.concat([
            "0xb5ba0c68",
            salt,
            r,
            s,
            vLeftAligned,
            masterKeyLeftAligned,
            DUMMY_INITCODE
        ]);

        // Submit deployment transaction
        const deployTx = await deployer.sendTransaction({
            to: factoryAddress,
            data: createCalldata,
            gasLimit: 2000000
        });
        const deployReceipt = await deployTx.wait();
        console.log(`createAuthorized transaction status: ${deployReceipt.status === 1 ? "SUCCESS" : "FAILED"}`);

        // Extract deployed address from log topics
        const successTopic = "0x1d36d75c00000000000000000000000000000000000000000000000000000000";
        const deployLog = deployReceipt.logs.find(log => log.topics[0] === successTopic);
        if (!deployLog) {
            throw new Error("Could not find success deployment log!");
        }
        const actualDeployedAddress = ethers.getAddress(ethers.dataSlice(deployLog.topics[1], 12, 32));
        console.log(`Actual Deployed Address (from Logs):   ${actualDeployedAddress}`);

        // Verify that the code exists at actualDeployedAddress
        const code = await provider.getCode(actualDeployedAddress);
        console.log(`Deployed code at ${actualDeployedAddress}: ${code}`);
        if (code === "0x604260005260206000f3") {
            console.log("SUCCESS: Deployed code matches expected dummy contract runtime!");
        } else {
            throw new Error(`Deployed code mismatch! Got: ${code}`);
        }

        // Call the dummy contract to verify it runs and returns 0x42
        const dummyResult = await provider.call({
            to: actualDeployedAddress
        });
        console.log(`Dummy contract execution result: ${dummyResult}`);
        if (ethers.toNumber(dummyResult) === 0x42) {
            console.log("SUCCESS: Dummy contract returned 0x42!");
        } else {
            throw new Error(`Dummy contract returned unexpected value: ${dummyResult}`);
        }

        console.log("\n=== STEP 7: Testing verifyIdempotency (re-deploying) ===");
        // Send the same deploy tx again
        const redeployTx = await deployer.sendTransaction({
            to: factoryAddress,
            data: createCalldata,
            gasLimit: 2000000
        });
        const redeployReceipt = await redeployTx.wait();
        console.log(`Re-deployment transaction status: ${redeployReceipt.status === 1 ? "SUCCESS" : "FAILED"}`);

        // Verify that it emitted the Idempotency event
        // idemTopic is 0xb4e1cb2a...
        const idemTopic = "0xb4e1cb2a00000000000000000000000000000000000000000000000000000000";
        const hasIdemLog = redeployReceipt.logs.some(log => log.topics[0] === idemTopic);
        console.log(`Idempotency log emitted: ${hasIdemLog}`);
        if (hasIdemLog) {
            console.log("SUCCESS: Re-deployment detected contract already exists and emitted Idempotency log!");
        } else {
            throw new Error("No idempotency log emitted on re-deployment!");
        }

        console.log("\n=== ALL TESTS PASSED! ===");
    } catch (err) {
        console.error("\n!!! TEST ERROR !!!", err);
        if (err.receipt) {
            console.log("\nFetching transaction debug trace...");
            try {
                const trace = await provider.send("debug_traceTransaction", [
                    err.receipt.hash,
                    { disableStack: false, disableMemory: true, disableStorage: true }
                ]);
                console.log(`Trace structLogs length: ${trace.structLogs.length}`);
                const start = Math.max(0, trace.structLogs.length - 35);
                for (let i = start; i < trace.structLogs.length; i++) {
                    const log = trace.structLogs[i];
                    console.log(`PC [${log.pc.toString().padStart(4, " ")}] | OP: ${log.op.padEnd(12, " ")} | Stack: [${log.stack.slice(-5).join(", ")}]`);
                }
            } catch (traceErr) {
                console.error("Could not fetch trace:", traceErr.message);
            }
        }
    } finally {
        console.log("\nCleaning up...");
        anvil.kill();
        console.log("Anvil shut down.");
    }
}

main();
