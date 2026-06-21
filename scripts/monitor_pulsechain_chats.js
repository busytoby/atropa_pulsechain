const { ethers } = require("ethers");
const net = require("net");

const PULSECHAIN_RPC = "https://rpc.pulsechain.com";
const LOG_EVENT_SIGNATURE = "LogEvent(string,uint64,uint64,string)";
const TOPIC_0 = ethers.id(LOG_EVENT_SIGNATURE);

const NEW_QING_SIGNATURE = "NewQing(address,address,uint256)";
const TOPIC_NEW_QING = ethers.id(NEW_QING_SIGNATURE);
const MAP_ADDRESS = "0xD3a7A95012Edd46Ea115c693B74c5e524b3DdA75";

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

const iface = new ethers.Interface([
    `event ${LOG_EVENT_SIGNATURE}`
]);

const mapIface = new ethers.Interface([
    `event ${NEW_QING_SIGNATURE}`
]);

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

function logToZmmMcp(event, source, details) {
    const payload = {
        jsonrpc: "2.0",
        method: "wave512.dilemma_log",
        params: {
            event: event,
            source: source,
            details: details
        },
        id: 1
    };
    
    const client = new net.Socket();
    client.setTimeout(2000);
    client.connect(10042, "127.0.0.1", () => {
        client.write(JSON.stringify(payload));
    });
    client.on("data", () => {
        client.destroy();
    });
    client.on("error", () => {
        client.destroy();
    });
}

function formatMessage(username, soul, aura, msg, qingName, timestampText) {
    const timeStr = timestampText ? `[${timestampText}] ` : "";
    return `${C.dim}${timeStr}${C.reset}${C.cyan}${C.bright}${qingName}${C.reset} | ${C.green}${C.bright}${username}${C.reset} ${C.dim}(S:${soul} A:${aura})${C.reset}\n  ${C.bright}${msg}${C.reset}\n`;
}

