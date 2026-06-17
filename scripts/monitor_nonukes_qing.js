const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PULSECHAIN_RPC = "https://rpc.pulsechain.com";
const LOCALHOST_RPC = "http://127.0.0.1:8545";

const NONUKES_ADDR = "0x174A0ad99c60c20D9B3d94c3095BC1fb9ddEFd62";
const LOG_EVENT_SIGNATURE = "LogEvent(string,uint64,uint64,string)";
const NEW_QING_SIGNATURE = "NewQing(address,address,uint256)";

const C = {
    reset: "\x1b[0m",
    bright: "\x1b[1m",
    dim: "\x1b[2m",
    blue: "\x1b[34m",
    green: "\x1b[32m",
    cyan: "\x1b[36m",
    yellow: "\x1b[33m",
    magenta: "\x1b[35m",
    red: "\x1b[31m"
};

// Check if running in localhost mode
const useLocalhost = process.argv.includes("--local") || process.env.USE_LOCALHOST === "true";
const rpcUrl = useLocalhost ? LOCALHOST_RPC : PULSECHAIN_RPC;

// Load MAP contract address from config
let mapAddress = null;
try {
    const configPath = path.join(__dirname, "../config/user_config.json");
    if (fs.existsSync(configPath)) {
        const config = JSON.parse(fs.readFileSync(configPath, "utf8"));
        const networkKey = useLocalhost ? "localhost" : "pulsechain";
        mapAddress = config.networks[networkKey]?.MAP;
    }
} catch (e) {
    console.error("Failed to load MAP address from config, using default if available.");
}

// Fallback to default localhost MAP if local is selected but config load failed
if (!mapAddress && useLocalhost) {
    mapAddress = "0xc6e7DF5E7b4f2A278906862b61205850344D4e7d";
}

async function main() {
    console.log(`${C.blue}Connecting to RPC at ${rpcUrl}...${C.reset}`);
    const provider = new ethers.JsonRpcProvider(rpcUrl);

    try {
        const latestBlock = await provider.getBlockNumber();
        console.log(`${C.green}Connected! Current Block: ${latestBlock}${C.reset}`);

        if (!mapAddress) {
            console.log(`${C.red}MAP contract address not configured for this network.${C.reset}`);
            console.log(`To run against localhost, use: node scripts/monitor_nonukes_qing.js --local`);
            return;
        }

        console.log(`${C.cyan}Checking MAP contract at ${mapAddress} for NoNukes QING...${C.reset}`);

        const mapInterface = new ethers.Interface([
            `event ${NEW_QING_SIGNATURE}`
        ]);

        // Query all NewQing events from the MAP contract
        const newQingTopic = ethers.id(NEW_QING_SIGNATURE);
        const logs = await provider.getLogs({
            fromBlock: "0x0",
            toBlock: "0x" + latestBlock.toString(16),
            address: mapAddress,
            topics: [newQingTopic]
        });

        let noNukesQingAddress = null;

        for (const log of logs) {
            try {
                const parsed = mapInterface.parseLog(log);
                const qingAddr = parsed.args[0];
                const integrativeAddr = parsed.args[1];
                if (integrativeAddr.toLowerCase() === NONUKES_ADDR.toLowerCase()) {
                    noNukesQingAddress = qingAddr;
                    break;
                }
            } catch (e) {
                // Ignore parsing errors for other/older formats
            }
        }

        if (!noNukesQingAddress) {
            console.log(`${C.yellow}NoNukes QING does not exist on the Map yet.${C.reset}`);
            console.log(`Address scanned for: ${NONUKES_ADDR}`);
            return;
        }

        console.log(`${C.green}${C.bright}NoNukes QING Found on Map!${C.reset}`);
        console.log(`QING Contract Address: ${C.yellow}${noNukesQingAddress}${C.reset}`);
        console.log(`${C.blue}Starting live chat monitoring for NoNukes QING...${C.reset}\n`);

        const qingInterface = new ethers.Interface([
            `event ${LOG_EVENT_SIGNATURE}`
        ]);
        const logEventTopic = ethers.id(LOG_EVENT_SIGNATURE);

        // Print initial block message
        console.log(`${C.dim}Waiting for chat events on NoNukes QING at ${noNukesQingAddress}...${C.reset}\n`);

        let lastCheckedBlock = latestBlock;
        setInterval(async () => {
            try {
                const currentBlock = await provider.getBlockNumber();
                if (currentBlock > lastCheckedBlock) {
                    const chatLogs = await provider.getLogs({
                        fromBlock: "0x" + (lastCheckedBlock + 1).toString(16),
                        toBlock: "0x" + currentBlock.toString(16),
                        address: noNukesQingAddress,
                        topics: [logEventTopic]
                    });

                    for (const log of chatLogs) {
                        try {
                            const parsed = qingInterface.parseLog(log);
                            const username = parsed.args[0];
                            const soul = parsed.args[1].toString();
                            const aura = parsed.args[2].toString();
                            const msg = parsed.args[3];
                            const timeStr = new Date().toLocaleTimeString();

                            console.log(`${C.dim}[${timeStr}]${C.reset} ${C.yellow}${C.bright}NoNukes QING${C.reset} | ${C.green}${C.bright}${username}${C.reset} ${C.dim}(S:${soul} A:${aura})${C.reset}\n  ${C.bright}${msg}${C.reset}\n`);
                        } catch (e) {
                            // Parsing error
                        }
                    }
                    lastCheckedBlock = currentBlock;
                }
            } catch (err) {
                // Ignore transient network errors
            }
        }, 3000);

    } catch (err) {
        console.error(`${C.red}Failed: ${err.message}${C.reset}`);
    }
}

main();
