const { ethers } = require("ethers");
const readline = require("readline");

// Network settings
const NETWORKS = {
    localhost: "http://127.0.0.1:8545",
    pulsechain: "https://rpc.pulsechain.com"
};

const LOG_EVENT_SIGNATURE = "LogEvent(string,uint64,uint64,string)";
const TOPIC_0 = ethers.id(LOG_EVENT_SIGNATURE);

// ANSI color helpers
const C = {
    reset: "\x1b[0m",
    bright: "\x1b[1m",
    dim: "\x1b[2m",
    blue: "\x1b[34m",
    green: "\x1b[32m",
    cyan: "\x1b[36m",
    yellow: "\x1b[33m",
    magenta: "\x1b[35m",
    red: "\x1b[31m",
    bgBlue: "\x1b[44m",
    bgCyan: "\x1b[46m"
};

const iface = new ethers.Interface([
    `event ${LOG_EVENT_SIGNATURE}`
]);

// Keep a local cache of contract address -> contract name
const nameCache = {};

async function getContractName(provider, address) {
    if (nameCache[address]) return nameCache[address];
    try {
        const contract = new ethers.Contract(address, ["function name() view returns (string)"], provider);
        const name = await contract.name();
        nameCache[address] = name;
        return name;
    } catch {
        nameCache[address] = "Unknown QING";
        return "Unknown QING";
    }
}

function formatMessage(username, soul, aura, msg, qingName, timestampText) {
    const timeStr = timestampText ? `[${timestampText}] ` : "";
    const prefix = `${C.dim}${timeStr}${C.reset}${C.cyan}${C.bright}${qingName}${C.reset} | ${C.green}${C.bright}${username}${C.reset} ${C.dim}(S:${soul} A:${aura})${C.reset}`;
    return `${prefix}\n  ${C.bright}${msg}${C.reset}\n`;
}

async function displayHistory(provider, blocksCount) {
    console.log(`${C.dim}Fetching last ${blocksCount} blocks of history...${C.reset}`);
    const latestBlock = await provider.getBlockNumber();
    const fromBlock = Math.max(0, latestBlock - blocksCount);

    const logs = await provider.getLogs({
        fromBlock: "0x" + fromBlock.toString(16),
        toBlock: "0x" + latestBlock.toString(16),
        topics: [TOPIC_0]
    });

    console.log(`${C.blue}=== History: Found ${logs.length} events ===${C.reset}\n`);

    for (const log of logs) {
        try {
            const parsed = iface.parseLog(log);
            const qingName = await getContractName(provider, log.address);
            
            // Try to fetch block timestamp
            let timeStr = "";
            try {
                const block = await provider.getBlock(log.blockNumber);
                timeStr = new Date(block.timestamp * 1000).toLocaleTimeString();
            } catch {
                timeStr = `Block ${log.blockNumber}`;
            }

            console.log(formatMessage(
                parsed.args[0], // username
                parsed.args[1].toString(), // soul
                parsed.args[2].toString(), // aura
                parsed.args[3], // msg
                qingName,
                timeStr
            ));
        } catch (err) {
            // Ignore parse errors (e.g. malformed data)
        }
    }
}

async function startLiveStream(provider) {
    console.log(`\n${C.green}=== Starting Live Stream (Polling every 5s) ===${C.reset}`);
    console.log(`${C.dim}Press Ctrl+C to exit${C.reset}\n`);

    let lastCheckedBlock = await provider.getBlockNumber();

    setInterval(async () => {
        try {
            const currentBlock = await provider.getBlockNumber();
            if (currentBlock > lastCheckedBlock) {
                const logs = await provider.getLogs({
                    fromBlock: "0x" + (lastCheckedBlock + 1).toString(16),
                    toBlock: "0x" + currentBlock.toString(16),
                    topics: [TOPIC_0]
                });

                for (const log of logs) {
                    try {
                        const parsed = iface.parseLog(log);
                        const qingName = await getContractName(provider, log.address);
                        const timeStr = new Date().toLocaleTimeString();

                        console.log(formatMessage(
                            parsed.args[0],
                            parsed.args[1].toString(),
                            parsed.args[2].toString(),
                            parsed.args[3],
                            qingName,
                            timeStr
                        ));
                    } catch (e) {}
                }

                lastCheckedBlock = currentBlock;
            }
        } catch (err) {
            // Quietly handle network drops
        }
    }, 5000);
}

function main() {
    console.clear();
    console.log(`${C.bgBlue}${C.bright}  DYSNOMIA QING/GWAT CHAT CLI TERMINAL  ${C.reset}\n`);

    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    rl.question(`Select Network:\n1. PulseChain Mainnet\n2. Local EVM (Anvil)\nChoose option (1-2): `, async (answer) => {
        let rpcUrl = NETWORKS.pulsechain;
        let networkName = "PulseChain Mainnet";

        if (answer.trim() === "2") {
            rpcUrl = NETWORKS.localhost;
            networkName = "Local EVM (Anvil)";
        }

        console.log(`\nConnecting to ${networkName} at ${rpcUrl}...`);
        const provider = new ethers.JsonRpcProvider(rpcUrl);

        try {
            const blockNum = await provider.getBlockNumber();
            console.log(`${C.green}Connected successfully! Current block: ${blockNum}${C.reset}\n`);
        } catch (e) {
            console.error(`${C.red}Connection failed. Make sure your local node/network is active: ${e.message}${C.reset}`);
            rl.close();
            process.exit(1);
        }

        rl.question(`Enter number of historical blocks to scan (default 43200 for ~5 days): `, async (blocksInput) => {
            const blocks = parseInt(blocksInput) || 43200;
            rl.close();

            await displayHistory(provider, blocks);
            await startLiveStream(provider);
        });
    });
}

main();
