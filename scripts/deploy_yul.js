const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const CONTRACTS_TO_DEPLOY = [
    { name: "musicMaker_v2", path: "../solidity/bin/musicMaker.yul", configKey: "musicMakerAddress" },
    { name: "diskSystem_v5", path: "../solidity/bin/diskSystem.yul", configKey: "diskSystemAddress" },
    { name: "genesis", path: "../solidity/bin/genesis.yul", configKey: "genesisAddress" },
    { name: "acousticOracle", path: "../solidity/bin/acousticOracle.yul", configKey: "acousticOracleAddress" },
    { name: "mathCoprocessor", path: "../solidity/bin/mathCoprocessor.yul", configKey: "mathCoprocessorAddress" },
    { name: "gameCoprocessor", path: "../solidity/bin/gameCoprocessor.yul", configKey: "gameCoprocessorAddress" },
    { name: "cpu6502_v18", path: "../solidity/bin/cpu6502.yul", configKey: "cpu6502Address" },
    { name: "graphicsSystem_v2", path: "../solidity/bin/graphicsSystem.yul", configKey: "graphicsSystemAddress" },
    { name: "speechSynthesizer", path: "../solidity/bin/speechSynthesizer.yul", configKey: "speechSynthesizerAddress" },
    { name: "zmachine", path: "../solidity/bin/zmachine.yul", configKey: "zmachineAddress" },
    { name: "zmachineParser", path: "../solidity/bin/zmachineParser.yul", configKey: "zmachineParserAddress" },
    { name: "keySystem", path: "../solidity/bin/keySystem.yul", configKey: "keySystemAddress" },
    { name: "bGraph_v1", path: "../solidity/bin/bGraph.yul", configKey: "bGraphAddress" },
    { name: "folklore", path: "../solidity/bin/folklore.yul", configKey: "folkloreAddress" },
    { name: "diyat", path: "../solidity/bin/diyat.yul", configKey: "diyatAddress" },
    { name: "ledger_v2", path: "../solidity/bin/ledger.yul", configKey: "ledgerAddress" },
    { name: "biorhythm", path: "../solidity/bin/biorhythm.yul", configKey: "biorhythmAddress" },
    { name: "dragonsLair", path: "../solidity/bin/dragonsLair.yul", configKey: "dragonsLairAddress" },
    { name: "starCastle", path: "../solidity/bin/starCastle.yul", configKey: "starCastleAddress" }
];

