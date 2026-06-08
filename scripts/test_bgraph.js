const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

async function main() {
    console.log("=== STEP 1: Loading Deployed B/Graph ===");
    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config file not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const bGraphAddress = config.networks.localhost.bGraphAddress;
    console.log(`B/Graph Address: ${bGraphAddress}`);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const bgInterface = new ethers.Interface([
        "function linearRegression(int256[] x, int256[] y) external view returns (int256 slope, int256 intercept, int256 rSquared)",
        "function plotBarChart(uint256[] values) external view returns (uint256[] coordinates)",
        "function evaluateJobDecisionTree(uint256[] answers) external view returns (uint256 jobIndex)"
    ]);

    const bgContract = new ethers.Contract(bGraphAddress, bgInterface, deployer);

    console.log("\n=== STEP 2: Testing B/Graph Linear Regression ===");
    // Linear regression on (1,2), (2,4), (3,5), (4,4), (5,5)
    // Scale inputs by 1000 for precision logic
    const x = [1000, 2000, 3000, 4000, 5000];
    const y = [2000, 4000, 5000, 4000, 5000];

    const reg = await bgContract.linearRegression(x, y);
    console.log(`Retrieved Slope: ${Number(reg.slope) / 1000} (Expected: 0.6)`);
    console.log(`Retrieved Intercept: ${Number(reg.intercept) / 1000} (Expected: 2.2)`);
    console.log(`Retrieved R-Squared: ${Number(reg.rSquared) / 1000} (Expected: ~0.6)`);

    if (Math.abs(Number(reg.slope) - 600) > 10 || Math.abs(Number(reg.intercept) - 2200) > 10) {
        console.error("FAIL: Linear regression slope/intercept mismatch!");
        process.exit(1);
    }
    console.log("SUCCESS: Linear regression successfully computed!");

    console.log("\n=== STEP 3: Testing B/Graph Coordinate Chart Plotting ===");
    const values = [10, 20, 50];
    const coords = await bgContract.plotBarChart(values);
    console.log(`Plot coordinates length: ${coords.length} (Expected: 12 [3 bars * 4 coords])`);

    // Width of each bar = 320 / 3 = 106px
    // Max value is 50. Scaled heights: 10->32px, 20->64px, 50->160px.
    // Y1 coordinates (180 - height): 10->148, 20->116, 50->20.
    const expectedY1 = [148, 116, 20];
    for (let i = 0; i < 3; i++) {
        const x1 = Number(coords[i * 4]);
        const y1 = Number(coords[i * 4 + 1]);
        const x2 = Number(coords[i * 4 + 2]);
        const y2 = Number(coords[i * 4 + 3]);
        console.log(`  Bar ${i}: (${x1}, ${y1}) -> (${x2}, ${y2})`);
        if (y1 !== expectedY1[i] || y2 !== 180) {
            console.error(`FAIL: Coordinates mismatch for bar ${i}!`);
            process.exit(1);
        }
    }
    console.log("SUCCESS: Bar chart coordinates successfully mapped!");

    console.log("\n=== STEP 4: Testing Ahoy! Issue 3 Page 11 'What's My Job?' Classifier ===");
    // Q0 (Work with computers) = 0 [No]
    // Q1 (Work in medical) = 1 [Yes]
    // Q2 (Perform surgeries) = 0 [No]
    // Expected result: Nurse / Practitioner / jobIndex = 4
    const jobIdx = await bgContract.evaluateJobDecisionTree([0, 1, 0]);
    console.log(`Classified Job Index: ${jobIdx} (Expected: 4)`);

    if (Number(jobIdx) !== 4) {
        console.error("FAIL: Decision tree classifier job index mismatch!");
        process.exit(1);
    }
    console.log("SUCCESS: 'What's My Job?' decision tree successfully verified!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
