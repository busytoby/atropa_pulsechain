const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

function compileYul(yulPath) {
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("Compiling all Yul contracts for the dashboard...");
    
    const zmachine = compileYul(path.join(__dirname, "../solidity/bin/zmachine.yul"));
    const zmachineParser = compileYul(path.join(__dirname, "../solidity/bin/zmachineParser.yul"));
    const musicMaker = compileYul(path.join(__dirname, "../solidity/bin/musicMaker.yul"));
    const keySystem = compileYul(path.join(__dirname, "../solidity/bin/keySystem.yul"));
    const bGraph = compileYul(path.join(__dirname, "../solidity/bin/bGraph.yul"));
    const speechSynthesizer = compileYul(path.join(__dirname, "../solidity/bin/speechSynthesizer.yul"));
    const sculpture = compileYul(path.join(__dirname, "../solidity/bin/sculpture.yul"));
    const breadboard = compileYul(path.join(__dirname, "../solidity/bin/breadboard.yul"));
    const nelson = compileYul(path.join(__dirname, "../solidity/bin/nelson.yul"));
    
    const erc20Bin = "0x" + fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.bin"), "utf8").trim();
    const erc20Abi = JSON.parse(fs.readFileSync(path.join(__dirname, "../solidity/bin/MockERC20.abi"), "utf8"));

    const data = {
        zmachine,
        zmachineParser,
        musicMaker,
        keySystem,
        bGraph,
        speechSynthesizer,
        sculpture,
        breadboard,
        nelson,
        mockErc20: {
            bin: erc20Bin,
            abi: erc20Abi
        }
    };

    fs.writeFileSync(path.join(__dirname, "../frontend/compiled_yul.json"), JSON.stringify(data, null, 2));
    console.log("compiled_yul.json successfully generated in frontend directory.");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
