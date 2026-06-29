const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

async function main() {
    console.log("=== Testing Yul-Native Folklore VM Execution ===");
    
    // Load compiled bytecode outputs
    const compiled = JSON.parse(fs.readFileSync(path.join(__dirname, "../frontend/compiled_yul.json"), "utf8"));
    const kernelBin = compiled.unix1;
    const utilsBin = compiled.unix1_utils;
    const shBin = compiled.sh;
    
    // Connect to local Node
    const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
    const signer = await provider.getSigner(0);
    
    console.log("[DEPLOY] Deploying Unix1 OS Contracts...");
    const kernelFactory = new ethers.ContractFactory([], kernelBin, signer);
    const kernel = await kernelFactory.deploy();
    await kernel.waitForDeployment();
    const kernelAddr = await kernel.getAddress();
    console.log(`  -> Kernel deployed at: ${kernelAddr}`);
    
    const utilsFactory = new ethers.ContractFactory([], utilsBin, signer);
    const utils = await utilsFactory.deploy();
    await utils.waitForDeployment();
    const utilsAddr = await utils.getAddress();
    console.log(`  -> Utilities deployed at: ${utilsAddr}`);

    // Create target file name hash for test binary
    const filenameHash = ethers.zeroPadValue(ethers.toUtf8Bytes("folklore_test.bin"), 32);

    // Folklore CPU Instructions:
    // Inst 0: Load immediate 5 to register 2 (op=1, rDst=2, val=5)
    const inst0 = "0x01020005" + "0".repeat(56);
    // Inst 1: Load immediate 10 to register 3 (op=1, rDst=3, val=10)
    const inst1 = "0x0103000a" + "0".repeat(56);
    // Inst 2: Add registers 2 and 3 and store in register 4 (op=2, rDst=4, rSrc1=2, val=3)
    const inst2 = "0x02040203" + "0".repeat(56);
    // Inst 3: Load immediate 15 to register 5 (op=1, rDst=5, val=15)
    const inst3 = "0x0105000f" + "0".repeat(56);

    console.log("[WRITE] Compiling Folklore test binary into Unix1 filesystem...");
    const compileSelector = "0x636f6d70";
    const compileData = ethers.concat([
        compileSelector,
        ethers.AbiCoder.defaultAbiCoder().encode(
            ["bytes32", "bytes32", "bytes32", "bytes32", "bytes32"],
            [filenameHash, inst0, inst1, inst2, inst3]
        )
    ]);
    const compileTx = await signer.sendTransaction({ to: utilsAddr, data: compileData });
    await compileTx.wait();
    console.log("  -> Binary written successfully.");

    console.log("[EXEC] Registering active binary with sys_exec on Kernel...");
    const execSelector = "0x5f2f5361";
    const execData = ethers.concat([
        execSelector,
        ethers.AbiCoder.defaultAbiCoder().encode(["uint256", "bytes32"], [0, filenameHash])
    ]);
    const execTx = await signer.sendTransaction({ to: kernelAddr, data: execData });
    await execTx.wait();
    
    console.log("[STEP] Executing 4 Folklore CPU instruction cycles via sys_step...");
    const stepSelector = "0x00c71a91";
    for (let step = 0; step < 4; step++) {
        const stepData = ethers.concat([
            stepSelector,
            ethers.AbiCoder.defaultAbiCoder().encode(["uint256"], [0])
        ]);
        const stepTx = await signer.sendTransaction({ to: kernelAddr, data: stepData });
        await stepTx.wait();
        console.log(`  -> Cycle ${step} stepped.`);
    }

    console.log("[VERIFY] Querying Kernel state table...");
    const stateSelector = "0x2e0f40cf";
    const stateResult = await provider.call({
        to: kernelAddr,
        data: stateSelector
    });
    
    console.log(`[SUCCESS] Folklore VM execution test completed. Kernel State Hex: ${stateResult}`);
}

main().catch(console.error);
