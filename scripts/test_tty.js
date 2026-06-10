const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing TTY Serial UART Frame Encoder & Decoder (tty.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/tty.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/tty.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const ttyABI = [
        "function encodeSerial(bytes calldata data, uint8 parityMode) external view returns (bytes memory)",
        "function decodeSerial(bytes calldata pulses, uint8 parityMode) external view returns (bytes memory)"
    ];

    const factory = new ethers.ContractFactory(ttyABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`TTYSerialLoop contract deployed at: ${contractAddr}\n`);

    // 1. Test encoding HELLO (no parity)
    console.log("Encoding 'HELLO' with No Parity...");
    const dataBytes = ethers.toUtf8Bytes("HELLO");
    const pulsesHex = await contract.encodeSerial(dataBytes, 0);
    const pulses = ethers.getBytes(pulsesHex);
    console.log(`Pulses count: ${pulses.length} (Expected: 50)`);
    if (pulses.length !== 50) {
        throw new Error("Invalid output pulses length for HELLO!");
    }

    // Verify first char 'H' (72 = 01001000 binary)
    // Frame: Start(0), LSB-first data (0,0,0,1,0,0,1,0), Stop(1)
    const expectedHFrame = [0, 0, 0, 0, 1, 0, 0, 1, 0, 1];
    const actualHFrame = Array.from(pulses.slice(0, 10));
    console.log(`Expected H Frame: [${expectedHFrame.join(",")}]`);
    console.log(`Actual H Frame:   [${actualHFrame.join(",")}]`);
    if (JSON.stringify(actualHFrame) !== JSON.stringify(expectedHFrame)) {
        throw new Error("Bit encoding for 'H' is incorrect!");
    }
    console.log("Passed!\n");

    // 2. Test decoding HELLO
    console.log("Decoding pulses back to ASCII...");
    const decodedBytes = await contract.decodeSerial(pulsesHex, 0);
    const decodedStr = ethers.toUtf8String(decodedBytes);
    console.log(`Decoded string: "${decodedStr}" (Expected: "HELLO")`);
    if (decodedStr !== "HELLO") {
        throw new Error("Decoded string mismatched!");
    }
    console.log("Passed!\n");

    // 3. Test Even Parity with 'A' (65 = 01000001) -> 2 ones (Even) -> Parity bit should be 0
    console.log("Encoding 'A' with Even Parity...");
    const aBytes = ethers.toUtf8Bytes("A");
    const aPulsesHex = await contract.encodeSerial(aBytes, 1);
    const aPulses = ethers.getBytes(aPulsesHex);
    console.log(`Pulses count: ${aPulses.length} (Expected: 11)`);
    // Frame: Start(0), Data (1,0,0,0,0,0,1,0), Parity(0), Stop(1)
    const expectedAFrame = [0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1];
    const actualAFrame = Array.from(aPulses);
    console.log(`Expected A Frame: [${expectedAFrame.join(",")}]`);
    console.log(`Actual A Frame:   [${actualAFrame.join(",")}]`);
    if (JSON.stringify(actualAFrame) !== JSON.stringify(expectedAFrame)) {
        throw new Error("Even parity bit encoding is incorrect!");
    }
    console.log("Passed!\n");

    // 4. Test decoding 'A' with Even Parity
    console.log("Decoding 'A' pulses...");
    const aDecodedBytes = await contract.decodeSerial(aPulsesHex, 1);
    const aDecodedStr = ethers.toUtf8String(aDecodedBytes);
    console.log(`Decoded string: "${aDecodedStr}" (Expected: "A")`);
    if (aDecodedStr !== "A") {
        throw new Error("Decoded character mismatched!");
    }
    console.log("Passed!\n");

    console.log(">>> ALL TTY SERIAL TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