async function main() {
    console.log(`${C.blue}Connecting to PulseChain RPC at ${PULSECHAIN_RPC}...${C.reset}`);
    const provider = new ethers.JsonRpcProvider(PULSECHAIN_RPC);

    try {
        const latestBlock = await provider.getBlockNumber();
        console.log(`${C.green}Connected! Current Block: ${latestBlock}${C.reset}\n`);

        const scanWindows = [
            { label: "5 days", blocks: 43200 },
            { label: "30 days", blocks: 259200 },
            { label: "90 days", blocks: 777600 },
            { label: "180 days", blocks: 1555200 },
            { label: "360 days", blocks: 3110400 },
            { label: "500 days", blocks: 4320000 }
        ];

        let logs = [];
        let qingLogs = [];
        let chosenFromBlock = 0;
        for (const window of scanWindows) {
            console.log(`${C.dim}Scanning back ${window.label}...${C.reset}`);
            const fromBlock = Math.max(0, latestBlock - window.blocks);
            try {
                logs = await provider.getLogs({
                    fromBlock: "0x" + fromBlock.toString(16),
                    toBlock: "0x" + latestBlock.toString(16),
                    topics: [TOPIC_0]
                });
                if (logs.length > 0) {
                    chosenFromBlock = fromBlock;
                    console.log(`${C.green}Found ${logs.length} logs in ${window.label} window.${C.reset}`);
                    break;
                }
            } catch (err) {
                console.error(`Failed to fetch logs for ${window.label}: ${err.message}`);
            }
        }

        try {
            const maxQingBlocks = 4320000; // 500 days
            const qingFromBlock = Math.max(0, latestBlock - maxQingBlocks);
            qingLogs = await provider.getLogs({
                fromBlock: "0x" + qingFromBlock.toString(16),
                toBlock: "0x" + latestBlock.toString(16),
                address: MAP_ADDRESS,
                topics: [TOPIC_NEW_QING]
            });
            console.log(`${C.green}Found ${qingLogs.length} NewQing logs in the 500-day window.${C.reset}`);
        } catch (err) {
            console.error(`Failed to fetch NewQing logs: ${err.message}`);
        }

        console.log(`${C.blue}=== Historical Feed (${logs.length} events) ===${C.reset}\n`);
        for (const log of logs) {
            try {
                const parsed = iface.parseLog(log);
                const qingName = await getContractName(provider, log.address);
                let timeStr = "";
                try {
                    const block = await provider.getBlock(log.blockNumber);
                    timeStr = new Date(block.timestamp * 1000).toLocaleTimeString();
                } catch {
                    timeStr = `Block ${log.blockNumber}`;
                }
                console.log(formatMessage(parsed.args[0], parsed.args[1].toString(), parsed.args[2].toString(), parsed.args[3], qingName, timeStr));
                
                const isVoid = qingName.toLowerCase().includes("void");
                const eventName = isVoid ? "M:VOID_CHAT" : "M:QING_CHAT";
                logToZmmMcp(eventName, qingName, `${parsed.args[0]} (S:${parsed.args[1]} A:${parsed.args[2]}) | ${parsed.args[3]}`);
            } catch (e) {}
        }

        console.log(`${C.yellow}=== Historical QING Creations (${qingLogs.length} events) ===${C.reset}\n`);
        for (const log of qingLogs) {
            try {
                const parsed = mapIface.parseLog(log);
                const qingAddr = parsed.args[0];
                const integrative = parsed.args[1];
                const waat = parsed.args[2].toString();
                let timeStr = "";
                try {
                    const block = await provider.getBlock(log.blockNumber);
                    timeStr = new Date(block.timestamp * 1000).toLocaleTimeString();
                } catch {
                    timeStr = `Block ${log.blockNumber}`;
                }
                console.log(`${C.dim}[${timeStr}]${C.reset} ${C.yellow}${C.bright}New QING Created:${C.reset} ${qingAddr} | Integrative: ${integrative} | Waat: ${waat}`);
                logToZmmMcp("M:NEW_QING", "MAP", `Qing: ${qingAddr} | Integrative: ${integrative} | Waat: ${waat}`);
            } catch (e) {}
        }

        console.log(`\n${C.green}=== Live Feed Monitoring Started ===${C.reset}\n`);
        
        let lastCheckedBlock = latestBlock;
        setInterval(async () => {
            try {
                const currentBlock = await provider.getBlockNumber();
                if (currentBlock > lastCheckedBlock) {
                    const freshLogs = await provider.getLogs({
                        fromBlock: "0x" + (lastCheckedBlock + 1).toString(16),
                        toBlock: "0x" + currentBlock.toString(16),
                        topics: [TOPIC_0]
                    });

                    for (const log of freshLogs) {
                        try {
                            const parsed = iface.parseLog(log);
                            const qingName = await getContractName(provider, log.address);
                            const timeStr = new Date().toLocaleTimeString();
                            console.log(formatMessage(parsed.args[0], parsed.args[1].toString(), parsed.args[2].toString(), parsed.args[3], qingName, timeStr));
                            
                            const isVoid = qingName.toLowerCase().includes("void");
                            const eventName = isVoid ? "M:VOID_CHAT" : "M:QING_CHAT";
                            logToZmmMcp(eventName, qingName, `${parsed.args[0]} (S:${parsed.args[1]} A:${parsed.args[2]}) | ${parsed.args[3]}`);
                        } catch (e) {}
                    }

                    const freshQingLogs = await provider.getLogs({
                        fromBlock: "0x" + (lastCheckedBlock + 1).toString(16),
                        toBlock: "0x" + currentBlock.toString(16),
                        address: MAP_ADDRESS,
                        topics: [TOPIC_NEW_QING]
                    });

                    for (const log of freshQingLogs) {
                        try {
                            const parsed = mapIface.parseLog(log);
                            const qingAddr = parsed.args[0];
                            const integrative = parsed.args[1];
                            const waat = parsed.args[2].toString();
                            const timeStr = new Date().toLocaleTimeString();
                            console.log(`[${timeStr}] ${C.yellow}${C.bright}New QING Created:${C.reset} ${qingAddr} | Integrative: ${integrative} | Waat: ${waat}`);
                            logToZmmMcp("M:NEW_QING", "MAP", `Qing: ${qingAddr} | Integrative: ${integrative} | Waat: ${waat}`);
                        } catch (e) {}
                    }

                    lastCheckedBlock = currentBlock;
                }
            } catch (err) {
                // Ignore transient network errors
            }
        }, 5000);

    } catch (err) {
        console.error(`${C.red}Initialization failed: ${err.message}${C.reset}`);
    }
}

main();
