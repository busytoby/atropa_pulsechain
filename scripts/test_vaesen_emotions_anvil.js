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

    // Set code of WITHOUTContract on Anvil to return 32 bytes of 0 (so balanceOf doesn'\''t revert)
    const withoutAddr = "0x173216Ed67eBF3E6767D86e8b3Ff32e0d64437bF";
    await provider.send("anvil_setCode", [withoutAddr, "0x60206000f3"]);
    console.log("Mocked WITHOUTContract bytecode on Anvil.");

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const localhost = config.networks.localhost;
    const choArtifact = getContractArtifact("01_cho.sol", "CHO");
    const cho = new ethers.Contract(localhost.CHO, choArtifact.abi, deployer);

    // Set code of VMREQ on Anvil using dynamically compiled MockVMREQ contract
    if (localhost.VMREQ) {
        const { execSync } = require("child_process");
        const mockSolPath = path.join(__dirname, "../tmp/MockVMREQ.sol");
        fs.writeFileSync(mockSolPath, `
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.21;
contract MockVMREQ {
    uint64 private counter = 10000000;
    function Random() public returns (uint64) {
        counter += 1;
        return 100000 + counter * 10000;
    }
    function modExp(uint256 _b, uint256 _e, uint256 _m) public pure returns (uint256) {
        if (_m == 0) return 1;
        if (_m == 1) return 1;
        uint256 result = 1;
        uint256 base = _b % _m;
        uint256 exp = _e;
        while (exp > 0) {
            if (exp % 2 == 1) {
                result = mulmod(result, base, _m);
            }
            base = mulmod(base, base, _m);
            exp /= 2;
        }
        if (result == 0) return 1;
        return result;
    }
    function modExp64(uint64 _b, uint64 _e, uint64 _m) public pure returns (uint64) {
        if (_m == 0) return 1;
        if (_m == 1) return 1;
        uint64 result = 1;
        uint64 base = _b % _m;
        uint64 exp = _e;
        while (exp > 0) {
            if (exp % 2 == 1) {
                result = uint64(mulmod(result, base, _m));
            }
            base = uint64(mulmod(base, base, _m));
            exp /= 2;
        }
        if (result == 0) return 1;
        return result;
    }
    function MotzkinPrime() public pure returns (uint64) {
        return 953467954114363;
    }
    function hashWith(address, address) public pure returns (uint256) {
        return 1;
    }
}
        `);
        try {
            const compileOut = execSync(`solc --bin-runtime "${mockSolPath}"`, { encoding: "utf8" });
            const lines = compileOut.split("\n");
            const binIndex = lines.findIndex(line => line.includes("Binary of the runtime part:"));
            if (binIndex !== -1) {
                const bytecode = "0x" + lines[binIndex + 1].trim();
                await provider.send("anvil_setCode", [localhost.VMREQ, bytecode]);
                console.log("Mocked VMREQ bytecode on Anvil with dynamic compiled contract.");
            }
        } catch (e) {
            console.error("Failed to compile MockVMREQ, using fallback static bytecode:", e.message);
            await provider.send("anvil_setCode", [localhost.VMREQ, "0x600160005260206000f3"]);
        }
    }

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

    // Find and clear Qu[1] to prevent infinite loop in CHO.Luo()
    console.log("Finding Qu mapping slot in CHO...");
    const abiCoderForSlot = new ethers.AbiCoder();
    const targetVal = "0x" + mapAddr.slice(2).toLowerCase().padStart(64, "0");
    for (let slot = 10; slot < 40; slot++) {
        const key = ethers.keccak256(
            abiCoderForSlot.encode(["uint256", "uint256"], [1n, BigInt(slot)])
        );
        const val = await provider.send("eth_getStorageAt", [localhost.CHO, key, "latest"]);
        if (val.toLowerCase() === targetVal) {
            console.log(` -> Found Qu mapping at slot: ${slot}`);
            await provider.send("anvil_setStorageAt", [localhost.CHO, key, "0x0000000000000000000000000000000000000000000000000000000000000000"]);
            console.log(" -> Cleared Qu[1] to allow MAP.New to succeed.");
            break;
        }
    }

    // Connect to XIA, XIE, and QI to resolve Fornax, Fomalhaute, and Eris
    console.log("Resolving and mocking Fornax, Fomalhaute, and Eris Shio contracts...");
    const xiaArtifact = getContractArtifact("03_xia.sol", "XIA");
    const xieArtifact = getContractArtifact("04_xie.sol", "XIE");
    const qiArtifact = getContractArtifact("01_qi.sol", "QI");

    const xia = new ethers.Contract(xiaAddr, xiaArtifact.abi, deployer);
    const xie = new ethers.Contract(xieAddr, xieArtifact.abi, deployer);
    const qi = new ethers.Contract(localhost.QI, qiArtifact.abi, deployer);

    let fornax, fomalhaute, eris;
    try {
        fornax = await xie.Fornax();
        fomalhaute = await xia.Fomalhaute();
        eris = await qi.Eris();

        // Overwrite QI, MAI, and XIE with updated bin-runtime bytecodes
        const qiRuntime = getContractArtifact("01_qi.sol", "QI").binRuntime;
        await provider.send("anvil_setCode", [localhost.QI, "0x" + qiRuntime]);
        console.log(` -> Overwrote QI bytecode at ${localhost.QI}`);

        const maiRuntime = getContractArtifact("02_mai.sol", "MAI").binRuntime;
        await provider.send("anvil_setCode", [localhost.MAI, "0x" + maiRuntime]);
        console.log(` -> Overwrote MAI bytecode at ${localhost.MAI}`);

        const xieRuntime = getContractArtifact("04_xie.sol", "XIE").binRuntime;
        await provider.send("anvil_setCode", [localhost.XIE, "0x" + xieRuntime]);
        console.log(` -> Overwrote XIE bytecode at ${localhost.XIE}`);
        const tethys = new ethers.Contract(localhost.CHO, getContractArtifact("01_cho.sol", "CHO").abi, deployer);
        console.log("Deployer CHO balance:", (await tethys.balanceOf(deployer.address)).toString());
        console.log("Deployer Fornax balance:", (await (new ethers.Contract(fornax, getContractArtifact("03_shio.sol", "SHIO").abi, deployer)).balanceOf(deployer.address)).toString());
        console.log("Deployer Fomalhaute balance:", (await (new ethers.Contract(fomalhaute, getContractArtifact("03_shio.sol", "SHIO").abi, deployer)).balanceOf(deployer.address)).toString());
        console.log("Deployer Eris balance:", (await (new ethers.Contract(eris, getContractArtifact("03_shio.sol", "SHIO").abi, deployer)).balanceOf(deployer.address)).toString());
    } catch (e) {
        console.error(" -> Failed to resolve or mock contracts:", e.message);
    }

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
        // Clear any old QING mapping at the computed Latitude/Longitude to avoid assert collision
        try {
            const heckeContractAddress = localhost.HECKE;
            const heckeArtifact = getContractArtifact("heckemeridians.sol", "Hecke");
            const hecke = new ethers.Contract(heckeContractAddress, heckeArtifact.abi, deployer);
            
            const luoVal = await cho.Luo.staticCall();
            const waat = luoVal * (await hecke.Meridians(0));
            
            const compliment = await hecke.Compliment(waat);
            const lat = compliment[1];
            const lon = compliment[0];
            
            const abiCoderForClearing = new ethers.AbiCoder();
            const key1 = ethers.keccak256(abiCoderForClearing.encode(["int256", "uint256"], [lat, 15n]));
            const key2 = ethers.keccak256(abiCoderForClearing.encode(["int256", "bytes32"], [lon, key1]));
            
            await provider.send("anvil_setStorageAt", [mapAddr, key2, "0x0000000000000000000000000000000000000000000000000000000000000000"]);
            console.log(` -> Cleared map slot at Lat: ${lat.toString()} Lon: ${lon.toString()} to prevent assert collision.`);
        } catch (e) {
            console.log(" -> Failed to clear map slot:", e.message);
        }

        console.log(`Checking CHO Luo...`);
        try {
            const onVal = await cho.On();
            console.log(` -> CHO On.Shio address: ${onVal[4]}`);
            const code = await provider.getCode(onVal[4]);
            console.log(` -> CHO On.Shio code length: ${code.length}`);
            
            const shioArtifact = getContractArtifact("03_shio.sol", "SHIO");
            const shioContract = new ethers.Contract(onVal[4], shioArtifact.abi, deployer);
            const rho = await shioContract.Rho();
            console.log(` -> On.Shio.Rho.Rod address: ${rho[0]}`);
            console.log(` -> On.Shio.Rho.Cone address: ${rho[1]}`);
            const rodCode = await provider.getCode(rho[0]);
            console.log(` -> On.Shio.Rho.Rod code length: ${rodCode.length}`);
            
            console.log(" -> Querying On.Shio.Luo staticCall...");
            try {
                const shioLuo = await shioContract.Luo.staticCall();
                console.log(" -> On.Shio.Luo returns:", shioLuo.toString());
            } catch (e) {
                console.log(" -> On.Shio.Luo staticCall failed:", e.message);
            }
            
            const coneCode = await provider.getCode(rho[1]);
            console.log(` -> On.Shio.Rho.Cone code length: ${coneCode.length}`);
            
            const reactorAddr = await cho.Reactor();
            console.log(` -> CHO Reactor address: ${reactorAddr}`);
            const reactorCode = await provider.getCode(reactorAddr);
            console.log(` -> CHO Reactor code length: ${reactorCode.length}`);

            console.log(" -> Querying CHO.React staticCall...");
            try {
                const reactRet = await cho.React.staticCall(0);
                console.log(" -> CHO.React returns:", reactRet);
            } catch (e) {
                console.log(" -> CHO.React staticCall failed:", e.message);
            }

            const luoVal = await cho.Luo.staticCall();
            console.log(` -> CHO Luo: ${luoVal.toString()}`);
        } catch (err) {
            console.log(` -> CHO Luo check failed:`, err.message);
        }
        console.log(`Deploying QING for ${emotion} via MAP...`);
        const txQing = await mapContract.New(lauAddress);
        const receiptQing = await txQing.wait();

        let qingAddress = null;
        for (const log of receiptQing.logs) {
            try {
                const parsed = mapContract.interface.parseLog(log);
                if (parsed && parsed.name === "NewQing") {
                    qingAddress = parsed.args[0];
                    break;
                }
            } catch (e) {
                // Ignore parsing errors for other contract logs
            }
        }
        if (!qingAddress) {
            throw new Error(`Failed to decode QING address for ${emotion}`);
        }
        console.log(` -> Deployed QING address: ${qingAddress}`);
        deployedQings.push({ emotion, qingAddress, lauAddress });
    }

    // 3. Distribute real tokens of CHO (Tethys), Fornax, Fomalhaute, Eris, and QINGs to all LAUs and QINGs
    console.log("\nDistributing real tokens to all LAU and QING contracts to avoid mocks...");
    const fornaxContract = new ethers.Contract(fornax, getContractArtifact("03_shio.sol", "SHIO").abi, deployer);
    const fomalhauteContract = new ethers.Contract(fomalhaute, getContractArtifact("03_shio.sol", "SHIO").abi, deployer);
    const erisContract = new ethers.Contract(eris, getContractArtifact("03_shio.sol", "SHIO").abi, deployer);
    const tethysContract = new ethers.Contract(localhost.CHO, getContractArtifact("01_cho.sol", "CHO").abi, deployer);

    for (const item of deployedQings) {
        // Resolve deployer balances and divide by 20 to distribute safely
        const fornaxBal = await fornaxContract.balanceOf(deployer.address);
        const fomalhauteBal = await fomalhauteContract.balanceOf(deployer.address);
        const erisBal = await erisContract.balanceOf(deployer.address);
        const tethysBal = await tethysContract.balanceOf(deployer.address);

        const fornaxAmt = fornaxBal / 20n;
        const fomalhauteAmt = fomalhauteBal / 20n;
        const erisAmt = erisBal / 20n;
        const tethysAmt = tethysBal / 20n;

        // Transfer CHO, Fornax, Fomalhaute, and Eris to the LAU token
        if (fornaxAmt > 0n) await (await fornaxContract.transfer(item.lauAddress, fornaxAmt)).wait();
        if (fomalhauteAmt > 0n) await (await fomalhauteContract.transfer(item.lauAddress, fomalhauteAmt)).wait();
        if (erisAmt > 0n) await (await erisContract.transfer(item.lauAddress, erisAmt)).wait();
        if (tethysAmt > 0n) await (await tethysContract.transfer(item.lauAddress, tethysAmt)).wait();

        // Transfer CHO, Fornax, and Eris to the QING contract itself
        if (fornaxAmt > 0n) await (await fornaxContract.transfer(item.qingAddress, fornaxAmt)).wait();
        if (erisAmt > 0n) await (await erisContract.transfer(item.qingAddress, erisAmt)).wait();
        if (tethysAmt > 0n) await (await tethysContract.transfer(item.qingAddress, tethysAmt)).wait();

        // Purchase QING tokens using LAU tokens to fund the deployer
        const lauContract = new ethers.Contract(item.lauAddress, getContractArtifact("11_lau.sol", "LAU").abi, deployer);
        const qingContract = new ethers.Contract(item.qingAddress, getContractArtifact("03_qing.sol", "QING").abi, deployer);

        // Impersonate CHO contract to call AddMarketRate as QING owner
        await provider.send("anvil_impersonateAccount", [localhost.CHO]);
        const choSigner = await provider.getSigner(localhost.CHO);
        
        // Fund CHO contract with some ETH so it can pay gas
        await provider.send("anvil_setBalance", [localhost.CHO, "0x16345785d8a0000"]); // 0.1 ETH

        const qingContractAsCho = qingContract.connect(choSigner);
        const rate = (await lauContract.totalSupply()) / 1000n;
        await (await qingContractAsCho.AddMarketRate(item.lauAddress, rate)).wait();

        // Stop impersonating
        await provider.send("anvil_stopImpersonatingAccount", [localhost.CHO]);

        const purchaseAmt = ethers.parseEther("10");
        console.log(` -> LAU Total Supply: ${await lauContract.totalSupply()}`);
        console.log(` -> Deployer LAU Balance: ${await lauContract.balanceOf(deployer.address)}`);
        console.log(` -> Calculated Rate: ${rate}`);
        const cost = (purchaseAmt * rate) / ethers.parseEther("1");
        console.log(` -> Calculated Cost: ${cost}`);

        await (await lauContract.approve(item.qingAddress, cost * 2n)).wait();
        await (await qingContract.Purchase(item.lauAddress, purchaseAmt)).wait();

        // Transfer QING tokens to ALL deployed LAU tokens
        const qingAmt = purchaseAmt / 5n;
        for (const target of deployedQings) {
            await (await qingContract.transfer(target.lauAddress, qingAmt)).wait();
        }
    }

    // 4. Test React to increase Epibars and Hypobars to > 0
    console.log("\nStarting SEI registration for player YUE...");
    
    // Clear Chan.Yan[deployer.address] to force fresh YUE deployment and registration in CHAN
    const abiCoder = new ethers.AbiCoder();
    const yanKey = ethers.keccak256(
        abiCoder.encode(["address", "uint256"], [deployer.address, 1])
    );
    await provider.send("anvil_setStorageAt", [localhost.CHAN, yanKey, "0x0000000000000000000000000000000000000000000000000000000000000000"]);
    
    const sei = new ethers.Contract(localhost.SEI, getContractArtifact("01_sei.sol", "SEI").abi, deployer);
    const startTx = await sei.Start(deployedQings[0].lauAddress, "Auncient Yue", "YUE");
    await startTx.wait();
    
    const chan = new ethers.Contract(localhost.CHAN, getContractArtifact("01_chan.sol", "CHAN").abi, deployer);
    let activeYue = await chan.Yan(deployer.address);
    console.log(` -> Player YUE registered at: ${activeYue}`);



    if (activeYue && activeYue !== "0x0000000000000000000000000000000000000000") {
        const yue = new ethers.Contract(activeYue, getContractArtifact("yue.sol", "YUE").abi, deployer);

        // Override deployer's YUE balance on Anvil
        console.log("Overriding YUE balance for deployer on Anvil...");
        for (let slot = 0; slot < 20; slot++) {
            const key = ethers.keccak256(
                abiCoder.encode(["address", "uint256"], [deployer.address, slot])
            );
            const value = "0x" + (1000n * 10n**18n + 1n).toString(16).padStart(64, "0");
            await provider.send("anvil_setStorageAt", [activeYue, key, value]);
        }
        console.log(`Deployer YUE Balance: ${(await yue.balanceOf(deployer.address)).toString()}`);

        for (const item of deployedQings) {
            console.log(`\nTriggering React on YUE for ${item.emotion} Qing (${item.qingAddress})...`);
            
            console.log(`Registering ${item.emotion} LAU token in CHO...`);
            try {
                const enterTx = await cho.Enter(item.lauAddress);
                await enterTx.wait();
                console.log(" -> Successfully entered in CHO.");
                const user = await cho.GetUser.staticCall();
                console.log(` -> CHO GetUser: Soul=${user[0].toString()}, On.Phi=${user[1][0]}, Entropy=${user[3].toString()}`);
            } catch (e) {
                console.log(" -> CHO Enter failed:", e.message);
            }

            // Override Qing.Entropy (slot 13) to 1 to avoid division by zero
            console.log(`Overriding QING Entropy for ${item.emotion}...`);
            await provider.send("anvil_setStorageAt", [item.qingAddress, "0x000000000000000000000000000000000000000000000000000000000000000d", "0x0000000000000000000000000000000000000000000000000000000000000001"]);



            // Override CHO.Delegates[deployer.address].Entropy (position 8 of slot 18) to 1
            const baseKey = ethers.keccak256(
                abiCoder.encode(["address", "uint256"], [deployer.address, 18])
            );
            const entropyKey = "0x" + (BigInt(baseKey) + 8n).toString(16);
            await provider.send("anvil_setStorageAt", [localhost.CHO, entropyKey, "0x0000000000000000000000000000000000000000000000000000000000000001"]);

            // Trigger react through CHAN contract
            const reactTx = await chan.ReactYue(activeYue, item.qingAddress, { gasLimit: 5000000 });
            await reactTx.wait();

            const bars = await yue.Bar(item.qingAddress);
            console.log(` -> Hypobar: ${bars[0].toString()} Gram`);
            console.log(` -> Epibar:  ${bars[1].toString()} Gram`);
            if (bars[0] > 0n && bars[1] > 0n) {
                console.log(` -> SUCCESS: Epibar and Hypobar are both > 0!`);
            } else {
                console.log(` -> WARNING: Epibar/Hypobar weights did not increase.`);
            }

            console.log(` -> Performing CHOA.Chat on q${item.emotion}...`);
            try {
                const choa = new ethers.Contract(localhost.CHOA, getContractArtifact("02_choa.sol", "CHOA").abi, deployer);
                const chatTx = await choa.Chat(item.qingAddress, `Hello q${item.emotion}`);
                const chatReceipt = await chatTx.wait();
                console.log(` -> CHOA.Chat transaction succeeded! Gas used: ${chatReceipt.gasUsed.toString()}`);
            } catch (e) {
                console.log(` -> CHOA.Chat failed:`, e.message);
            }
        }
    } else {
        console.log("\nSkipped React tests: No active YUE contract was resolved.");
    }

    // 4. Test Soeng Contract Outputs
    console.log("\n--- Querying Intermediate Soeng Contract Outputs ---");
    let activeZi = localhost.ZI;
    let activePang = localhost.PANG;
    
    // Always deploy fresh ZI and PANG to ensure they point to the newly deployed CHOA
    try {
        console.log("Deploying a fresh ZI instance...");
        const ziArtifact = getContractArtifact("05_zi.sol", "ZI");
        const ziFactory = new ethers.ContractFactory(ziArtifact.abi, ziArtifact.bin, deployer);
        const ziContract = await ziFactory.deploy(localhost.CHOA);
        await ziContract.waitForDeployment();
        activeZi = await ziContract.getAddress();
        console.log(` -> Fresh ZI deployed at: ${activeZi}`);
        
        console.log("Deploying a fresh PANG instance...");
        const pangArtifact = getContractArtifact("06_pang.sol", "PANG");
        const pangFactory = new ethers.ContractFactory(pangArtifact.abi, pangArtifact.bin, deployer);
        const pangContract = await pangFactory.deploy(activeZi);
        await pangContract.waitForDeployment();
        activePang = await pangContract.getAddress();
        console.log(` -> Fresh PANG deployed at: ${activePang}`);
    } catch (e) {
        console.error(" -> Failed to deploy fresh ZI/PANG:", e.message);
    }

    if (xiaAddr && xieAddr && activeZi && activePang) {
        const xiaArtifact = getContractArtifact("03_xia.sol", "XIA");
        const xieArtifact = getContractArtifact("04_xie.sol", "XIE");
        const ziArtifact = getContractArtifact("05_zi.sol", "ZI");
        const pangArtifact = getContractArtifact("06_pang.sol", "PANG");

        const xia = new ethers.Contract(xiaAddr, xiaArtifact.abi, deployer);
        const xie = new ethers.Contract(xieAddr, xieArtifact.abi, deployer);
        const zi = new ethers.Contract(activeZi, ziArtifact.abi, deployer);
        const pang = new ethers.Contract(activePang, pangArtifact.abi, deployer);

        for (const item of deployedQings) {
            console.log(`\nQuerying Soeng outputs for ${item.emotion} Qing (${item.qingAddress})...`);
            try {
                const qingContract = new ethers.Contract(item.qingAddress, getContractArtifact("03_qing.sol", "QING").abi, deployer);
                const waatVal = await qingContract.Waat();
                
                const charge = await xia.Charge.staticCall(waatVal);
                console.log(` -> XIA.Charge: ${charge.toString()}`);
                
                const power = await xie.Power.staticCall(waatVal);
                console.log(` -> XIE.Power -> Charge: ${power[0].toString()}, Omicron: ${power[1].toString()}, Omega: ${power[2].toString()}`);
                
                const spin = await zi.Spin.staticCall(waatVal);
                console.log(` -> ZI.Spin -> Iota: ${spin[0].toString()}, Omicron: ${spin[1].toString()}, Omega: ${spin[2].toString()}, Eta: ${spin[3].toString()}`);
                
                const push = await pang.Push.staticCall(waatVal);
                console.log(` -> PANG.Push -> Iota: ${push[0].toString()}, Omicron: ${push[1].toString()}, Eta: ${push[2].toString()}, Omega: ${push[3].toString()}, Charge: ${push[4].toString()}`);
                
                if (charge > 0n && power[0] > 0n && spin[0] > 0n && push[0] > 0n) {
                    console.log(` -> SUCCESS: All intermediate Soeng outputs are > 0!`);
                }
            } catch (e) {
                console.error(`Error querying Soeng contract for ${item.emotion}:`, e.message);
            }
        }
    } else {
        console.log("Skipped Soeng tests: XIA, XIE, ZI, or PANG addresses not resolved.");
    }

    console.log("\n=== EMOTION QUANTIZATION TEST SUITE COMPLETED ===");
}

main().catch(console.error);
