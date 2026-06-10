const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Video Display Terminal Emulator (vdt.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/vdt.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/vdt.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const vdtABI = [
        "function writeChar(uint8 char) external returns (uint256)",
        "function getScreenRAM() external view returns (bytes memory)",
        "function getCursor() external view returns (uint256 col, uint256 row)",
        "function clearScreen() external returns (uint256)"
    ];

    const factory = new ethers.ContractFactory(vdtABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`VDTTerminal contract deployed at: ${contractAddr}\n`);

    // 1. Initial State
    let cursor = await contract.getCursor();
    console.log(`Initial Cursor: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 0) {
        throw new Error("Initial cursor should be at (0, 0)");
    }

    let screenBytes = await contract.getScreenRAM();
    let screenStr = ethers.toUtf8String(screenBytes);
    console.log(`Initial screen bytes length: ${screenBytes.length} / string length: ${screenStr.length}`);
    if (screenStr !== " ".repeat(1920)) {
        throw new Error("Initial screen is not all spaces!");
    }

    // 2. Write standard printable characters
    console.log("Writing 'HELLO'...");
    const gasLimit = 15000000;
    for (const char of "HELLO") {
        await (await contract.writeChar(char.charCodeAt(0), { gasLimit })).wait();
    }

    cursor = await contract.getCursor();
    console.log(`Cursor after HELLO: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 5 || Number(cursor.row) !== 0) {
        throw new Error("Cursor should be at (5, 0)");
    }

    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    console.log(`Screen line 0 start: "${screenStr.slice(0, 10)}"`);
    if (screenStr.slice(0, 5) !== "HELLO") {
        throw new Error("Screen characters mismatched!");
    }

    // 3. Test Backspace (0x08)
    console.log("Testing backspace...");
    await (await contract.writeChar(0x08, { gasLimit })).wait();
    cursor = await contract.getCursor();
    console.log(`Cursor after backspace: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 4 || Number(cursor.row) !== 0) {
        throw new Error("Cursor should be at (4, 0) after backspace");
    }
    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    if (screenStr.slice(0, 5) !== "HELL ") {
        throw new Error("Backspace did not clear the character at previous cursor!");
    }

    // 4. Test Carriage Return (0x0D)
    console.log("Testing Carriage Return...");
    await (await contract.writeChar(0x0D, { gasLimit })).wait();
    cursor = await contract.getCursor();
    console.log(`Cursor after CR: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 0) {
        throw new Error("Cursor should be at (0, 0) after CR");
    }

    // 5. Test Newline (0x0A)
    console.log("Testing Newline...");
    await (await contract.writeChar(0x0A, { gasLimit })).wait();
    cursor = await contract.getCursor();
    console.log(`Cursor after LF: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 1) {
        throw new Error("Cursor should be at (0, 1) after LF");
    }

    // 6. Test line wrap
    console.log("Testing line wrap (writing 85 'A's)...");
    for (let i = 0; i < 85; i++) {
        await (await contract.writeChar("A".charCodeAt(0), { gasLimit })).wait();
    }
    cursor = await contract.getCursor();
    console.log(`Cursor after wrap: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 5 || Number(cursor.row) !== 2) {
        throw new Error("Cursor not wrapped correctly");
    }

    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    let row1 = screenStr.slice(80, 160);
    console.log(`Row 1 has ${row1.split('A').length - 1} 'A's`);
    let row2 = screenStr.slice(160, 240);
    console.log(`Row 2 start: "${row2.slice(0, 10)}"`);
    if (row1 !== "A".repeat(80) || row2.slice(0, 5) !== "AAAAA") {
        throw new Error("Wrapping content is incorrect!");
    }

    // 7. Test scrolling
    console.log("Filling screen to trigger scrolling...");
    for (let i = 0; i < 22; i++) {
        await (await contract.writeChar(0x0A, { gasLimit })).wait();
    }
    cursor = await contract.getCursor();
    console.log(`Cursor before final scroll: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 23) {
        throw new Error("Cursor should be at (0, 23) before scroll");
    }

    console.log("Testing scroll validation...");
    await (await contract.clearScreen({ gasLimit })).wait();
    for (const char of "ROW0") {
        await (await contract.writeChar(char.charCodeAt(0), { gasLimit })).wait();
    }
    await (await contract.writeChar(0x0A, { gasLimit })).wait();
    for (const char of "ROW1") {
        await (await contract.writeChar(char.charCodeAt(0), { gasLimit })).wait();
    }
    
    for (let i = 0; i < 22; i++) {
        await (await contract.writeChar(0x0A, { gasLimit })).wait();
    }
    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    console.log(`Before scroll: row 0 starts with "${screenStr.slice(0, 10)}", row 1 starts with "${screenStr.slice(80, 90)}"`);
    if (screenStr.slice(0, 4) !== "ROW0" || screenStr.slice(80, 84) !== "ROW1") {
        throw new Error("Initial placement before scroll failed");
    }

    console.log("Triggering scroll up...");
    await (await contract.writeChar(0x0A, { gasLimit })).wait();
    
    cursor = await contract.getCursor();
    console.log(`Cursor after scroll: col=${cursor.col}, row=${cursor.row}`);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 23) {
        throw new Error("Cursor should be at (0, 23) after scroll");
    }

    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    console.log(`After scroll: row 0 starts with "${screenStr.slice(0, 10)}"`);
    if (screenStr.slice(0, 4) !== "ROW1") {
        throw new Error("Scrolling did not shift row 1 to row 0 correctly!");
    }

    // 8. Test Clear Screen
    console.log("Testing Clear Screen...");
    await (await contract.clearScreen({ gasLimit })).wait();
    cursor = await contract.getCursor();
    screenBytes = await contract.getScreenRAM();
    screenStr = ethers.toUtf8String(screenBytes);
    if (Number(cursor.col) !== 0 || Number(cursor.row) !== 0 || screenStr !== " ".repeat(1920)) {
        throw new Error("Clear screen failed!");
    }

    console.log(">>> ALL VDT TERMINAL TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
