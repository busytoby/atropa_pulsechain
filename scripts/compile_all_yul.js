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
    const mahoney = compileYul(path.join(__dirname, "../solidity/bin/mahoney.yul"));
    const estle = compileYul(path.join(__dirname, "../solidity/bin/estle.yul"));
    const i8080 = compileYul(path.join(__dirname, "../solidity/bin/i8080.yul"));
    const tkis = compileYul(path.join(__dirname, "../solidity/bin/tkis.yul"));
    const lights = compileYul(path.join(__dirname, "../solidity/bin/lights.yul"));
    const pulsecatcher = compileYul(path.join(__dirname, "../solidity/bin/pulsecatcher.yul"));
    const tvcolor = compileYul(path.join(__dirname, "../solidity/bin/tvcolor.yul"));
    const auntsally = compileYul(path.join(__dirname, "../solidity/bin/auntsally.yul"));
    const vdt = compileYul(path.join(__dirname, "../solidity/bin/vdt.yul"));
    const debuggerYul = compileYul(path.join(__dirname, "../solidity/bin/debugger.yul"));
    const tms9900 = compileYul(path.join(__dirname, "../solidity/bin/tms9900.yul"));
    
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
        mahoney,
        estle,
        i8080,
        tkis,
        lights,
        pulsecatcher,
        tvcolor,
        auntsally,
        vdt,
        debugger: debuggerYul,
        tms9900,
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
