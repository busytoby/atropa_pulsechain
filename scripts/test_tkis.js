const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Total Kitchen Information System (TKIS) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/tkis.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/tkis.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const tkisABI = [
        "function addIngredient(bytes32 name, uint256 quantity) external returns (uint256)",
        "function getIngredient(bytes32 name) external view returns (uint256)",
        "function addRecipe(bytes32 recipeName, bytes32[] ingredientNames, uint256[] quantities) external returns (uint256)",
        "function checkRecipe(bytes32 recipeName) external view returns (uint256)",
        "function cookRecipe(bytes32 recipeName) external returns (uint256)"
    ];

    const factory = new ethers.ContractFactory(tkisABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`TKIS contract deployed at: ${contractAddr}\n`);

    const apples = ethers.encodeBytes32String("Apples");
    const flour = ethers.encodeBytes32String("Flour");
    const applePie = ethers.encodeBytes32String("ApplePie");

    // 1. Add ingredients
    console.log("Adding 10 Apples and 5 Flour to inventory...");
    let tx = await contract.addIngredient(apples, 10);
    await tx.wait();
    tx = await contract.addIngredient(flour, 5);
    await tx.wait();

    let applesQty = await contract.getIngredient(apples);
    let flourQty = await contract.getIngredient(flour);
    console.log(`Inventory -> Apples: ${applesQty.toString()}, Flour: ${flourQty.toString()}`);
    if (Number(applesQty) !== 10 || Number(flourQty) !== 5) {
        console.error("FAIL: Ingredient quantities mismatched!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 2. Add Recipe
    console.log("Adding recipe: ApplePie (requires 3 Apples, 2 Flour)...");
    tx = await contract.addRecipe(applePie, [apples, flour], [3, 2]);
    await tx.wait();
    console.log("Recipe added.");

    // 3. Check availability
    let available = await contract.checkRecipe(applePie);
    console.log(`ApplePie availability check: ${available.toString()} (Expected: 1)`);
    if (Number(available) !== 1) {
        console.error("FAIL: ApplePie should be available!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 4. Cook recipe (1st time)
    console.log("Cooking ApplePie (1st time)...");
    tx = await contract.cookRecipe(applePie);
    await tx.wait();

    applesQty = await contract.getIngredient(apples);
    flourQty = await contract.getIngredient(flour);
    console.log(`Inventory -> Apples: ${applesQty.toString()} (Expected: 7), Flour: ${flourQty.toString()} (Expected: 3)`);
    if (Number(applesQty) !== 7 || Number(flourQty) !== 3) {
        console.error("FAIL: Cook deduction failed!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 5. Cook recipe (2nd time)
    console.log("Cooking ApplePie (2nd time)...");
    tx = await contract.cookRecipe(applePie);
    await tx.wait();

    applesQty = await contract.getIngredient(apples);
    flourQty = await contract.getIngredient(flour);
    console.log(`Inventory -> Apples: ${applesQty.toString()} (Expected: 4), Flour: ${flourQty.toString()} (Expected: 1)`);
    if (Number(applesQty) !== 4 || Number(flourQty) !== 1) {
        console.error("FAIL: Cook deduction failed on 2nd run!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 6. Check availability again (should fail since we only have 1 Flour but need 2)
    available = await contract.checkRecipe(applePie);
    console.log(`ApplePie availability check: ${available.toString()} (Expected: 0)`);
    if (Number(available) !== 0) {
        console.error("FAIL: ApplePie should NOT be available!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 7. Attempt to cook again (should fail/revert)
    console.log("Attempting to cook ApplePie a 3rd time (expecting revert)...");
    try {
        const tx3 = await contract.cookRecipe(applePie);
        await tx3.wait();
        console.error("FAIL: Cooking should have reverted due to insufficient ingredients!");
        process.exit(1);
    } catch (e) {
        console.log("Passed (reverted as expected)!\n");
    }

    console.log(">>> ALL TKIS DATABASE TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