// Compile Yul contract using solc
function compileYul(yulPath) {
    const absolutePath = path.resolve(__dirname, yulPath);
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation in solc output for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let deployer, masterKeyWallet;

    try {
        const signers = await provider.listAccounts();
        deployer = signers[0];
        // We use accounts[1] as the authorization master key
        masterKeyWallet = new ethers.Wallet(
            "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d"
        ).connect(provider);
        console.log(`Connected. Deployer: ${deployer.address}`);
    } catch (err) {
        console.error("Could not connect to local EVM. Make sure Anvil is running on port 8545.");
        process.exit(1);
    }

    // Load config
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    let factoryAddress = config.networks.localhost.immutableFactoryAddress;

    // Check if factory is already deployed
    let factoryExists = false;
    if (factoryAddress) {
        try {
            const code = await provider.getCode(factoryAddress);
            if (code !== "0x") {
                factoryExists = true;
                console.log(`Singular Factory already deployed at: ${factoryAddress}`);
            }
        } catch (err) {
            // Address invalid or node reset
        }
    }

    if (!factoryExists) {
        console.log("Factory not found or reset. Compiling and deploying singular factory...");
        const factoryBytecode = compileYul("../solidity/bin/immutableFactory.yul");
        
        const factoryTx = await deployer.sendTransaction({
            data: factoryBytecode,
            gasLimit: 2000000
        });
        const receipt = await factoryTx.wait();
        factoryAddress = receipt.contractAddress;
        
        // Update config
        config.networks.localhost.immutableFactoryAddress = factoryAddress;
        console.log(`Singular Factory deployed at: ${factoryAddress}`);
    }

    console.log("\n=== Deploying Yul Contracts via CREATE2 Factory ===");

    for (const contract of CONTRACTS_TO_DEPLOY) {
        console.log(`\nProcessing: ${contract.name}...`);
        const absoluteYulPath = path.resolve(__dirname, contract.path);
        if (!fs.existsSync(absoluteYulPath)) {
            console.log(`  File ${contract.path} does not exist. Skipping deployment and preserving existing config.`);
            continue;
        }
        const bytecode = compileYul(contract.path);
        const bytecodeHash = ethers.keccak256(bytecode);

        // Calculate CREATE2 predicted address using a deterministic salt derived from the contract name
        const salt = ethers.id(contract.name);
        
        // resolve(bytes32 salt, bytes32 bytecodeHash) -> Selector: 0x05de9943
        const resolveSelector = "0x05de9943";
        const resolveCalldata = ethers.concat([resolveSelector, salt, bytecodeHash]);
        const predictedAddressRaw = await provider.call({
            to: factoryAddress,
            data: resolveCalldata
        });
        const predictedAddress = ethers.getAddress(predictedAddressRaw);
        console.log(`  Predicted Address: ${predictedAddress}`);

        // Check if already deployed
        const existingCode = await provider.getCode(predictedAddress);
        if (existingCode !== "0x") {
            console.log(`  Contract ${contract.name} is already deployed at ${predictedAddress}. Skipping.`);
            config.networks.localhost[contract.configKey] = predictedAddress;
            continue;
        }

        // Sign bytecode hash
        const signature = masterKeyWallet.signingKey.sign(bytecodeHash);
        const vLeftAligned = ethers.concat([ethers.toBeHex(signature.v), ethers.zeroPadValue("0x", 31)]);
        const masterKeyLeftAligned = ethers.concat([masterKeyWallet.address, ethers.zeroPadValue("0x", 12)]);

        // createAuthorized(bytes32 salt, bytes32 r, bytes32 s, uint8 v, address masterKey) + bytecode
        const createCalldata = ethers.concat([
            "0xb5ba0c68",
            salt,
            signature.r,
            signature.s,
            vLeftAligned,
            masterKeyLeftAligned,
            bytecode
        ]);

        const deployTx = await deployer.sendTransaction({
            to: factoryAddress,
            data: createCalldata,
            gasLimit: 25000000
        });
        await deployTx.wait();
        console.log(`  Deployed: ${contract.name} successfully at ${predictedAddress}`);

        // Save address in config
        config.networks.localhost[contract.configKey] = predictedAddress;
    }

    // Deploy Batcher
    console.log("\nCompiling and deploying Batcher...");
    const batcherArtifactPath = path.join(__dirname, "../solidity/Batcher.json");
    if (fs.existsSync(batcherArtifactPath)) {
        const batcherArtifact = JSON.parse(fs.readFileSync(batcherArtifactPath, "utf8"));
        const batcherBytecode = "0x" + batcherArtifact.contracts["solidity/Batcher.sol:Batcher"].bin;
        
        let batcherAddress = config.networks.localhost.batcherAddress;
        let batcherExists = false;
        if (batcherAddress) {
            try {
                const code = await provider.getCode(batcherAddress);
                if (code !== "0x") {
                    batcherExists = true;
                    console.log(`Batcher already deployed at: ${batcherAddress}`);
                }
            } catch (e) {}
        }
        if (!batcherExists) {
            const batcherTx = await deployer.sendTransaction({
                data: batcherBytecode
            });
            const receipt = await batcherTx.wait();
            batcherAddress = receipt.contractAddress;
            console.log(`Batcher deployed at: ${batcherAddress}`);
        }
        config.networks.localhost.batcherAddress = batcherAddress;
    } else {
        console.warn("solidity/Batcher.json not found! Skipping batcher deployment.");
    }

    // Bind Parser to ZMachine
    const zmachineAddress = config.networks.localhost.zmachineAddress;
    const zmachineParserAddress = config.networks.localhost.zmachineParserAddress;
    if (zmachineAddress && zmachineParserAddress) {
        console.log("\nLinking ZMachineParser to ZMachine...");
        const zmContract = new ethers.Contract(zmachineAddress, [
            "function bindParserAddress(address parser) public returns (bool)"
        ], deployer);
        await (await zmContract.bindParserAddress(zmachineParserAddress)).wait();
        console.log("  Successfully linked ZMachineParser!");
    }

    // Bind MathCoprocessor to CPU6502 for deployer
    const cpu6502Address = config.networks.localhost.cpu6502Address;
    const mathCoprocessorAddress = config.networks.localhost.mathCoprocessorAddress;
    if (cpu6502Address && mathCoprocessorAddress) {
        console.log("\nLinking MathCoprocessor to CPU6502...");
        const cpuContract = new ethers.Contract(cpu6502Address, [
            "function pokeUser(address user, uint256 addr, uint256 val) public returns (uint256)"
        ], deployer);
        await (await cpuContract.pokeUser(deployer.address, 54697, mathCoprocessorAddress)).wait();
        console.log("  Successfully linked MathCoprocessor!");
    }

    // Bind GameCoprocessor to CPU6502 for deployer
    const gameCoprocessorAddress = config.networks.localhost.gameCoprocessorAddress;
    if (cpu6502Address && gameCoprocessorAddress) {
        console.log("\nLinking GameCoprocessor to CPU6502...");
        const cpuContract = new ethers.Contract(cpu6502Address, [
            "function pokeUser(address user, uint256 addr, uint256 val) public returns (uint256)"
        ], deployer);
        await (await cpuContract.pokeUser(deployer.address, 54698, gameCoprocessorAddress)).wait();
        console.log("  Successfully linked GameCoprocessor!");
    }

    // Write updated config back to file
    fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
    console.log("\n=== DEPLOYMENT AND CONFIG REGISTRY COMPLETE ===");
}

main().catch(console.error);
