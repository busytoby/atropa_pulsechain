const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

async function main() {
    console.log("=== STEP 1: Loading Deployed MusicMaker ===");
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const musicMakerAddress = config.networks.localhost.musicMakerAddress;
    console.log(`MusicMaker Address: ${musicMakerAddress}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const mInterface = new ethers.Interface([
        "function peek(uint16 addr) external view returns (uint8)",
        "function colortonePlay(uint8 key, uint8 voice) external returns (uint256)",
        "function colortoneRelease(uint8 voice) external returns (uint256)",
        "function playSequence(uint16[] calldata notes, uint8[] calldata voices, uint16[] calldata durations) external returns (uint256)"
    ]);

    const mContract = new ethers.Contract(musicMakerAddress, mInterface, deployer);

    console.log("\n=== STEP 2: Testing MusiCalc Colortone Play Note ===");
    // Play key 0 (C-4, MIDI 60) on Voice 1
    let tx = await mContract.colortonePlay(0, 1);
    await tx.wait();
    console.log("Colortone Key 0 (C-4) played on Voice 1");

    const freqLo = await mContract.peek(54272);
    const freqHi = await mContract.peek(54273);
    const ctrlReg = await mContract.peek(54276);

    const freq = (Number(freqHi) << 8) | Number(freqLo);
    console.log(`Retrieved Voice 1 Freq: ${freq} (Expected: 6000)`);
    console.log(`Retrieved Voice 1 Ctrl: ${ctrlReg} (Expected: 17)`);

    if (freq !== 6000 || Number(ctrlReg) !== 17) {
        console.error("FAIL: Colortone Play parameters mismatch!");
        process.exit(1);
    }
    console.log("SUCCESS: Colortone Play triggered successfully!");

    console.log("\n=== STEP 3: Testing MusiCalc Colortone Release Note ===");
    tx = await mContract.colortoneRelease(1);
    await tx.wait();
    console.log("Colortone Voice 1 Released");

    const releasedCtrl = await mContract.peek(54276);
    console.log(`Retrieved Voice 1 Ctrl: ${releasedCtrl} (Expected: 16)`);

    if (Number(releasedCtrl) !== 16) {
        console.error("FAIL: Colortone Release failed to clear Gate bit!");
        process.exit(1);
    }
    console.log("SUCCESS: Colortone Release triggered successfully!");

    console.log("\n=== STEP 4: Testing MusiCalc Sequencer (playSequence) ===");
    // Play a sequence of 3 notes on Voice 2: [60, 62, 64] with durations [4, 4, 8]
    const seqTx = await mContract.playSequence([60, 62, 64], [2, 2, 2], [4, 4, 8]);
    const seqReceipt = await seqTx.wait();
    
    // Parse total duration from return data
    // In ethers, to read return value of a state-modifying transaction, we can staticCall it:
    const totalDuration = await mContract.playSequence.staticCall([60, 62, 64], [2, 2, 2], [4, 4, 8]);
    console.log(`Sequence Total Duration: ${totalDuration} (Expected: 16)`);

    const voice2Lo = await mContract.peek(54279);
    const voice2Hi = await mContract.peek(54280);
    const voice2Freq = (Number(voice2Hi) << 8) | Number(voice2Lo);
    console.log(`Retrieved Voice 2 Freq (Last Note 64): ${voice2Freq} (Expected: 6400)`);

    if (Number(totalDuration) !== 16 || voice2Freq !== 6400) {
        console.error("FAIL: Sequence execution validation failed!");
        process.exit(1);
    }
    console.log("SUCCESS: MusiCalc Sequencer sequence successfully executed and verified!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
