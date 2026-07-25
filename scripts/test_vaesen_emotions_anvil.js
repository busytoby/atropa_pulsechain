const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

function getContractArtifact(filename, contractName) {
    const filePath = path.join(__dirname, `../Wallet/bin/Contracts/${filename}.json`);
    if (!fs.existsSync(filePath)) {
        throw new Error(`Artifact file ${filename}.json not found at ${filePath}`);
    }
    const fileContent = JSON.parse(fs.readFileSync(filePath, "utf8"));
    for (const key of Object.keys(fileContent.contracts)) {
        if (key.endsWith(`:${contractName}`)) {
            return fileContent.contracts[key];
        }
    }
    throw new Error(`Artifact for ${contractName} not found in ${filename}`);
}

async function main() {
    console.log("=== AUNCIENT EMOTION QUANTIZATION TEST SUITE ===");
    console.log("Connecting to local EVM Anvil node...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Failed to connect to Anvil. Make sure Anvil is running at http://127.0.0.1:8545");
        process.exit(1);
    }
    const deployer = signers[0];
    console.log(`Using account: ${deployer.address}`);

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const localhost = config.networks.localhost;

    const lauFactoryAddr = localhost.LAUFactory;
    const mapAddr = localhost.MAP;
    const xiaAddr = localhost.XIA;
    const xieAddr = localhost.XIE;
    const ziAddr = localhost.ZI;

    if (!lauFactoryAddr || !mapAddr) {
        console.error("Missing LAUFactory or MAP in user_config.json");
        process.exit(1);
    }

    const factoryArtifact = getContractArtifact("11c_laufactory.sol", "LAUFactory");
    const factory = new ethers.Contract(lauFactoryAddr, factoryArtifact.abi, deployer);

    const mapArtifact = getContractArtifact("map.sol", "MAP");
    const mapContract = new ethers.Contract(mapAddr, mapArtifact.abi, deployer);

    // List of Vaesen emotions
    const emotions = ["AFFECTION", "FEAR", "ANGER", "JOY", "SADNESS"];
    const deployedQings = [];

    for (const emotion of emotions) {
        console.log(`\n--- Processing Emotion: ${emotion} ---`);
        
        // 1. Create LAU Token for the emotion
        console.log(`Deploying LAU Token for ${emotion}...`);
        const txLau = await factory.New(emotion, emotion);
        const receiptLau = await txLau.wait();
        
        let lauAddress = null;
        if (receiptLau.logs && receiptLau.logs.length > 0) {
            const log = receiptLau.logs.find(x => x.address && x.address.toLowerCase() !== lauFactoryAddr.toLowerCase());
            lauAddress = log ? log.address : receiptLau.logs[0].address;
        }
        if (!lauAddress) {
            throw new Error(`Failed to resolve LAU address for ${emotion}`);
        }
        console.log(` -> Deployed LAU Token address: ${lauAddress}`);

        // Set username to finalize setup
        const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
        const lauContract = new ethers.Contract(lauAddress, lauArtifact.abi, deployer);
        const txName = await lauContract["Username(string)"]("mariarahel");
        await txName.wait();

        // 2. Create QING for the emotion token via MAP
        console.log(`Deploying QING for ${emotion} via MAP...`);
        const txQing = await mapContract.New(lauAddress);
        const receiptQing = await txQing.wait();

        let qingAddress = null;
        const newQingLog = receiptQing.logs.find(x => x.topics[0] === "0x692bc76be0a4b0d71d90e36ee97b1f3c4141095ea3c22b01b9f620da2e4b5e2e");
        if (newQingLog) {
            const decoded = mapContract.interface.decodeEventLog("NewQing", newQingLog.data, newQingLog.topics);
            qingAddress = decoded[0];
        }
        if (!qingAddress) {
            // Fallback: use first log address
            qingAddress = receiptQing.logs[0].address;
        }
        console.log(` -> Deployed QING address: ${qingAddress}`);
        deployedQings.push({ emotion, qingAddress, lauAddress });
    }

    // 3. Test React to increase Epibars and Hypobars to > 0
    let activeYue = localhost.activeYue || localhost.YUE;
    if (!activeYue || activeYue === "0x0000000000000000000000000000000000000000") {
        console.log("\nNo active YUE session in config. Querying SEI to resolve...");
        const seiAddr = localhost.SEI;
        if (seiAddr) {
            const seiArtifact = getContractArtifact("01_sei.sol", "SEI");
            const sei = new ethers.Contract(seiAddr, seiArtifact.abi, deployer);
            try {
                const session = await sei.Chi();
                activeYue = session[0];
            } catch (e) {}
        }
    }

    if (activeYue && activeYue !== "0x0000000000000000000000000000000000000000") {
        console.log(`\nConnecting to active YUE contract at: ${activeYue}`);
        const yueArtifact = getContractArtifact("yue.sol", "YUE");
        const yue = new ethers.Contract(activeYue, yueArtifact.abi, deployer);

        for (const item of deployedQings) {
            console.log(`\nTriggering React on YUE for ${item.emotion} Qing (${item.qingAddress})...`);
            
            // Mint some LAU tokens to the user on this emotion token so balance > 0
            // and setup mapping
            const qingArtifact = getContractArtifact("03_qing.sol", "QING");
            const qingContract = new ethers.Contract(item.qingAddress, qingArtifact.abi, deployer);
            
            try {
                const joinTx = await qingContract.Join(item.lauAddress);
                await joinTx.wait();
            } catch (e) {
                console.log("Join skipped or failed:", e.message);
            }

            const reactTx = await yue.React(item.qingAddress);
            await reactTx.wait();

            const bars = await yue.Bar(item.qingAddress);
            console.log(` -> Hypobar: ${bars[0].toString()} Gram`);
            console.log(` -> Epibar:  ${bars[1].toString()} Gram`);
            if (bars[0] > 0n && bars[1] > 0n) {
                console.log(` -> SUCCESS: Epibar and Hypobar are both > 0!`);
            } else {
                console.log(` -> WARNING: Epibar/Hypobar weights did not increase.`);
            }
        }
    } else {
        console.log("\nSkipped React tests: No active YUE contract was resolved.");
    }

    // 4. Test Soeng Contract Outputs
    console.log("\n--- Querying Intermediate Soeng Contract Outputs ---");
    if (xiaAddr && xieAddr && ziAddr) {
        const xiaArtifact = getContractArtifact("03_xia.sol", "XIA");
        const xieArtifact = getContractArtifact("04_xie.sol", "XIE");
        const ziArtifact = getContractArtifact("05_zi.sol", "ZI");

        const xia = new ethers.Contract(xiaAddr, xiaArtifact.abi, deployer);
        const xie = new ethers.Contract(xieAddr, xieArtifact.abi, deployer);
        const zi = new ethers.Contract(ziAddr, ziArtifact.abi, deployer);

        for (const item of deployedQings) {
            console.log(`\nQuerying Soeng outputs for ${item.emotion} Qing (${item.qingAddress})...`);
            try {
                const charge = await xia.Charge(item.qingAddress);
                console.log(` -> XIA.Charge: ${charge.toString()}`);
                
                const power = await xie.Power(item.qingAddress);
                console.log(` -> XIE.Power -> Charge: ${power[0].toString()}, Omicron: ${power[1].toString()}, Omega: ${power[2].toString()}`);
                
                const spin = await zi.Spin(item.qingAddress);
                console.log(` -> ZI.Spin -> Iota: ${spin[0].toString()}, Omicron: ${spin[1].toString()}, Omega: ${spin[2].toString()}, Eta: ${spin[3].toString()}`);
                
                if (charge > 0n && power[0] > 0n && spin[0] > 0n) {
                    console.log(` -> SUCCESS: All intermediate Soeng outputs are > 0!`);
                }
            } catch (e) {
                console.error(`Error querying Soeng contract for ${item.emotion}:`, e.message);
            }
        }
    } else {
        console.log("Skipped Soeng tests: XIA, XIE, or ZI addresses not configured.");
    }

    console.log("\n=== EMOTION QUANTIZATION TEST SUITE COMPLETED ===");
}

main().catch(console.error);
