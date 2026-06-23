const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Helper to load contract JSON output
function getContractArtifact(filename, contractName) {
    const filePath = path.join(__dirname, `../Wallet/bin/Contracts/${filename}.json`);
    const fileContent = JSON.parse(fs.readFileSync(filePath, "utf8"));
    // Format inside compile output is: file_path:contract_name
    for (const key of Object.keys(fileContent.contracts)) {
        if (key.endsWith(`:${contractName}`)) {
            return fileContent.contracts[key];
        }
    }
    throw new Error(`Artifact for ${contractName} not found in ${filename}`);
}

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log(`Deployer address: ${deployer.address}`);

    const deployedAddresses = {};

    async function deploy(filename, contractName, args = []) {
        console.log(`Deploying ${contractName}...`);
        const artifact = getContractArtifact(filename, contractName);
        const factory = new ethers.ContractFactory(artifact.abi, artifact.bin, deployer);
        const contract = await factory.deploy(...args);
        await contract.waitForDeployment();
        const address = await contract.getAddress();
        console.log(` -> ${contractName} deployed at: ${address}`);
        deployedAddresses[contractName] = address;
        return contract;
    }

    // 1. VMREQ
    await deploy("00b_vmreq.sol", "VMREQ");

    // 2. SHAFactory
    await deploy("02c_shafactory.sol", "SHAFactory");

    // 3. SHIOFactory
    await deploy("03c_shiofactory.sol", "SHIOFactory");

    // 4. YI
    await deploy("04_yi.sol", "YI", [
        deployedAddresses["SHAFactory"],
        deployedAddresses["SHIOFactory"],
        deployedAddresses["VMREQ"]
    ]);

    // 5. ZHENG
    await deploy("05_zheng.sol", "ZHENG", [deployedAddresses["YI"]]);

    // 6. ZHOU
    await deploy("06_zhou.sol", "ZHOU", [deployedAddresses["ZHENG"]]);

    // 7. YAU
    await deploy("07_yau.sol", "YAU", [deployedAddresses["ZHOU"]]);

    // 8. YANG
    await deploy("08_yang.sol", "YANG", [deployedAddresses["YAU"]]);

    // 9. SIU
    await deploy("09_siu.sol", "SIU", [deployedAddresses["YANG"]]);

    // 10. VOID
    await deploy("10_void.sol", "VOID", [deployedAddresses["SIU"]]);

    // 11. ATTRIBUTE
    await deploy("attribute.sol", "ATTRIBUTE", [deployedAddresses["VOID"]]);

    // 12. LAUFactory
    await deploy("11c_laufactory.sol", "LAUFactory", [deployedAddresses["VOID"]]);

    // 13. STRINGLIB
    await deploy("stringlib.sol", "STRINGLIB", [deployedAddresses["VOID"]]);

    // 15. COREREACTIONSLIB
    await deploy("reactions_core.sol", "COREREACTIONSLIB", [deployedAddresses["VOID"]]);

    // 16. CHO
    const cho = await deploy("01_cho.sol", "CHO", [deployedAddresses["VOID"]]);

    // 17. Register CHO for talk
    console.log("Registering CHO for Talk in COREREACTIONSLIB...");
    const corereactions = new ethers.Contract(deployedAddresses["COREREACTIONSLIB"], getContractArtifact("reactions_core.sol", "COREREACTIONSLIB").abi, deployer);
    const txRegister = await corereactions.RegisterChoForTalk(deployedAddresses["CHO"]);
    await txRegister.wait();
    console.log("CHO Registered.");

    // 14. Deploy Hecke (Real Contract, No Mocks)
    console.log("Deploying Hecke...");
    const heckeArtifact = getContractArtifact("heckemeridians.sol", "Hecke");
    const heckeFactory = new ethers.ContractFactory(heckeArtifact.abi, heckeArtifact.bin, deployer);
    const hecke = await heckeFactory.deploy(deployedAddresses["CHO"]);
    await hecke.waitForDeployment();
    const heckeAddress = await hecke.getAddress();
    deployedAddresses["HECKE"] = heckeAddress;
    console.log(` -> Hecke deployed at: ${heckeAddress}`);

    // 18. Deploy MAP (Requires ChoAddress, HeckeAddress)
    const map = await deploy("map.sol", "MAP", [deployedAddresses["CHO"], deployedAddresses["HECKE"]]);

    // Deploy a test LAU to construct QINGs
    console.log("Deploying a test LAU token...");
    const laufactory = new ethers.Contract(deployedAddresses["LAUFactory"], getContractArtifact("11c_laufactory.sol", "LAUFactory").abi, deployer);
    const txLau = await laufactory.New("User Token 1", "USERTOKEN1");
    const receiptLau = await txLau.wait();
    const lauAddress = receiptLau.logs[0].address || receiptLau.logs[0].args[1];
    console.log(` -> Test LAU Token deployed at: ${lauAddress}`);
    deployedAddresses["TestLAU"] = lauAddress;

    // 19. Deploy QI
    // QING requires an integrative address. We deploy a QING via the MAP contract acting as factory:
    console.log("Deploying VOID QING via MAP.New()...");
    const txQing = await map.New(deployedAddresses["VOID"]);
    const receiptQing = await txQing.wait();
    
    // Parse the NewQing event to find the address of the newly deployed QING
    const newQingEvent = receiptQing.logs.find(x => x.fragment && x.fragment.name === "NewQing") || receiptQing.logs[0];
    const qingAddress = newQingEvent.args ? newQingEvent.args[0] : newQingEvent.address;
    console.log(` -> VOID QING deployed at: ${qingAddress}`);
    deployedAddresses["VOID_QING"] = qingAddress;

    await deploy("01_qi.sol", "QI", [deployedAddresses["VOID_QING"]]);

    // 20. MAI
    await deploy("02_mai.sol", "MAI", [deployedAddresses["QI"]]);

    // 21. XIA
    await deploy("03_xia.sol", "XIA", [deployedAddresses["MAI"]]);

    // 22. XIE
    await deploy("04_xie.sol", "XIE", [deployedAddresses["XIA"]]);

    // 23. CHAN
    await deploy("01_chan.sol", "CHAN", [deployedAddresses["XIE"]]);

    // 24. SEI
    await deploy("01_sei.sol", "SEI", [deployedAddresses["CHAN"]]);

    // 25. CHOA
    await deploy("02_choa.sol", "CHOA", [deployedAddresses["SEI"]]);

    // 26. CHEON
    await deploy("02_cheon.sol", "CHEON", [deployedAddresses["SEI"]]);

    console.log("\n==================================================");
    console.log("ALL SYSTEMS & DOMAINS DEPLOYED TO LOCAL EVM:");
    console.log("==================================================");
    console.log(JSON.stringify(deployedAddresses, null, 2));
    console.log("==================================================");

    // Write to user_config.json
    if (fs.existsSync(CONFIG_PATH)) {
        const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
        config.networks.localhost = {
            ...config.networks.localhost,
            ...deployedAddresses
        };
        fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
        console.log("Saved deployed addresses to config/user_config.json under localhost.");
    }
}

main().catch(console.error);
